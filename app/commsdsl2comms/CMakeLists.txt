set (APP_NAME "commsdsl2comms")

add_subdirectory (src)
add_subdirectory (test)

if (UNIX AND COMMSDSL_INSTALL_APPS)
    install(
        PROGRAMS script/${APP_NAME}.sh
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()
