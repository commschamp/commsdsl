# GENERATED
# OUTPUT
# CLEANUP_DIRS

function (copyIfDifferent generated output)
    if (("${generated}" STREQUAL "") OR ("${output}" STREQUAL "")) 
        message (FATAL_ERROR "Bad directory name(s)")
    endif ()
    
    file(GLOB_RECURSE genFiles RELATIVE "${generated}/" "${generated}/*")
    foreach( f ${genFiles} )
      set(dest "${output}/${f}")
      set(src "${generated}/${f}")
      #message (STATUS "Copying ${src} -> ${dest}")
      execute_process(
        COMMAND ${CMAKE_COMMAND}
            -E copy_if_different ${src} ${dest})
    endforeach()    
endfunction ()

function (deleteRemoved generated output)
    file(GLOB_RECURSE genFiles RELATIVE "${generated}/" "${generated}/*")
    file(GLOB_RECURSE outFiles RELATIVE "${output}/" "${output}/*")
    cmake_policy(SET CMP0057 NEW)
    foreach( f ${outFiles} )
        if (NOT "${f}" IN_LIST genFiles)
            message (STATUS "Removing: ${output}/${f}")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E remove ${output}/${f})
        endif ()    
    endforeach()    
endfunction ()

message (STATUS "Copying: ${GENERATED} --> ${OUTPUT}")
copyIfDifferent ("${GENERATED}" "${OUTPUT}")
foreach (d ${CLEANUP_DIRS})
    deleteRemoved ("${GENERATED}/${d}" "${OUTPUT}/${d}")
endforeach()
