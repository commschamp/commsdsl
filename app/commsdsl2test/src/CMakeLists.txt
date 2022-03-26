
set (
    src
    Cmake.cpp
    Generator.cpp
    ProgramOptions.cpp
    Test.cpp
    main.cpp
)

add_executable(${APP_NAME} ${src})
target_link_libraries(${APP_NAME} PRIVATE cc::${PROJECT_NAME})
#target_compile_definitions (${APP_NAME} PRIVATE -DBOOST_NO_CXX11_SCOPED_ENUMS -DCC_TAG=${CC_TAG})

#target_compile_options(${APP_NAME} PRIVATE
#    $<$<CXX_COMPILER_ID:GNU>: -Wno-undef>
#    $<$<CXX_COMPILER_ID:MSVC>: /wd4251>
#) 

if (WIN32)
    target_link_libraries(${APP_NAME} PRIVATE Setupapi.lib Ws2_32.lib imm32.lib winmm.lib)
endif ()

if (COMMSDSL_INSTALL_APPS)
    install(TARGETS ${APP_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()

