if (NOT BUILD_TESTING)
    return ()
endif () 

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

find_program (EMCMAKE_EXECUTABLE NAMES "emcmake" REQUIRED)
message (STATUS "Found emcmake: ${EMCMAKE_EXECUTABLE}")

if (NOT TARGET cc::comms)
    message (FATAL_ERROR "cc::comms target is expected to be present")
endif ()

get_target_property(comms_include_dirs cc::comms INTERFACE_INCLUDE_DIRECTORIES)

add_custom_target(${APP_NAME}.all_tests
    COMMAND ${CMAKE_COMMAND} -E echo "All ${APP_NAME} tests are built")

include(ExternalProject)

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

find_program (NODE_EXECUTABLE NAMES node nodejs REQUIRED)
message (STATUS "Found node: ${NODE_EXECUTABLE}")
execute_process(
    COMMAND ${NODE_EXECUTABLE} --version
    OUTPUT_VARIABLE NODE_VERSION_OUT)

string(REGEX REPLACE "^v" "" NODE_VERSION ${NODE_VERSION_OUT})
set (extra_node_params)
if ("${NODE_VERSION}" VERSION_GREATER_EQUAL "18")
    list (APPEND extra_node_params "--no-experimental-fetch")
endif ()

foreach (name ${tests})
    set (test_dir "${tests_path}/${name}")
    file (GLOB schema_files ${test_dir}/Schema*.xml)
    set (this_test_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
    set (code_input_dir "${this_test_dir}/src")
    set (code_input_param)
    if (EXISTS "${code_input_dir}/")
        set (code_input_param -c ${code_input_dir})
    endif()    

    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})

    set (rm_tmp_tgt ${APP_NAME}.${name}_rm_tmp_tgt)
    add_custom_target(${rm_tmp_tgt}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
    )

    add_custom_command(
        OUTPUT ${output_dir}.tmp
        DEPENDS ${schema_files} ${APP_NAME} ${rm_tmp_tgt}
        COMMAND $<TARGET_FILE:${APP_NAME}> -s --warn-as-err ${code_input_param} -o ${output_dir}.tmp ${schema_files}
    )

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}" -DCLEANUP_DIRS="."
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS ${output_dir}.tmp "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_files} ${APP_NAME})

    if ("${CMAKE_VERSION}" VERSION_GREATER_EQUAL 3.15)
        set_target_properties(${output_tgt} PROPERTIES ADDITIONAL_CLEAN_FILES "${output_dir}.tmp;${output_dir}")
    endif ()            

    add_dependencies(${APP_NAME}.all_tests ${output_tgt})         

    set (build_tgt ${APP_NAME}.${name}_build_tgt)
    set (build_dir ${output_dir}_build)
    set (install_dir ${build_dir}/install)
    set (prot_include_dir "${PROJECT_BINARY_DIR}/app/commsdsl2comms/test/${name}_install/include")
    set (extra_includes_path ${prot_include_dir} ${comms_include_dirs})
    string (REPLACE ";" "|" extra_includes_path_str "${extra_includes_path}")    

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}")          

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E touch "${output_dir}/dummy.txt")        

    ExternalProject_Add(
        ${build_tgt}
        BUILD_ALWAYS TRUE
        DEPENDS ${output_tgt} commsdsl2comms.${name}_build_tgt
        SOURCE_DIR ${output_dir}
        BINARY_DIR ${build_dir}
        INSTALL_DIR ${install_dir}
        CMAKE_COMMAND ${EMCMAKE_EXECUTABLE} ${CMAKE_COMMAND}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM}
        CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
        LIST_SEPARATOR |
        CMAKE_ARGS 
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${install_dir}
            -DOPT_MODULARIZE=ON
            -DOPT_EXTRA_INCLUDE_DIRS=${extra_includes_path_str}
            -DOPT_USE_CCACHE=${COMMSDSL_USE_CCACHE}
            -DOPT_CCACHE_EXECUTABLE=${COMMSDSL_CCACHE_EXECUTABLE}            
    )     

    add_dependencies(${APP_NAME}.all_tests ${build_tgt})     

    if (EXISTS ${this_test_dir}/emscripten_${name}.js)
        add_test(
            NAME ${APP_NAME}.${name}
            COMMAND ${NODE_EXECUTABLE} ${extra_node_params} ${this_test_dir}/emscripten_${name}.js
            WORKING_DIRECTORY ${build_dir}
        )    

        set_property(TEST ${APP_NAME}.${name} PROPERTY ENVIRONMENT "NODE_PATH=${install_dir}/js")
    endif ()

endforeach ()
