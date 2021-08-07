find_package(Boost ${COMMSDSL_BOOST_VERSION} REQUIRED
    COMPONENTS program_options filesystem)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

set (
    src
    "main.cpp"
    "ProgramOptions.cpp"
    "Logger.cpp"
    "Generator.cpp"
    "Namespace.cpp"
    "Message.cpp"
    "Field.cpp"
    "IntField.cpp"
    "EnumField.cpp"
    "SetField.cpp"
    "FloatField.cpp"
    "RefField.cpp"
    "BitfieldField.cpp"
    "OptionalField.cpp"
    "BundleField.cpp"
    "StringField.cpp"
    "DataField.cpp"
    "ListField.cpp"
    "VariantField.cpp"
    "FieldBase.cpp"
    "DefaultOptions.cpp"
    "MsgId.cpp"
    "Interface.cpp"
    "AllMessages.cpp"
    "Dispatch.cpp"
    "Frame.cpp"
    "Layer.cpp"
    "PayloadLayer.cpp"
    "IdLayer.cpp"
    "SizeLayer.cpp"
    "SyncLayer.cpp"
    "ValueLayer.cpp"
    "ChecksumLayer.cpp"
    "CustomLayer.cpp"
    "Cmake.cpp"
    "common.cpp"
    "Plugin.cpp"
    "Doxygen.cpp"
    "Version.cpp"
    "Test.cpp"
    "License.cpp"
)

add_executable(${APP_NAME} ${src})
target_link_libraries(${APP_NAME} PRIVATE ${CMAKE_PROJECT_NAME} ${Boost_LIBRARIES} ${LIBCOMMSDSL_LIBRARIES})
target_compile_definitions (${APP_NAME} PRIVATE -DBOOST_NO_CXX11_SCOPED_ENUMS -DCC_TAG=${CC_TAG})

target_compile_options(${APP_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>: -Wno-undef>
    $<$<CXX_COMPILER_ID:MSVC>: /wd4251>
) 

if (WIN32)
    target_link_libraries(${APP_NAME} PRIVATE Setupapi.lib Ws2_32.lib imm32.lib winmm.lib)
endif ()

install(TARGETS ${APP_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

