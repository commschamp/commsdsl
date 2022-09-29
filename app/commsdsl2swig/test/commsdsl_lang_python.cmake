target_link_libraries(${swig_tgt} Python3::Python)

if (EXISTS ${this_test_dir}/${lang}/swig_${name}.py)
    add_test(
        NAME ${APP_NAME}.${name}_${lang}
        COMMAND ${Python_EXECUTABLE} ${this_test_dir}/${lang}/swig_${name}.py
        WORKING_DIRECTORY ${swig_output_dir}
    )
endif ()