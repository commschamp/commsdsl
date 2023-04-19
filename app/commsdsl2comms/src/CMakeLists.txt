set (
    src
    CommsBitfieldField.cpp
    CommsBundleField.cpp
    CommsChecksumLayer.cpp
    CommsCmake.cpp
    CommsCustomLayer.cpp
    CommsDataField.cpp
    CommsDefaultOptions.cpp
    CommsDispatch.cpp
    CommsDoxygen.cpp
    CommsEnumField.cpp
    CommsField.cpp
    CommsFieldBase.cpp
    CommsFloatField.cpp
    CommsFrame.cpp
    CommsGenerator.cpp
    CommsIdLayer.cpp
    CommsInputMessages.cpp
    CommsIntField.cpp
    CommsInterface.cpp
    CommsLayer.cpp
    CommsListField.cpp
    CommsMessage.cpp
    CommsMsgFactory.cpp
    CommsMsgId.cpp
    CommsNamespace.cpp
    CommsOptionalField.cpp
    CommsPayloadLayer.cpp
    CommsProgramOptions.cpp
    CommsRefField.cpp
    CommsSchema.cpp
    CommsSetField.cpp
    CommsSizeLayer.cpp
    CommsSyncLayer.cpp
    CommsStringField.cpp
    CommsValueLayer.cpp
    CommsVariantField.cpp
    CommsVersion.cpp
    main.cpp
)

add_executable(${APP_NAME} ${src})
target_link_libraries(${APP_NAME} PRIVATE cc::${PROJECT_NAME})
commsdsl_platform_specific_link(${APP_NAME})
target_compile_definitions (${APP_NAME} PRIVATE -DCOMMS_TAG=${COMMS_TAG})

if (COMMSDSL_INSTALL_APPS)
    install(TARGETS ${APP_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()

