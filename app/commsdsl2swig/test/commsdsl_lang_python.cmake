
if (EXISTS ${this_test_dir}/${lang}/swig_${name}.py)
    add_test(
        NAME ${APP_NAME}.${name}_${lang}
        COMMAND ${Python_EXECUTABLE} ${this_test_dir}/${lang}/swig_${name}.py
        WORKING_DIRECTORY ${swig_output_dir}
    )

    set_property(TEST ${APP_NAME}.${name}_${lang} PROPERTY ENVIRONMENT "PYTHONPATH=${build_dir}:${build_dir}/output_python")
endif ()