set (src
    Swig.cpp
    SwigBitfieldField.cpp
    SwigBundleField.cpp
    SwigChecksumLayer.cpp
    SwigCmake.cpp
    SwigComms.cpp
    SwigCustomLayer.cpp
    SwigDataBuf.cpp
    SwigDataField.cpp
    SwigEnumField.cpp
    SwigField.cpp
    SwigFloatField.cpp
    SwigFrame.cpp
    SwigGenerator.cpp
    SwigIdLayer.cpp
    SwigInputMessages.cpp
    SwigInterface.cpp
    SwigIntField.cpp
    SwigLayer.cpp
    SwigListField.cpp
    SwigMessage.cpp
    SwigMsgHandler.cpp
    SwigMsgId.cpp
    SwigNamespace.cpp
    SwigOptionalField.cpp
    SwigPayloadLayer.cpp
    SwigProgramOptions.cpp
    SwigProtocolOptions.cpp
    SwigRefField.cpp
    SwigSchema.cpp
    SwigSetField.cpp
    SwigSizeLayer.cpp
    SwigStringField.cpp
    SwigSyncLayer.cpp
    SwigValueLayer.cpp
    SwigVariantField.cpp
    SwigVersion.cpp
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

