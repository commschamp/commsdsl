set (INTERNAL_LIBXML_TGT)

while (TRUE)
    if (NOT COMMSDSL_FORCE_INTERNAL_LIBXML_BUILD)
        find_package(LibXml2 QUIET)
        if (LIBXML2_FOUND)
            # Find again just to display info
            find_package(LibXml2 REQUIRED)
            break()
        endif ()
    endif ()

    set (INTERNAL_LIBXML_TGT "libxml2_tgt")
    set (LIBXML2_DIR "${CMAKE_CURRENT_BINARY_DIR}/libxml2")
    set (LIBXML2_SRC_DIR "${LIBXML2_DIR}/src")
    set (LIBXML2_BIN_DIR "${LIBXML2_DIR}/build")
    set (LIBXML2_INSTALL_DIR "${LIBXML2_BIN_DIR}/install")
    set (LIBXML2_LIB_NAME libxml2.a)
    if (WIN32)
        set (LIBXML2_LIB_NAME libxml2sd.lib)
        if (("${CMAKE_BUILD_TYPE}" STREQUAL "Release") OR
            ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel") OR
            ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"))
            set (LIBXML2_LIB_NAME libxml2s.lib)
        endif ()
    endif ()

    include(ExternalProject)
    ExternalProject_Add(
        ${INTERNAL_LIBXML_TGT}
        PREFIX "${LIBXML2_DIR}"
        STAMP_DIR "${LIBXML2_DIR}/stamp"
        GIT_REPOSITORY "https://github.com/GNOME/libxml2.git"
        GIT_TAG "v2.12.5"
        GIT_SHALLOW 1
        UPDATE_DISCONNECTED 1
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM}
        CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${LIBXML2_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DBUILD_SHARED_LIBS=OFF -DLIBXML2_WITH_HTTP=OFF
            -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_PROGRAMS=OFF -DLIBXML2_WITH_PYTHON=OFF
            -DLIBXML2_WITH_TESTS=OFF -DLIBXML2_WITH_THREADS=OFF
            -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_ZLIB=OFF
        SOURCE_DIR "${LIBXML2_SRC_DIR}"
        BINARY_DIR "${LIBXML2_BIN_DIR}"
        INSTALL_DIR "${LIBXML2_INSTALL_DIR}"

        # BUILD_BYPRODUCTS are needed by Ninja
        BUILD_BYPRODUCTS <INSTALL_DIR>/lib/${LIBXML2_LIB_NAME}
    )

    set (LIBXML2_FOUND TRUE)
    set (LIBXML2_INCLUDE_DIR "${LIBXML2_INSTALL_DIR}/include/libxml2")
    set (LIBXML2_LIBRARIES "${LIBXML2_INSTALL_DIR}/lib/${LIBXML2_LIB_NAME}")

    set (LIBXML2_DEFINITIONS "LIBXML_STATIC")
    file(MAKE_DIRECTORY ${LIBXML2_INCLUDE_DIR})

    add_library(libxml2 STATIC IMPORTED GLOBAL)
    add_dependencies(libxml2 ${INTERNAL_LIBXML_TGT})
    set_target_properties(libxml2 PROPERTIES
        IMPORTED_LOCATION ${LIBXML2_LIBRARIES}
        IMPORTED_IMPLIB ${LIBXML2_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES "${LIBXML2_INCLUDE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS "${LIBXML2_DEFINITIONS}"
    )
    add_library(LibXml2::LibXml2 ALIAS libxml2)

    break()
endwhile ()

if (LIBXML2_FOUND AND (NOT TARGET LibXml2::LibXml2))
    # Older versions of cmake may not define LibXml2::LibXml2 target
    add_library(LibXml2::LibXml2 UNKNOWN IMPORTED)
    set_target_properties(LibXml2::LibXml2 PROPERTIES
        IMPORTED_LOCATION ${LIBXML2_LIBRARIES}
        IMPORTED_IMPLIB ${LIBXML2_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES "${LIBXML2_INCLUDE_DIRS}"
    )
endif ()

if (NOT TARGET LibXml2::LibXml2)
    message (FATAL_ERROR "LibXml2 dependency is not found and not built internally" )
endif ()

set (
    parse_src
    "parse/ParseAlias.cpp"
    "parse/ParseAliasImpl.cpp"
    "parse/ParseBitfieldField.cpp"
    "parse/ParseBitfieldFieldImpl.cpp"
    "parse/ParseBundleField.cpp"
    "parse/ParseBundleFieldImpl.cpp"
    "parse/ParseChecksumLayerImpl.cpp"
    "parse/ParseCustomLayerImpl.cpp"
    "parse/ParseDataField.cpp"
    "parse/ParseDataFieldImpl.cpp"
    "parse/ParseEnumField.cpp"
    "parse/ParseEnumFieldImpl.cpp"
    "parse/ParseField.cpp"
    "parse/ParseFieldImpl.cpp"
    "parse/ParseFloatField.cpp"
    "parse/ParseFloatFieldImpl.cpp"
    "parse/ParseFrame.cpp"
    "parse/ParseFrameImpl.cpp"
    "parse/ParseIdLayerImpl.cpp"
    "parse/ParseIntField.cpp"
    "parse/ParseIntFieldImpl.cpp"
    "parse/ParseInterface.cpp"
    "parse/ParseInterfaceImpl.cpp"
    "parse/ParseLayer.cpp"
    "parse/ParseLayerImpl.cpp"
    "parse/ParseListField.cpp"
    "parse/ParseListFieldImpl.cpp"
    "parse/ParseMessage.cpp"
    "parse/ParseMessageImpl.cpp"
    "parse/ParseNamespace.cpp"
    "parse/ParseNamespaceImpl.cpp"
    "parse/ParseOptCond.cpp"
    "parse/ParseOptCondImpl.cpp"
    "parse/ParseOptionalField.cpp"
    "parse/ParseOptionalFieldImpl.cpp"
    "parse/ParsePayloadLayerImpl.cpp"
    "parse/ParseProtocol.cpp"
    "parse/ParseProtocolImpl.cpp"
    "parse/ParseRefField.cpp"
    "parse/ParseRefFieldImpl.cpp"
    "parse/ParseSchema.cpp"
    "parse/ParseSchemaImpl.cpp"
    "parse/ParseSetField.cpp"
    "parse/ParseSetFieldImpl.cpp"
    "parse/ParseSizeLayerImpl.cpp"
    "parse/ParseStringField.cpp"
    "parse/ParseStringFieldImpl.cpp"
    "parse/ParseSyncLayerImpl.cpp"
    "parse/ParseValueLayerImpl.cpp"
    "parse/ParseVariantField.cpp"
    "parse/ParseVariantFieldImpl.cpp"
    "parse/ParseXmlWrap.cpp"
    "parse/parse_common.cpp"
)

set (
    gen_src
    gen/GenBitfieldField.cpp
    gen/GenBundleField.cpp
    gen/GenChecksumLayer.cpp
    gen/GenCustomLayer.cpp
    gen/GenDataField.cpp
    gen/GenElem.cpp
    gen/GenEnumField.cpp
    gen/GenField.cpp
    gen/GenFloatField.cpp
    gen/GenFrame.cpp
    gen/GenGenerator.cpp
    gen/GenIdLayer.cpp
    gen/GenIntField.cpp
    gen/GenInterface.cpp
    gen/GenLayer.cpp
    gen/GenListField.cpp
    gen/GenLogger.cpp
    gen/GenMessage.cpp
    gen/GenNamespace.cpp
    gen/GenOptionalField.cpp
    gen/GenProgramOptions.cpp
    gen/GenPayloadLayer.cpp
    gen/GenRefField.cpp
    gen/GenSchema.cpp
    gen/GenSetField.cpp
    gen/GenSizeLayer.cpp
    gen/GenStringField.cpp
    gen/GenSyncLayer.cpp
    gen/GenValueLayer.cpp
    gen/GenVariantField.cpp
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

