set (
    src
    ToolsQtBitfieldField.cpp
    ToolsQtBundleField.cpp
    ToolsQtChecksumLayer.cpp
    ToolsQtCmake.cpp
    ToolsQtCustomLayer.cpp
    ToolsQtDataField.cpp
    ToolsQtDefaultOptions.cpp
    ToolsQtEnumField.cpp
    ToolsQtField.cpp
    ToolsQtFloatField.cpp
    ToolsQtFrame.cpp
    ToolsQtGenerator.cpp
    ToolsQtIdLayer.cpp
    ToolsQtInputMessages.cpp
    ToolsQtIntField.cpp
    ToolsQtInterface.cpp
    ToolsQtLayer.cpp
    ToolsQtListField.cpp
    ToolsQtMessage.cpp
    ToolsQtMsgFactory.cpp
    ToolsQtNamespace.cpp
    ToolsQtOptionalField.cpp
    ToolsQtPayloadLayer.cpp
    ToolsQtProgramOptions.cpp
    ToolsQtPlugin.cpp
    ToolsQtRefField.cpp
    ToolsQtSetField.cpp
    ToolsQtSizeLayer.cpp
    ToolsQtStringField.cpp
    ToolsQtSyncLayer.cpp
    ToolsQtValueLayer.cpp
    ToolsQtVariantField.cpp
    main.cpp
)

add_executable(${APP_NAME} ${src})
target_link_libraries(${APP_NAME} PRIVATE cc::${PROJECT_NAME})
commsdsl_platform_specific_link(${APP_NAME})

if (COMMSDSL_INSTALL_APPS)
    install(TARGETS ${APP_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()

