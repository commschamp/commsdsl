if (NOT COMMSDSL_BUILD_COMMSDSL2WIRESHARK)
    return()
endif ()

set (APP_NAME "commsdsl2wireshark")

add_subdirectory (src)
add_subdirectory (test)
