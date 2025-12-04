
function (test_func name)
    set (testDir "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
    set (tests "${testDir}/${name}Test.th")
    set (testName "libcommsdsl.${name}Test")

    set (valgrand_opt)
    if (COMMSDSL_VALGRIND_TESTS AND VALGRIND_EXECUTABLE)
        set (valgrand_opt VALGRIND_EXECUTABLE ${VALGRIND_EXECUTABLE})
    endif ()
    cc_cxxtest_add_test(NAME ${testName} SRC ${tests} ${valgrand_opt} NO_COMMS_LIB_DEP)

    target_compile_definitions (${testName} PRIVATE "-DSCHEMAS_DIR=\"${testDir}\"")
    target_link_libraries(${testName} PRIVATE ${COMMON_TEST_LIB_NAME} ${PROJECT_NAME})

    target_compile_options(${testName} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>: /wd4251>
    )

    add_dependencies(libcommsdsl.all_tests ${testName})

endfunction ()

#################################################################

if (NOT BUILD_TESTING)
    return ()
endif ()

include_directories (
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

set (COMMON_TEST_LIB_NAME "CommonTest")
set (common_test_src
    "CommonTestSuite.cpp"
)

if (CMAKE_COMPILER_IS_GNUCC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "7.9"))
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers")
endif ()

add_library(${COMMON_TEST_LIB_NAME} STATIC ${common_test_src})
target_link_libraries(${COMMON_TEST_LIB_NAME} PRIVATE ${PROJECT_NAME} cxxtest::cxxtest)

target_compile_options(${COMMON_TEST_LIB_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>: /wd4251>
)

add_custom_target(libcommsdsl.all_tests
    COMMAND ${CMAKE_COMMAND} -E echo "All libcommsdsl tests are built")

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
test_func (variant)
test_func (message)
test_func (schema)
test_func (namespace)
test_func (interface)
test_func (frame)
test_func (alias)
