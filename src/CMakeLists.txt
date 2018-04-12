set (INTERNAL_LIBXML_TGT)
while (TRUE)
    if (UNIX)
        # Use libxml2 from system repositories
        break()
    endif ()

    if ((NOT "${LIBXML2_INCLUDE_DIR}" STREQUAL "") AND (NOT "${LIBXML2_LIBRARIES}" STREQUAL ""))
        # External build of libxml2 is provided
        break()
    endif ()

    if (NOT MSVC)
        message (FATAL_ERROR "At this moment only MSVC compiler is supported for windows builds")
    endif ()

    set (INTERNAL_LIBXML_TGT "libxml2_tgt")
    set (LIBXML2_DIR "${CMAKE_CURRENT_BINARY_DIR}/libxml2")
    set (LIBXML2_SRC_DIR "${LIBXML2_DIR}/src")
    set (LIBXML2_BIN_DIR "${LIBXML2_SRC_DIR}/win32")

    set (LIBXML2_CRUNTIME)
    if (("${CMAKE_BUILD_TYPE}" STREQUAL "") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "None") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
        set (LIBXML2_CRUNTIME "cruntime=/MDd")
    endif ()

    include(ExternalProject)
    ExternalProject_Add(
        ${INTERNAL_LIBXML_TGT}
        PREFIX "${LIBXML2_DIR}"
        STAMP_DIR "${LIBXML2_DIR}"
        GIT_REPOSITORY "git://git.gnome.org/libxml2"
        GIT_TAG "v2.9.7"
        CONFIGURE_COMMAND
            cscript ${LIBXML2_BIN_DIR}/configure.js ftp=no html=no iconv=no compiler=msvc static=yes ${LIBXML2_CRUNTIME} bindir=install\\bin incdir=install\\include libdir=install\\lib sodir=install\\bin
        SOURCE_DIR "${LIBXML2_SRC_DIR}"
        BINARY_DIR "${LIBXML2_BIN_DIR}"
        BUILD_COMMAND
            nmake /f Makefile.msvc
        INSTALL_COMMAND
            nmake /f Makefile.msvc install
    )

    set (LIBXML2_FOUND TRUE)
    set (LIBXML2_INCLUDE_DIR "${LIBXML2_BIN_DIR}/install/include/libxml2")
    set (LIBXML2_LIBRARIES "${LIBXML2_BIN_DIR}/install/lib/libxml2_a.lib")
    set (LIBXML2_DEFINITIONS "/DLIBXML_STATIC")
    break()
endwhile ()

if (NOT LIBXML2_FOUND)
    find_package(LibXml2 REQUIRED)
endif ()

add_definitions (-DBBMP_LIB_EXPORT)

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
    "RefFieldImpl.cpp"
)

add_library(${PROJECT_NAME} SHARED ${src})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBXML2_LIBRARIES})
target_include_directories(${PROJECT_NAME} 
    PRIVATE ${LIBXML2_INCLUDE_DIR}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
    TARGETS ${PROJECT_NAME} EXPORT "LibBbmpTargets"
    ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
    INCLUDES DESTINATION $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

export(
    TARGETS ${PROJECT_NAME}
    FILE "${PROJECT_BINARY_DIR}/LibBbmpTargets.cmake")

export(
    PACKAGE LibBbmp
)

configure_file(LibBbmpConfig.cmake.in
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/LibBbmpConfig.cmake" @ONLY)

install(
    FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/LibBbmpConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/LibBbmp/cmake/
)

install(
    EXPORT LibBbmpTargets DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/LibBbmp/cmake/
)
