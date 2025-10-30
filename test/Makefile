#
# @file Makefile
# @author Manny Peterson <manny@heliosproj.org>
# @brief Makefile for HeliOS unit tests
#
# @copyright
# HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
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

# Include paths (absolute for compilation database)
INCLUDES_ABS = -I$(shell pwd)/$(BASE_DIR)/src \
               -I$(shell pwd)/$(BASE_DIR)/drivers/ramdisk \
               -I$(shell pwd)/$(BASE_DIR)/drivers/block \
               -I$(shell pwd)/$(BASE_DIR)/drivers/char

# Include paths (relative for normal compilation)
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
analyze: nomic compile_commands
	@$(NOMIC_BIN) analyze $(BASE_DIR)/src/*.c \
		--compilation-database $(BASE_DIR) \
		--rules $(BASE_DIR)/extras/code_standard/code_standard.yaml \
		--format json \
		--output $(ANALYSIS_REPORT)
	@echo "Analysis report saved to: $(ANALYSIS_REPORT)"

# Generate compilation database for nomic
compile_commands:
	@python3 -c "import json, glob, os; base = os.path.abspath('$(BASE_DIR)'); commands = [{'directory': base, 'command': 'gcc -std=c11 -Wall $(INCLUDES_ABS) $(DEFINES) -c ' + os.path.abspath(f), 'file': os.path.abspath(f)} for f in glob.glob('$(BASE_DIR)/src/*.c')]; json.dump(commands, open(os.path.join(base, 'compile_commands.json'), 'w'), indent=2)"

# Run tests with gdb for debugging
debug: $(TARGET)
	@gdb ./$(TARGET)

# Clean build artifacts
clean:
	@rm -f $(TARGET)
	@rm -f $(TEST_REPORT) $(ANALYSIS_REPORT)
	@rm -f $(BASE_DIR)/compile_commands.json
	@rm -rf bin
	@rm -rf $(NOMIC_BUILD_DIR)

# Phony targets
.PHONY: all test analyze debug clean help nomic compile_commands

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