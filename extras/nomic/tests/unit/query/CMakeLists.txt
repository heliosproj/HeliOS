cmake_minimum_required(VERSION 3.16)

# Query layer unit tests
file(GLOB_RECURSE QUERY_TEST_SOURCES *.cpp)

if(QUERY_TEST_SOURCES)
    add_executable(test_query ${QUERY_TEST_SOURCES})
    target_link_libraries(test_query
        nomic_query
        nomic_core
        gtest_main
        gmock
    )

    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(test_query)
endif()