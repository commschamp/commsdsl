set (APP_NAME "commsdsl2comms")

add_subdirectory (src)
add_subdirectory (test)

if (UNIX)
    install(
        PROGRAMS script/${APP_NAME}.sh
        DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
    )
endif ()
