set (src
    EmscriptenBitfieldField.cpp
    EmscriptenBundleField.cpp
    EmscriptenChecksumLayer.cpp
    EmscriptenCmake.cpp
    EmscriptenComms.cpp
    EmscriptenCustomLayer.cpp
    EmscriptenDataBuf.cpp
    EmscriptenDataField.cpp
    EmscriptenEnumField.cpp
    EmscriptenField.cpp
    EmscriptenFloatField.cpp
    EmscriptenFrame.cpp
    EmscriptenGenerator.cpp
    EmscriptenIdLayer.cpp
    EmscriptenInputMessages.cpp
    EmscriptenInterface.cpp
    EmscriptenIntField.cpp
    EmscriptenLayer.cpp
    EmscriptenListField.cpp
    EmscriptenMessage.cpp
    EmscriptenMsgHandler.cpp
    EmscriptenMsgId.cpp
    EmscriptenNamespace.cpp
    EmscriptenOptionalField.cpp
    EmscriptenPayloadLayer.cpp
    EmscriptenProgramOptions.cpp
    EmscriptenProtocolOptions.cpp
    EmscriptenRefField.cpp
    EmscriptenSchema.cpp
    EmscriptenSetField.cpp
    EmscriptenSizeLayer.cpp
    EmscriptenStringField.cpp
    EmscriptenSyncLayer.cpp
    EmscriptenValueLayer.cpp
    EmscriptenVariantField.cpp
    EmscriptenVersion.cpp
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

