
install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/commsdsl
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

file (READ "${CMAKE_CURRENT_SOURCE_DIR}/include/commsdsl/version.h" version_file)
string (REGEX MATCH "COMMSDSL_MAJOR_VERSION ([0-9]*)U*" _ ${version_file})
set (major_ver ${CMAKE_MATCH_1})
string (REGEX MATCH "COMMSDSL_MINOR_VERSION ([0-9]*)U*" _ ${version_file})
set (minor_ver ${CMAKE_MATCH_1})
string (REGEX MATCH "COMMSDSL_PATCH_VERSION ([0-9]*)U*" _ ${version_file})
set (patch_ver ${CMAKE_MATCH_1})
set (COMMSDSL_VERSION "${major_ver}.${minor_ver}.${patch_ver}")


add_subdirectory(src)
add_subdirectory(test)
