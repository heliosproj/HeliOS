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


# Compiler
CC = /usr/bin/gcc
CXX = /usr/bin/c++

# Base directory (parent of test directory)
BASE_DIR = ..

# Nomic directories
NOMIC_DIR = $(BASE_DIR)/extras/nomic
NOMIC_BUILD_DIR = $(NOMIC_DIR)/build
NOMIC_BIN = $(NOMIC_BUILD_DIR)/nomic

# Compiler flags
CFLAGS = -fdiagnostics-color=always \
         -O0 \
         -ggdb \
         -ansi \
         -pedantic \
         -Wall \
         -Wextra

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
          -DCONFIG_MEMORY_REGION_SIZE=0x10000u \
          -DCONFIG_ENABLE_IO_SUBSYSTEM

# Output binary
TARGET = bin/test

# Report output files
TEST_REPORT = test-report.json
ANALYSIS_REPORT = analysis-report.json

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
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $(SOURCES)

# Create bin directory if it doesn't exist
bin:
	@mkdir -p bin

# Build Nomic semantic analyzer
nomic:
	@mkdir -p $(NOMIC_BUILD_DIR)
	@cd $(NOMIC_BUILD_DIR) && cmake .. && $(MAKE) -j$$(nproc 2>/dev/null || echo 4)

# Run the tests
test: $(TARGET)
	@./$(TARGET) --json $(TEST_REPORT)
	@echo "Test report saved to: $(TEST_REPORT)"

# Run Nomic semantic source code analyzer
analyze: nomic
	@$(NOMIC_BIN) analyze $(BASE_DIR)/src/*.c \
		--rules $(NOMIC_DIR)/helios.yaml \
		--format json \
		--output $(ANALYSIS_REPORT) 2>/dev/null || true
	@echo "Analysis report saved to: $(ANALYSIS_REPORT)"

# Run tests with gdb for debugging
debug: $(TARGET)
	@gdb ./$(TARGET)

# Clean build artifacts
clean:
	@rm -f $(TARGET)
	@rm -f $(TEST_REPORT) $(ANALYSIS_REPORT)
	@rm -rf bin
	@rm -rf $(NOMIC_BUILD_DIR)

# Phony targets
.PHONY: all test analyze debug clean help nomic

# Help target
help:
	@echo "  all      - Build the unit test binary (default)"
	@echo "  test     - Run unit tests and generate test-report.json"
	@echo "  analyze  - Run Nomic semantic analyzer and generate analysis-report.json"
	@echo "  nomic    - Build the Nomic C semantic analyzer"
	@echo "  debug    - Run tests in gdb debugger"
	@echo "  clean    - Remove all build artifacts and Nomic build directory"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build tests"
	@echo "  make test         # Run tests"
	@echo "  make nomic        # Build Nomic analyzer"
	@echo "  make analyze      # Build Nomic and analyze source code"
	@echo "  make clean all    # Clean rebuild"