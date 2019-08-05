get_filename_component(LIBCOMMSDSL_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${LIBCOMMSDSL_CMAKE_DIR}/LibCommsdslTargets.cmake")
if (TARGET commsdsl::commsdsl)
    set (LIBCOMMSDSL_FOUND TRUE)
    get_target_property (LIBCOMMSDSL_INCLUDE_DIRS commsdsl::commsdsl INTERFACE_INCLUDE_DIRECTORIES)    
    get_target_property (LIBCOMMSDSL_LIBRARIES commsdsl::commsdsl IMPORTED_LOCATION_NOCONFIG)
endif ()


