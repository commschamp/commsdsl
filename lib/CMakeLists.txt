
install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/commsdsl
    DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}
)

add_subdirectory(src)
add_subdirectory(test)

FILE(GLOB_RECURSE libcommsdsl.all_headers "*.h")
add_custom_target(libcommsdsl.all_headers_tgt SOURCES ${libcommsdsl.all_headers})
