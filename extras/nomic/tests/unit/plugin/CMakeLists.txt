cmake_minimum_required(VERSION 3.16)

# Plugin layer unit tests
file(GLOB_RECURSE PLUGIN_TEST_SOURCES *.cpp)

if(PLUGIN_TEST_SOURCES)
    add_executable(test_plugin ${PLUGIN_TEST_SOURCES})
    target_link_libraries(test_plugin
        nomic_plugin
        nomic_core
        gtest_main
        gmock
        ${CMAKE_DL_LIBS}  # For dynamic loading
    )

    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(test_plugin)
endif()