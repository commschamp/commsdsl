# Manual of **commsdsl2test**

## Overview
The **commsdsl2test** is a code generation tool provided by this project.
It generates CMake project that can be used to fuzz test the protocol definition
code generated by the **commsdsl2comms**.

## Command Line Arguments
The **commsdsl2test** utility has multiple command line arguments, please
use `-h` option for the full list as well as default option values. 

```
$> /path/to/commsdsl2test -h
```
Below is a summary of most important ones.

### Selecting Schema Files
Selecting of the schema files is very similar to how it is done for the
[commsdsl2comms](Manual_commsdsl2comms.md#selecting-schema-files).

List all the schema files at the end of the command line arguments:

```
$> /path/to/commsdsl2test <args> schema1.xml schema2.xml schema3.xml ...
```
The schema files will be processed **in order** of their listing.

When the input files are listed in the single file:
```
$> /path/to/commsdsl2test -i schemas_list.txt
```

When a schemas listing file contains *relative* paths to the schema files use
`-p` option to specify the absolute path prefix.
```
$> /path/to/commsdsl2test -i schemas_list.txt -p /path/to/schemas/dir
```

### Output Directory
By default the output CMake project is written to the current directory. It
is possible to change that using `-o` option.
```
$> /path/to/commsdsl2test -o /some/output/dir schema.xml
```

### Injecting Custom Code
The **commsdsl2test** utility allows injection of custom code into the
generated project in case the default code is incomplete. For this
purpose `-c` option with path to directory containing custom code snippets is used.
```
$> /path/to/commsdsl2test -c /path/to/custom/code schema.xml
```
Please read [Custom Code](#custom-code) section below for more details on
how to format and where to place the custom code.

### Changing Main Namespace
The code generated by the **commsdsl2test** depends on the code generated by
the **commsdsl2comms**. If the code generated by the **commsdsl2comms**
[changed its main namespace](Manual_commsdsl2comms.md#changing-main-namespace)
then it is required to change the main namespace for the code generated by
the **commsdsl2test** as well.
```
$> /path/to/commsdsl2test -n other_ns_name schema.xml
```

## Custom Code
As was already mentioned earlier, **commsdsl2test** utility allows injection
of custom code in the generated code.
The [CommsChampion Tools](https://github.com/commschamp/cc_tools_qt) themselves
use C++17 (or later). It is recommended to also use C++17 code snippets for the
[custom code injection](#injecting-custom-code) (using `-c` command line option
specifying directory with custom code snippets).

The custom code injection for the project generated by the **commsdsl2test**
is expected to contain some extra header / source files which can be used by
the generated code.

The generated _CMakeLists.txt_ contains multiple configuration options that
can be used and force the generated code to use definitions from the injected
files. See [TestingGeneratedProtocol.md](TestingGeneratedProtocol.md) for
details.
