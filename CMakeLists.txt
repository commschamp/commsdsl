cmake_minimum_required (VERSION 3.10)
project ("commsdsl")

option (COMMSDSL_WARN_AS_ERR "Treat warning as error" ON)
option (COMMSDSL_USE_CCACHE "Use ccache on UNIX systems if it's available" ON)
option (COMMSDSL_INSTALL_LIBRARY "Install commdsl library" OFF )
option (COMMSDSL_INSTALL_LIBRARY_HEADERS "Install commdsl library headers" ${COMMSDSL_INSTALL_LIBRARY})
option (COMMSDSL_BUILD_COMMSDSL2COMMS "Build commsdsl2comms" ON)
option (COMMSDSL_BUILD_COMMSDSL2TEST "Build commsdsl2test" OFF)
option (COMMSDSL_BUILD_COMMSDSL2TOOLS_QT "Build commsdsl2tools_qt" OFF)
option (COMMSDSL_BUILD_COMMSDSL2SWIG "Build commsdsl2swig" OFF)
option (COMMSDSL_BUILD_COMMSDSL2EMSCRIPTEN "Build commsdsl2emscripten" OFF)
option (COMMSDSL_INSTALL_APPS "Install applications" ON)
option (COMMSDSL_BUILD_UNIT_TESTS "Build unittests." OFF)
option (COMMSDSL_BUILD_COMMSDSL2COMMS_TESTS "Build commsdsl2comms unittests." ${COMMSDSL_BUILD_UNIT_TESTS})
option (COMMSDSL_BUILD_COMMSDSL2TOOLS_QT_TESTS "Build commsdsl2tools_qt unittests." ${COMMSDSL_BUILD_UNIT_TESTS})
option (COMMSDSL_BUILD_COMMSDSL2TOOLS_QT_TESTS_CHUNK1 "Build chunk1 commsdsl2tools_qt unittests." ${COMMSDSL_BUILD_COMMSDSL2TOOLS_QT_TESTS})
option (COMMSDSL_BUILD_COMMSDSL2TOOLS_QT_TESTS_CHUNK2 "Build chunk2 commsdsl2tools_qt unittests." ${COMMSDSL_BUILD_COMMSDSL2TOOLS_QT_TESTS})
option (COMMSDSL_BUILD_COMMSDSL2TOOLS_QT_TESTS_CHUNK3 "Build chunk3 commsdsl2tools_qt unittests." ${COMMSDSL_BUILD_COMMSDSL2TOOLS_QT_TESTS})
option (COMMSDSL_BUILD_COMMSDSL2TEST_TESTS "Build commsdsl2test unittests." ${COMMSDSL_BUILD_UNIT_TESTS})
option (COMMSDSL_VALGRIND_TESTS "Enable testing with valgrind (applicable when COMMSDSL_BUILD_UNIT_TESTS is on)" OFF)
option (COMMSDSL_TEST_USE_SANITIZERS "Build unittiests with sanitizers (applicable when COMMSDSL_BUILD_UNIT_TESTS is on)" ON)
option (COMMSDSL_TEST_BUILD_DOC "Build documentation target in generated projects (applicable when COMMSDSL_BUILD_UNIT_TESTS is on)" OFF)
option (COMMSDSL_FORCE_INTERNAL_LIBXML_BUILD "Force internal build of libxml2 when external one is not found" OFF)

# Additional variables to be used if needed
# ---------------------------
# COMMSDSL_TESTS_CXX_STANDARD - C++ standard to use in unittests
# COMMSDSL_TESTS_C_COMPILER - C compiler to build unittests
# COMMSDSL_TESTS_CXX_COMPILER - C++ compiler to build unittests
# COMMSDSL_EXTERNALS_DIR - Directory to contain sources for external projects, defaults to ${PROJECT_SOURCE_DIR}/externals.
# COMMSDSL_TESTS_QT_VERSION - The major Qt version for tests, defaults to 5
# COMMSDSL_TESTS_CXX_STANDARD - The C++ standard to use in tests, defaults to 11

# Other used CMake variables
# CMAKE_CXX_STANDARD - The C++ standard to use during the compilation, defaults to 17 (min)

