if (NOT BUILD_TESTING)
    return ()
endif () 

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2new/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2new/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

commsdsl_ensure_comms_target ()

foreach (name ${tests})
    set (test_dir "${tests_path}/${name}")
    set (schema_file "${test_dir}/Schema.xml")
    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})

    set (rm_tmp_tgt ${APP_NAME}.${name}_rm_tmp_tgt)
    add_custom_target(${rm_tmp_tgt}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
    )

    add_custom_command(
        OUTPUT ${output_dir}.tmp
        DEPENDS ${schema_file} ${APP_NAME} ${rm_tmp_tgt}
        COMMAND $<TARGET_FILE:${APP_NAME}> --warn-as-err -o ${output_dir}.tmp ${schema_file}
    )

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}"
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS ${output_dir}.tmp "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_file} ${APP_NAME})


#    set (build_tgt ${APP_NAME}.${name}_build_tgt)
#    set (build_dir ${output_dir}/build)
#    set (install_dir ${build_dir}/install)

#    execute_process (
#        COMMAND ${CMAKE_COMMAND} -E make_directory "${build_dir}")

#    set (dep_install_dir ${dep_prefix_path}/${name}/build/install)
#    add_custom_target(${build_tgt} ALL
#        COMMAND ${CMAKE_COMMAND} -E make_directory "${build_dir}"
#        COMMAND 
#            ${CMAKE_COMMAND} 
#                ${output_dir}
#                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
#                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
#                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
#                -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
#                -DCMAKE_INSTALL_PREFIX=${install_dir}
#                -DCMAKE_PREFIX_PATH="${LibComms_DIR}\;${dep_install_dir}"
#        COMMAND
#            ${CMAKE_COMMAND} --build ${build_dir} --config ${CMAKE_BUILD_TYPE} --target install
#        WORKING_DIRECTORY ${build_dir}
#        DEPENDS ${output_tgt} commsdsl2new.${name}_build_tgt)
endforeach ()
