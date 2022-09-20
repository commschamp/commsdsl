set (src
    Swig.cpp
    SwigBitfieldField.cpp
    SwigBundleField.cpp
    SwigChecksumLayer.cpp
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
    SwigInterface.cpp
    SwigIntField.cpp
    SwigLayer.cpp
    SwigListField.cpp
    SwigMessage.cpp
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

