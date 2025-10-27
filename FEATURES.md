# HeliOS 0.5.0 - New Features

This document details the new features introduced in HeliOS version 0.5.0 compared to version 0.4.1.

---

## Major Features

### 1. FAT32 Filesystem Support

HeliOS 0.5.0 introduces comprehensive FAT32 filesystem support, enabling file and directory operations on embedded devices.

**Key Components:**
- `src/fs.c` / `src/fs.h` - Complete FAT32 filesystem implementation
- Volume management (mount, unmount, format, get info)
- File operations (open, close, read, write, seek, tell, truncate, sync)
- Directory operations (open, close, read directory entries)
- Support for multiple file open modes (read, write, append, create)
- Configurable maximum path length

**API Functions:**
- Volume Management: `xFSMount()`, `xFSUnmount()`, `xFSGetVolumeInfo()`, `xFSFormat()`
- File Operations: `xFileOpen()`, `xFileClose()`, `xFileRead()`, `xFileWrite()`, `xFileSeek()`, `xFileTell()`, `xFileGetSize()`, `xFileSync()`, `xFileTruncate()`, `xFileEOF()`
- Directory Operations: `xDirOpen()`, `xDirClose()`, `xDirRead()`

**Reference:** `src/fs.h`

---

### 2. Console and Shell Subsystem

A new interactive console subsystem provides command-line interface capabilities for embedded systems.

**Features:**
- Interactive command shell with command buffer
- Current working directory tracking
- Echo control
- Device readiness checking
- ASCII art banner display
- String and path utility functions

**Key Components:**
- `src/console.c` / `src/console.h` - Console implementation
- `vConsoleTask()` - Console task entry point
- String utilities: `__strlen__()`, `__strcpy__()`, `__strcmp__()`, `__strcat__()`, `__strchr__()`, etc.

**Configuration:**
- `CONFIG_CONSOLE_MAX_COMMAND_LENGTH` - Maximum command buffer size
- `CONFIG_ENABLE_IO_SUBSYSTEM` - Enables console subsystem

**Reference:** `src/console.h`

---

### 3. Unified Driver Architecture

Version 0.5.0 introduces a standardized driver interface architecture that separates hardware-agnostic driver logic from hardware-specific I/O implementations.

#### 3.1 Block I/O Driver Interface

A standard interface for block-based storage devices (SD cards, RAM disks, EEPROMs, etc.).

**Features:**
- Hardware-agnostic block driver layer
- Support for multiple transfer modes (blocking, non-blocking, DMA, interrupt)
- Configurable block operations (read, write)
- Driver capability reporting

**Key Components:**
- `drivers/block/block_io_interface.h` - Block I/O interface definition
- `drivers/block/block_driver.c` / `block_driver.h` - Generic block driver implementation

**Data Structures:**
- `BlockIORequest_t` - Block I/O request structure
- `BlockIOInfo_t` - Driver capability information

**Reference:** `drivers/block/block_io_interface.h`

#### 3.2 Character I/O Driver Interface

A standard interface for character-based devices (UART, USART, SPI, I2C, USB CDC, etc.).

**Features:**
- Hardware-agnostic character driver layer
- Multiple transfer modes (blocking, non-blocking, DMA, interrupt)
- Configurable parameters (baud rate, parity, stop bits, data bits)
- RX/TX buffer flushing operations
- Status reporting

**Key Components:**
- `drivers/char/char_io_interface.h` - Character I/O interface definition
- `drivers/char/char_driver.c` / `char_driver.h` - Generic character driver implementation

**Data Structures:**
- `CharIORequest_t` - Character I/O request structure
- `CharIOInfo_t` - Driver capability information
- `CharIOParams_t` - Communication parameters

**Reference:** `drivers/char/char_io_interface.h`

#### 3.3 RAM Disk Driver

A RAM-based block storage driver for testing and simulation.

**Features:**
- 1MB default storage capacity (configurable)
- Full read/write support
- Position control commands
- Disk clear functionality
- Statistics reporting
- Block I/O interface compatible

**Key Components:**
- `drivers/ramdisk/ramdisk_driver.c` / `ramdisk_driver.h`

**Configuration:**
- `RAMDISK_SIZE_BYTES` - Total RAM disk size (default: 1MB)

**Reference:** `drivers/ramdisk/ramdisk_driver.h`

#### 3.4 STM32 USART Driver

Hardware-specific I/O driver for STM32 USART peripherals.

**Features:**
- STM32 HAL library integration
- Support for all STM32 families (F0-F7, H7, L0-L5, G0/G4, WB, WL)
- Multiple transfer modes (blocking, interrupt, DMA)
- Circular RX/TX buffering for interrupt mode
- Configurable communication parameters
- Error detection and reporting

**Key Components:**
- `drivers/usart_stm32/usart_stm32_driver.c` / `usart_stm32_driver.h`

