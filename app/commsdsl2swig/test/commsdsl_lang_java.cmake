
if (EXISTS ${this_test_dir}/${lang}/SwigTest.java)
    add_custom_target(
        ${APP_NAME}.${name}_${lang}_javac ALL
        COMMAND ${Java_JAVAC_EXECUTABLE} -Xdiags:verbose -d ${build_dir} -cp ${build_dir} ${this_test_dir}/${lang}/*.java 
        DEPENDS ${build_tgt}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_dependencies(${APP_NAME}.all_tests ${APP_NAME}.${name}_${lang}_javac)

    add_test(
        NAME ${APP_NAME}.${name}_${lang}
        COMMAND ${Java_JAVA_EXECUTABLE} -ea -cp ${build_dir} SwigTest 
        WORKING_DIRECTORY ${build_dir}
    )    

    set_property(TEST ${APP_NAME}.${name}_${lang} PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=${build_dir}")
endif ()



