cmake_minimum_required(VERSION 3.16)

# Core layer unit tests
file(GLOB_RECURSE CORE_TEST_SOURCES *.cpp)

add_executable(test_core ${CORE_TEST_SOURCES})
target_link_libraries(test_core
    nomic_core
    gtest_main
    gmock
    ${LLVM_LIBS}
    clangAST
    clangBasic
    clangFrontend
    clangTooling
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(test_core)