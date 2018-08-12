# PROJ_DIR
# COMMS_INSTALL_DIR
# OPT_QT_DIR
# CONFIG

set (build_dir "${PROJ_DIR}/build")
file (MAKE_DIRECTORY ${build_dir})
execute_process(
    COMMAND ${CMAKE_COMMAND} -DOPT_FULL_SOLUTION=OFF -DOPT_CC_MAIN_INSTALL_DIR=${COMMS_INSTALL_DIR} -DOPT_QT_DIR=${OPT_QT_DIR} ${PROJ_DIR}
    WORKING_DIRECTORY ${build_dir}
    RESULT_VARIABLE cmake_result
)

if (NOT ${cmake_result} EQUAL 0)
    message (FATAL_ERROR "CMake has failed in ${PROJ_DIR}")
endif ()

execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${build_dir} --target install --config ${CONFIG}
    WORKING_DIRECTORY ${build_dir}
    RESULT_VARIABLE cmake_result
)

if (NOT ${cmake_result} EQUAL 0)
    message (FATAL_ERROR "Build has failed in ${PROJ_DIR}")
endif ()

