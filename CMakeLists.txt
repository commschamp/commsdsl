cmake_minimum_required (VERSION 3.1)
project ("commsdsl2comms")

option (COMMSDSL2COMMS_NO_WARN_AS_ERR "Do NOT treat warning as error" OFF)
option (COMMSDSL2COMMS_NO_CCACHE "Disable use of ccache on UNIX system" OFF)

if (NOT CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 11)
endif()

if ("${COMMSDSL2COMMS_INSTALL_DIR}" STREQUAL "")
    set (COMMSDSL2COMMS_INSTALL_DIR "${CMAKE_BINARY_DIR}/install")
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
    
    if (NOT COMMSDSL2COMMS_NO_WARN_AS_ERR)
        list (APPEND extra_flags_list "-Werror")
    endif ()
    
    string(REPLACE ";" " " extra_flags "${extra_flags_list}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${extra_flags}")
    
elseif (MSVC)

    add_definitions( "/wd4503" "/wd4309" "/wd4267" "-D_SCL_SECURE_NO_WARNINGS")

    if (NOT COMMSDSL2COMMS_NO_WARN_AS_ERR)
        add_definitions("/WX")
    endif ()
    
endif ()

enable_testing ()

if ((UNIX) AND (NOT COMMSDSL2COMMS_NO_CCACHE))
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)
endif ()

add_subdirectory(src)
#add_subdirectory(test)

