find_package(Boost REQUIRED
    COMPONENTS program_options filesystem)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

if (NOT "${LIBBBMP_INSTALL_DIR}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH ${LIBBBMP_INSTALL_DIR})
endif ()

find_package(LibBbmp REQUIRED NO_MODULE)

set (
    src
    "main.cpp"
    "ProgramOptions.cpp"
)

add_executable(${PROJECT_NAME} ${src})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} bbmp)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} Setupapi.lib Ws2_32.lib imm32.lib winmm.lib)
endif ()

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${BBMP2COMMS_INSTALL_DIR}/bin
)

FILE(GLOB_RECURSE all_headers "*.h")
add_custom_target(all_headers_tgt SOURCES ${all_headers})