**Configuration:**
- `USART_RX_BUFFER_SIZE` - Receive buffer size (default: 512 bytes)
- `USART_TX_BUFFER_SIZE` - Transmit buffer size (default: 512 bytes)

**Reference:** `drivers/usart_stm32/usart_stm32_driver.h`

---

### 4. Nomic - C Semantic Source Code Analyzer

A powerful static analysis tool for C code with custom rule-based code quality checking.

**Features:**
- Semantic analysis using LLVM/Clang
- Custom YAML-based rule definitions
- Multiple analysis capabilities:
  - Taint analysis
  - Concurrency analysis
  - Cross-function analysis
  - Memory safety analysis
  - IR (Intermediate Representation) analysis
- Pattern matching engine
- JSON and SARIF output formats
- CLI interface with progress reporting
- Coding standards enforcement

**Key Components:**
- Complete standalone tool in `extras/nomic/`
- CMake-based build system
- Extensible DSL for rule definitions
- Built-in HeliOS coding standard

**Tools:**
- AST visitor and query system
- Pattern matcher for multi-line code
- Compilation database support
- Diagnostic reporting

**Reference:** `extras/nomic/README.md`

---

### 5. Memory Management Improvements

Significant enhancements to the memory management subsystem.

#### 5.1 Variable-Sized Block Allocation

Replaced fixed-size block allocation with variable-sized blocks for more efficient memory usage.

**Features:**
- Dynamic block sizing based on allocation requests
- Block header metadata for integrity checking
- Configurable memory alignment
- Reduced memory overhead

**Data Structures:**
- `BlockHeader_t` - Block metadata with checksum and size information

#### 5.2 Enhanced Memory API

New and improved memory management functions.

**New Functions:**
- `xMemFreeAll()` - Free all allocated memory in the heap region
- `xMemGetHeapStats()` - Get detailed heap memory statistics
- `xMemGetKernelStats()` - Get detailed kernel memory statistics

**Improvements:**
- Better memory alignment support (`CONFIG_MEMORY_ALIGNMENT`)
- Improved block header validation
- Enhanced statistics tracking

**Reference:** `src/mem.h`, `src/mem.c`

#### 5.3 Memory Configuration

More flexible and better-documented memory configuration.

**Configuration Options:**
- `CONFIG_MEMORY_REGION_SIZE` - Size in bytes for each memory region
- `CONFIG_MEMORY_ALIGNMENT` - Memory alignment requirements
- Separate heap and kernel memory regions
- Configurable block header size

**Reference:** `src/config.h:205`

---

### 6. Object Validity Checking

All kernel objects now include a `valid` field for runtime integrity verification.

**Features:**
- Validity flags on all major kernel objects
- Runtime object integrity checking
- Protection against use of uninitialized or corrupted objects

**Affected Types:**
- `Device_t` - Device objects
- `Task_t` - Task objects
- `TaskNotification_t` - Task notification objects
- `TaskRunTimeStats_t` - Runtime statistics
- `TaskInfo_t` - Task information
- `MemoryRegionStats_t` - Memory statistics
- `QueueMessage_t` - Queue messages
- `SystemInfo_t` - System information

**Reference:** `src/types.h`

---

### 7. Enhanced C Macro System

Standardized macro system for consistent code patterns and error handling.

**Key Macros:**

#### Return Handling:
- `FUNCTION_ENTER` - Function entry with return variable initialization
- `FUNCTION_EXIT` - Function exit with return value
- `__ReturnOk__()` - Set return value to OK
- `__ReturnError__()` - Set return value to Error

#### Pointer Validation:
- `__PointerIsNotNull__(ptr)` - Validate pointer is not null
- `__PointerIsNull__(ptr)` - Check if pointer is null

#### Assertion Macros:
- `__AssertOnElse__()` - Assert handler for else branches
- Enhanced assertion system with file and line tracking

#### Flag and State Macros:
- Renamed and standardized flag checking macros
- Interrupt control macros
- Entry in-use macros

**Benefits:**
- Consistent error handling patterns
- Reduced code duplication
- Improved code readability
- Better assertion tracking

**Reference:** `src/defines.h`

---

### 8. Runtime Byte Order Detection

Automatic detection of system endianness at runtime.

**Features:**
- Runtime little-endian/big-endian detection
- Exposed through `SystemInfo_t` structure
- `littleEndian` field in system information

**API:**
- `xSystemGetSystemInfo()` - Returns system information including byte order

**Reference:** `src/sys.c`, `src/types.h`

---

### 9. Arduino Helper Functions

New C++ helper functions for Arduino platform integration.

**Features:**
- Byte array to String conversion for Arduino platforms
- Support for all major Arduino architectures (AVR, SAM, SAMD, ESP8266, STM32, Teensy)

**Functions:**
- `xByte2String()` - Convert byte array to Arduino String object

**Key Components:**
- `src/arduino_helpers.c` / `arduino_helpers.h`

