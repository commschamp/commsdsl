if (NOT COMMSDSL_BUILD_APPS)
    return ()
endif ()

add_subdirectory (commsdsl2comms)
add_subdirectory (commsdsl2new)
add_subdirectory (commsdsl2test)
