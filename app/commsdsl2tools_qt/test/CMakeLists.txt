if ((NOT BUILD_TESTING) OR (NOT COMMSDSL_BUILD_COMMSDSL2TOOLS_QT_TESTS))
    return ()
endif ()

find_package(LibComms REQUIRED)

if ("${LibComms_DIR}" STREQUAL "")
    message (FATAL_ERROR "The comms library found but the directory is not known")
endif ()

find_package(cc_tools_qt REQUIRED)

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

add_custom_target(${APP_NAME}.all_tests
    COMMAND ${CMAKE_COMMAND} -E echo "All ${APP_NAME} tests are built")

include(ExternalProject)

foreach (name ${tests})
    set (test_dir "${tests_path}/${name}")
    file (GLOB schema_files ${test_dir}/Schema*.xml)
    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})

    set (this_test_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
    set (code_input_dir "${this_test_dir}/src")
    set (code_input_param)
    if (EXISTS "${code_input_dir}/")
        set (code_input_param -c ${code_input_dir})
    endif()

    set (extra_params)
    if (EXISTS "${this_test_dir}/commsdsl2tools_qt.params")
        file (READ "${this_test_dir}/commsdsl2tools_qt.params" extra_params_str)
        string (REPLACE " " ";" extra_params "${extra_params_str}")
    endif ()

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
        COMMAND $<TARGET_FILE:${APP_NAME}> -s --warn-as-err ${code_input_param} ${extra_params} -o ${output_dir}.tmp ${schema_files}
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}" -DCLEANUP_DIRS="."
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_files} $<TARGET_FILE:${APP_NAME}>)

    set (build_tgt ${APP_NAME}.${name}_build_tgt)
    set (build_dir ${output_dir}_build)
    set (install_dir ${build_dir}/install)

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${build_dir}")

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}")

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E touch "${output_dir}/dummy.txt")

    set (prot_tgt "commsdsl2comms.${name}_build_tgt")
    set (prot_install_dir "${PROJECT_BINARY_DIR}/app/commsdsl2comms/test/${name}_install")
    set (build_prefix_path ${CMAKE_PREFIX_PATH} ${prot_install_dir})
    string (REPLACE ";" "|" build_prefix_path_str "${build_prefix_path}")

    ExternalProject_Add(
        ${build_tgt}
        BUILD_ALWAYS TRUE
        DEPENDS ${output_tgt} commsdsl2comms.${name}_build_tgt
        SOURCE_DIR ${output_dir}
        BINARY_DIR ${build_dir}
        INSTALL_DIR ${install_dir}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM}
        CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
        LIST_SEPARATOR |
        CMAKE_ARGS
            -DCMAKE_C_COMPILER=${COMMSDSL_TESTS_C_COMPILER} -DCMAKE_CXX_COMPILER=${COMMSDSL_TESTS_CXX_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
            -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
            -DCMAKE_INSTALL_PREFIX=${install_dir}
            -DCMAKE_PREFIX_PATH=${build_prefix_path_str}
            -DOPT_WARN_AS_ERR=ON
            -DOPT_USE_CCACHE=${COMMSDSL_USE_CCACHE}
            -DOPT_CCACHE_EXECUTABLE=${COMMSDSL_CCACHE_EXECUTABLE}
            -DOPT_QT_MAJOR_VERSION=${COMMSDSL_TESTS_QT_VERSION}
    )

    add_dependencies(${APP_NAME}.all_tests ${build_tgt})
endforeach ()
