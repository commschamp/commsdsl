set (
    src
    ToolsQtChecksumLayer.cpp
    ToolsQtCmake.cpp
    ToolsQtCustomLayer.cpp
    ToolsQtDefaultOptions.cpp
    ToolsQtFrame.cpp
    ToolsQtGenerator.cpp
    ToolsQtIdLayer.cpp
    ToolsQtInterface.cpp
    ToolsQtLayer.cpp
    ToolsQtMessage.cpp
    ToolsQtMsgFactory.cpp
    ToolsQtNamespace.cpp
    ToolsQtPayloadLayer.cpp
    ToolsQtProgramOptions.cpp
    ToolsQtPlugin.cpp
    ToolsQtSizeLayer.cpp
    ToolsQtSyncLayer.cpp
    ToolsQtValueLayer.cpp
    ToolsQtVersion.cpp
    main.cpp
)

add_executable(${APP_NAME} ${src})
target_link_libraries(${APP_NAME} PRIVATE cc::${PROJECT_NAME})
commsdsl_platform_specific_link(${APP_NAME})
add_dependencies(${ALL_APPS_TGT} ${APP_NAME})

if (COMMSDSL_INSTALL_APPS)
    install(TARGETS ${APP_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()

