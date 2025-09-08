if (NOT COMMSDSL_BUILD_COMMSDSL2C)
    return()
endif ()

set (APP_NAME "commsdsl2c")

add_subdirectory (src)
add_subdirectory (test)