# Nomic C Semantic Source Code Analyzer
# Copyright (C) 2025 Manny Peterson <me@mannypeterson.com>
#
# This file is part of Nomic.
#
# Nomic is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Nomic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Nomic. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.20)
project(Nomic VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find LLVM/Clang
find_package(LLVM 14 REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Download and configure external dependencies
include(FetchContent)

# spdlog for logging
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0
)

# yaml-cpp for YAML parsing
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.7.0
)

# nlohmann/json for JSON output
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)

# fmt for formatting
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.1.1
)

FetchContent_MakeAvailable(spdlog yaml-cpp json fmt)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Collect all source files
file(GLOB_RECURSE NOMIC_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Collect LLVM/Clang libraries
# Note: Clang libraries already include necessary LLVM components transitively
# Explicitly linking LLVM libs can cause command line option registration conflicts

set(CLANG_LIBS
    clangAST
    clangBasic
    clangDriver
    clangFrontend
    clangLex
    clangParse
    clangSema
    clangSerialization
    clangTooling
)

# Build Nomic executable
add_executable(nomic ${NOMIC_SOURCES})

target_link_libraries(nomic
    PRIVATE
    ${CLANG_LIBS}
    spdlog::spdlog
    yaml-cpp
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# Installation
install(TARGETS nomic DESTINATION bin)
install(DIRECTORY include/nomic DESTINATION include)
