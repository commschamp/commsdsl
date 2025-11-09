if ((NOT BUILD_TESTING) OR (NOT COMMSDSL_BUILD_COMMSDSL2SWIG_TESTS))
    return ()
endif ()

set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)
set (languages python java csharp)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(Java REQUIRED COMPONENTS Runtime)

add_custom_target(${APP_NAME}.all_tests
    COMMAND ${CMAKE_COMMAND} -E echo "All ${APP_NAME} tests are built")

include(ExternalProject)

foreach (name ${tests})
    set (test_dir "${tests_path}/${name}")
    file (GLOB schema_files ${test_dir}/Schema*.xml)
    set (this_test_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
    set (code_input_dir "${this_test_dir}/src")
    set (code_input_param)
    if (EXISTS "${code_input_dir}/")
        set (code_input_param -c ${code_input_dir})
    elseif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/common_src)
        set (code_input_param -c ${CMAKE_CURRENT_SOURCE_DIR}/common_src)
    endif()

    set (extra_params)
    if (EXISTS "${this_test_dir}/commsdsl2swig.params")
        file (READ "${this_test_dir}/commsdsl2swig.params" extra_params_str)
        string (REPLACE " " ";" extra_params "${extra_params_str}")
    endif ()

    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
        COMMAND $<TARGET_FILE:${APP_NAME}> -s --warn-as-err ${code_input_param} ${extra_params} -o ${output_dir}.tmp ${schema_files}
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}" -DCLEANUP_DIRS="."
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_files} $<TARGET_FILE:${APP_NAME}>)

    add_custom_command(
        OUTPUT ${output_dir}/${name}.i
        DEPENDS ${output_tgt}
        COMMAND ${CMAKE_COMMAND} -E echo "The swig interface file ${output_dir}/${name}.i is ready"
    )

    set_source_files_properties(
        ${output_dir}/${name}.i
        PROPERTIES
            GENERATED TRUE
            CPLUSPLUS ON
    )

    if ("${CMAKE_VERSION}" VERSION_GREATER_EQUAL 3.15)
        set_target_properties(${output_tgt} PROPERTIES ADDITIONAL_CLEAN_FILES "${output_dir}.tmp;${output_dir}")
    endif ()

    set (build_tgt ${APP_NAME}.${name}_build_tgt)
    set (build_dir ${output_dir}_build)
    set (install_dir ${build_dir}/install)
    set (prot_install_dir "${PROJECT_BINARY_DIR}/app/commsdsl2comms/test/${name}_install")
    set (build_prefix_path ${CMAKE_PREFIX_PATH} ${prot_install_dir})
    string (REPLACE ";" "|" build_prefix_path_str "${build_prefix_path}")
    string (REPLACE ";" "|" languages_str "${languages}")

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}")

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E touch "${output_dir}/dummy.txt")

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
            -DOPT_SWIG_LANGUAGES=${languages_str}
            -DOPT_USE_CCACHE=${COMMSDSL_USE_CCACHE}
            -DOPT_CCACHE_EXECUTABLE=${COMMSDSL_CCACHE_EXECUTABLE}
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E echo "Skipping install"
    )

    add_dependencies(${APP_NAME}.all_tests ${build_tgt})

    foreach (lang ${languages})
        set (update_inc ${CMAKE_CURRENT_SOURCE_DIR}/commsdsl_lang_${lang}.cmake)
        if (EXISTS ${update_inc})
            include (${update_inc})
        endif ()
    endforeach()
endforeach ()
