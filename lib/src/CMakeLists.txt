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

    set(LIBXML2_COMPILER)
    set(LIBXML2_PATHS "prefix=install")
    set(LIBXML2_BUILD_COMMAND)
    if (MINGW)
        set(LIBXML2_COMPILER "compiler=mingw")
        set(LIBXML2_BUILD_CMD make -f Makefile.mingw)
    elseif (MSVC)
        set(LIBXML2_COMPILER "compiler=msvc")
        set(LIBXML2_BUILD_CMD nmake /f Makefile.msvc)
    else()
        message (FATAL_ERROR "At this moment only MSVC and MINGW compilers are supported for windows builds")
    endif ()

    set (INTERNAL_LIBXML_TGT "libxml2_tgt")
    set (LIBXML2_DIR "${CMAKE_CURRENT_BINARY_DIR}/libxml2")
    set (LIBXML2_SRC_DIR "${LIBXML2_DIR}/src")
    set (LIBXML2_BIN_DIR "${LIBXML2_SRC_DIR}/win32")
    set (LIBXML2_INSTALL_DIR "${LIBXML2_BIN_DIR}/install")

    set (LIBXML2_CRUNTIME)
    if (("${CMAKE_BUILD_TYPE}" STREQUAL "") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "None") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
        set (LIBXML2_CRUNTIME "cruntime=/MDd")
    endif ()

    include(ExternalProject)
    ExternalProject_Add(
        ${INTERNAL_LIBXML_TGT}
        PREFIX "${LIBXML2_DIR}"
        STAMP_DIR "${LIBXML2_DIR}/stamp"
        GIT_REPOSITORY "https://github.com/GNOME/libxml2.git"
        GIT_TAG "v2.9.7"
        UPDATE_DISCONNECTED 1
        CONFIGURE_COMMAND
            cscript ${LIBXML2_BIN_DIR}/configure.js ftp=no html=no iconv=no ${LIBXML2_COMPILER} static=yes ${LIBXML2_CRUNTIME} ${LIBXML2_PATHS}
        SOURCE_DIR "${LIBXML2_SRC_DIR}"
        BINARY_DIR "${LIBXML2_BIN_DIR}"
        INSTALL_DIR "${LIBXML2_INSTALL_DIR}"
        BUILD_COMMAND
            ${LIBXML2_BUILD_CMD}
        INSTALL_COMMAND
            ${LIBXML2_BUILD_CMD} install
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
    src
    "Protocol.cpp"
    "Field.cpp"
    "Namespace.cpp"
    "IntField.cpp"
    "FloatField.cpp"
    "EnumField.cpp"
    "SetField.cpp"
    "BitfieldField.cpp"
    "BundleField.cpp"
    "StringField.cpp"
    "DataField.cpp"
    "ListField.cpp"
    "RefField.cpp"
    "OptionalField.cpp"
    "VariantField.cpp"
    "OptCond.cpp"
    "Message.cpp"
    "Interface.cpp"
    "Frame.cpp"
    "Layer.cpp"
    "ProtocolImpl.cpp"
    "NamespaceImpl.cpp"
    "XmlWrap.cpp"
    "SchemaImpl.cpp"
    "Schema.cpp"
    "common.cpp"
    "FieldImpl.cpp"
    "IntFieldImpl.cpp"
    "FloatFieldImpl.cpp"
    "EnumFieldImpl.cpp"
    "SetFieldImpl.cpp"
    "BitfieldFieldImpl.cpp"
    "BundleFieldImpl.cpp"
    "StringFieldImpl.cpp"
    "DataFieldImpl.cpp"
    "ListFieldImpl.cpp"
    "RefFieldImpl.cpp"
    "OptionalFieldImpl.cpp"
    "VariantFieldImpl.cpp"
    "OptCondImpl.cpp"
    "MessageImpl.cpp"
    "InterfaceImpl.cpp"
    "FrameImpl.cpp"
    "LayerImpl.cpp"
    "PayloadLayerImpl.cpp"
    "IdLayerImpl.cpp"
    "SizeLayerImpl.cpp"
    "SyncLayerImpl.cpp"
    "ChecksumLayerImpl.cpp"
    "ValueLayerImpl.cpp"
    "CustomLayerImpl.cpp"
    "Alias.cpp"
    "AliasImpl.cpp"
)

add_library(${PROJECT_NAME} ${COMMSDSL_LIBRARY_FORCED_TYPE} ${src})
add_dependencies(${PROJECT_NAME} LibXml2::LibXml2)
target_link_libraries(${PROJECT_NAME} PRIVATE LibXml2::LibXml2)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC Setupapi.lib Ws2_32.lib imm32.lib winmm.lib)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${COMMSDSL_VERSION})

target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/include>
    INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE COMMSDSL_LIB_EXPORT)

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
        NAMESPACE commsdsl::
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
   
