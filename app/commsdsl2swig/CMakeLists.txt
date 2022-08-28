if (NOT COMMSDSL_BUILD_COMMSDSL2SWIG)
    return()
endif ()

set (APP_NAME "commsdsl2swig")

add_subdirectory (src)
add_subdirectory (test)
