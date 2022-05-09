
#################################################################

function (test_func name)
    set (test_dir "${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test/${name}")
    set (schema_file "${test_dir}/Schema.xml")
    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})
    set (code_input_dir "${test_dir}/src")
    set (code_input_param)
    if (EXISTS "${code_input_dir}/")
        set (code_input_param "-c${code_input_dir}")
    endif()

    set (rm_tmp_tgt ${APP_NAME}.${name}_rm_tmp_tgt)
    add_custom_target(${rm_tmp_tgt}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
    )

    add_custom_command(
        OUTPUT ${output_dir}.tmp
        DEPENDS ${schema_file} ${APP_NAME} ${rm_tmp_tgt}
        COMMAND $<TARGET_FILE:${APP_NAME}> --warn-as-err -o ${output_dir}.tmp "${code_input_param}" ${schema_file}
    )

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}"
            -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"
        DEPENDS ${output_dir}.tmp "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file} ${APP_NAME})

endfunction ()

#################################################################

function (add_clang_options name)
    set (testName "${name}Test")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options (${testName} PRIVATE ${ARGV})
    endif ()

endfunction ()

#################################################################

if (NOT BUILD_TESTING)
    return ()
endif () 

test_func (test1)
test_func (test2)
test_func (test3)
test_func (test4)
test_func (test5)
test_func (test6)
test_func (test7)
test_func (test8)
test_func (test9)
test_func (test10)
test_func (test11)
test_func (test12)
test_func (test13)
test_func (test14)
test_func (test15)
test_func (test16)
test_func (test17)
test_func (test18)
test_func (test19)
test_func (test20)
test_func (test21)
test_func (test22)
test_func (test23)
test_func (test24)
test_func (test25)
test_func (test26)
test_func (test27)
test_func (test28)
test_func (test29)
test_func (test30)
test_func (test31)
test_func (test32)
test_func (test33)
test_func (test34)
test_func (test35)
test_func (test36)
test_func (test37)
test_func (test38)
test_func (test39)
test_func (test40)
test_func (test41)
test_func (test42)
test_func (test43)
test_func (test44)
test_func (test45)
test_func (test46)
