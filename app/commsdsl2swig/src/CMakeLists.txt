set (src
    Swig.cpp
    SwigBitfieldField.cpp
    SwigBundleField.cpp
    SwigComms.cpp
    SwigDataField.cpp
    SwigEnumField.cpp
    SwigField.cpp
    SwigFloatField.cpp
    SwigGenerator.cpp
    SwigIntField.cpp
    SwigListField.cpp
    SwigNamespace.cpp
    SwigOptionalField.cpp
    SwigProgramOptions.cpp
    SwigRefField.cpp
    SwigSetField.cpp
    SwigStringField.cpp
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

