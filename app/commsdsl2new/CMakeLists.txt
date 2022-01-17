set (APP_NAME "commsdsl2new")

set (
    src
    Cmake.cpp
    Generator.cpp
    ProgramOptions.cpp
    main.cpp
)

add_executable(${APP_NAME} ${src})
target_link_libraries(${APP_NAME} PRIVATE cc::${PROJECT_NAME})
target_compile_definitions (${APP_NAME} PRIVATE -DCOMMS_TAG=${COMMS_TAG})

if (WIN32)
    target_link_libraries(${APP_NAME} PRIVATE Setupapi.lib Ws2_32.lib imm32.lib winmm.lib)
endif ()

if (COMMSDSL_INSTALL_APPS)
    install(TARGETS ${APP_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()

