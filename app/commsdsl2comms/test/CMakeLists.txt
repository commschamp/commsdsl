#################################################################

function (test_func name)
    set (test_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
    file (GLOB schema_files ${test_dir}/Schema*.xml)
    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})
    set (code_input_dir "${test_dir}/src")
    set (code_input_param)
    if (EXISTS "${code_input_dir}/")
        set (code_input_param -c ${code_input_dir})
    endif()

    set (extra_bundle_param)
    if (EXISTS "${test_dir}/extra")
        file(GLOB extra_bundle_files ${test_dir}/extra/*)
        string (REPLACE ";" "," extra_bundle_param_value "${extra_bundle_files}")
        set (extra_bundle_param --extra-messages-bundle "${extra_bundle_param_value}")
    endif()    

    set (rm_tmp_tgt ${APP_NAME}.${name}_rm_tmp_tgt)
    add_custom_target(${rm_tmp_tgt}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
    )

    add_custom_command(
        OUTPUT ${output_dir}.tmp
        DEPENDS ${schema_files} ${APP_NAME} ${rm_tmp_tgt}
        COMMAND $<TARGET_FILE:${APP_NAME}> -d -s --warn-as-err -o ${output_dir}.tmp ${code_input_param} ${extra_bundle_param} ${schema_files}
    )

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}" -DCLEANUP_DIRS="doc;include" 
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS ${output_dir}.tmp "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_files} ${APP_NAME})


    set (build_tgt ${APP_NAME}.${name}_build_tgt)
    set (build_dir ${output_dir}/build)
    set (install_dir ${build_dir}/install)

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
        LIST_SEPARATOR |
        CMAKE_ARGS 
            -DCMAKE_C_COMPILER=${COMMSDSL_TESTS_C_COMPILER} -DCMAKE_CXX_COMPILER=${COMMSDSL_TESTS_CXX_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
            -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
            -DCMAKE_INSTALL_PREFIX=${install_dir}
            -DOPT_REQUIRE_COMMS_LIB=OFF
    )          

    set (tests "${test_dir}/${name}Test.th")

    set (testName "${APP_NAME}.${name}Test")

    set (valgrand_opt)
    if (COMMSDSL_VALGRIND_TESTS AND VALGRIND_EXECUTABLE)
        set (valgrand_opt VALGRIND_EXECUTABLE ${VALGRIND_EXECUTABLE})
    endif ()

    cc_cxxtest_add_test(NAME ${testName} SRC ${tests} ${valgrand_opt})

    add_dependencies(${testName} ${build_tgt})
    target_include_directories (${testName} PRIVATE "${install_dir}/include")

    target_compile_options(${testName} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -ftemplate-depth=2048 ${COMMSDSL_SANITIZER_OPTS}>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-old-style-cast -ftemplate-depth=2048 ${COMMSDSL_SANITIZER_OPTS}>
    )    

    if (NOT "${COMMSDSL_SANITIZER_OPTS}" STREQUAL "")
        target_link_options(${testName} PRIVATE
            $<$<CXX_COMPILER_ID:GNU>:${COMMSDSL_SANITIZER_OPTS}>
            $<$<CXX_COMPILER_ID:Clang>:${COMMSDSL_SANITIZER_OPTS}>
        ) 
    endif ()    

    if (COMMSDSL_TEST_BUILD_DOC AND DOXYGEN_FOUND)
        add_custom_target(${APP_NAME}.doc_${name} ALL
            COMMAND COMMAND ${CMAKE_COMMAND} --build ${build_dir} --target doc_${name}
            DEPENDS ${build_tgt} 
        )
    endif ()
        
endfunction ()

#################################################################

if (NOT BUILD_TESTING)
    return ()
endif () 

find_package(LibComms REQUIRED)

set (COMMSDSL_SANITIZER_OPTS)
if (COMMSDSL_TEST_USE_SANITIZERS AND
    ((CMAKE_COMPILER_IS_GNUCC) OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")))
    set (COMMSDSL_SANITIZER_OPTS 
        -fno-omit-frame-pointer 
        -fno-sanitize-recover=address 
        -fsanitize=address
        -fno-sanitize-recover=undefined
        -fsanitize=undefined)
endif ()

if ((NOT "${COMMSDSL_SANITIZER_OPTS}" STREQUAL "") AND (${CMAKE_VERSION} VERSION_LESS "3.13"))
    message (WARNING "The CMake version is too old, expected at least 3.13, sanitizers are disabled")
    set (COMMSDSL_SANITIZER_OPTS)
endif ()

if ((NOT "${COMMSDSL_SANITIZER_OPTS}" STREQUAL "") AND (NOT "${VALGRIND_EXECUTABLE}" STREQUAL ""))
    message (WARNING "Cannot run valgrind when tests compiled with sanitizers")
    set (VALGRIND_EXECUTABLE) # Disable valgrind tests
endif ()

find_package (Doxygen)

if (CMAKE_COMPILER_IS_GNUCC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "7.9"))
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers")
endif ()

include(ExternalProject)

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
test_func (test47)
test_func (test48)
test_func (test49)
test_func (test50)