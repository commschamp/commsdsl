if ((NOT BUILD_TESTING) OR (NOT COMMSDSL_BUILD_COMMSDSL2WIRESHARK_TESTS))
    return ()
endif ()

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

add_custom_target(${APP_NAME}.all_tests
    COMMAND ${CMAKE_COMMAND} -E echo "All ${APP_NAME} tests are built")

include(ExternalProject)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_program(TSHARK_EXECUTABLE tshark)
if (NOT TSHARK_EXECUTABLE)
    message (WARNING "Failed to find tshark, cannot really test wireshark plugins")
endif ()

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
    if (EXISTS "${this_test_dir}/commsdsl2wireshark.params")
        file (READ "${this_test_dir}/commsdsl2wireshark.params" extra_params_str)
        string (REPLACE " " ";" extra_params "${extra_params_str}")
    endif ()

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
        COMMAND $<TARGET_FILE:${APP_NAME}> -s -d --warn-as-err --code-inject-comments ${code_input_param} ${extra_params} -o ${output_dir}.tmp ${schema_files}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/common_test.lua ${output_dir}.tmp/
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}" -DCLEANUP_DIRS="."
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_files} $<TARGET_FILE:${APP_NAME}>)

    if ("${CMAKE_VERSION}" VERSION_GREATER_EQUAL 3.15)
        set_target_properties(${output_tgt} PROPERTIES ADDITIONAL_CLEAN_FILES "${output_dir}.tmp;${output_dir}")
    endif ()

    add_dependencies(${APP_NAME}.all_tests ${output_tgt})

    if (NOT TSHARK_EXECUTABLE)
        continue ()
    endif ()

    if ("${CMAKE_VERSION}" VERSION_GREATER_EQUAL 3.15)
        set_target_properties(${output_tgt} PROPERTIES ADDITIONAL_CLEAN_FILES "${output_dir}.tmp;${output_dir}")
    endif ()

    if (NOT TARGET Python::Interpreter)
        continue ()
    endif ()

    set (pcap_gen_script "${this_test_dir}/${name}.py")
    if (NOT EXISTS "${pcap_gen_script}")
        continue()
    endif ()

    set (pcap_tgt ${APP_NAME}.${name}_pcap_tgt)
    set (pcap_output_dir "${CMAKE_CURRENT_BINARY_DIR}/${name}_pcap")
    set (pcap_output ${pcap_output_dir}/${name}.pcap)
    add_custom_target(${pcap_tgt} ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${pcap_output_dir}
        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}:$ENV{PYTHONPATH}"
            ${Python3_EXECUTABLE} -B ${pcap_gen_script} ${pcap_output}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${output_tgt} ${pcap_gen_script})

    add_dependencies(${APP_NAME}.all_tests ${pcap_tgt})

    add_test(
        NAME ${APP_NAME}.${name}
        COMMAND ${TSHARK_EXECUTABLE} -r ${pcap_output} -X lua_script:common_test.lua -O ${name}
        WORKING_DIRECTORY ${output_dir}
    )
endforeach ()
