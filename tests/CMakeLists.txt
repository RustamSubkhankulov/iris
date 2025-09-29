# Find GoogleTest
find_package(GTest REQUIRED)

file(GLOB_RECURSE TESTS_SOURCES CONFIGURE_DEPENDS *.cpp)
add_executable(iris_tests ${TESTS_SOURCES})

# Link libraries
target_link_libraries(iris_tests
    ${PROJECT_NAME}
    GTest::gtest
    GTest::gtest_main
)

# Set output directory
set_target_properties(iris_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

# Add tests
gtest_discover_tests(iris_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)