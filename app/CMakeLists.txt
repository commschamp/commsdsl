if (NOT COMMSDSL_BUILD_APPS)
    return ()
endif ()

#add_subdirectory (commsdsl2old)
add_subdirectory (commsdsl2new)
add_subdirectory (commsdsl2test)
add_subdirectory (commsdsl2tools_qt)
