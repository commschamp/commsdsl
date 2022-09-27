if (NOT TARGET JNI::JNI)
    message(FATAL_ERROR "TARGET JNI::JNI does not exist")
endif ()
target_link_libraries(${swig_tgt} JNI::JNI)
