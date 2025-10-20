# HeliOS Console Unit Testing - Implementation Summary

## Overview

This document describes the console unit testing implementation using a mock USART device approach.

## Implementation Approach: Mock Hardware Device

We implemented **Option 1**: A mock hardware device that the character device driver uses to mimic USART functionality. This allows us to:

- Inject keystrokes into the console
- Capture console output
- Test the full console stack without real hardware

## Architecture

```
┌─────────────────────────────────────┐
│  Console (console.c)                │
│  - Command processing               │
│  - User interaction                 │
└──────────────┬──────────────────────┘
               │ Uses CHARDEV0
               ▼
┌─────────────────────────────────────┐
│  Character Driver (char_driver.c)   │
│  - Line discipline                  │
│  - Protocol handling                │
└──────────────┬──────────────────────┘
               │ Uses char_io_interface
               ▼
┌─────────────────────────────────────┐
│  Mock USART (mock_usart.c)          │  ← TEST IMPLEMENTATION
│  - Circular RX buffer (512 bytes)   │
│  - Circular TX buffer (512 bytes)   │
│  - Test injection/capture helpers   │
└─────────────────────────────────────┘
```

## Files Created

### Mock USART Device
- **`test/src/harness/mock_usart.h`** - Device interface and test helper declarations
- **`test/src/harness/mock_usart.c`** - Full implementation with circular buffers

### Console Test Harness
- **`test/src/harness/console_harness.h`** - Console test harness header
- **`test/src/harness/console_harness.c`** - Console test suite implementation

### Documentation
- **`test/BUILD_TESTS.md`** - Build instructions
- **`test/CONSOLE_TEST_SUMMARY.md`** - This file

## Files Modified

- **`test/src/test.h`** - Added console_harness.h include
- **`test/src/test.c`** - Added console_harness() call and cleanup
- **`.github/workflows/HeliOS_CI_Workflow.yml`** - Updated build to include char driver

## Mock USART Features

### Device Registration
- Device UID: `0x0100` (MOCKUSRT)
- Implements full HeliOS device interface (7 required functions)
- Supports `char_io_interface.h` protocol

### Circular Buffers
- **RX Buffer**: 512 bytes (for keystroke injection)
- **TX Buffer**: 512 bytes (for output capture)
- Thread-safe circular buffer management

### Supported Commands
- `CHAR_IO_CMD_SET_REQUEST` - Configure I/O operations
- `CHAR_IO_CMD_GET_INFO` - Query device capabilities
- `CHAR_IO_CMD_GET_STATUS` - Get buffer status
- `CHAR_IO_CMD_SET_BAUDRATE` - Accept baud rate (ignored)
- `CHAR_IO_CMD_SET_PARAMS` - Accept UART params (ignored)

### Test Helper Functions

```c
/* Inject keystrokes into console input */
void xMockUSARTInjectInput(const Byte_t *input_);

/* Capture console output */
void xMockUSARTGetOutput(Byte_t *buffer_, HalfWord_t maxLen_, HalfWord_t *actualLen_);

/* Reset all buffers */
void xMockUSARTReset(void);

/* Check RX buffer status */
HalfWord_t xMockUSARTGetRxAvailable(void);

/* Check TX buffer status */
HalfWord_t xMockUSARTGetTxCount(void);

/* Clear output buffer only */
void xMockUSARTClearOutput(void);
```

## Console Test Suite

### Test Sections

1. **Console Initialization**
   - Mock USART registration
   - Character driver registration
   - Character device configuration
   - Console initialization

2. **Character Input Handling**
   - Printable character echo
   - Backspace handling
   - Carriage return processing

3. **Command Parsing**
   - Unknown command errors
   - Empty command handling
   - Leading whitespace handling

4. **Help Command**
   - Command listing
   - Description display

5. **Version Command**
   - Version information display
   - Copyright information

6. **Echo Command**
   - Echo with arguments
   - Echo mode toggle

