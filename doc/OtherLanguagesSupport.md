# Other Languages Support

The [CommsChampion Ecosystem](https://commschamp.github.io/) is mostly about C++ and embedded systems. However,
other high level languages can also be supported by generation of the bindings (glue code) using
[SWIG](https://www.swig.org/). Please refer to the [documentation](https://www.swig.org/Doc4.0/SWIGDocumentation.html)
to make sure that the chosen target language is properly supported.

Also, to be able to properly use the protocol definition code in the target high level language the callbacks need to
be properly supported. [SWIG](https://www.swig.org/) calls this feature
[directors](https://www.swig.org/Doc4.0/SWIGDocumentation.html#SWIGPlus_target_language_callbacks). Please
make sure the selected target language supports this feature. Not all of them do.

In case [SWIG](https://www.swig.org/) doesn't support required target language, but the latter has its own
way to support C++ glue code, it's still beneficial to understand the way how [SWIG](https://www.swig.org/)
support is implemented. It can be used as a reference to define wrapping classes required for the target language
bindings.

The generation of the [SWIG](https://www.swig.org/) interface file(s) is performed using **commsdsl2swig** code
generator, please make sure its build is enabled by using appropriate cmake [options](../CMakeLists.txt).


## Build Order

- Build and install [COMMS Library](https://github.com/commschamp/comms) project.
- Generate the protocol definition files using **commsdsl2comms**.
- Properly build and install the generated protocol definition project. It will generate appropriate cmake config
  file allowing the protocol headers only library to be found using `find_package()`.
- Generate the swig bindings using **commsdsl2swig**. It generates **CMakeLists.txt** file allowing the generated project being built
  separately, but some extra code injection may be required (explained later).
- Build the project generated by the **commsdsl2swig** providing relevant configuration (specifying the target language(s)).
  Use **CMAKE_PREFIX_PATH** to specify path(s) to the installation directory of the protocol definition project as well as
  [COMMS Library](https://github.com/commschamp/comms).
- If applicable (for languages like Java or C#), perform extra compilation of the target language sources generated by the [swig](https://www.swig.org/).
  This step can be done as part of the previous one using extra code injection to append relevant instructions to the
  **CMakeLists.txt** file generated by the **commsdsl2swig**.

Note that [swig](https://www.swig.org/) generates the following:

- C++ sources with bindings (glue code), which needs to be compiled as a shared library (.so/.dll).
- Target language wrapping sources, which use the glue code functions. They need to be compiled (if applicable) separately
  using target language compile, for example `javac` for Java.

Also note that bindings shared library needs to be properly loaded (if it's not done by the swig-generated integration code)
by the client side before its glue code can be used.

For example in Java:
```java
public class MyProtocol {
    static {
        System.loadLibrary("my_prot");
    }

    ...
}
```

For more details please refer to the language specific swig [documentation](https://www.swig.org/Doc4.0/SWIGDocumentation.html).

## Produced Output

- **include** folder - Contains header files with the declaration of the wrapper C++ classes, bindings for which are expected to be generated.
  Note that they don't contain real C++ code and aren't used by any C++ compiler. They are parsed by the [swig](https://www.swig.org/).
  However, the directory structure resembles the one produced by the **commsdsl2comms** for easy navigation. The developer
  should use these files as reference to determine what functionality is available in the target language.
- **&lt;protocol_name&gt;.i** - [SWIG](https://www.swig.org/) interface file containing real C++ code of the glue code classes definition
  as well as some swig directives specifying some extra functionality.
- **CMakeLists.txt** - CMake project definition file allowing output to be built as a separate project.


## Generated CMake Project
As it was mentioned above, the output of the **commsdsl2swig**
(like any other [commsdsl](https://github.com/commschamp/commsdsl) code generator) contains **CMakeLists.txt**.
It can be used to build it as a separate cmake project. Note that the generated **CMakeLists.txt** is
incomplete. Many target languages require external libraries for glue code. The comments inside the
generated **CMakeLists.txt** indicate what extra source files need to be used to inject extra modifications
to the file.

For example Python and Java support may require:

CMakeLists.txt.prepend:
```
find_package(Python3 REQUIRED COMPONENTS Development)
find_package(JNI REQUIRED)
find_package(Java REQUIRED COMPONENTS Runtime Development)
```

CMakeLists.txt.prepend_lang:
```
set (CMAKE_SWIG_FLAGS -Werror)
if ("${lang}" STREQUAL "python")
    list (APPEND CMAKE_SWIG_FLAGS -Wall)
elseif ("${lang}" STREQUAL "java")
    list (APPEND CMAKE_SWIG_FLAGS -Wall -package my_prot_pkg)
endif ()
```

CMakeLists.txt.append:
```
target_link_libraries(my_prot_swig_python Python3::Python)
target_link_libraries(my_prot_swig_java JNI::JNI)

add_custom_target(
    my_prot_swig_java_compile ALL
    COMMAND ${Java_JAVAC_EXECUTABLE} -Xdiags:verbose -d . ${CMAKE_CURRENT_BINARY_DIR}/output_java/*.java
    DEPENDS ${my_prot_swig_java}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
```

Note that the generated **CMakeLists.txt** uses [swig_add_library()](https://cmake.org/cmake/help/latest/module/UseSWIG.html) cmake
function. It creates multiple targets, but the provided name is the created shared library of the glue code.
Also refer to the cmake [documentation](https://cmake.org/cmake/help/latest/module/UseSWIG.html) and use the
injected code to modify the default behavior if needed.

## Class Naming
In order to produce unified bindings code for all the languages (some of which may not include package / namespace feature), the
C++ scope of the class is flattened by replacing `::` with `_` and omitting the top level namespace by default.

For example:
```xml
<shema name="my_prot">
    <fields>
        <int name="F1" type="uint32" />
    </fields>

    <message name="Msg1" id="1">
        <ref field="F1" />
    </message>
</schema>
```
The generated class declaration files are going to reside in `include/my_prot/field/F1.h` and
`include/my_prot/message/Msg1.h`. The respective classes will be named `field_F1` and `message_Msg1`.

The classes are not prefixed with the protocol name namespace because the swig interface file names the whole
module with the protocol name.
```
%module(directors="1") my_prot;
```
Some of the target programming languages, like Python, automatically put the classes into the specified module and
they are referenced using the module name:
```python
msg = my_prot.message_Msg1()
```
However, there are target languages, like Java or C#, that don't do it. To achieve the same effect it is recommended to
pass relevant options (`-package` for Java or `-namespace` for C#) to the swig invocation.

When there are [multiple different schema names](https://commschamp.github.io/commsdsl_spec/#intro-multiple-schemas) in
use, then all the generated classes are prefixed with their schema (protocol) name automatically to differentiate between them.

Also, the **commsdsl2swig** code generator has the `force-main-ns-in-names` command line option forcing the main namespace
prefixing of the generated classes.

## Raw Data Buffer
When dealing with raw data buffers, the binding C++ classes expect to work with `std::vector<unsigned char>`
(defined in `include/DataBuf.h`). The [SWIG](https://www.swig.org/) is expected to generate appropriate
wrapper and/or translation functionality.

For example, in Python, it is possible to use
[bytearray](https://www.w3schools.com/python/ref_func_bytearray.asp), [list of ints](https://www.w3schools.com/python/python_lists.asp),
or [tuple of ints](https://www.w3schools.com/python/python_tuples.asp). The swig is expected to convert it
to the relevant vector automatically.

When requesting Java code generation, the `DataBuf` becomes are real class with one of its constructors defined as:
```java
public DataBuf(short[] initialElements) {...}
```

When requesting C# code generation, the `DataBuf` also becomes are real class with the following constructors:
```csharp
public DataBuf(global::System.Collections.IEnumerable c) {...}
public DataBuf(global::System.Collections.Generic.IEnumerable<byte> c) {...}
```

When working with Java and/or C#, the developer is required to copy relevant data from the native raw data buffer,
used in the client code, to the created `DataBuf` object. The latter then passed to the relevant protocol processing
member functions.

## Message Handling
Following the same convention as with the protocol generated by the **commsdsl2comms** (unless the
protocol definition defines its own `<interface>`) the default interface class is named `Message` and
resides in `include/my_prot/Message.h`. The defined interface is really polymorphic (although not
really visible in the generated class declaration):
```cpp
class MsgHandler;

class Message
{
public:
    virtual ~Message();

    const char* name() const;
    MsgId getId() const;
    comms_ErrorStatus read(const DataBuf& buf);
    comms_ErrorStatus write(DataBuf& buf) const;
    bool refresh();
    unsigned long length() const;
    bool valid() const;
    void dispatch(MsgHandler& handler);
protected:
    Message();
    Message(const Message& other);
};
```

The `dispatch()` member function allows polymorphic dispatch to the handler object.

The `MsgHandler` is expected to define virtual handling member functions. Due to the fact that not all target
languages can have overloaded member functions with the same name, the name of the handling function is expected
to contain the message class name as well:
```cpp
class MsgHandler
{
public:
    virtual ~MsgHandler();

    virtual void handle_message_Msg1(message_Msg1& msg);
    virtual void handle_message_Msg2(message_Msg2& msg);

    virtual void handle_Message(Message& msg);
};
```
By default every `handle_message_X()` calls the interface invocation `handle_Message()`, which by default
does nothing. The `MsgHandler` class is marked as the swig [director](https://www.swig.org/Doc4.0/SWIGDocumentation.html#SWIGPlus_target_language_callbacks)
allowing its extension by the target language.

The extending class in the target language can override any of the necessary virtual functions.

For example in Python:
```python
class ClientMsgHandler(my_prot.MsgHandler):
    def __init__(self):
        my_prot.MsgHandler.__init__(self)

    def handle_message_Msg1(self, msg):
        ...

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")
```
Or in Java:
```java
public class ClientMsgHandler extends my_prot_pkg.MsgHandler {
    ...
    public void handle_message_Msg1(my_prot_pkg.message_Msg1 msg) {
        ...
    }

    public void handle_Message(my_prot_pkg.Message msg) {
        assert false;
    }
}
```

## Working with Frames
Every protocol definition is expected to define its framing:
```xml
<shema name="my_prot">
    <frame name="ProtFrame">
        ...
    </frame>
</schema>
```
The wrapping class interface definition will reside in `include/my_prot/frame/ProtFrame.h`
and look like this:
```cpp
struct frame_ProtFrame_AllFields
{
   ...
};

class frame_ProtFrame
{
public:
    ...

    unsigned long processInputData(const DataBuf& buf, MsgHandler& handler);
    unsigned long processInputDataSingleMsg(const DataBuf& buf, MsgHandler& handler, frame_ProtFrame_AllFields* allFields = nullptr);
    DataBuf writeMessage(const Message& msg);
    comms_ErrorStatus appendMessage(const Message& msg, DataBuf& buf);
};
```
This is the primary integration point between the protocol library and the client code. The raw data received over the
I/O link is fed into the frame object to create and process the decoded messages. The frame functionality
is also used to serialize the created message object before the raw data output is sent back over the
I/O link.

The `processInputData()` will process bytes in the input buffer, decode them, create appropriate message objects and dispatch the latter to the
provided handler. The function returns number of consumed bytes in the buffer. It's up the the client code caller to
manage the input buffer and delete consumed bytes from it. In case the function returns amount of consumed bytes less than
held by the buffer, it means that framing of the last message is incomplete (not all bytes received). The remaining bytes
need to be preserved and then re-process attempted when new raw data comes in.

The `processInputDataSingleMsg()` is very similar to `processInputData()`, but allows creation and handling only one message object
at a time. It also provides an opportunity (via `allFields` output parameter) to get and analyze the message frame constructing fields.
In cases the function invocation returns `0` when the input buffer is not empty, then it means there are not enough bytes
to properly construct the message. The remaining bytes
need to be preserved and then re-process attempted when new raw data comes in.

The `writeMessage()` needs to be used to frame and serialize any message object. The returned output buffer needs to be sent
to its destination over the I/O link.

The `appendMessage()` is very similar to the `writeMessage()`, but appends the serialized message bytes to the provided buffer.
It can be usefull to serialize multiple messages into the same buffer and then then them all in one go.

## Memory Management
As the rule of thumb: Do **NOT** store references to the objects you have not **explicitly** created. For example the
invocation of the frame's `processInputData()` results in creation of the message object, then dispatching it to the
handler. Once the handling function returns, the allocated message object is deleted / destructed by C++ code. If such object
needs to be stored, use explicit copy-construction.

For example in Python:
```python
class ClientMsgHandler(my_prot.MsgHandler):
    def handle_message_Msg1(self, msg):
        self.msg1 = msg                        # Bad, creates dangling reference
        self.msg1 = my_prot.message_Msg1(msg)  # Good, uses explicit copy construction
```

Also extra care is required when working with a reference to a data member. It mustn't be accessed after
the reference to the holding object is invalidated.
```python
m = my_prot.message_Msg1()
f1 = m.field_f1()              # Reference to a member field
m = None                       # May result in destruction of the message object
f1.setValue(1)                 # Access to invalid memory
```

Another thing worth mentioning is that `writeMessage()` member function of the frame class described earlier
returns data buffer **by value**. Although [swig](https://www.swig.org/) still uses dynamic memory allocation and
returns a pointer to the dynamically allocated object, the wrapping class in cooperation with the swig binding (glue)
code take care of this memory and automatically delete the dynamically allocated vector at the time when
target language wrapping class is destructed. Treat this data buffer object as **explicitly** allocated, i.e.
reference to it can be stored as long as needed.

Memory of the **explicitly** allocated objects are expected to be managed by the target language and its
garbage collector. There should be no need to explicitly delete the allocated memory.

## Working with Fields
An access to the message fields and their values is very similar to the one used when working directly with C++ code.
Every message has `field_x()` accessor function(s) for the member field object(s), and the latter have
`getValue()` and `setValue()` member functions to get / set the field's value.

For example:
```xml
<shema name="my_prot">
    <message name="Msg1" id="1">
        <int name="F1" type="uint8" />
    </message>
    <frame name="ProtFrame">
        ...
    </frame>
</schema>
```
The Python code may look like this:
```python
msg1 = my_prot.message_Msg1()
msg1.field_f1().setValue(123)
frame = my_prot.frame_ProtFrame()
buf = frame.writeMessage(msg1)
... # Send raw data from buffer over I/O link
```

## Working with &lt;ref&gt; Fields
It is quite common to define field in the global space and then `<ref>`-erence it as the
member field of the `<message>`.
```xml
<shema name="my_prot">
    <fields>
        <int name="F1" type="uint8" />
    </fields>
    <message name="Msg1" id="1">
        <ref field="F1" />
    </message>
</schema>
```
When working with C++ there is a direct inheritance relationship between generated
`my_prot::field::F1` and `my_prot::message::Msg1Fields::F1` and the member functions and types
of the first can be used when working with the second. However, this is not the
case with the generated wrapper classes:

- `field_F1` --> `my_prot::field::F1`
- `message_Msg1Fields_F1` --> `my_prot::message::Msg1Fields::F1` --> `my_prot::field::F1`

It is visible there is no direct inheritance relationship between `field_F1` and `message_Msg1Fields_F1`.
As the result the member functions and types of the first cannot be seamlessly used when working
with the second. To workaround this problem every wrapping class of the `<ref>` field has `ref()` member
function to do the explicit conversion to the global field type:
```python
msg1 = my_prot.message_Msg1()
msg1.field_f1().ref().setValue(123)
```
Note that for some languages, like C#, the `ref` can be a keyword and cannot be used as a function name. In
such cases [swig](https://www.swig.org/) automatically renames the function to `ref_()`.

## Working with &lt;enum&gt; Fields
The `<enum>` field wrapping declaration class defined in the **include** folder and processed by
the [swig](https://www.swig.org/) re-defines its `ValueType`. Using this information
[swig](https://www.swig.org/) can generated relevant target language constants and/or enum type to be used.
However, the definition and the use of the enum values in the target languages may differ. For example:
```xml
<shema name="my_prot">
    <fields>
        <enum name="F1" type="uint8">
            <validValue name="V0" val="0" />
            <validValue name="V1" val="1" />
        </enum>
    </fields>
    <message name="Msg1" id="1">
        <ref field="F1" />
    </message>
</schema>
```
The field declaration code will look like this:
```cpp
class field_F1
{
public:
    enum class ValueType : unsigned short
    {
        V0 = 0,
        V1 = 1,

        // --- Extra values generated for convenience ---
        FirstValue = 0, // First defined value.
        LastValue = 1, // Last defined value.
        ValuesLimit = 2, // Upper limit for defined values.
    };

    ...
};
```
In Python the enum constants are defined by joining (with '_') the `ValueType` and the actual value name: `ValueType_V0`, `ValueType_V1`, ...
```python
msg = my_prot.message_Msg1()
msg.field_f1().ref().setValue(my_prot.field_F1.ValueType_V1)
```

In languages like Java and C# the enum names are used with regular '.' separator.
```java
msg = new my_prot_pkg.message_Msg1();
msg.field_f1().ref().setValue(my_prot_pkg.field_F1.ValueType.V1);
```

## Working with Strings
The [SWIG](https://www.swig.org/) is expected to provide seamless conversion between the
`const char*` and `std::string` C++ data types to the strings of the target language.

For example:
```xml
<shema name="my_prot">
    <message name="Msg1" id="1">
        <string name="F1" />
    </message>
</schema>
```
The generated field wrapper class `message_Msg1Fields_F1` will define its `ValueType` to be
`std::string`.

The target language may use the relevant strings directly:
```python
m = my_prot.message_Msg1()
m.field_f1().setValue("hello")    # Expects const std::string&
f1_name = m.field_f1().name()     # Returns const char*
f1_val = m.field_f1().getValue()  # Returns const std::string&
print(f'{f1_name} = {f1_val}')
```

## Working with &lt;data&gt; Fields
The wrapping class of the `<data>` field defines its `ValueType` as `std::vector<unsigned char>`,
which is the same as the [DataBuf](#raw-data-buffer) described earlier. The client code
is expected to work with the `DataBuf` object and its provided interface when dealing with the value of the
relevant `<data>` field.

## Working with &lt;list&gt; Fields
The `ValueType` of every `<list>` field is defined to be `std::vector` of the element field. The
generated [SWIG](https://www.swig.org/) interface contains relevant `%template` directive to
generate bindings (glue code) for this type. As the result, the target language code can use
member functions of the [std::vector](https://en.cppreference.com/w/cpp/container/vector) to manipulate
and access it. For example, in python:
```python
msg = my_prog.message_Msg1() # Create message object
f1 = msg.field_f1()          # Access the list field
f1Vec = f1.value()           # Access the std::vector of the element storage
f1Vec.resize(2)              # Update size of the vector
f1Vec[0].setValue(...)       # Set the first element value
f1Vec[1].setValue(...)       # Set the second element value
```

## Working with &lt;variant&gt; Fields
Just like working with regular C++ code generated by the **commsdsl2comms**, the wrapping
C++ declaration class processed by the [swig](https://www.swig.org/) provides
`initField_X()` and `accessField_X()` to initialize and access when already initialized specific
member. The wrapping class also provides `currentFieldExec()` to be able to determine
the actual held member field and dispatch it to the right handling function:
```cpp
struct field_Variant1_Handler
{
    virtual ~field_Variant1_Handler();

    virtual void handle_p1(field_Variant1Members_P1& field);
    virtual void handle_p2(field_Variant1Members_P2& field);
};

class field_Variant1
{
public:
    field_Variant1Members_P1& initField_p1();
    field_Variant1Members_P1& accessField_p1();

    field_Variant1Members_P2& initField_p2();
    field_Variant1Members_P2& accessField_p2();
    ...
    void currentFieldExec(field_Variant1_Handler& handler);
    ...
};
```
The relevant `X_Handler` class is marked as the [director](https://www.swig.org/Doc4.0/SWIGDocumentation.html#SWIGPlus_target_language_callbacks)
allowing its extension and overriding relevant member functions in the target language, just like it
is done with the [message handling](#message-handling).

## Working with Units
Working with units in the C++ code is straightforward, just use appropriate function from the
[comms::units](https://commschamp.github.io/comms_doc/namespacecomms_1_1units.html) namespace.

For example, let's assume there is a field containing 1/10 of the millimeters.
```xml
<shema name="my_prot">
    <fields>
        <int name="Distance" type="int32" units="mm" scaling="1/10" displayDecimals="1" />
    </fields>
    <message name="Msg1" id="1">
        <ref field="Distance" />
    </message>
</schema>
```
In case the application code works with meters rather millimeters, the relevant code would like like:
```cpp
comms::units::setMeters(msg1.field_distance(), 1.123)
```
or
```cpp
auto distInMeters = comms::units::getMeters<double>(msg1.field_distance());
```
When working with swig wrappers there is a need to explicitly provide the wrapping member
functions for the relevant field class(es). The **commsdsl2swig** coge generator provides
a way to inject the custom code the same way as it does **commmsdsl2comms**.

In the case described above we would like to add member functions allowing to get / set meters
to the fields value. To achieve that we need to create appropriate file injecting "public"
portion of extra code to the field definition:

include/my_prot/field/Distance.h.public
```cpp
double getMeters() const
{
    return comms::units::getMeters<double>(*this);
}

void setMeters(double val)
{
    comms::units::setMeters(*this, val);
}
```
The **commsdsl2swig** will put the code above in the relevant header allowing generation of
the relevant glue code to the extra functions by the [swig](https://www.swig.org/). The
latter just ignores contents of the functions.

The **commsdsl2swig** will also put the same code into the actual class definition in the
generated swig interface file actually implementing the required functionality.

Then the target language can use the added extra functions:
```python
m = my_prot.message_Msg1()
m.field_distance().ref().setMeters(1.234)
```

## Java Tips
When Java code is generated, the name of the `%module` from the swig interface file (equivalent to the schema / protocol name)
does **NOT** find its way as the global package name. Instead it is used to create a class containing all the
global functions and constants. In order to wrap all the relevant protocol classes into the package, extra
`-package` parameter needs to be passed to the [swig](https://www.swig.org/) code generator. When using
[swig_add_library()](https://cmake.org/cmake/help/latest/module/UseSWIG.html) cmake function, just define
`CMAKE_SWIG_FLAGS` variable accordingly:
```
set (CMAKE_SWIG_FLAGS -Wall -package my_prot_pkg)
```
Note, that you cannot use the same name as your `%module`. It will create a names clash between the package and the
class containing globals. However it is possible to use a trick of renaming your swig module:
```
set (CMAKE_SWIG_FLAGS -Wall -module Globals -package my_prot)
```
It will rename your class containing global functions and constants to "Globals" and allow using schema name as the
package.

## C# Tips
Very similar to [Java](#java-tips), the `%module` name does not automatically become a global namespace. Instead,
it is used as the class name containing global functions and constants. To provide a global namespace
use extra `-namespace` parameter (instead of `-package` used for Java) when invoking [swig](https://www.swig.org/).

Also be aware that [ref](https://learn.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/ref) is a keyword in
C#. As the result `ref()` member function of the `<ref>` field gets automatically renamed
by the [swig](https://www.swig.org/) to `ref_()`.

## Updates to SWIG Interface File
The [SWIG](https://www.swig.org/) interface file generated by the **commsdsl2swig** may not contain all the desired definitions
required by some chosen target language. It is possible to inject extra code in two places: beginning and end.
To add extra code at the beginning of the file, use `<protocol_name>.i.prepend` file. Use it to add global language
specific definitions. Using `<protocol_name>.i.append` file allows injecting code at the end. Even when
adding some code at the end it's possible to update any portion of the swig sections using
[code insertion blocks](https://www.swig.org/Doc4.0/SWIGDocumentation.html#SWIG_nn42).
