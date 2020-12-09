
#################################################################

function (test_func name)
    set (schema_file "${CMAKE_CURRENT_SOURCE_DIR}/${name}/Schema.xml")
    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})
    set (code_input_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}/src")
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

    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${name}/${name}Test.th")

    set (testName "${APP_NAME}.${name}Test")

    set (valgrand_opt)
    if (COMMSDSL_VALGRIND_TESTS AND VALGRIND_EXECUTABLE)
        set (valgrand_opt VALGRIND_EXECUTABLE ${VALGRIND_EXECUTABLE})
    endif ()

    cc_cxxtest_add_test(NAME ${testName} SRC ${tests} ${valgrand_opt})

    add_dependencies(${testName} ${output_tgt})
    target_include_directories (${testName} PRIVATE "${output_dir}/include")

    target_compile_options(${testName} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -ftemplate-depth=2048 ${COMMSDSL_SANITIZER_OPTS}>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-old-style-cast -ftemplate-depth=2048 ${COMMSDSL_SANITIZER_OPTS}>
    )    

    if (NOT "${COMMSDSL_SANITIZER_OPTS}" STREQUAL "")
        target_link_options(${testName} PRIVATE
            $<$<CXX_COMPILER_ID:GNU>:${COMMSDSL_SANITIZER_OPTS}>
            $<$<CXX_COMPILER_ID:Clang>:${COMMSDSL_SANITIZER_OPTS}>
        ) 
    endif ()    

    while (TRUE)
        if (NOT COMMSDSL_TEST_BUILD_CC_PLUGIN)
            message (STATUS "Skipping build of ${APP_NAME}.${name}_plugin_build_tgt")
            break ()
        endif ()
        
        if (NOT TARGET cc::comms_champion)
            message (FATAL_ERROR "cc::comms_champion target is not defined")
        endif () 
        
        set (plugin_build_tgt ${APP_NAME}.${name}_plugin_build_tgt)
        add_custom_target(${plugin_build_tgt} ALL
            COMMAND ${CMAKE_COMMAND}
                -DPROJ_DIR=${output_dir} 
                -DCOMMS_INSTALL_DIR="${COMMS_INSTALL_DIR}"
                -DOPT_QT_DIR=${COMMSDSL_QT_DIR}
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
                -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
                -P "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake"
            DEPENDS ${output_tgt} ${output_dir}.tmp "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake" ${testName}
        )

        if (DOXYGEN_FOUND)
            add_custom_target(doc_${name}
                COMMAND COMMAND ${CMAKE_COMMAND} --build ${output_dir}/build --target doc_${name}
                DEPENDS ${plugin_build_tgt} 
            )
        endif ()
        
        break()
    endwhile()

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

if ("${COMMS_INSTALL_DIR}" STREQUAL "")
    if (("${CC_CMAKE_DIR}" STREQUAL "") OR (NOT EXISTS "${CC_CMAKE_DIR}/CC_External.cmake"))
        message (FATAL_ERROR "Missing CC_External.cmake script, should be provided by the comms_champipon project")
    endif ()

    include (${CC_CMAKE_DIR}/CC_External.cmake)

    set (qt_dir_opt)
    if (NOT "${COMMSDSL_QT_DIR}" STREQUAL "")
        set (qt_dir_opt QT_DIR ${COMMSDSL_QT_DIR})
    endif ()    

    set (COMMS_BUILD_DIR "${CMAKE_BINARY_DIR}/comms_champion")
    set (COMMS_INSTALL_DIR "${COMMS_BUILD_DIR}/install")
    cc_build_as_external_project(
        SRC_DIR ${CC_SRC_DIR} 
        BUILD_DIR ${COMMS_BUILD_DIR}/comms_champion
        INSTALL_DIR ${COMMS_INSTALL_DIR}
        TAG ${CC_TAG} 
        ${qt_dir_opt}
        NO_SYSTEM_HEADERS
        CMAKE_ARGS 
            -DCC_NO_UNIT_TESTS=ON
            -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
            -DCC_NO_WARN_AS_ERR=ON -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCC_LIBS_ONLY=ON
    )
    
else ()
    list (APPEND CMAKE_PREFIX_PATH "${COMMS_INSTALL_DIR}")
    find_package(CommsChampion NO_MODULE)
endif ()

if (NOT TARGET cc::comms)
    message (FATAL_ERROR "cc::comms target is not defined")
endif () 

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

