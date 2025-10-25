# UNCRUSTIFY-OFF
#
# @file Makefile
# @author Manny Peterson <manny@heliosproj.org>
# @brief Makefile for HeliOS unit tests
# @version 0.5.0
# @date 2024-10-24
#
# @copyright
# HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
#
# UNCRUSTIFY-ON

# Compiler
CC = /usr/bin/gcc

# Base directory (parent of test directory)
BASE_DIR = ..

# Compiler flags
CFLAGS = -fdiagnostics-color=always \
         -O0 \
         -ggdb \
         -ansi \
         -pedantic \
         -Wall \
         -Wextra \
         -Wno-pointer-to-int-cast \
         -Wno-unused-parameter

# Include paths
INCLUDES = -I$(BASE_DIR)/src \
           -I$(BASE_DIR)/drivers/ramdisk \
           -I$(BASE_DIR)/drivers/block \
           -I$(BASE_DIR)/drivers/char \
           -I$(BASE_DIR)/test/src \
           -I$(BASE_DIR)/test/src/harness \
           -I$(BASE_DIR)/test/src/unit

# Preprocessor definitions
DEFINES = -DPOSIX_ARCH_OTHER \
          -DCONFIG_ENABLE_SYSTEM_ASSERT \
          -DCONFIG_MEMORY_REGION_SIZE_IN_BLOCKS=0x1C20u \
          -DUNIT_TEST_COLORIZE

# Output binary
TARGET = bin/test

# Compliance checker output file
CHECK_REPORT = compliance-report.json

# Source files
SOURCES = $(wildcard $(BASE_DIR)/src/*.c) \
          $(wildcard $(BASE_DIR)/drivers/ramdisk/*.c) \
          $(wildcard $(BASE_DIR)/drivers/block/*.c) \
          $(wildcard $(BASE_DIR)/drivers/char/*.c) \
          $(wildcard src/*.c) \
          $(wildcard src/harness/*.c) \
          $(wildcard src/unit/*.c)

# Default target
all: $(TARGET)

# Build the test binary
$(TARGET): $(SOURCES) | bin
	@echo "Building HeliOS unit tests..."
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $(SOURCES)
	@echo "Build complete: $(TARGET)"

# Create bin directory if it doesn't exist
bin:
	@mkdir -p bin

# Run the tests
test: $(TARGET)
	@echo "Running HeliOS unit tests..."
	@./$(TARGET)

# Run source code compliance checker
check:
	@echo "Running HeliOS source code compliance checker..."
	@cd $(BASE_DIR)/extras/check && ./check -o json ../../src/*.c -f ../../test/$(CHECK_REPORT) || true
	@echo "Compliance report saved to: $(CHECK_REPORT)"
	@echo "Run 'jq . $(CHECK_REPORT)' to view formatted output"

# Run tests with gdb for debugging
debug: $(TARGET)
	@echo "Starting HeliOS unit tests in debugger..."
	@gdb ./$(TARGET)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(TARGET)
	@rm -f $(CHECK_REPORT)
	@rm -rf bin
	@echo "Clean complete"

# Phony targets
.PHONY: all test check debug clean help

# Help target
help:
	@echo "HeliOS Unit Test Makefile Targets"
	@echo "================================="
	@echo "  all   - Build the unit test binary (default)"
	@echo "  test  - Build and run the unit tests"
	@echo "  check - Run compliance checker and save JSON report"
	@echo "  debug - Run tests in gdb debugger"
	@echo "  clean - Remove build artifacts and compliance report"
	@echo "  help  - Show this help message"