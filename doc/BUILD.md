# How to Build
This project uses CMake as its build system. Please open main
[CMakeLists.txt](../CMakeLists.txt) file and review available options as well as
mentioned available parameters, which can be used in addition to standard
ones provided by CMake itself, to modify the default build.

**NOTE** that only **commsdsl2comms** code generator is enabled by default. Please
make sure to enable other tools in case they are needed.

**Also note** that **libcommsdsl** uses [libxml2](http://xmlsoft.org)
to parse XML schema files. On Windows systems there is no need to build [libxml2](http://xmlsoft.org)
separately. The build process will check it out automatically and build static
library itself. However, it is possible to externally build it and provide a path
to it using standard **CMAKE_PREFIX_PATH** variable.

For example:

### Default Linux Build
```
$> cd /source/of/this/project
$> mkdir build && cd build
$> cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PWD/install
$> make install
```

### Full Linux Build
```
$> cd /source/of/this/project
$> mkdir build && cd build
$> cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PWD/install \
    -DCOMMSDSL_BUILD_COMMSDSL2TOOLS_QT=ON -DCOMMSDSL_BUILD_COMMSDSL2SWIG=ON \
    -DCOMMSDSL_BUILD_COMMSDSL2EMSCRIPTEN=ON -DCOMMSDSL_BUILD_COMMSDSL2LATEX=ON \
    -DCOMMSDSL_BUILD_COMMSDSL2C=ON
$> make install
```

### Default Windows Build
```
$> cd C:\source\of\this\project
$> mkdir build
$> cd build
$> cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%cd%/install
$> nmake install
```

