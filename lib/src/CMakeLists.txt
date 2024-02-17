set (INTERNAL_LIBXML_TGT)

while (TRUE)
    if ((NOT WIN32) OR (NOT COMMSDSL_WIN_ALLOW_LIBXML_BUILD))
        # Use libxml2 from system / external repositories
        break()
    endif ()

    find_package(LibXml2 QUIET)
    if (LIBXML2_FOUND)
        # Find again just to display info
        find_package(LibXml2 REQUIRED) 
        break()
    endif ()

    set (INTERNAL_LIBXML_TGT "libxml2_tgt")
    set (LIBXML2_DIR "${CMAKE_CURRENT_BINARY_DIR}/libxml2")
    set (LIBXML2_SRC_DIR "${LIBXML2_DIR}/src")
    set (LIBXML2_BIN_DIR "${LIBXML2_SRC_DIR}/win32")
    set (LIBXML2_INSTALL_DIR "${LIBXML2_BIN_DIR}/install")

    include(ExternalProject)
    ExternalProject_Add(
        ${INTERNAL_LIBXML_TGT}
        PREFIX "${LIBXML2_DIR}"
        STAMP_DIR "${LIBXML2_DIR}/stamp"
        GIT_REPOSITORY "https://github.com/GNOME/libxml2.git"
        GIT_TAG "v2.12.5"
        UPDATE_DISCONNECTED 1
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM}
        CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${LIBXML2_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
            -DBUILD_SHARED_LIBS=OFF -DLIBXML2_WITH_HTML=OFF -DLIBXML2_WITH_HTTP=OFF
            -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_PROGRAMS=OFF -DLIBXML2_WITH_PYTHON=OFF
            -DLIBXML2_WITH_SAX1=OFF -DLIBXML2_WITH_TESTS=OFF -DLIBXML2_WITH_THREADS=OFF
            -DLIBXML2_WITH_WRITER=OFF -DLIBXML2_WITH_XINCLUDE=OFF -DLIBXML2_WITH_LZMA=OFF
            -DLIBXML2_WITH_ZLIB=OFF
        SOURCE_DIR "${LIBXML2_SRC_DIR}"
        BINARY_DIR "${LIBXML2_BIN_DIR}"
        INSTALL_DIR "${LIBXML2_INSTALL_DIR}"
        # BUILD_BYPRODUCTS needed by Ninja
        BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libxml2_a.lib
    )

    set (LIBXML2_FOUND TRUE)
    set (LIBXML2_INCLUDE_DIR "${LIBXML2_INSTALL_DIR}/include/libxml2")
    set (LIBXML2_LIBRARIES "${LIBXML2_INSTALL_DIR}/lib/libxml2_a.lib")
    set (LIBXML2_DEFINITIONS "/DLIBXML_STATIC")
    file(MAKE_DIRECTORY ${LIBXML2_INCLUDE_DIR})

    add_library(libxml2 STATIC IMPORTED GLOBAL)
    add_dependencies(libxml2 ${INTERNAL_LIBXML_TGT})
    set_target_properties(libxml2 PROPERTIES
        IMPORTED_LOCATION ${LIBXML2_LIBRARIES}
        IMPORTED_IMPLIB ${LIBXML2_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES "${LIBXML2_INCLUDE_DIR}"
    )
    add_library(LibXml2::LibXml2 ALIAS libxml2)

    break()
endwhile ()

if (NOT LIBXML2_FOUND)
    find_package(LibXml2 REQUIRED)
endif ()

if (LIBXML2_FOUND AND (NOT TARGET LibXml2::LibXml2))
    # Older versions of cmake may not define LibXml2::LibXml2 target
    add_library(LibXml2::LibXml2 UNKNOWN IMPORTED)
    set_target_properties(LibXml2::LibXml2 PROPERTIES
        IMPORTED_LOCATION ${LIBXML2_LIBRARIES}
        IMPORTED_IMPLIB ${LIBXML2_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES "${LIBXML2_INCLUDE_DIRS}"
    )
endif ()

set (
    parse_src
    "parse/Alias.cpp"
    "parse/AliasImpl.cpp"    
    "parse/BitfieldField.cpp"
    "parse/BitfieldFieldImpl.cpp"
    "parse/BundleField.cpp"
    "parse/BundleFieldImpl.cpp"
    "parse/ChecksumLayerImpl.cpp"
    "parse/CustomLayerImpl.cpp"
    "parse/DataField.cpp"
    "parse/DataFieldImpl.cpp"
    "parse/EnumField.cpp"
    "parse/EnumFieldImpl.cpp"
    "parse/Field.cpp"
    "parse/FieldImpl.cpp"
    "parse/FloatField.cpp"
    "parse/FloatFieldImpl.cpp"
    "parse/Frame.cpp"
    "parse/FrameImpl.cpp"
    "parse/IdLayerImpl.cpp"
    "parse/IntField.cpp"
    "parse/IntFieldImpl.cpp"
    "parse/Interface.cpp"
    "parse/InterfaceImpl.cpp"
    "parse/Layer.cpp"
    "parse/LayerImpl.cpp"
    "parse/ListField.cpp"
    "parse/ListFieldImpl.cpp"
    "parse/Message.cpp"
    "parse/MessageImpl.cpp"
    "parse/Namespace.cpp"
    "parse/NamespaceImpl.cpp"
    "parse/OptCond.cpp"
    "parse/OptCondImpl.cpp"
    "parse/OptionalField.cpp"
    "parse/OptionalFieldImpl.cpp"
    "parse/PayloadLayerImpl.cpp"
    "parse/Protocol.cpp"
    "parse/ProtocolImpl.cpp"
    "parse/RefField.cpp"
    "parse/RefFieldImpl.cpp"
    "parse/Schema.cpp"
    "parse/SchemaImpl.cpp"
    "parse/SetField.cpp"
    "parse/SetFieldImpl.cpp"
    "parse/SizeLayerImpl.cpp"
    "parse/SyncLayerImpl.cpp"
    "parse/StringField.cpp"
    "parse/StringFieldImpl.cpp"
    "parse/ValueLayerImpl.cpp"
    "parse/VariantField.cpp"
    "parse/VariantFieldImpl.cpp"
    "parse/XmlWrap.cpp"
    "parse/common.cpp"
)

set (
    gen_src
    gen/BitfieldField.cpp
    gen/BundleField.cpp
    gen/ChecksumLayer.cpp
    gen/CustomLayer.cpp
    gen/DataField.cpp
    gen/Elem.cpp
    gen/EnumField.cpp
    gen/Field.cpp
    gen/FloatField.cpp
    gen/Frame.cpp
    gen/Generator.cpp
    gen/IdLayer.cpp
    gen/IntField.cpp
    gen/Interface.cpp
    gen/Layer.cpp
    gen/ListField.cpp
    gen/Logger.cpp
    gen/Message.cpp
    gen/Namespace.cpp
    gen/OptionalField.cpp
    gen/ProgramOptions.cpp
    gen/PayloadLayer.cpp
    gen/RefField.cpp
    gen/Schema.cpp
    gen/SetField.cpp
    gen/SizeLayer.cpp
    gen/StringField.cpp
    gen/SyncLayer.cpp
    gen/ValueLayer.cpp
    gen/VariantField.cpp
    gen/comms.cpp
    gen/strings.cpp
    gen/util.cpp
)

add_library(${PROJECT_NAME} STATIC ${parse_src} ${gen_src})
add_library(cc::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_dependencies(${PROJECT_NAME} LibXml2::LibXml2)
target_link_libraries(${PROJECT_NAME} PRIVATE LibXml2::LibXml2)

if ((CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND
    (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0"))
    target_link_libraries(${PROJECT_NAME} PUBLIC stdc++fs)
endif ()

commsdsl_platform_specific_link(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${COMMSDSL_VERSION})

target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/include>
    INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

#target_compile_definitions(${PROJECT_NAME} PRIVATE COMMSDSL_LIB_EXPORT)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>: /wd4251>
) 

if (COMMSDSL_INSTALL_LIBRARY)
    install(
        TARGETS ${PROJECT_NAME} EXPORT "LibCommsdslTargets"
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endif ()

if (COMMSDSL_INSTALL_LIBRARY_HEADERS)
    export(
        TARGETS ${PROJECT_NAME}
        FILE "${PROJECT_BINARY_DIR}/LibCommsdslTargets.cmake")

    export(
        PACKAGE LibCommsdsl
    )

    configure_file(LibCommsdslConfig.cmake.in
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/LibCommsdslConfig.cmake" @ONLY)

    install(
        FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/LibCommsdslConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/LibCommsdsl/cmake/
    )

    install(
        EXPORT LibCommsdslTargets 
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/LibCommsdsl/cmake/
        NAMESPACE cc::
    )

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CMAKE_BINARY_DIR}/LibCommsdslConfigVersion.cmake
        VERSION ${COMMSDSL_VERSION}
        COMPATIBILITY AnyNewerVersion)
        
    install (
        FILES ${CMAKE_BINARY_DIR}/LibCommsdslConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/LibCommsdsl/cmake/
    )     
endif ()
   
