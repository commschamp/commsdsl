if (NOT BUILD_TESTING)
    return ()
endif () 

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)
set (languages "python")

cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)

find_package(LibComms REQUIRED)
find_package(SWIG REQUIRED OPTIONAL_COMPONENTS ${LANGUAGES})
find_package(Python3 COMPONENTS Interpreter Development)
include (UseSWIG)
set (UseSWIG_TARGET_NAME_PREFERENCE STANDARD)

add_custom_target(${APP_NAME}.all_tests
    COMMAND ${CMAKE_COMMAND} -E echo "All ${APP_NAME} tests are built")

include(ExternalProject)

set (CMAKE_SWIG_FLAGS -Wall -Werror)

foreach (name ${tests})
    set (test_dir "${tests_path}/${name}")
    file (GLOB schema_files ${test_dir}/Schema*.xml)
    foreach (lang ${languages})
        set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name}_${lang})

        set (this_test_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
        set (code_input_dir "${this_test_dir}/${lang}/src")
        set (code_input_param)
        if (EXISTS "${code_input_dir}/")
            set (code_input_param -c ${code_input_dir})
        endif()     

        set (rm_tmp_tgt ${APP_NAME}.${name}_rm_tmp_tgt)
        add_custom_target(${rm_tmp_tgt}
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
        )

        add_custom_command(
            OUTPUT ${output_dir}.tmp
            DEPENDS ${schema_files} ${APP_NAME} ${rm_tmp_tgt}
            COMMAND $<TARGET_FILE:${APP_NAME}> -s --warn-as-err ${code_input_param} -o ${output_dir}.tmp ${schema_files}
        )

        set (output_tgt ${APP_NAME}.${name}_${lang}_output_tgt)
        add_custom_target(${output_tgt} ALL
            COMMAND ${CMAKE_COMMAND}
                -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}" -DCLEANUP_DIRS="."
                -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
            DEPENDS ${output_dir}.tmp "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_files} ${APP_NAME})

        add_custom_command(
            OUTPUT ${output_dir}/${name}.i
            DEPENDS ${output_tgt}
            COMMAND ${CMAKE_COMMAND} -E echo "The swig interface file ${output_dir}/${name}.i is ready"
        )            

        set (protocol_install_dir ${dep_prefix_path}/${name}/build/install)

        set_source_files_properties(
            ${output_dir}/${name}.i
            PROPERTIES
                GENERATED TRUE
                CPLUSPLUS ON
        )
        set (swig_tgt ${APP_NAME}_${name}_${lang})
        set (test_build_dir ${output_dir}_build)
        swig_add_library(${swig_tgt} LANGUAGE ${lang} SOURCES ${output_dir}/${name}.i)
        target_link_libraries(${swig_tgt} cc::comms)
        add_dependencies(${swig_tgt} ${output_tgt} commsdsl2comms.${name}_build_tgt)
        target_include_directories(${swig_tgt} PRIVATE ${protocol_install_dir}/include)
        include (${CMAKE_CURRENT_SOURCE_DIR}/commsdsl_lang_${lang}.cmake)

        set (common_gcc_clang_opts
            -Wno-old-style-cast -Wno-cast-qual -Wno-shadow -Wno-double-promotion
        )

        set (gcc_opts ${common_gcc_clang_opts})

        if (CMAKE_COMPILER_IS_GNUCC)
            if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0")
                list (APPEND gcc_opts
                    "-Wno-duplicated-branches" 
                )
            endif()                        
        endif ()

        set (clang_opts ${common_gcc_clang_opts})

        target_compile_options(${swig_tgt} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
            $<$<CXX_COMPILER_ID:GNU>:${gcc_opts}>
            $<$<CXX_COMPILER_ID:Clang>:${clang_opts}>
        )            

        add_dependencies(${APP_NAME}.all_tests ${swig_tgt})
    endforeach()    
endforeach ()
