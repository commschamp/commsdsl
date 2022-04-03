
set (
    src
    ToolsQtBitfieldField.cpp
    ToolsQtBundleField.cpp
    ToolsQtCmake.cpp
    ToolsQtEnumField.cpp
    ToolsQtField.cpp
    ToolsQtFloatField.cpp
    ToolsQtGenerator.cpp
    ToolsQtIntField.cpp
    ToolsQtProgramOptions.cpp
    ToolsQtSetField.cpp
    ToolsQtStringField.cpp
    main.cpp
)

add_executable(${APP_NAME} ${src})
target_link_libraries(${APP_NAME} PRIVATE cc::${PROJECT_NAME})

if (WIN32)
    target_link_libraries(${APP_NAME} PRIVATE Setupapi.lib Ws2_32.lib imm32.lib winmm.lib)
endif ()

if (COMMSDSL_INSTALL_APPS)
    install(TARGETS ${APP_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()

