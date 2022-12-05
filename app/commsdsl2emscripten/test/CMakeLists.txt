if (NOT BUILD_TESTING)
    return ()
endif () 

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

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

    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})

    set (rm_tmp_tgt ${APP_NAME}.${name}_rm_tmp_tgt)
    add_custom_target(${rm_tmp_tgt}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
    )

    add_custom_command(
        OUTPUT ${output_dir}.tmp
        DEPENDS ${schema_files} ${APP_NAME} ${rm_tmp_tgt}
        COMMAND $<TARGET_FILE:${APP_NAME}> -s --warn-as-err ${code_input_param} -o ${output_dir}.tmp ${schema_files}
    )

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}" -DCLEANUP_DIRS="."
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS ${output_dir}.tmp "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_files} ${APP_NAME})

    add_dependencies(${APP_NAME}.all_tests ${output_tgt})         

endforeach ()
