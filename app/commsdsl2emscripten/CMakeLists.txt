if (NOT COMMSDSL_BUILD_COMMSDSL2EMSCRIPTEN)
    return()
endif ()

set (APP_NAME "commsdsl2emscripten")

add_subdirectory (src)
add_subdirectory (test)
