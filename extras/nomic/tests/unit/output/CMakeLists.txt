cmake_minimum_required(VERSION 3.16)

# Output layer unit tests
file(GLOB_RECURSE OUTPUT_TEST_SOURCES *.cpp)

if(OUTPUT_TEST_SOURCES)
    add_executable(test_output ${OUTPUT_TEST_SOURCES})
    target_link_libraries(test_output
        nomic_output
        nomic_core
        gtest_main
        gmock
    )

    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(test_output)
endif()