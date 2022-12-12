set (src
    EmscriptenBitfieldField.cpp
    EmscriptenBundleField.cpp
    EmscriptenComms.cpp
    EmscriptenDataBuf.cpp
    EmscriptenDataField.cpp
    EmscriptenEnumField.cpp
    EmscriptenField.cpp
    EmscriptenFloatField.cpp
    EmscriptenGenerator.cpp
    EmscriptenInterface.cpp
    EmscriptenIntField.cpp
    EmscriptenListField.cpp
    EmscriptenMessage.cpp
    EmscriptenMsgId.cpp
    EmscriptenOptionalField.cpp
    EmscriptenProgramOptions.cpp
    EmscriptenProtocolOptions.cpp
    EmscriptenRefField.cpp
    EmscriptenSetField.cpp
    EmscriptenStringField.cpp
    EmscriptenVariantField.cpp
    EmscriptenVersion.cpp
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

