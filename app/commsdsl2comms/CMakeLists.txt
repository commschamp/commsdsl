if (NOT COMMSDSL_BUILD_COMMSDSL2COMMS)
    return()
endif ()

set (APP_NAME "commsdsl2comms")

add_subdirectory (src)
add_subdirectory (test)