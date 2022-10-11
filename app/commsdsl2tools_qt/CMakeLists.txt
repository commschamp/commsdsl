if (NOT COMMSDSL_BUILD_COMMSDSL2TOOLS_QT)
    return()
endif ()

set (APP_NAME "commsdsl2tools_qt")

add_subdirectory (src)
add_subdirectory (test)