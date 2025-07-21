set (
    src
    Latex.cpp
    LatexCmake.cpp
    LatexGenerator.cpp
    LatexProgramOptions.cpp
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

