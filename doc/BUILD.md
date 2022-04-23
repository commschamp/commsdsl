# How to Build
This project uses CMake as its build system. Please open main
[CMakeLists.txt](../CMakeLists.txt) file and review available options as well as
mentioned available parameters, which can be used in addition to standard 
ones provided by CMake itself, to modify the default build. 

**NOTE**, that **libcommsdsl** uses [libxml2](http://xmlsoft.org)
to parse XML schema files. On Windows systems there is no need to build [libxml2](http://xmlsoft.org) 
separately. The build process will check it out automatically and build static 
library itself. For example:

### Linux Build
```
$> cd /source/of/this/project
$> mkdir build && cd build
$> cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PWD/install ..
$> make install
```
### Windows Build
```
$> cd C:\source\of\this\project
$> mkdir build
$> cd build
$> cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%cd%/install ..
$> nmake install
```
 
