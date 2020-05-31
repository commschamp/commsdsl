set (CC_EXTERNAL_TGT "comms_champion_external")
set (CC_EXTERNAL)

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

    set (runner "${APP_NAME}.${name}TestRunner.cpp")

    CXXTEST_ADD_TEST (${testName} ${runner} ${tests})
    add_dependencies(${testName} ${output_tgt})
    target_include_directories (${testName} PRIVATE "${output_dir}/include")
    target_link_libraries(${testName} cc::comms)

    if (CC_EXTERNAL)
        add_dependencies(${testName} ${CC_EXTERNAL_TGT})
    endif ()
    
    if (COMMSDSL_VALGRIND_TESTS AND VALGRIND_EXECUTABLE)
        add_test(NAME "${testName}_valgrind"
             COMMAND ${CMAKE_COMMAND}
                -DTEST_EXECUTABLE=$<TARGET_FILE:${testName}>
                -DVALGRIND_EXECUTABLE=${VALGRIND_EXECUTABLE}
                -P ${PROJECT_SOURCE_DIR}/cmake/ScriptRunUnitTestWithValgrind.cmake)
    endif ()

    while (TRUE)
        if (NOT CC_COMMS_CHAMPION_FOUND)
            message (WARNING "cc::comms_champion is not available")
            break ()
        endif ()

        if (NOT TARGET cc::comms_champion)
            message (FATAL_ERROR "cc::comms_champion target is not defined")
        endif () 

        set (plugin_build_tgt ${APP_NAME}.${name}_plugin_build_tgt)
        add_custom_target(${plugin_build_tgt} ALL
            COMMAND ${CMAKE_COMMAND}
                -DPROJ_DIR=${output_dir} -DPATH_ENV="$ENV{PATH}"
                -DCOMMS_INSTALL_DIR="${COMMS_INSTALL_DIR}"
                -DOPT_QT_DIR=${COMMSDSL_QT_DIR}
                ${COMPILER_OPTIONS}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
                -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
                -P "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake"
            DEPENDS ${output_tgt} ${output_dir}.tmp "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake" ${testName}
        )

        if (CC_EXTERNAL)
            add_dependencies(${plugin_build_tgt} ${CC_EXTERNAL_TGT})
        endif ()
        
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

macro (cc_external)
    set (CC_EXTERNAL TRUE)
    set (cc_main_dir "${CMAKE_BINARY_DIR}/comms_champion")
    set (cc_src_dir "${EXTERNALS_DIR}/comms_champion")
    set (cc_bin_dir "${cc_main_dir}/build")
    set (COMMS_INSTALL_DIR "${cc_main_dir}/install")
    set (cc_qt_dir_opt)
    set (cc_repo "https://github.com/arobenko/comms_champion.git")

    find_package (Git REQUIRED)
    if (EXISTS "${cc_src_dir}/.git")
        execute_process (
            COMMAND ${GIT_EXECUTABLE} pull 
            WORKING_DIRECTORY ${cc_src_dir}
        )
    
        execute_process (
            COMMAND ${GIT_EXECUTABLE} checkout ${CC_TAG}
            WORKING_DIRECTORY ${cc_src_dir}
        )
    else ()
        execute_process (
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${cc_src_dir}"
        )
        
        execute_process (
            COMMAND ${CMAKE_COMMAND} -E make_directory "${cc_src_dir}"
        )        
        
        execute_process (
            COMMAND 
                ${GIT_EXECUTABLE} clone -b ${CC_TAG} --depth 1 ${cc_repo} ${cc_src_dir}
            RESULT_VARIABLE git_result
        )

        if (NOT "${git_result}" STREQUAL "0")
            message (WARNING "git clone/checkout failed")
        endif ()
    endif ()

    set (EXT_CC_INSTALL_DIR ${COMMS_INSTALL_DIR})
    include (${cc_src_dir}/cmake/DefineExternalProjectTargets.cmake)

    include(ExternalProject)
    if (NOT "${COMMSDSL_QT_DIR}" STREQUAL "")
        set (cc_qt_dir_opt -DCC_QT_DIR=${COMMSDSL_QT_DIR})
    endif ()

    set (cc_libs_only_opt "-DCC_LIBS_ONLY=ON")

    ExternalProject_Add(
        "${CC_EXTERNAL_TGT}"
        PREFIX "${cc_main_dir}"
        STAMP_DIR "${cc_main_dir}"
        GIT_REPOSITORY "${cc_repo}"
        GIT_TAG "${CC_TAG}"
        SOURCE_DIR "${cc_src_dir}"
        BINARY_DIR "${cc_bin_dir}"
        INSTALL_DIR "${COMMS_INSTALL_DIR}"
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${COMMS_INSTALL_DIR}
            -DCC_NO_UNIT_TESTS=ON ${cc_libs_only_opt} -DCC_NO_WARN_AS_ERR=ON
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} ${COMPILER_OPTIONS}
            -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
            ${cc_qt_dir_opt}
    )
endmacro()

#################################################################


if (NOT CXXTEST_FOUND)
    return()
endif ()

if ("${COMMS_INSTALL_DIR}" STREQUAL "")
    cc_external ()
else ()
    list (APPEND CMAKE_PREFIX_PATH "${COMMS_INSTALL_DIR}")
    find_package(CommsChampion NO_MODULE)
endif ()

if (NOT CC_COMMS_FOUND)
    message (WARNING "Path ${COMMS_INSTALL_DIR} doesn't contain COMMS library headers, unittesting is skipped")
    return()
endif ()

if (NOT TARGET cc::comms)
    message (FATAL_ERROR "cc::comms target is not defined")
endif () 

include_directories ("${CXXTEST_INCLUDE_DIR}")

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


