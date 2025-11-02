cmake_minimum_required(VERSION 3.16)

# Integration tests
file(GLOB_RECURSE INTEGRATION_TEST_SOURCES *.cpp)

if(INTEGRATION_TEST_SOURCES)
    add_executable(test_integration ${INTEGRATION_TEST_SOURCES})
    target_link_libraries(test_integration
        nomic_core
        nomic_query
        nomic_analysis
        nomic_plugin
        nomic_output
        nomic_api
        nomic_utils
        gtest_main
        gmock
    )

    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(test_integration)
endif()