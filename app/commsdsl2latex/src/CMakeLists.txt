set (
    src
    Latex.cpp
    LatexBitfieldField.cpp
    LatexBundleField.cpp
    LatexDataField.cpp
    LatexCmake.cpp
    LatexEnumField.cpp
    LatexField.cpp
    LatexFloatField.cpp
    LatexGenerator.cpp
    LatexIntField.cpp
    LatexListField.cpp
    LatexMessage.cpp
    LatexNamespace.cpp
    LatexOptionalField.cpp
    LatexProgramOptions.cpp
    LatexRefField.cpp
    LatexSchema.cpp
    LatexSetField.cpp
    LatexStringField.cpp
    LatexVariantField.cpp
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