# Deprecated options for backward compatibility, use the ones above for new builds.
# ---------------------------
# option (COMMSDSL_NO_WARN_AS_ERR "Do NOT treat warning as error" OFF)
# option (COMMSDSL_NO_CCACHE "Disable use of ccache on UNIX system" OFF)
# option (COMMSDSL_NO_TESTS "Disable unittesting" OFF)
# option (COMMSDSL_WIN_ALLOW_LIBXML_BUILD "Allow internal build of libxml2 on Windows platforms" ON)

################################################

set (CMAKE_SCIPTS_DIR "${PROJECT_SOURCE_DIR}/cmake")
include (${CMAKE_SCIPTS_DIR}/CommsdslFuncs.cmake)
include (${CMAKE_SCIPTS_DIR}/CommsdslCompile.cmake)

################################################

# Check usage of deprecated options and ovewrite the default values of proper ones
commsdsl_negate_option(COMMSDSL_NO_WARN_AS_ERR COMMSDSL_WARN_AS_ERR)
commsdsl_negate_option(COMMSDSL_NO_CCACHE COMMSDSL_USE_CCACHE)
commsdsl_negate_option(COMMSDSL_NO_TESTS COMMSDSL_BUILD_UNIT_TESTS)

if (WIN32 AND COMMSDSL_WIN_ALLOW_LIBXML_BUILD)
    set (COMMSDSL_FORCE_INTERNAL_LIBXML_BUILD ON)
endif ()

################################################

if ("${COMMSDSL_TESTS_QT_VERSION}" STREQUAL "")
    set (COMMSDSL_TESTS_QT_VERSION 5)
endif ()

if ("${CMAKE_CXX_STANDARD}" STREQUAL "")
    set (CMAKE_CXX_STANDARD 17)
endif ()

if ("${CMAKE_CXX_STANDARD}" VERSION_LESS "17")
    message (FATAL_ERROR "Use C++17 or later to compile this project.")
endif()

if ("${COMMSDSL_TESTS_CXX_STANDARD}" STREQUAL "")
    set (COMMSDSL_TESTS_CXX_STANDARD ${CMAKE_CXX_STANDARD})
endif ()

if (NOT COMMSDSL_TESTS_C_COMPILER)
    set (COMMSDSL_TESTS_C_COMPILER ${CMAKE_C_COMPILER})
endif ()

if (NOT COMMSDSL_TESTS_CXX_COMPILER)
    set (COMMSDSL_TESTS_CXX_COMPILER ${CMAKE_CXX_COMPILER})
endif ()

if (NOT COMMSDSL_EXTERNALS_DIR)
    set (COMMSDSL_EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")
endif ()

# Compiler options
set (warn_opt)
if (COMMSDSL_WARN_AS_ERR)
    set (warn_opt WARN_AS_ERR)
endif ()

set (ccache_opt)
if ((UNIX) AND (COMMSDSL_USE_CCACHE))
    set (ccache_opt USE_CCACHE)
endif ()

commsdsl_compile(${warn_opt} ${ccache_opt})
commsdsl_msvc_force_warn_opt("/W4")
set (COMMSDSL_FOUND_COMMS_INTERNAL OFF CACHE INTERNAL "Mark COMMS library found" FORCE)
function (commsdsl_mark_comms_found)
    set (COMMSDSL_FOUND_COMMS_INTERNAL ON CACHE INTERNAL "Mark COMMS library found" FORCE)
endfunction ()

################################################

if (COMMSDSL_BUILD_UNIT_TESTS)
    find_package(LibComms REQUIRED)
    include (${LibComms_DIR}/CC_CxxtestFuncs.cmake)
    cc_get_cxxtest(INSTALL_PREFIX ${COMMSDSL_EXTERNALS_DIR})

    if (COMMSDSL_VALGRIND_TESTS)
        find_program(VALGRIND_EXECUTABLE NAMES "valgrind")
    endif ()    

    enable_testing()
endif ()    

include(GNUInstallDirs)

add_subdirectory(lib)
add_subdirectory(app)

