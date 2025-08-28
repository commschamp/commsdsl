if (NOT COMMSDSL_BUILD_COMMSDSL2LATEX)
    return()
endif ()

set (APP_NAME "commsdsl2latex")

add_subdirectory (src)
add_subdirectory (test)