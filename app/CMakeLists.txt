set (ALL_APPS_TGT "commsdsl.all_apps")
add_custom_target(${ALL_APPS_TGT}
    COMMAND ${CMAKE_COMMAND} -E echo "All applications are built")

add_subdirectory (commsdsl2comms)
add_subdirectory (commsdsl2emscripten)
add_subdirectory (commsdsl2latex)
add_subdirectory (commsdsl2swig)
add_subdirectory (commsdsl2test)
add_subdirectory (commsdsl2tools_qt)

