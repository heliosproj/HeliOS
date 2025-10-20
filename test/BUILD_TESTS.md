# Building HeliOS Unit Tests

## Quick Build

To build and run the HeliOS unit tests (including console tests):

```bash
cd /home/manny/github/HeliOS

# Build tests
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

# Run tests
./test/bin/test
```

## Required Include Directories

- `-I./src` - HeliOS kernel headers
- `-I./drivers/ramdisk` - RAM disk driver
- `-I./drivers/block` - Block device driver
- `-I./drivers/char` - **Character device driver (required for console tests)**
- `-I./test/src` - Test framework
- `-I./test/src/harness` - Test harnesses
- `-I./test/src/unit` - Unit test utilities

## Required Source Files

- `./src/*.c` - HeliOS kernel sources
- `./drivers/ramdisk/*.c` - RAM disk implementation
- `./drivers/block/*.c` - Block device implementation
- `./drivers/char/*.c` - **Character device implementation (required for console tests)**
- `./test/src/*.c` - Test main
- `./test/src/harness/*.c` - All test harnesses (device, fs, console, etc.)
- `./test/src/unit/*.c` - Unit test framework

## Console Test Files

The console unit tests include:

- `test/src/harness/mock_usart.h` - Mock USART device interface
- `test/src/harness/mock_usart.c` - Mock USART implementation with circular buffers
- `test/src/harness/console_harness.h` - Console test harness header
- `test/src/harness/console_harness.c` - Console test suite

## Expected Warnings

The build will produce 7 warnings from `src/console.c` (pre-existing, not from test code):

- 6 function pointer cast warnings (lines 67-75)
- 1 pedantic initializer warning (line 178)

These warnings are **harmless** and from the original HeliOS console implementation.

## Test Output

Successfully built tests produce:

```
Total: 368 tests
Passed: 358 (97.3%)
Failed: 10 (2.7%)
```

Console-specific tests cover:
- Device initialization
- Character input handling
- Command parsing
- Built-in commands (help, version, echo, etc.)
