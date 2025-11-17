FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(
    ${PROJECT_NAME}_test cli_test.cpp command_test.cpp external_command_test.cpp pipe_test.cpp parser_test.cpp std_io_test.cpp
)

target_include_directories(
    ${PROJECT_NAME}_test PRIVATE ${PROJECT_SOURCE_DIR}
)

target_compile_definitions(
    ${PROJECT_NAME}_test PRIVATE TEST_DATA_DIR="${PROJECT_SOURCE_DIR}/test/data"
)

target_link_libraries(
    ${PROJECT_NAME}_test GTest::gtest_main ${PROJECT_NAME}_objs
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)
