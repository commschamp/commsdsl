cmake_minimum_required (VERSION 3.1)
project ("commsdsl")

option (COMMSDSL_NO_WARN_AS_ERR "Do NOT treat warning as error" OFF)
option (COMMSDSL_NO_CCACHE "Disable use of ccache on UNIX system" OFF)
option (COMMSDSL_NO_TESTS "Disable unittesting" OFF)
option (COMMSDSL_VALGRIND_TESTS "Enable testing with valgrind" OFF)

# Additional variables to be used if needed
# ---------------------------
# COMMSDSL_QT_DIR - Path to Qt5 installation directory, needed to build generated testing projects.
# COMMS_INSTALL_DIR - Path to externally built and installed CommsChampion project
# COMMSDSL_TESTS_CXX_STANDARD - C++ standard to use in unittests

if (NOT CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 14)
endif()

if ("${COMMSDSL_INSTALL_DIR}" STREQUAL "")
    set (COMMSDSL_INSTALL_DIR "${CMAKE_BINARY_DIR}/install")
endif ()

if ((CMAKE_COMPILER_IS_GNUCC) OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    set (extra_flags_list
        "-Wall" "-Wextra" "-Wcast-align" "-Wcast-qual" "-Wctor-dtor-privacy"
        "-Wmissing-include-dirs"
        "-Woverloaded-virtual" "-Wredundant-decls" "-Wshadow" "-Wundef" "-Wunused"
        "-Wno-unknown-pragmas" "-fdiagnostics-show-option"
    )

    if (CMAKE_COMPILER_IS_GNUCC)
        list (APPEND extra_flags_list
            "-Wnoexcept" "-Wlogical-op" "-Wstrict-null-sentinel"
        )

        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8")
            list (APPEND extra_flags_list
                "-Wno-pragmas" "-Wno-type-limits"
            )
        endif()

    endif ()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        list (APPEND extra_flags_list "-Wno-dangling-field -Wno-unused-command-line-argument")
    endif ()
    
    if (NOT COMMSDSL_NO_WARN_AS_ERR)
        list (APPEND extra_flags_list "-Werror")
    endif ()
    
    string(REPLACE ";" " " extra_flags "${extra_flags_list}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${extra_flags}")
    
elseif (MSVC)

    add_definitions("/wd4251" "/wd4503" "/wd4309" "/wd4267" "-D_SCL_SECURE_NO_WARNINGS")

    if (NOT COMMSDSL_NO_WARN_AS_ERR)
        add_definitions("/WX")
    endif ()
    
endif ()

enable_testing ()

if ((UNIX) AND (NOT COMMSDSL_NO_CCACHE))
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)
endif ()

while (TRUE)
    if (COMMSDSL_NO_TESTS)
        break ()
    endif ()
        
    find_package (Git)
    if (NOT GIT_FOUND)
        message (WARNING "Git is required to check out external projects for testing")
        break ()
    endif ()

    set (repo_name "cxxtest")
    set (repo_tag "4.4")
    set (repo_url "https://github.com/CxxTest/cxxtest.git")
    set (stamp "${CMAKE_CURRENT_BINARY_DIR}/${repo_name}.stamp")
    if ((EXISTS ${stamp}) AND (EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${repo_name}"))
        execute_process (
            COMMAND ${GIT_EXECUTABLE} checkout ${repo_tag}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${repo_name}
        )
    else ()
        execute_process (
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/${repo_name}"
        )

        execute_process (
            COMMAND 
                ${GIT_EXECUTABLE} clone -b ${repo_tag} --depth 1 ${repo_url} ${CMAKE_CURRENT_BINARY_DIR}/${repo_name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RESULT_VARIABLE git_result
        )

        if (NOT "${git_result}" STREQUAL "0")
            message (WARNING "git clone/checkout of ${repo_name} failed")
            break ()
        endif ()
 
        execute_process (
            COMMAND ${CMAKE_COMMAND} -E touch "${stamp}"
        )
    endif ()

    set (CXXTEST_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/cxxtest")
    set (CXXTEST_PYTHON_TESTGEN_EXECUTABLE "${CXXTEST_INCLUDE_DIR}/bin/cxxtestgen")

    find_package (CxxTest)
    if (NOT CXXTEST_FOUND)
        message (WARNING "Wrong cxxtest paths, must be fixed, cannot do testing...")
        break()
    endif ()
    
    if (COMMSDSL_VALGRIND_TESTS)
        if ("${COMMSDSL_VALGRAND_EXE_NAME}" STREQUAL "")
            set (COMMSDSL_VALGRAND_EXE_NAME "valgrind")
        endif ()
        find_program(VALGRIND_EXECUTABLE NAMES "${COMMSDSL_VALGRAND_EXE_NAME}")
    endif ()    
    
    enable_testing ()
    break ()
endwhile ()

include(GNUInstallDirs)

set (CC_TAG "v1.0")

add_subdirectory(lib)
add_subdirectory(app)