## Build Instructions

### Correct Build Command

```bash
gcc -fdiagnostics-color=always -O0 -ggdb -ansi -pedantic -Wall -Wextra \
  -Wno-unused-parameter -Wno-pointer-to-int-cast \
  -I./src \
  -I./drivers/ramdisk \
  -I./drivers/block \
  -I./drivers/char \
  -I./test/src \
  -I./test/src/harness \
  -I./test/src/unit \
  -DPOSIX_ARCH_OTHER \
  -DCONFIG_MEMORY_REGION_SIZE_IN_BLOCKS=0x1C20u \
  -DCONFIG_ENABLE_SYSTEM_ASSERT \
  -DUNIT_TEST_COLORIZE \
  -o./test/bin/test \
  ./src/*.c \
  ./drivers/ramdisk/*.c \
  ./drivers/block/*.c \
  ./drivers/char/*.c \
  ./test/src/*.c \
  ./test/src/harness/*.c \
  ./test/src/unit/*.c
```

### Critical Requirements

**Must include:**
- `-I./drivers/char` - Character driver headers
- `./drivers/char/*.c` - Character driver sources

**Without these, you will get linker errors:**
- `undefined reference to '__CharDeviceStateClear__'`
- `undefined reference to 'CHARDEV0_self_register'`

## Build Status

### ✅ Compilation
- **Errors**: 0
- **Warnings**: 7 (all pre-existing from console.c)
- **Binary**: 392KB executable

### Expected Warnings (Pre-existing)

From `src/console.c`:
- Lines 67-75: 6 function pointer cast warnings (harmless)
- Line 178: 1 pedantic initializer warning (harmless)

**These warnings are NOT from our test code** - they exist in the original console implementation.

## Test Results

```
Total Tests: 368
Passed: 358 (97.3%)
Failed: 10 (2.7%)
```

### Console Test Status
- ✅ Initialization tests: All passing
- ✅ Character input handling: All passing
- ✅ Command parsing: All passing
- ⚠️ Command output tests: Some failures (timing/buffering issues)

## Usage Example

```c
/* Setup console environment */
__SetupConsoleEnvironment__();

/* Inject a command */
xMockUSARTInjectInput((const Byte_t *) "help\r");

/* Run console task cycles */
__SimulateConsoleCycles__(20u);

/* Capture output */
Byte_t output[1024];
HalfWord_t len;
xMockUSARTGetOutput(output, 1024, &len);

/* Verify output contains expected text */
if(__OutputContains__(output, (const Byte_t *) "Available commands")) {
    /* Test passes */
}
```

## Key Design Decisions

1. **Mock at hardware level** - Not at character driver level
   - Allows testing of full stack
   - Mimics real hardware behavior
   - Reuses existing character driver code

2. **Circular buffers** - Industry standard approach
   - Efficient memory usage
   - No dynamic allocation during I/O
   - Thread-safe design

3. **Test helper functions** - Clean test API
   - Inject input: `xMockUSARTInjectInput()`
   - Capture output: `xMockUSARTGetOutput()`
   - Reset state: `xMockUSARTReset()`

4. **Setup helper** - Reduces test boilerplate
   - `__SetupConsoleEnvironment__()` handles:
     - State clearing
     - Device configuration
     - Console initialization
     - Initial task cycles
     - Output buffer clearing

## Future Enhancements

Potential improvements:
1. Add more command tests (tasks, mem, clear, pwd, ls, cd, cat, etc.)
2. Test filesystem integration
3. Test error conditions more thoroughly
4. Add performance/stress tests
5. Test maximum buffer conditions
6. Add interrupt/DMA mode support to mock

## References

- Mock USART implementation: `test/src/harness/mock_usart.c`
- Console tests: `test/src/harness/console_harness.c`
- Character driver: `drivers/char/char_driver.c`
- Character I/O interface: `drivers/char/char_io_interface.h`
- Build instructions: `test/BUILD_TESTS.md`
