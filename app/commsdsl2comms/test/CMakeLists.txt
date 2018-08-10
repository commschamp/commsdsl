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
        DEPENDS ${output_dir}.tmp "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file} ${APP_NAME})

    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${name}/${name}Test.th")

    set (testName "${APP_NAME}.${name}Test")

    set (runner "${APP_NAME}.${name}TestRunner.cpp")

    CXXTEST_ADD_TEST (${testName} ${runner} ${tests})
    add_dependencies(${testName} ${output_tgt})
    target_include_directories (${testName} PRIVATE "${output_dir}/include")

    if (CC_EXTERNAL)
        add_dependencies(${testName} ${CC_EXTERNAL_TGT})
    endif ()
    
#    if (COMMSDSL_VALGRIND_TESTS AND VALGRIND_EXECUTABLE)
#        add_test(NAME "${testName}_valgrind"
#             COMMAND ${CMAKE_COMMAND}
#                -DTEST_EXECUTABLE=$<TARGET_FILE:${testName}>
#                -DVALGRIND_EXECUTABLE=${VALGRIND_EXECUTABLE}
#                -P ${CMAKE_SOURCE_DIR}/cmake/ScriptRunUnitTestWithValgrind.cmake)
#    endif ()

    if (CC_COMMS_CHAMPION_FOUND)
        set (plugin_build_tgt ${APP_NAME}.${name}_plugin_build_tgt)
        add_custom_target(${plugin_build_tgt} ALL
            COMMAND ${CMAKE_COMMAND}
                -DPROJ_DIR=${output_dir}
                -DCOMMS_INSTALL_DIR="${COMMS_INSTALL_DIR}"
                -DCONFIG=$<CONFIG>
                -P "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake"
            DEPENDS ${output_tgt} ${output_dir}.tmp "${CMAKE_CURRENT_LIST_DIR}/BuildPlugin.cmake"
        )

        if (CC_EXTERNAL)
            add_dependencies(${plugin_build_tgt} ${CC_EXTERNAL_TGT})
        endif ()
    endif ()

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
        include(ExternalProject)

        set (cc_tag "develop")
        set (cc_main_dir "${CMAKE_BINARY_DIR}/comms_champion")
        set (cc_src_dir "${cc_main_dir}/src")
        set (cc_bin_dir "${cc_main_dir}/build")
        set (cc_install_dir "${cc_main_dir}/install")
        set (cc_qt_dir_opt)

        if (NOT "${COMMSDSL_QT_DIR}" STREQUAL "")
            set (cc_qt_dir_opt -DCC_QT_DIR=${COMMSDSL_QT_DIR})
        endif ()

        set (CC_PLUGIN_LIBRARIES "comms_champion")
        set (CC_COMMS_FOUND TRUE)
        set (CC_COMMS_CHAMPION_FOUND TRUE)
        set (CC_CMAKE_DIR ${LIB_INSTALL_DIR}/cmake)
        set (CC_EXTERNAL TRUE)
        set (CC_INCLUDE_DIRS "${cc_install_dir}/${CMAKE_INSTALL_INCLUDEDIR}")
        set (CC_PLUGIN_LIBRARY_DIRS "${cc_install_dir}/${CMAKE_INSTALL_LIBDIR}")
        set (COMMS_INSTALL_DIR "${cc_install_dir}")

        ExternalProject_Add(
            "${CC_EXTERNAL_TGT}"
            PREFIX "${cc_bin_dir}"
            STAMP_DIR "${cc_bin_dir}"
            GIT_REPOSITORY "https://github.com/arobenko/comms_champion.git"
            GIT_TAG "${cc_tag}"
            SOURCE_DIR "${cc_src_dir}"
            CMAKE_ARGS
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCC_INSTALL_DIR=${cc_install_dir}
                -DCC_NO_UNIT_TESTS=ON -DCC_NO_WARN_AS_ERR=ON -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                ${cc_qt_dir_opt}
            BINARY_DIR "${cc_bin_dir}"
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

include_directories(${CC_INCLUDE_DIRS})

if (CC_PLUGIN_LIBRARY_DIRS)
    link_directories("${CC_PLUGIN_LIBRARY_DIRS}")
endif ()

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
test_func (test21)

#add_clang_options (test11 "-Wno-c++11-narrowing")
