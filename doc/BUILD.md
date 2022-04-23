# How to Build
This project uses CMake as its build system. Please open main
[CMakeLists.txt](../CMakeLists.txt) file and review available options as well as
mentioned available parameters, which can be used in addition to standard 
ones provided by CMake itself, to modify the default build. 

**NOTE**, that **libcommsdsl** uses [libxml2](http://xmlsoft.org)
to parse XML schema files, and **commsdsl2old** uses [Boost](https://www.boost.org)
to parse its command line parameters (_boost::program_options_),
perform filesystem operations (_boost::filesystem_), and various string manipulation
algorithms. In case Boost libraries are not installed in expected default location
(mostly happens on Windows systems), use variables described in the relevant
[CMake documentation](https://cmake.org/cmake/help/v3.8/module/FindBoost.html) 
to help CMake find required libraries and headers.
It is recommended to use `-DBoost_USE_STATIC_LIBS=ON` parameter to force
linkage with static Boost libraries (especially for Windows systems).
Also on Windows systems there is no need to build [libxml2](http://xmlsoft.org) 
separately. The build process will check it out automatically and build static 
library itself. For example:

### Linux Build
```
$> cd /source/of/this/project
$> mkdir build && cd build
$> cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PWD/install -DCOMMSDSL_BUILD_UNIT_TESTS=OFF ..
$> make install
```
### Windows Build
```
$> cd C:\source\of\this\project
$> mkdir build
$> cd build
$> cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release \ 
    -DCMAKE_INSTALL_PREFIX=%cd%/install -DCOMMSDSL_BUILD_UNIT_TESTS=OFF \
    -DBOOST_ROOT="C:\Libraries\boost_1_65_1" -DBoost_USE_STATIC_LIBS=ON ..
$> nmake install
```
 
