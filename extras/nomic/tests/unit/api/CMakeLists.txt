cmake_minimum_required(VERSION 3.16)

# API layer unit tests
file(GLOB_RECURSE API_TEST_SOURCES *.cpp)

if(API_TEST_SOURCES)
    add_executable(test_api ${API_TEST_SOURCES})
    target_link_libraries(test_api
        nomic_api
        nomic_core
        gtest_main
        gmock
    )

    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(test_api)
endif()