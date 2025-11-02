cmake_minimum_required(VERSION 3.16)

# Analysis layer unit tests
file(GLOB_RECURSE ANALYSIS_TEST_SOURCES *.cpp)

if(ANALYSIS_TEST_SOURCES)
    add_executable(test_analysis ${ANALYSIS_TEST_SOURCES})
    target_link_libraries(test_analysis
        nomic_analysis
        nomic_core
        nomic_query
        gtest_main
        gmock
    )

    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(test_analysis)
endif()