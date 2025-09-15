set (
    src
    CBitfieldField.cpp
    CBundleField.cpp
    CCmake.cpp
    CDataField.cpp
    CEnumField.cpp
    CFloatField.cpp
    CField.cpp
    CGenerator.cpp
    CIntField.cpp
    CListField.cpp
    CMessage.cpp
    CNamespace.cpp
    COptionalField.cpp
    CProgramOptions.cpp
    CProtocolOptions.cpp
    CRefField.cpp
    CSchema.cpp
    CSetField.cpp
    CStringField.cpp
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

