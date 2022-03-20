function (commsdsl_ensure_comms_target)
    if (TARGET cc::comms)
        return()
    endif ()

    if (NOT TARGET cc::comms)
        find_package(LibComms QUIET)
    endif ()
    
    if (TARGET cc::comms)
        # Just to make find visible
        find_package(LibComms REQUIRED)
        return ()
    endif ()

    set (repo "${CC_COMMS_REPO}")
    if ("${repo}" STREQUAL "")
        set (repo "https://github.com/commschamp/comms.git")
    endif ()

    set (tag "${CC_TAG}")
    if ("${tag}" STREQUAL "")
        set (tag "master")
    endif ()

    if (NOT COMMAND cc_prefetch)
        include (${PROJECT_SOURCE_DIR}/cmake/CC_Prefetch.cmake)
    endif ()

    set (externals_dir "${COMMSDSL_EXTERNALS_DIR}")
    if ("${externals_dir}" STREQUAL "")
        set (externals_dir "${CMAKE_CURRENT_BINARY_DIR}/externals")
    endif ()

    set (src_dir "${externals_dir}/comms/src")
    cc_prefetch(SRC_DIR ${src_dir} TAG ${tag} REPO ${repo})

    set (build_file_include ${src_dir}/cmake/CC_CommsExternal.cmake)
    if (NOT EXISTS ${build_file_include})
        message (FATAL_ERROR "Required file ${build_file_include} doesn't exist")
    endif ()    

    include (${build_file_include})
    set (build_dir "${CMAKE_CURRENT_BINARY_DIR}/externals/comms/build")
    set (install_dir "${build_dir}/install")
    cc_comms_build_during_config(
        SRC_DIR ${src_dir}
        BUILD_DIR ${build_dir}
        CMAKE_ARGS 
            -DCMAKE_INSTALL_PREFIX=${install_dir}
            -DCC_COMMS_BUILD_UNIT_TESTS=OFF
        NO_REPO
    )

    list (APPEND CMAKE_PREFIX_PATH ${install_dir})
    find_package(LibComms REQUIRED)
endfunction()