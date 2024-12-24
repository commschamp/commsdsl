# Overview
This project is a member of [CommsChampion Ecosystem](https://commschamp.github.io).
It provides multiple code generators to produce C++11 binary protocol
definition code as well as other satellite components that can be used to (fuzz) test
the protocol definition, visualize the message contents, debug the protocol messages exchange,
as well as create bindings (glue code) for other high level programming languages.

# What's Inside
- **commsdsl2comms** - A code generator, that produces C++11 code for binary
protocol definition out of [CommsDSL](https://github.com/commschamp/CommsDSL-Specification) 
schema files. The generated outcome is actually a CMake project that can be used to
properly install protocol definition headers as well as relevant cmake configuration files. 
For details on how to use the tool, please read the 
[commsdsl2comms Manual](doc/Manual_commsdsl2comms.md) 
documentation page. For details on the generated CMake project please read the
[Generated CMake Project Walkthrough](doc/GeneratedProjectWalkthrough.md)
documentation page.
- **commsdsl2test** - A code generator that produces C++11 code for fuzz
testing of the protocol definition produced by the **commsdsl2comms**.
Details are in the [Testing Generated Protocol Code](doc/TestingGeneratedProtocolCode.md) documentation
page. Build requires explicit cmake enable [option](CMakeLists.txt).
- **commsdsl2tools_qt** - A code generator, that produces the protocol
definition plugin code for [CommmsChampion Tools](https://github.com/commschamp/cc_tools_qt),
which can be used to visualize message contents as well as debug / observe exchange
of the messages between different systems. Details are in the
[Visual Protocol Analysis](doc/VisualProtocolAnalysis.md) documentation page.
Build requires explicit cmake enable [option](CMakeLists.txt).
- **commsdsl2swig** - A code generator that produces [SWIG](https://www.swig.org) interface 
file(s) for the protocol definition produced by the **commsdsl2comms**.
It allows generation of the bindings (glue code) to other high level 
programming languages using external [swig](https://www.swig.org) utility.
Details are in the [Other Languages Support](doc/OtherLanguagesSupport.md) documentation page.
Build requires explicit cmake enable [option](CMakeLists.txt).
- **commsdsl2emscripten** - A code generator that produces [emscripten](https://emscripten.org/)  
bindings for the protocol definition produced by the **commsdsl2comms**.
It allows compilation of the protocol definition C++ code into WebAssembly as well as
generation javascript bindings to it.
Details are in the [WebAssembly Support](doc/WebAssemblySupport.md) documentation page.
Build requires explicit cmake enable [option](CMakeLists.txt).
- **libcommsdsl** - A C++ library containing common functionality for parsing of the
[CommsDSL](https://github.com/commschamp/CommsDSL-Specification) schema files as
well code generation. It can be used to implement independent code generators.
NOTE, that at this moment, the library is not documented. Please
[get in touch](#contact-information) in case you need it. I'll let you know
when it's going to be ready.

# License
The code of this project (libraries and tools it contains)
is licensed under [Apache v2.0](https://www.apache.org/licenses/LICENSE-2.0) license.

The generated code has no license, the vendor is free to
pick any as long as it's compatible with the
[license](https://commschamp.github.io/licenses/) of the
relevant [CommsChampion Ecosystem](https://commschamp.github.io) project.

# Tutorial
The [cc_tutorial](https://github.com/commschamp/cc_tutorial/) project contains a 
tutorial on how to use 
[CommsDSL](https://commschamp.github.io/commsdsl_spec/) to define binary communication protocol,
**commsdsl2comms** to generate code, and 
[COMMS Library](https://github.com/commschamp/comms) to customize and 
integrate the protocol definition with the business logic of the application.

# How to Build
Detailed instructions on how to build and install all the components can be
found in [doc/BUILD.md](doc/BUILD.md) file.

# Other Documentation
Please check the [doc](doc) folder for the available additional documentation.

# Versioning
This project will use [Semantic Versioning](https://semver.org/), where
**MAJOR** number will be equal to the latest **DSL** version 
(The first number of [CommsDSL](https://github.com/commschamp/CommsDSL-Specification)
version) it supports. The **MINOR** number will indicate various improvements
in the code of this repository, and **PATCH** number will indicate various bug fixes.

# Supported Compilers
This project (the code generator and [CommsDSL](https://github.com/commschamp/CommsDSL-Specification) 
parsing library) is implemented using C++17 programming language. As the result,
the supported compilers are:
- **GCC**: >=8
- **Clang**: >=7
- **MSVC**: >= 2017

The **generated** projects however contain C++11 valid code and supports a bit earlier
versions of the compilers:
- **GCC**: >=4.8
- **Clang**: >=3.8
- **MSVC**: >= 2015

# Branching Model
This repository will follow the 
[Successful Git Branching Model](http://nvie.com/posts/a-successful-git-branching-model/).

The **master** branch will always point to the latest release, the
development is performed on **develop** branch. As the result it is safe
to just clone the sources of this repository and use it without
any extra manipulations of looking for the latest stable version among the tags and
checking it out.

# Asking For Help
With time the [CommsChampion Ecosystem](https://commschamp.github.io) has grown with both
features and complexity. Sometimes it can become challenging, especially for first time users,
to find a proper way to define the required binary protocol, especially quite complex ones.
The most efficient way to ask for help would be:

1. forking the official [tutorial](https://github.com/commschamp/cc_tutorial/) project on github
2. create a separate folder (say **howto100**) where the minimal portion of
the required protocol which exposes the encountered problem is defined.
3. [get in touch](#contact-information) providing an info on the forked project and encountered problem.

The suggested resolution will be provided by a pull request and/or a comment. After the problem
is resolved, the forked project can be deleted.

# Contact Information
For bug reports, feature requests, or any other question you may open an issue
here in **github** or e-mail me directly to: **arobenko@gmail.com**. I usually
respond within 24 hours.

