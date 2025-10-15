set (
    src
    CBitfieldField.cpp
    CBundleField.cpp
    CChecksumLayer.cpp
    CCmake.cpp
    CCustomLayer.cpp
    CDataField.cpp
    CEnumField.cpp
    CErrorStatus.cpp
    CFloatField.cpp
    CField.cpp
    CFrame.cpp
    CGenerator.cpp
    CIdLayer.cpp
    CInterface.cpp
    CIntField.cpp
    CLayer.cpp
    CListField.cpp
    CMessage.cpp
    CMsgHandler.cpp
    CMsgId.cpp
    CNamespace.cpp
    COptionalField.cpp
    CPayloadLayer.cpp
    CProgramOptions.cpp
    CProtocolOptions.cpp
    CRefField.cpp
    CSchema.cpp
    CSetField.cpp
    CSizeLayer.cpp
    CStringField.cpp
    CSyncLayer.cpp
    CValueLayer.cpp
    CVariantField.cpp
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

