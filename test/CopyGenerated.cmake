# GENERATED
# OUTPUT

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

message (STATUS "Copying: ${GENERATED} --> ${OUTPUT}")
copyIfDifferent ("${GENERATED}" "${OUTPUT}")
