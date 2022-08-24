if (NOT COMMSDSL_BUILD_COMMSDSL2TEST)
    return()
endif ()

set (APP_NAME "commsdsl2test")

add_subdirectory (src)
add_subdirectory (test)