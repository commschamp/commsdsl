cmake_minimum_required (VERSION 3.1)
project ("commsdsl")

option (COMMSDSL_NO_WARN_AS_ERR "Do NOT treat warning as error" OFF)
option (COMMSDSL_NO_CCACHE "Disable use of ccache on UNIX system" OFF)
option (COMMSDSL_NO_TESTS "Disable unittesting" OFF)
option (COMMSDSL_VALGRIND_TESTS "Enable testing with valgrind" OFF)
option (COMMSDSL_TEST_BUILD_CC_PLUGIN "Build CommsChampion plugin in commsdsl2comms unittiesting" ON)
option (COMMSDSL_TEST_USE_SANITIZERS "Build unittiests with sanitizers" ON)
option (COMMSDSL_WIN_ALLOW_LIBXML_BUILD "Allow internal build of libxml2 on Windows platforms" ON)

# Additional variables to be used if needed
# ---------------------------
# COMMSDSL_QT_DIR - Path to Qt5 installation directory, needed to build generated testing projects.
# COMMS_INSTALL_DIR - Path to externally built and installed CommsChampion project
# COMMSDSL_TESTS_CXX_STANDARD - C++ standard to use in unittests
# COMMSDSL_EXTERNALS_DIR - Directory to contain sources for external projects, defaults to ${PROJECT_SOURCE_DIR}/externals.
# CC_TAG - Tag/branch of CommsChampion project to use instead of default

if (NOT CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 14)
endif()

if ("${CC_TAG}" STREQUAL "")
    set (CC_TAG "develop")
endif()

set (CMAKE_SCIPTS_DIR "${PROJECT_SOURCE_DIR}/cmake")

if (NOT COMMSDSL_EXTERNALS_DIR)
    set (COMMSDSL_EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")
endif ()

if (COMMS_INSTALL_DIR)
    set (CC_CMAKE_DIR ${COMMS_INSTALL_DIR}/lib/LibComms/cmake)
else ()
    set (CC_SRC_DIR "${COMMSDSL_EXTERNALS_DIR}/comms_champion")
    include (${CMAKE_SCIPTS_DIR}/CC_Prefetch.cmake)
    cc_prefetch(SRC_DIR ${CC_SRC_DIR} TAG ${CC_TAG})
    set (CC_CMAKE_DIR ${CC_SRC_DIR}/cmake)
endif ()

################################################
# Compiler options
set (warn_opt)
if (NOT COMMSDSL_NO_WARN_AS_ERR)
    set (warn_opt WARN_AS_ERR)
endif ()

set (ccache_opt)
if ((UNIX) AND (NOT COMMSDSL_NO_CCACHE))
    set (ccache_opt USE_CCACHE)
endif ()

include (${CC_CMAKE_DIR}/CC_Compile.cmake)
cc_compile(${warn_opt} ${ccache_opt})
cc_msvc_force_warn_opt("/W4")

#if (MSVC)
#    add_definitions("/wd4251" "/wd4503" "/wd4309" "/wd4267" "-D_SCL_SECURE_NO_WARNINGS")
#endif ()

################################################

if (NOT COMMSDSL_NO_TESTS)
    include (${CC_CMAKE_DIR}/CC_CxxtestFuncs.cmake)
    cc_get_cxxtest(INSTALL_PREFIX ${COMMSDSL_EXTERNALS_DIR})

    if (COMMSDSL_VALGRIND_TESTS)
        find_program(VALGRIND_EXECUTABLE NAMES "valgrind")
    endif ()    
endif ()

include(GNUInstallDirs)

add_subdirectory(lib)
add_subdirectory(app)

