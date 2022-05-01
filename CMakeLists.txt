cmake_minimum_required (VERSION 3.1)
project ("commsdsl")

option (COMMSDSL_WARN_AS_ERR "Treat warning as error" ON)
option (COMMSDSL_USE_CCACHE "Use ccache on UNIX systems if it's available" ON)
option (COMMSDSL_INSTALL_LIBRARY "Install commdsl library" ON )
option (COMMSDSL_INSTALL_LIBRARY_HEADERS "Install commdsl library headers" ${COMMSDSL_INSTALL_LIBRARY})
option (COMMSDSL_BUILD_APPS "Build applications" ON)
option (COMMSDSL_INSTALL_APPS "Build applications" ${COMMSDSL_BUILD_APPS})
option (COMMSDSL_BUILD_UNIT_TESTS "Build unittests." OFF)
option (COMMSDSL_VALGRIND_TESTS "Enable testing with valgrind (applicable when COMMSDSL_BUILD_UNIT_TESTS is on)" OFF)
option (COMMSDSL_TEST_USE_SANITIZERS "Build unittiests with sanitizers (applicable when COMMSDSL_BUILD_UNIT_TESTS is on)" ON)
option (COMMSDSL_TEST_BUILD_DOC "Build documentation target in generated projects (applicable when COMMSDSL_BUILD_UNIT_TESTS is on)" OFF)
option (COMMSDSL_WIN_ALLOW_LIBXML_BUILD "Allow internal build of libxml2 on Windows platforms" ON)

# Additional variables to be used if needed
# ---------------------------
# COMMSDSL_QT_DIR - Path to Qt5 installation directory, needed to build generated testing projects.
# COMMS_INSTALL_DIR - Path to externally built and installed COMMS library project
# COMMSDSL_TESTS_CXX_STANDARD - C++ standard to use in unittests
# COMMSDSL_EXTERNALS_DIR - Directory to contain sources for external projects, defaults to ${PROJECT_SOURCE_DIR}/externals.
# COMMSDSL_BOOST_VERSION - Boost version passed to relevant find_package invocation.
# COMMS_TAG - Tag/branch of COMMS library project to use instead of default
# CC_COMMS_REPO - Repository of COMMS library project, if not specified default github CommsChampion Ecosystem repo is going to be used.

# Deprecated options for backward compatibility, use the ones above for new builds.
# ---------------------------
# option (COMMSDSL_NO_WARN_AS_ERR "Do NOT treat warning as error" OFF)
# option (COMMSDSL_NO_CCACHE "Disable use of ccache on UNIX system" OFF)
# option (COMMSDSL_NO_TESTS "Disable unittesting" OFF)

################################################

set (CMAKE_SCIPTS_DIR "${PROJECT_SOURCE_DIR}/cmake")
include (${CMAKE_SCIPTS_DIR}/CommsdslFuncs.cmake)
include (${CMAKE_SCIPTS_DIR}/CommsdslCompile.cmake)
################################################

# Check usage of deprecated options and ovewrite the default values of proper ones
commsdsl_negate_option(COMMSDSL_NO_WARN_AS_ERR COMMSDSL_WARN_AS_ERR)
commsdsl_negate_option(COMMSDSL_NO_CCACHE COMMSDSL_USE_CCACHE)
commsdsl_negate_option(COMMSDSL_NO_TESTS COMMSDSL_BUILD_UNIT_TESTS)

################################################

if (NOT CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 17)
endif()

if (NOT COMMSDSL_TESTS_CXX_STANDARD)
    set (COMMSDSL_TESTS_CXX_STANDARD 11)
endif ()

if ("${COMMS_TAG}" STREQUAL "")
    set (COMMS_TAG "master")
endif()

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

################################################

if (COMMSDSL_BUILD_UNIT_TESTS)
    while (TRUE)
        if (NOT "${COMMS_INSTALL_DIR}" STREQUAL "")
            set (CC_CMAKE_DIR ${COMMS_INSTALL_DIR}/lib/LibComms/cmake)
            break()
        endif ()

        find_package(LibComms QUIET)
        if (TARGET cc::comms)
            if ("${LibComms_DIR}" STREQUAL "")
                message (FATAL_ERROR "The comms library found but the directory is not known")
            endif ()

            set (CC_CMAKE_DIR ${LibComms_DIR})
            break()
        endif ()

        commsdsl_ensure_comms_target ()
        set (CC_CMAKE_DIR ${LibComms_DIR})
        break ()
    endwhile ()

    include (${CC_CMAKE_DIR}/CC_CxxtestFuncs.cmake)
    cc_get_cxxtest(INSTALL_PREFIX ${COMMSDSL_EXTERNALS_DIR})

    if (COMMSDSL_VALGRIND_TESTS)
        find_program(VALGRIND_EXECUTABLE NAMES "valgrind")
    endif ()    

    enable_testing()
endif ()    

include(GNUInstallDirs)

add_subdirectory(lib)
add_subdirectory(app)

