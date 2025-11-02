if ((NOT BUILD_TESTING) OR (NOT COMMSDSL_BUILD_COMMSDSL2C_TESTS))
    return ()
endif ()

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

if (NOT TARGET cc::comms)
    message (FATAL_ERROR "cc::comms target is expected to be present")
endif ()

get_target_property(comms_include_dirs cc::comms INTERFACE_INCLUDE_DIRECTORIES)

add_custom_target(${APP_NAME}.all_tests
    COMMAND ${CMAKE_COMMAND} -E echo "All ${APP_NAME} tests are built")

include(ExternalProject)

set (COMMSDSL_SANITIZER_OPTS)
if (COMMSDSL_TEST_USE_SANITIZERS AND
    (NOT COMMSDSL_BUILD_WITH_SANITIZERS) AND
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

set (SANITIZERS_OPT "OFF")
if (COMMSDSL_BUILD_WITH_SANITIZERS OR (NOT "${COMMSDSL_SANITIZER_OPTS}" STREQUAL ""))
    set (SANITIZERS_OPT "ON")
endif ()

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

foreach (name ${tests})
    set (this_test_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}")

    if (EXISTS ${this_test_dir}/disabled.txt)
        continue ()
    endif ()

    if (WIN32 AND (EXISTS ${this_test_dir}/win_disabled.txt))
        continue ()
    endif ()

    set (test_dir "${tests_path}/${name}")
    file (GLOB schema_files ${test_dir}/Schema*.xml)
    set (code_input_dir "${this_test_dir}/src")
    set (code_input_param)
    if (EXISTS "${code_input_dir}/")
        set (code_input_param -c ${code_input_dir})
    endif()

    set (extra_params)
    if (EXISTS "${this_test_dir}/commsdsl2c.params")
        file (READ "${this_test_dir}/commsdsl2c.params" extra_params_str)
        string (REPLACE " " ";" extra_params "${extra_params_str}")
    endif ()

    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})
    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
        COMMAND $<TARGET_FILE:${APP_NAME}> -d -s --warn-as-err ${code_input_param} ${extra_params} -o ${output_dir}.tmp ${schema_files}
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}" -DCLEANUP_DIRS="."
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_files} $<TARGET_FILE:${APP_NAME}>)

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

    set (lib_name lib${name}_c.a)
    if (WIN32)
        set (lib_name ${name}_c.lib)
    endif ()    
    
    ExternalProject_Add(
        ${build_tgt}
        BUILD_ALWAYS TRUE
        DEPENDS ${output_tgt} commsdsl2comms.${name}_build_tgt
        SOURCE_DIR ${output_dir}
        BINARY_DIR ${build_dir}
        INSTALL_DIR ${install_dir}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM}
        CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
        CMAKE_GENERATOR_INSTANCE ${CMAKE_GENERATOR_INSTANCE}
        LIST_SEPARATOR |
        CMAKE_ARGS
            -DCMAKE_C_COMPILER=${COMMSDSL_TESTS_C_COMPILER} -DCMAKE_CXX_COMPILER=${COMMSDSL_TESTS_CXX_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
            -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
            -DCMAKE_INSTALL_PREFIX=${install_dir}
            -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
            -DOPT_FIND_COMMS=OFF
            -DOPT_FIND_PROTOCOL=OFF
            -DOPT_WITH_DEFAULT_SANITIZERS=${SANITIZERS_OPT}
            -DOPT_EXTRA_INCLUDE_DIRS=${extra_includes_path_str}
            -DOPT_USE_CCACHE=${COMMSDSL_USE_CCACHE}
            -DOPT_CCACHE_EXECUTABLE=${COMMSDSL_CCACHE_EXECUTABLE}

        # BUILD_BYPRODUCTS are needed by Ninja
        BUILD_BYPRODUCTS <INSTALL_DIR>/lib/${lib_name}            
    )

    add_dependencies(${APP_NAME}.all_tests ${build_tgt})

    if (NOT EXISTS ${this_test_dir}/${name}Test.th)
        continue()
    endif ()    

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${install_dir}/include")    

    add_library(${name}_c STATIC IMPORTED GLOBAL)
    add_dependencies(${name}_c ${build_tgt})
    set_target_properties(${name}_c PROPERTIES
        IMPORTED_LOCATION ${install_dir}/lib/${lib_name}
        IMPORTED_IMPLIB ${install_dir}/lib/${lib_name}
        INTERFACE_INCLUDE_DIRECTORIES "${install_dir}/include"
    )
    add_library(cc::${name}_c ALIAS ${name}_c)

    set (testName "${APP_NAME}.${name}Test")

    set (valgrand_opt)
    if (COMMSDSL_VALGRIND_TESTS AND VALGRIND_EXECUTABLE)
        set (valgrand_opt VALGRIND_EXECUTABLE ${VALGRIND_EXECUTABLE})
    endif ()

    cc_cxxtest_add_test(NAME ${testName} SRC ${this_test_dir}/${name}Test.th ${valgrand_opt})
    add_dependencies(${APP_NAME}.all_tests ${testName})

    add_dependencies(${testName} ${build_tgt})
    target_link_libraries(${testName} PRIVATE cc::${name}_c)

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
    
endforeach ()
