if (NOT BUILD_TESTING)
    return ()
endif () 

commsdsl_ensure_comms_target ()
if ("${LibComms_DIR}" STREQUAL "")
    message (FATAL_ERROR "The comms library found but the directory is not known")
endif ()

if (NOT "${COMMSDSL_QT_DIR}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH ${COMMSDSL_QT_DIR})    
endif ()

set (CC_TOOLS_QT_CMAKE_DIR)
while (TRUE)
    if (TARGET cc::cc_tools_qt)
        break ()
    endif()

    find_package(cc_tools_qt QUIET)
    if (TARGET cc::cc_tools_qt)
        set (CC_TOOLS_QT_CMAKE_DIR "${cc_tools_qt_DIR}")
        break ()
    endif()    

    set (cc_tools_qt_src_dir ${COMMSDSL_EXTERNALS_DIR}/cc_tools_qt)
    set (cc_tools_qt_build_dir ${CMAKE_CURRENT_BINARY_DIR}/cc_tools_qt/build)
    set (cc_tools_qt_install_dir ${CMAKE_CURRENT_BINARY_DIR}/cc_tools_qt/install)
    set (CC_TOOLS_QT_CMAKE_DIR "${cc_tools_qt_install_dir}")

    list (APPEND CMAKE_PREFIX_PATH ${LibComms_DIR})   

    string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ALT_SEP "${CMAKE_PREFIX_PATH}") 

    include(ExternalProject)
    ExternalProject_Add(
        "cc_tools_qt_tgt"
        PREFIX "${cc_tools_qt_build_dir}"
        GIT_REPOSITORY https://github.com/commschamp/cc_tools_qt.git
        GIT_TAG "${CC_TOOLS_QT_TAG}"
        SOURCE_DIR "${cc_tools_qt_src_dir}"
        BINARY_DIR "${cc_tools_qt_build_dir}"
        INSTALL_DIR "${cc_tools_qt_install_dir}"
        LIST_SEPARATOR |
        CMAKE_ARGS 
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} 
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
            -DCMAKE_INSTALL_PREFIX=${cc_tools_qt_install_dir}
            -DCC_TOOLS_QT_EXTERNAL_COMMS=ON -DCC_TOOLS_QT_BUILD_APPS=OFF
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ALT_SEP}
    ) 

    set (cc_platform_specific)
    set (cc_tools_qt_lib "cc_tools_qt")

    if (WIN32)
        set (cc_lib_name ${cc_tools_qt_lib}.lib)
        list (APPEND cc_platform_specific Setupapi.lib Ws2_32.lib opengl32.lib imm32.lib winmm.lib)
    else ()
        set (cc_lib_name lib${cc_tools_qt_lib}.so)
    endif ()

    add_library(cc::cc_tools_qt UNKNOWN IMPORTED GLOBAL)
    file (MAKE_DIRECTORY ${cc_tools_qt_install_dir}/include)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Widgets REQUIRED)     
    set (interface_link_libs cc::comms Qt5::Widgets Qt5::Core)       

    if (cc_platform_specific)
        list (APPEND interface_link_libs ${cc_platform_specific})
    endif ()
    
    set_target_properties(cc::cc_tools_qt PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${cc_tools_qt_install_dir}/include
        IMPORTED_LOCATION ${cc_tools_qt_install_dir}/${cc_lib_name}
        INTERFACE_LINK_LIBRARIES "${interface_link_libs}"
    )

    add_dependencies(cc::cc_tools_qt ${cc_tools_qt_build_dir})
    break() 
endwhile ()

set (dep_prefix_path ${PROJECT_BINARY_DIR}/app/commsdsl2comms/test)
set (tests_path ${PROJECT_SOURCE_DIR}/app/commsdsl2comms/test)
file(GLOB tests RELATIVE ${tests_path} ${tests_path}/test*)

foreach (name ${tests})
    set (test_dir "${tests_path}/${name}")
    set (schema_file "${test_dir}/Schema.xml")
    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${name})

    set (rm_tmp_tgt ${APP_NAME}.${name}_rm_tmp_tgt)
    add_custom_target(${rm_tmp_tgt}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${output_dir}.tmp
    )

    add_custom_command(
        OUTPUT ${output_dir}.tmp
        DEPENDS ${schema_file} ${APP_NAME} ${rm_tmp_tgt}
        COMMAND $<TARGET_FILE:${APP_NAME}> --warn-as-err -o ${output_dir}.tmp ${schema_file}
    )

    set (output_tgt ${APP_NAME}.${name}_output_tgt)
    add_custom_target(${output_tgt} ALL
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${output_dir}.tmp" -DOUTPUT="${output_dir}"
            -P "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake"
        DEPENDS ${output_dir}.tmp "${PROJECT_SOURCE_DIR}/cmake/CopyGenerated.cmake" ${schema_file} ${APP_NAME})


    set (build_tgt ${APP_NAME}.${name}_build_tgt)
    set (build_dir ${output_dir}/build)
    set (install_dir ${build_dir}/install)

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${build_dir}")

    set (prot_tgt "commsdsl2comms.${name}_build_tgt")
    set (prot_install_dir "${PROJECT_BINARY_DIR}/app/commsdsl2comms/test/${name}/build/install")
    set (qt_path_param)
    if (NOT "${COMMSDSL_QT_DIR}" STREQUAL "")
        set (qt_path_param "\;${COMMSDSL_QT_DIR}")
    endif ()    

    add_custom_target(${build_tgt} ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${build_dir}"
        COMMAND 
            ${CMAKE_COMMAND} 
                ${output_dir}
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
                -DCMAKE_CXX_STANDARD=${COMMSDSL_TESTS_CXX_STANDARD}
                -DCMAKE_INSTALL_PREFIX=${install_dir}
                -DCMAKE_PREFIX_PATH="${LibComms_DIR}\;${prot_install_dir}\;${CC_TOOLS_QT_CMAKE_DIR}${qt_path_param}"
        COMMAND
            ${CMAKE_COMMAND} --build ${build_dir} --config ${CMAKE_BUILD_TYPE} --target install
        WORKING_DIRECTORY ${build_dir}
        DEPENDS ${output_tgt} commsdsl2comms.${name}_build_tgt)

    if (TARGET "cc_tools_qt_tgt")        
        add_dependencies(${build_tgt} "cc_tools_qt_tgt")
    endif ()
endforeach ()
