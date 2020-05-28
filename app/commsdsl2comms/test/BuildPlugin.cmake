# PROJ_DIR
# COMMS_INSTALL_DIR
# OPT_QT_DIR
# CMAKE_C_COMPILER
# CMAKE_CXX_COMPILER
# CMAKE_BUILD_TYPE
# CMAKE_CXX_STANDARD
# PATH_ENV

if (NOT "${PATH_ENV}" STREQUAL "")
    message (STATUS "Updating environment PATH=${PATH_ENV}")
    set (ENV{PATH} "${PATH_ENV}")
endif ()

set (build_dir "${PROJ_DIR}/build")
file (MAKE_DIRECTORY ${build_dir})
message (STATUS "Compiling with CMAKE_C_COMPILER=${CMAKE_C_COMPILER} and CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
execute_process(
    COMMAND ${CMAKE_COMMAND} 
        -DOPT_CC_MAIN_INSTALL_DIR=${COMMS_INSTALL_DIR} 
        -DOPT_BUILD_TEST=ON -DOPT_BUILD_PLUGIN=ON
        -DOPT_QT_DIR=${OPT_QT_DIR} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} 
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} -DCMAKE_INSTALL_PREFIX=${build_dir}/install
        ${PROJ_DIR}
    WORKING_DIRECTORY ${build_dir}
    RESULT_VARIABLE cmake_result
)

if (NOT ${cmake_result} EQUAL 0)
    message (FATAL_ERROR "CMake has failed in ${PROJ_DIR}")
endif ()

execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${build_dir} --target install
    WORKING_DIRECTORY ${build_dir}
    RESULT_VARIABLE cmake_result
)

if (NOT ${cmake_result} EQUAL 0)
    message (FATAL_ERROR "Build has failed in ${PROJ_DIR}")
endif ()

