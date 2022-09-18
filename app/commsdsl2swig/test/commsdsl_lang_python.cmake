target_link_libraries(${swig_tgt} Python3::Python)

if (EXISTS ${this_test_dir}/${lang}/swig_${name}.py)
    add_test(
        NAME ${APP_NAME}.${name}_${lang}_test 
        COMMAND ${Python_EXECUTABLE} ${this_test_dir}/${lang}/swig_${name}.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif ()