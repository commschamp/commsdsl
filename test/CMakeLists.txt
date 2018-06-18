function (checkout_external url tag name)
    set (stamp "${CMAKE_CURRENT_BINARY_DIR}/${name}.stamp")
    if ((EXISTS ${stamp}) AND (EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${name}"))
        execute_process (
            COMMAND ${GIT_EXECUTABLE} checkout ${tag}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${name}
        )
    else ()
        execute_process (
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/${name}"
        )

        execute_process (
            COMMAND ${GIT_EXECUTABLE} clone -b ${tag} --depth 1 ${url} ${CMAKE_CURRENT_BINARY_DIR}/${name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RESULT_VARIABLE git_result
        )

        if (NOT "${git_result}" STREQUAL "0")
            message (WARNING "git clone/checkout of ${name} failed")
        else ()
            execute_process (
                COMMAND ${CMAKE_COMMAND} -E touch "${stamp}"
            )
        endif ()
    endif ()
endfunction ()

#################################################################

function (test_func name)
    set (schema_file "${CMAKE_CURRENT_SOURCE_DIR}/${name}/Schema.xml")
    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})
    add_custom_command(
        OUTPUT ${output_dir}.tmp
        DEPENDS ${schema_file} ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
        COMMAND $<TARGET_FILE:${PROJECT_NAME}> -o ${output_dir}.tmp ${schema_file}
    )

    set (output_tgt ${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}"
            -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"
        DEPENDS ${output_dir}.tmp "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${name}/${name}Test.th")

    set (testName "${name}Test")

    set (runner "${name}TestRunner.cpp")

    CXXTEST_ADD_TEST (${testName} ${runner} ${tests})
    add_dependencies(${testName} ${output_tgt})
    target_include_directories (${testName} PRIVATE "${output_dir}/include")

    # TODO: uncomment
#    set (plugin_build_tgt ${name}_plugin_build_tgt)
#    add_custom_target(${plugin_build_tgt} ALL
#        COMMAND ${CMAKE_COMMAND}
#            -DPROJ_DIR=${output_dir}
#            -DCOMMS_INSTALL_DIR="${COMMS_INSTALL_DIR}"
#            -DCONFIG=$<CONFIG>
#            -P "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake"
#        DEPENDS ${output_tgt} ${output_dir}.tmp "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake"
#    )

endfunction ()

#################################################################

function (add_clang_options name)
    set (testName "${name}Test")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options (${testName} PRIVATE ${ARGV})
    endif ()

endfunction ()

#################################################################

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

find_package (Git)
if (NOT GIT_FOUND)
    message (WARNING "Git is required to check out external projects for testing")
    return()
endif ()

checkout_external("https://github.com/CxxTest/cxxtest.git" "4.4" "cxxtest")
set (CXXTEST_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/cxxtest")
set (CXXTEST_PYTHON_TESTGEN_EXECUTABLE "${CXXTEST_INCLUDE_DIR}/bin/cxxtestgen")

find_package (CxxTest)
if (NOT CXXTEST_FOUND)
    message (WARNING "Wrong cxxtest paths, must be fixed, cannot do testing...")
    return()
endif ()

include_directories ("${CXXTEST_INCLUDE_DIR}")

test_func (test1)
test_func (test2)
test_func (test3)
test_func (test4)
test_func (test5)
test_func (test6)
test_func (test7)

#add_clang_options (test11 "-Wno-c++11-narrowing")

FILE(GLOB_RECURSE all_test_headers "*.h")
add_custom_target(all_test_headers_tgt SOURCES ${all_test_headers})
