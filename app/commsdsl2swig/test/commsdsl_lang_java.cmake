if (NOT TARGET JNI::JNI)
    message(FATAL_ERROR "TARGET JNI::JNI does not exist")
endif ()
target_link_libraries(${swig_tgt} JNI::JNI)

if (EXISTS ${this_test_dir}/${lang}/SwigTest.java)
    add_custom_target(
        ${swig_tgt}_javac ALL
        COMMAND ${Java_JAVAC_EXECUTABLE} -d . ${this_test_dir}/${lang}/*.java *.java
        DEPENDS ${swig_tgt}
        WORKING_DIRECTORY ${swig_output_dir}
    )

    add_test(
        NAME ${APP_NAME}.${name}_${lang}
        COMMAND ${Java_JAVA_EXECUTABLE} -ea SwigTest
        WORKING_DIRECTORY ${swig_output_dir}
    )    

    set_property(TEST ${APP_NAME}.${name}_${lang} PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}")
endif ()



