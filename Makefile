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

# Base directory (now the current directory)
BASE_DIR = .

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
TARGET = test/bin/test

# Report output files
TEST_REPORT = test/test-report.json
ANALYSIS_REPORT = test/analysis-report.json

# Source files
SOURCES = $(wildcard $(BASE_DIR)/src/*.c) \
          $(wildcard $(BASE_DIR)/drivers/ramdisk/*.c) \
          $(wildcard $(BASE_DIR)/drivers/block/*.c) \
          $(wildcard $(BASE_DIR)/drivers/char/*.c) \
          $(wildcard test/src/*.c) \
          $(wildcard test/src/harness/*.c) \
          $(wildcard test/src/unit/*.c)

# Default target
all: $(TARGET)

# Build the test binary
$(TARGET): $(SOURCES) | test/bin
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $(SOURCES)

# Create bin directory if it doesn't exist
test/bin:
	@mkdir -p test/bin

# Build Nomic semantic analyzer
nomic:
	@mkdir -p $(NOMIC_BUILD_DIR)
	@cd $(NOMIC_BUILD_DIR) && cmake .. && $(MAKE) -j$$(nproc 2>/dev/null || echo 4)

# Run the tests
test: $(TARGET)
	@./$(TARGET) --json $(TEST_REPORT)

# Run Nomic semantic source code analyzer
analyze: nomic compile_commands
	@$(NOMIC_BIN) analyze \
		$(BASE_DIR)/src/*.c \
		$(BASE_DIR)/drivers/ramdisk/*.c \
		$(BASE_DIR)/drivers/block/*.c \
		$(BASE_DIR)/drivers/char/*.c \
		--compilation-database $(BASE_DIR) \
		--rules $(BASE_DIR)/extras/code_standard/code_standard.yaml \
		--format json \
		--output $(ANALYSIS_REPORT)

# Generate compilation database for nomic
compile_commands:
	@python3 -c "import json, glob, os; base = os.path.abspath('$(BASE_DIR)'); files = glob.glob('$(BASE_DIR)/src/*.c') + glob.glob('$(BASE_DIR)/drivers/ramdisk/*.c') + glob.glob('$(BASE_DIR)/drivers/block/*.c') + glob.glob('$(BASE_DIR)/drivers/char/*.c'); commands = [{'directory': base, 'command': 'gcc -std=c11 -Wall $(INCLUDES_ABS) $(DEFINES) -c ' + os.path.abspath(f), 'file': os.path.abspath(f)} for f in files]; json.dump(commands, open(os.path.join(base, 'compile_commands.json'), 'w'), indent=2)"

# Run tests with gdb for debugging
debug: $(TARGET)
	@gdb ./$(TARGET)

# Clean build artifacts
clean:
	@rm -f $(TARGET)
	@rm -f $(TEST_REPORT) $(ANALYSIS_REPORT)
	@rm -f $(BASE_DIR)/compile_commands.json
	@rm -rf test/bin
	@rm -rf $(NOMIC_BUILD_DIR)

# Sync version from VERSION file to all project files
version:
	@if [ ! -f VERSION ]; then \
		echo "ERROR: VERSION file not found at VERSION"; \
		exit 1; \
	fi
	@VERSION=$$(cat VERSION | tr -d '[:space:]'); \
	if ! echo "$$VERSION" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$$'; then \
		echo "ERROR: Invalid version format in VERSION file: $$VERSION"; \
		echo "Expected format: X.Y.Z (e.g., 0.5.0)"; \
		exit 1; \
	fi; \
	MAJOR=$$(echo $$VERSION | cut -d'.' -f1); \
	MINOR=$$(echo $$VERSION | cut -d'.' -f2); \
	PATCH=$$(echo $$VERSION | cut -d'.' -f3); \
	MAJOR_HEX=$$(printf "0x%xu" $$MAJOR); \
	MINOR_HEX=$$(printf "0x%xu" $$MINOR); \
	PATCH_HEX=$$(printf "0x%xu" $$PATCH); \
	echo "Version: $$VERSION"; \
	echo "Components: Major=$$MAJOR, Minor=$$MINOR, Patch=$$PATCH"; \
	echo "Hex format: $$MAJOR_HEX, $$MINOR_HEX, $$PATCH_HEX"; \
	echo ""; \
	echo "Updating files..."; \
	echo ""; \
	sed -i "s/#define OS_MAJOR_VERSION_NO 0x[0-9a-fA-F]*u \/\* [0-9]* \*\//#define OS_MAJOR_VERSION_NO $$MAJOR_HEX \/\* $$MAJOR \*\//" src/defines.h && echo "✓ src/defines.h (MAJOR)"; \
	sed -i "s/#define OS_MINOR_VERSION_NO 0x[0-9a-fA-F]*u \/\* [0-9]* \*\//#define OS_MINOR_VERSION_NO $$MINOR_HEX \/\* $$MINOR \*\//" src/defines.h && echo "✓ src/defines.h (MINOR)"; \
	sed -i "s/#define OS_PATCH_VERSION_NO 0x[0-9a-fA-F]*u \/\* [0-9]* \*\//#define OS_PATCH_VERSION_NO $$PATCH_HEX \/\* $$PATCH \*\//" src/defines.h && echo "✓ src/defines.h (PATCH)"; \
	sed -i "s/\"version\":\"[^\"]*\"/\"version\":\"$$VERSION\"/" library.json && echo "✓ library.json"; \
	sed -i "s/^version=.*/version=$$VERSION/" library.properties && echo "✓ library.properties"; \
	sed -i "s/PROJECT_NUMBER = Kernel [0-9.]*'/PROJECT_NUMBER = Kernel $$VERSION'/" .github/workflows/HeliOS_CI_Workflow.yml && echo "✓ .github/workflows/HeliOS_CI_Workflow.yml"; \

# Uncrustify configuration target
config:
	@cd $(BASE_DIR)/extras/uncrustify && bash config.sh

# Uncrustify format target
uncrustify: config
	@find $(BASE_DIR)/src -name "*.c" ! -name "HeliOS.h" ! -name "config.h" -exec uncrustify -c $(BASE_DIR)/extras/uncrustify/helios.cfg --no-backup {} \;
	@find $(BASE_DIR)/src -name "*.h" ! -name "HeliOS.h" ! -name "config.h" -exec uncrustify -c $(BASE_DIR)/extras/uncrustify/helios.cfg --no-backup {} \;
	@find $(BASE_DIR)/drivers -name "*.c" -exec uncrustify -c $(BASE_DIR)/extras/uncrustify/helios.cfg --no-backup {} \;
	@find $(BASE_DIR)/drivers -name "*.h" -exec uncrustify -c $(BASE_DIR)/extras/uncrustify/helios.cfg --no-backup {} \;

# Phony targets
.PHONY: all test analyze debug clean help nomic compile_commands version config uncrustify

# Help target
help:
	@echo "  all        - Build the unit test binary (default)"
	@echo "  test       - Run unit tests and generate test-report.json"
	@echo "  analyze    - Run Nomic semantic analyzer on src/ and drivers/ and generate analysis-report.json"
	@echo "  nomic      - Build the Nomic C semantic analyzer"
	@echo "  version    - Sync version from VERSION file to all project files"
	@echo "  config     - Generate uncrustify helios.cfg from defaults.cfg"
	@echo "  uncrustify - Format all source and driver files with uncrustify"
	@echo "  debug      - Run tests in gdb debugger"
	@echo "  clean      - Remove all build artifacts and Nomic build directory"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build tests"
	@echo "  make test         # Run tests"
	@echo "  make nomic        # Build Nomic analyzer"
	@echo "  make analyze      # Build Nomic and analyze source code"
	@echo "  make version      # Update all files with version from VERSION file"
	@echo "  make config       # Generate uncrustify configuration"
	@echo "  make uncrustify   # Format all code with uncrustify"
	@echo "  make clean all    # Clean rebuild"
