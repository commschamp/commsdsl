if ((NOT BUILD_TESTING) OR (NOT COMMSDSL_BUILD_COMMSDSL2LATEX_TESTS))
    return ()
endif ()

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

find_package(LibComms REQUIRED)

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
    if (EXISTS "${this_test_dir}/commsdsl2latex.params")
        file (READ "${this_test_dir}/commsdsl2latex.params" extra_params_str)
        string (REPLACE " " ";" extra_params "${extra_params_str}")
    endif ()

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
        COMMAND $<TARGET_FILE:${APP_NAME}> -s --warn-as-err --code-inject-comments ${code_input_param} ${extra_params} -o ${output_dir}.tmp ${schema_files}
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}" -DCLEANUP_DIRS="."
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_files} $<TARGET_FILE:${APP_NAME}>)

    if ("${CMAKE_VERSION}" VERSION_GREATER_EQUAL 3.15)
        set_target_properties(${output_tgt} PROPERTIES ADDITIONAL_CLEAN_FILES "${output_dir}.tmp;${output_dir}")
    endif ()

    set (build_tgt ${APP_NAME}.${name}_build_tgt)
    set (build_dir ${output_dir}_build)
    set (install_dir ${output_dir}_install)

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${build_dir}")

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}")

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E touch "${output_dir}/dummy.txt")

    ExternalProject_Add(
        ${build_tgt}
        BUILD_ALWAYS TRUE
        DEPENDS ${output_tgt}
        SOURCE_DIR ${output_dir}
        BINARY_DIR ${build_dir}
        INSTALL_DIR ${install_dir}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM}
        CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
        LIST_SEPARATOR |
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${install_dir}
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
            -DOPT_WARN_AS_ERR=ON
        BUILD_COMMAND
            ${CMAKE_COMMAND} --build ${build_dir} --config ${CMAKE_BUILD_TYPE} --target all_artifacts
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E echo "No install for documentation build"
    )

    add_dependencies(${APP_NAME}.all_tests ${build_tgt})

endforeach ()
