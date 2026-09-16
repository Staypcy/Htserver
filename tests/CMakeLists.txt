file(GLOB HTSERVER_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.CC"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.CXX"
)

list(SORT HTSERVER_TEST_SOURCES)
redefine_file_macro("${HTSERVER_TEST_SOURCES}")

find_package(Threads REQUIRED)

set(HTSERVER_TEST_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/tests")

foreach(TEST_SOURCE IN LISTS HTSERVER_TEST_SOURCES)
    message(STATUS "==========${TEST_SOURCE}")
    get_filename_component(TEST_NAME "${TEST_SOURCE}" NAME_WE)
    message(STATUS "==========${TEST_NAME}")

    if(TARGET "${TEST_NAME}")
        message(FATAL_ERROR "Cannot create test target '${TEST_NAME}':a target with the name already exists")
    endif()

    add_executable("${TEST_NAME}" "${TEST_SOURCE}")

    #htserver中的PUBLIC依赖会让测试用例同时继承头文件路径和htserver_option
    target_link_libraries("${TEST_NAME}" PRIVATE htserver Threads::Threads)

    set_target_properties("${TEST_NAME}" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${HTSERVER_TEST_OUTPUT_DIRECTORY}"
    )

    #多配置生成器默认会追加 Debug/Release 子目录，这里保持输出路径一致
    foreach(CONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER "${CONFIG}" CONFIG_UPPER)
        set_target_properties("${TEST_NAME}" PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}" "${HTSERVER_TEST_OUTPUT_DIRECTORY}"
        )
    endforeach(CONFIG)

    add_test(NAME "${TEST_NAME}" COMMAND "${TEST_NAME}")

endforeach()

add_subdirectory(inner_tests)