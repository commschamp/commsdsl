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
    set (testDir "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
    set (tests "${testDir}/${name}Test.th")
    set (testName "${name}Test")
    set (runner "${name}TestRunner.cpp")

    CXXTEST_ADD_TEST (${testName} ${runner} ${tests})
    target_compile_definitions (${testName} PRIVATE "-DSCHEMAS_DIR=\"${testDir}\"")
    target_link_libraries(${testName} ${COMMON_TEST_LIB_NAME} ${PROJECT_NAME})

endfunction ()

#################################################################

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

include_directories (
    "${CXXTEST_INCLUDE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

set (COMMON_TEST_LIB_NAME "CommonTest")
set (common_test_src
    "CommonTestSuite.cpp"
)

add_library(${COMMON_TEST_LIB_NAME} STATIC ${common_test_src})
target_link_libraries(${COMMON_TEST_LIB_NAME} PRIVATE ${PROJECT_NAME})

test_func (int)
test_func (float)
test_func (enum)
test_func (set)
test_func (bitfield)
test_func (bundle)
test_func (string)
test_func (data)
test_func (list)
test_func (ref)
test_func (optional)
test_func (message)