**Supported Platforms:**
- Arduino AVR, SAM, SAMD
- ESP8266
- STM32
- Teensy (all variants)

**Reference:** `src/arduino_helpers.h`

---

### 10. HeliOS Coding Standard Specification

Formal JSON-based coding standard for HeliOS development.

**Features:**
- 27+ coding rules across multiple categories
- DSL-like rule definitions
- Categories include:
  - Function structure
  - Parameter validation
  - Control flow
  - Return handling
  - Memory management
  - Variable naming
  - Type usage
  - Numeric literals
  - Macro usage
  - Error checking
  - API naming conventions
  - Scope visibility

**Key Rules:**
- Function entry/exit macro requirements
- Parameter validation patterns
- Return value handling conventions
- Memory safety requirements
- API naming conventions

**Key Components:**
- `extras/standard/HELIOS_CODE_STANDARD.json` - Formal standard specification
- Integrated with Nomic static analyzer

**Reference:** `extras/standard/HELIOS_CODE_STANDARD.json`

---

### 11. Enhanced Configuration System

Significantly improved configuration documentation and options.

**Improvements:**
- Detailed Doxygen comments for all configuration options
- Better documentation of trade-offs and implications
- More granular control over kernel features
- New configuration options:
  - `CONFIG_ENABLE_IO_SUBSYSTEM` - Enable/disable I/O subsystem
  - `CONFIG_FS_MAX_PATH_LENGTH` - Maximum filesystem path length
  - `CONFIG_CONSOLE_MAX_COMMAND_LENGTH` - Console command buffer size
  - Enhanced memory configuration options

**Better Documentation:**
- Usage notes for each configuration option
- Examples and best practices
- Performance and memory impact information
- Cross-references to related settings

**Reference:** `src/config.h`

---

### 12. PlatformIO CI Support

New continuous integration workflow for PlatformIO builds.

**Features:**
- Automated PlatformIO builds for multiple platforms
- Board-specific compilation testing
- Example compilation verification
- Parallel build support

**Key Components:**
- `.github/workflows/HeliOS_PlatformIO.yml` - PlatformIO CI workflow
- Support for multiple Arduino-compatible boards

**Reference:** `.github/workflows/HeliOS_PlatformIO.yml`

---

### 13. Improved Build and Testing Infrastructure

Enhanced testing and build systems.

**Improvements:**
- New test harness infrastructure:
  - Console harness (`test/src/harness/console_harness.c`)
  - Filesystem harness (`test/src/harness/fs_harness.c`)
  - System harness (`test/src/harness/sys_harness.c`)
  - Mock USART (`test/src/harness/mock_usart.c`)
  - Consolidated memory harness (replacing separate memory_1/memory_2 harnesses)
- New test Makefile for easier test builds
- Validation check audit documentation

**Key Components:**
- `test/Makefile` - Test build system
- Enhanced harness framework
- `validation_check_audit.md` - Validation documentation

---

### 14. Documentation Improvements

**New Documentation:**
- `BUILD_GUIDE_ST.md` - STMicroelectronics build guide
- `SUPPORT.md` - Support and community information
- Updated Developer's Guide PDF
- Enhanced CONTRIBUTING.md

**Updated Templates:**
- New feature request template
- Updated bug report template
- Updated support request template
- Pull request template

---

## Breaking Changes and Migration Notes

### Stream Renaming

- `stream.c` renamed to `streams.c`
- `stream.h` renamed to `streams.h`
- Update include statements accordingly

### Memory Management

- Fixed-size block system replaced with variable-sized blocks
- Memory region size now configured in bytes (`CONFIG_MEMORY_REGION_SIZE`) instead of block count
- Update memory size calculations in existing code

### Type System Changes

- All public-facing structures now include `valid` field
- Code accessing structure sizes may need updates
- Initialize `valid` field when creating objects

### Configuration Changes

- Some configuration defaults have changed
- Review `src/config.h` for new defaults
- Update application configuration as needed

### API Additions (Backward Compatible)

- New functions added but existing APIs remain compatible
- Legacy code continues to work without modifications
- Consider migrating to new enhanced APIs for better functionality

---

## Summary Statistics

- **Files Added:** 100+
- **Files Modified:** 50+
- **Major Subsystems Added:** 4 (Filesystem, Console, Driver Architecture, Nomic)
- **New Drivers:** 4 (Block, Character, RAM Disk, STM32 USART)
- **Version:** 0.4.1 → 0.5.0
- **Development Commits:** 250+

---

## Version Information

- **Release Version:** 0.5.0
- **Previous Version:** 0.4.1
- **Release Date:** 2025
- **License:** GPL-2.0-or-later
- **Copyright:** HeliOS Project © 2020-2026

---

For detailed API documentation, please refer to the HeliOS Developer's Guide (`doc/HeliOS_Developers_Guide.pdf`) and individual header files in the `src/` directory.
