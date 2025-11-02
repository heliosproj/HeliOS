cmake_minimum_required(VERSION 3.16)
project(NomicAnalyzer VERSION 1.0.0 LANGUAGES C CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

# Enable testing
enable_testing()

# Find packages
find_package(Threads REQUIRED)

# Download and build Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Find Clang/LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Find Clang
find_package(Clang REQUIRED CONFIG)
message(STATUS "Found Clang")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# Add LLVM definitions
add_definitions(${LLVM_DEFINITIONS})

# Source files for each layer
file(GLOB_RECURSE CORE_SOURCES src/core/*.cpp src/core/*.c)
file(GLOB_RECURSE QUERY_SOURCES src/query/*.cpp src/query/*.c)
file(GLOB_RECURSE ANALYSIS_SOURCES src/analysis/*.cpp src/analysis/*.c)
file(GLOB_RECURSE PLUGIN_SOURCES src/plugin/*.cpp src/plugin/*.c)
file(GLOB_RECURSE OUTPUT_SOURCES src/output/*.cpp src/output/*.c)
file(GLOB_RECURSE API_SOURCES src/api/*.cpp src/api/*.c)
file(GLOB_RECURSE UTILS_SOURCES src/utils/*.cpp src/utils/*.c)

# Core library (static)
add_library(nomic_core STATIC ${CORE_SOURCES})
target_link_libraries(nomic_core PUBLIC
    clangAST
    clangBasic
    clangLex
    clangParse
    clangSema
    clangAnalysis
    clangFrontend
    clangTooling
    ${LLVM_LIBS}
)

# Find SQLite3 and JsonCpp using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Query library (static)
add_library(nomic_query STATIC ${QUERY_SOURCES})
target_link_libraries(nomic_query PUBLIC
    nomic_core
    ${SQLITE3_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)
target_include_directories(nomic_query PUBLIC
    ${SQLITE3_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)
target_compile_options(nomic_query PUBLIC
    ${SQLITE3_CFLAGS_OTHER}
    ${JSONCPP_CFLAGS_OTHER}
)

# Analysis library (static)
add_library(nomic_analysis STATIC ${ANALYSIS_SOURCES})
target_link_libraries(nomic_analysis PUBLIC nomic_core nomic_query)

# Plugin library (shared for dynamic loading)
add_library(nomic_plugin SHARED ${PLUGIN_SOURCES})
target_link_libraries(nomic_plugin PUBLIC nomic_core nomic_query nomic_analysis)

# Output library (static)
add_library(nomic_output STATIC ${OUTPUT_SOURCES})
target_link_libraries(nomic_output PUBLIC nomic_core nomic_analysis)

# API library (shared for C API)
add_library(nomic_api SHARED ${API_SOURCES})
target_link_libraries(nomic_api PUBLIC
    nomic_core
    nomic_query
    nomic_analysis
    nomic_plugin
    nomic_output
)

# Utils library (static)
add_library(nomic_utils STATIC ${UTILS_SOURCES})

# Main executable
add_executable(nomic src/main.cpp)
target_link_libraries(nomic
    nomic_core
    nomic_query
    nomic_analysis
    nomic_plugin
    nomic_output
    nomic_api
    nomic_utils
    Threads::Threads
)

# Unit tests
add_subdirectory(tests/unit/core)
add_subdirectory(tests/unit/query)
add_subdirectory(tests/unit/analysis)
add_subdirectory(tests/unit/plugin)
add_subdirectory(tests/unit/output)
add_subdirectory(tests/unit/api)

# Integration tests
add_subdirectory(tests/integration)

# Install rules
install(TARGETS nomic DESTINATION bin)
install(TARGETS nomic_api DESTINATION lib)
install(DIRECTORY include/nomic DESTINATION include)

# CPack configuration
set(CPACK_PACKAGE_NAME "Nomic")
set(CPACK_PACKAGE_VENDOR "Nomic C Semantic Code Analyzer")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Commercial-grade C Semantic Code Analyzer")
include(CPack)