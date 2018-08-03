
function (test_func name)
    set (schema_file "${CMAKE_CURRENT_SOURCE_DIR}/${name}/Schema.xml")
    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})
    set (code_input_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}/src")
    set (code_input_param)
    if (EXISTS "${code_input_dir}/")
        set (code_input_param "-c${code_input_dir}")
    endif()

    add_custom_command(
        OUTPUT ${output_dir}.tmp
        DEPENDS ${schema_file} ${APP_NAME}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
        COMMAND $<TARGET_FILE:${APP_NAME}> --warn-as-err -o ${output_dir}.tmp "${code_input_param}" ${schema_file}
    )

    set (output_tgt ${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}"
            -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"
        DEPENDS ${output_dir}.tmp "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${name}/${name}Test.th")

    set (testName "${APP_NAME}.${name}Test")

    set (runner "${APP_NAME}.${name}TestRunner.cpp")

    CXXTEST_ADD_TEST (${testName} ${runner} ${tests})
    add_dependencies(${testName} ${output_tgt})
    target_include_directories (${testName} PRIVATE "${output_dir}/include")
    
    if (COMMSDSL_VALGRIND_TESTS AND VALGRIND_EXECUTABLE)
        add_test(NAME "${testName}_valgrind"
             COMMAND ${CMAKE_COMMAND}
                -DTEST_EXECUTABLE=$<TARGET_FILE:${testName}>
                -DVALGRIND_EXECUTABLE=${VALGRIND_EXECUTABLE}
                -P ${CMAKE_SOURCE_DIR}/cmake/ScriptRunUnitTestWithValgrind.cmake)
    endif ()    

    set (plugin_build_tgt ${APP_NAME}.${name}_plugin_build_tgt)
    add_custom_target(${plugin_build_tgt} ALL
        COMMAND ${CMAKE_COMMAND}
            -DPROJ_DIR=${output_dir}
            -DCOMMS_INSTALL_DIR="${COMMS_INSTALL_DIR}"
            -DCONFIG=$<CONFIG>
            -P "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake"
        DEPENDS ${output_tgt} ${output_dir}.tmp "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake"
    )

endfunction ()

#################################################################

function (add_clang_options name)
    set (testName "${name}Test")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options (${testName} PRIVATE ${ARGV})
    endif ()

endfunction ()

#################################################################

if (NOT CXXTEST_FOUND)
    return()
endif ()

if ("${COMMS_INSTALL_DIR}" STREQUAL "")
    message (WARNING "Path to COMMS library installation hasn't been provided, unittesting is skipped. Please use COMMS_INSTALL_DIR variable.")
    return ()
endif ()

list (APPEND CMAKE_PREFIX_PATH "${COMMS_INSTALL_DIR}")
find_package(CommsChampion NO_MODULE)
if (NOT CC_COMMS_FOUND)
    message (WARNING "Path ${COMMS_INSTALL_DIR} doesn't contain COMMS library headers, unittesting is skipped")
    return()
endif ()
include_directories(${CC_INCLUDE_DIRS})

include_directories ("${CXXTEST_INCLUDE_DIR}")

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

#add_clang_options (test11 "-Wno-c++11-narrowing")
