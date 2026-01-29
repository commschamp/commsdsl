set (src
    Wireshark.cpp
    WiresharkBitfieldField.cpp
    WiresharkBundleField.cpp
    WiresharkDataField.cpp
    WiresharkEnumField.cpp
    WiresharkField.cpp
    WiresharkFloatField.cpp
    WiresharkFrame.cpp
    WiresharkGenerator.cpp
    WiresharkIntField.cpp
    WiresharkListField.cpp
    WiresharkNamespace.cpp
    WiresharkOptionalField.cpp
    WiresharkProgramOptions.cpp
    WiresharkRefField.cpp
    WiresharkSchema.cpp
    WiresharkSetField.cpp
    WiresharkStringField.cpp
    WiresharkVariantField.cpp
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
