# HeliOS Coding Standard

## Table of Contents
1. [Introduction](#introduction)
2. [C90/ANSI C Compliance](#c90ansi-c-compliance)
3. [Code Formatting](#code-formatting)
4. [Naming Conventions](#naming-conventions)
5. [Function Scope Control](#function-scope-control)
6. [Defensive Programming](#defensive-programming)
7. [Error Handling](#error-handling)
8. [Memory Management](#memory-management)
9. [Documentation Standards](#documentation-standards)
10. [File Organization](#file-organization)
11. [Testing Standards](#testing-standards)
12. [Build and Compilation](#build-and-compilation)

## Introduction

This document defines the coding standards for the HeliOS Embedded Operating System. All contributions must adhere to these standards to ensure code quality, maintainability, and consistency across the codebase.

### Core Principles
- **Reliability**: Code must be robust and handle all error conditions
- **Portability**: Strict C90/ANSI C compliance for maximum compatibility
- **Clarity**: Code should be self-documenting with clear intent
- **Quality**: Minimum 80% test coverage with comprehensive test suites
- **Testability**: All code must be unit testable with clear test documentation

### Key Requirements
- **C90/ANSI C** strict compliance (no C99/C11 features)
- **MISRA C** guidelines followed where applicable
- **Test Coverage**: 80% line, 75% branch, 100% public API function coverage
- **Defensive Programming** with comprehensive input validation
- **Single Exit Point** for all functions (MISRA Rule 14.7)
- **No Dynamic Memory** allocation (static pools only)

## C90/ANSI C Compliance

### Compiler Settings
All code must compile cleanly with the following flags:
```bash
-ansi -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast
```

### C90 Requirements

#### Variable Declarations
```c
/* CORRECT - All declarations at beginning of block */
void function(void) {
  Word_t i = 0x0u;
  Byte_t *buffer = null;
  Size_t size = 0x0u;

  /* Code begins after all declarations */
  for(i = 0x0u; i < size; i++) {
    /* ... */
  }
}

/* INCORRECT - C99 style */
void function(void) {
  for(int i = 0; i < size; i++) {  /* ERROR: Declaration in for loop */
    /* ... */
  }
}
```

#### Comments
```c
/* CORRECT - C style comments only */
/* Multi-line comment
 * continues here
 */

// INCORRECT - C++ style comments not allowed in C90
```

#### Initializers
```c
/* CORRECT - Simple initializers */
Byte_t array[4] = {0x00u, 0x01u, 0x02u, 0x03u};

/* INCORRECT - Designated initializers (C99) */
struct Example ex = {.field = 5};  /* Not allowed */
```

## Code Formatting

### Uncrustify Configuration
All code must be formatted using the project's uncrustify configuration:
```bash
./extras/uncrustify/helios.cfg
```

### Indentation and Spacing
- Use 2 spaces for indentation (no tabs)
- Maximum line length: 120 characters
- Opening braces on same line for functions
- Spaces around operators

```c
/* CORRECT */
Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if(__PointerIsNull__(addr_) || (0x0u == size_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  /* Function body */

  FUNCTION_EXIT;
}
```

### Alignment
```c
/* Align structure members */
typedef struct Example_s {
  Word_t     field1;
  HalfWord_t field2;
  Byte_t     field3;
} Example_t;
```

## Naming Conventions

### Type Names

#### General Type Convention
All custom types end with `_t` suffix to clearly distinguish them from variables and functions:
```c
typedef Word_t HalfWord_t;  /* Basic type aliasing */
typedef struct Task_s Task_t;  /* Structure types */
typedef enum Return_e Return_t;  /* Enumeration types */
```

#### Structure Naming
Structures follow a three-part naming pattern:
1. **Tag name**: PascalCase with `_s` suffix
2. **Type name**: Same as tag but with `_t` suffix
3. **Members**: camelCase (no underscores)

```c
/* Structure definition pattern */
typedef struct DeviceName_s {
  HalfWord_t uid;                /* camelCase members */
  Byte_t name[CONFIG_SIZE];
  DeviceState_t state;
  Word_t bytesWritten;
  Word_t bytesRead;
  Base_t available;

  /* Function pointers follow same naming */
  Return_t (*init)(struct DeviceName_s *device_);
  Return_t (*read)(struct DeviceName_s *device_, Size_t *size_, Addr_t **data_);

  /* Self-referential pointer */
  struct DeviceName_s *next;
} DeviceName_t;
```

#### Enumeration Naming
Enumerations follow a specific pattern for both the type and values:
1. **Tag name**: PascalCase with `_e` suffix
2. **Type name**: Same as tag but with `_t` suffix
3. **Enum values**: PascalCase with type name as prefix (without _t)

```c
/* Enumeration definition pattern */
typedef enum TaskState_e {
  TaskStateSuspended,    /* Type name prefix + descriptive suffix */
  TaskStateRunning,
  TaskStateWaiting,
  TaskStateBlocked
} TaskState_t;

typedef enum DeviceMode_e {
  DeviceModeReadOnly,    /* Clear, hierarchical naming */
  DeviceModeWriteOnly,
  DeviceModeReadWrite
} DeviceMode_t;

typedef enum Return_e {
  ReturnOK,              /* Special case: shorter prefix for common type */
  ReturnError
} Return_t;
```

#### Include Guards for Types
All type definitions must use include guards with the type name in uppercase:
```c
#ifndef TASKSTATE_T_
  #define TASKSTATE_T_
  typedef enum TaskState_e {
    TaskStateSuspended,
    TaskStateRunning
  } TaskState_t;
#endif /* ifndef TASKSTATE_T_ */
```

#### Typedef Patterns
Different typedef patterns for various use cases:

```c
/* Basic type aliasing */
typedef unsigned char Byte_t;
typedef unsigned short HalfWord_t;
typedef unsigned int Word_t;

/* Function pointer types */
typedef Return_t (*TaskFunction_t)(TaskParm_t *parm_);
typedef Return_t (*DeviceInitFunction_t)(Device_t *device_);

/* Opaque types (void aliasing) */
typedef void TaskParm_t;
typedef void Addr_t;

/* Forward declarations */
typedef struct Task_s Task_t;  /* Forward declare when needed */

/* Complex nested structures */
typedef struct MemoryRegion_s {
  struct MemoryEntry_s {      /* Nested struct with _s suffix */
    Word_t magic;
    Byte_t free;
    HalfWord_t blocks;
  } entries[CONFIG_MAX_ENTRIES];
  Size_t totalSize;
  Size_t usedSize;
} MemoryRegion_t;
```

#### Union Naming (if used)
Unions follow the same pattern as structures:
```c
typedef union Register_u {     /* _u suffix for union tag */
  Word_t word;
  struct {
    HalfWord_t low;
    HalfWord_t high;
  } halfWords;
  Byte_t bytes[4];
} Register_t;                  /* _t suffix for type */
```

#### Naming Examples from HeliOS
```c
/* System state enumerations */
typedef enum SchedulerState_e {
  SchedulerStateSuspended,
  SchedulerStateRunning
} SchedulerState_t;

/* Device management structures */
typedef struct Device_s {
  HalfWord_t uid;
  Byte_t name[CONFIG_DEVICE_NAME_BYTES];
  DeviceState_t state;
  DeviceMode_t mode;
  /* ... */
} Device_t;

/* Task information structures */
typedef struct TaskInfo_s {
  Task_t *task;
  Byte_t *name;
  TaskState_t state;
  TaskFunction_t function;
  TaskParm_t *parm;
} TaskInfo_t;
```

### Function Names

#### Public API Functions (User-facing)
Prefix with 'x' for public APIs:
```c
Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);
Return_t xTaskCreate(Task_t **task_, const Byte_t *name_, TaskFunction_t function_, TaskParm_t *parm_);
```

#### FreeRTOS-style Task Functions
Prefix with 'v' for void task functions:
```c
void vConsoleTask(Task_t *task_, TaskParm_t *parm_);
void vIdleTask(Task_t *task_, TaskParm_t *parm_);
```

#### Kernel Internal Functions
Double underscore prefix and suffix:
```c
Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
Return_t __DeviceListFind__(const HalfWord_t uid_, Device_t **device_);
```

#### Module Private Functions
Static functions with double underscore prefix:
```c
static Return_t __ConsoleWriteString__(const Byte_t *str_);
static Return_t __ParseCommand__(const Byte_t *cmd_);
```

#### Device Driver Functions
Device drivers define a `DEVICE_NAME` macro and use it consistently:
```c
/* At the top of the driver source file */
#define DEVICE_NAME CHARDEV0  /* Define the device name once */

/* Self-registration function uses DEVICE_NAME with ## concatenation */
Return_t DEVICE_NAME##_self_register(void);

/* Interface functions use TO_FUNCTION macro with DEVICE_NAME */
Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);
Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);
Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);
Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);
Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);
Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);

/* Example: If DEVICE_NAME is CHARDEV0:
 * TO_FUNCTION(DEVICE_NAME, _init) expands to CHARDEV0_init
 * DEVICE_NAME##_self_register expands to CHARDEV0_self_register */
```

### Parameter Names
All parameters end with underscore:
```c
Return_t function(const Byte_t *buffer_, Size_t size_, Word_t *result_);
```

### Global Variables
Module-level static variables use camelCase:
```c
static ConsoleState_t consoleState;
static Volume_t *mountedVolume = null;
```

### Constants and Macros
All uppercase with underscores:
```c
#define CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS 0x1C20u
#define CHAR_NULL 0x00u
#define MAX_BUFFER_SIZE 0x100u
```

### Magic Numbers
All magic numbers must be defined with meaningful names:
```c
/* INCORRECT */
if(len > 256) { /* What is 256? */
  return ERROR;
}

/* CORRECT */
#define MAX_PATH_LENGTH 0x100u  /* 256 characters */

if(len > MAX_PATH_LENGTH) {
  return ERROR;
}
```

## Function Scope Control

### Public API (x-prefix)
User-facing functions that form the public API:
```c
/* In HeliOS.h file */
Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);

/* In header file */
Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);

/* In source file */
Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}
```

### Kernel Internal (__prefix__suffix__)
Functions accessible across kernel modules but not to users:
```c
/* In header file - marked as kernel internal */
Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
```

### Module Private (static)
Functions only accessible within a single source file:
```c
/* Forwward delcration in .c file */
static Return_t __ValidateInput__(const void *data_, Size_t size_);

/* Not in header file - only in .c file */
static Return_t __ValidateInput__(const void *data_, Size_t size_) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}
```

### Device Interface Functions
Device drivers use the `TO_FUNCTION` macro for interface implementation:
```c
/* Device self-registration function (no TO_FUNCTION) */
Return_t DEVICENAME_self_register(void);

/* Device interface functions use TO_FUNCTION macro */
Return_t TO_FUNCTION(DEVICENAME, _init)(Device_t *device_) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}

Return_t TO_FUNCTION(DEVICENAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}

Return_t TO_FUNCTION(DEVICENAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}

Return_t TO_FUNCTION(DEVICENAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}

Return_t TO_FUNCTION(DEVICENAME, _simple_read)(Device_t *device_, Byte_t *data_) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}

Return_t TO_FUNCTION(DEVICENAME, _simple_write)(Device_t *device_, Byte_t data_) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}

/* Example: Character device driver using DEVICE_NAME macro */
/* In the driver source file, DEVICE_NAME would be defined at the top */
Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_) {
  FUNCTION_ENTER;

  if(__PointerIsNull__(device_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  /* Initialize device state */
  device_->state = DeviceStateRunning;
  __ReturnOk__();

  FUNCTION_EXIT;
}

/* The TO_FUNCTION macro expands using the DEVICE_NAME definition:
 * If DEVICE_NAME is defined as CHARDEV0:
 * TO_FUNCTION(DEVICE_NAME, _init) becomes CHARDEV0_init
 * This ensures consistent naming across all device drivers */
```

## Defensive Programming

### MISRA C Compliance
HeliOS follows MISRA C guidelines where applicable to ensure safety and reliability:

#### Key MISRA Rules Enforced
- **Rule 14.7**: A function shall have a single point of exit at the end
  - Use `FUNCTION_ENTER` and `FUNCTION_EXIT` macros
  - Store return values and exit through single point

- **Rule 16.10**: If a function returns error information, that error shall be tested
  ```c
  /* INCORRECT - Ignoring return value */
  xMemAlloc(&buffer, size);  /* MISRA violation */

  /* CORRECT - Always check returns */
  if(OK(xMemAlloc(&buffer, size))) {
    /* Handle success */
  } else {
    /* Handle error */
  }
  ```

- **Rule 17.4**: Array indexing shall be the only allowed form of pointer arithmetic
  ```c
  /* INCORRECT */
  ptr = ptr + offset;  /* Direct pointer arithmetic */

  /* CORRECT */
  data = array[index];  /* Array indexing */
  ```

- **Rule 20.4**: Dynamic heap memory allocation shall not be used
  - HeliOS uses static memory pools instead of malloc/free
  - Memory is allocated from pre-defined regions

- **Rule 21.1**: Minimization of run-time failures shall be ensured by defensive programming
  - All inputs validated
  - All array bounds checked
  - All pointers verified before dereferencing

### Pointer Validation
Always validate pointers before use:
```c
Return_t function(void *ptr_, Size_t *size_) {
  FUNCTION_ENTER;

  /* Validate all input pointers */
  if(__PointerIsNull__(ptr_) || __PointerIsNull__(size_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  /* Safe to use pointers now */

  FUNCTION_EXIT;
}
```

### Size and Range Validation
```c
Return_t function(Size_t size_, Word_t index_) {
  FUNCTION_ENTER;

  /* Validate size is non-zero */
  if(0x0u == size_) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  /* Validate index is in range */
  if(index_ >= MAX_INDEX) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  FUNCTION_EXIT;
}
```

### Buffer Overflow Prevention
```c
/* Always use bounded string operations */
Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_) {
  FUNCTION_ENTER;

  Size_t i = 0x0u;

  if(__PointerIsNull__(dest_) || __PointerIsNull__(src_) || (0x0u == destSize_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  /* Copy up to destSize_ - 1 to leave room for null terminator */
  while((CHAR_NULL != src_[i]) && (i < (destSize_ - 0x1u))) {
    dest_[i] = src_[i];
    i++;
  }

  dest_[i] = CHAR_NULL;  /* Always null terminate */
  __ReturnOk__();

  FUNCTION_EXIT;
}
```

### State Validation
```c
if(DeviceStateRunning != device->state) {
  __ReturnError__();
  __AssertOnElse__();
  FUNCTION_EXIT;
}
```

### Additional MISRA C Guidelines

#### Control Flow
- **No goto statements** (MISRA Rule 15.1)
- **No recursion** (MISRA Rule 17.2)
- **Switch statements must have default case** (MISRA Rule 15.3)
```c
switch(state) {
  case STATE_IDLE:
    ProcessIdle();
    break;
  case STATE_ACTIVE:
    ProcessActive();
    break;
  default:
    /* Handle unexpected state */
    __AssertOnElse__();
    break;
}
```

#### Type Safety
- **No implicit type conversions** (MISRA Rule 10.1)
```c
/* INCORRECT */
Word_t value = 0.5;  /* Implicit float to int */

/* CORRECT */
Word_t value = (Word_t)0x0u;  /* Explicit type */
```

#### Loop Safety
- **Loop counters shall not be modified in loop body** (MISRA Rule 13.6)
```c
/* INCORRECT */
for(i = 0x0u; i < size; i++) {
  if(condition) {
    i++;  /* MISRA violation - modifying loop counter */
  }
}

/* CORRECT */
for(i = 0x0u; i < size; i++) {
  /* Loop counter only modified by for statement */
}
```

#### Preprocessor Usage
- **#define shall not be used for code blocks** (MISRA Rule 19.4)
- **Macros shall be parenthesized** (MISRA Rule 19.10)
```c
/* INCORRECT */
#define SQUARE(x) x * x  /* Missing parentheses */

/* CORRECT */
#define SQUARE(x) ((x) * (x))  /* Properly parenthesized */
```

## Error Handling

### Return Value Convention
All functions that can fail must return `Return_t`:
```c
typedef enum Return_e {
  RETURN_OK = 0,
  RETURN_ERROR = -1
} Return_t;
```

### Error Handling Macros
```c
/* Success return */
__ReturnOk__();

/* Error return */
__ReturnError__();

/* Assert on else clause */
if(OK(result)) {
  /* Success path */
} else {
  __AssertOnElse__();  /* Log assertion in debug builds */
}
```

### Error Propagation
```c
Return_t function(void) {
  FUNCTION_ENTER;

  if(OK(xMemAlloc(&buffer, size))) {
    if(OK(ProcessBuffer(buffer))) {
      xMemFree(buffer);
      __ReturnOk__();
    } else {
      xMemFree(buffer);  /* Clean up on error */
      __ReturnError__();
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}
```

### Resource Cleanup
Always clean up resources on error:
```c
Return_t function(void) {
  FUNCTION_ENTER;

  File_t *file = null;
  Byte_t *buffer = null;

  if(OK(xFileOpen(&file, path, mode))) {
    if(OK(xMemAlloc(&buffer, size))) {
      if(OK(xFileRead(file, size, &buffer))) {
        /* Process data */
        xMemFree(buffer);
        xFileClose(file);
        __ReturnOk__();
      } else {
        xMemFree(buffer);     /* Clean up buffer */
        xFileClose(file);     /* Clean up file */
        __ReturnError__();
        __AssertOnElse__();
      }
    } else {
      xFileClose(file);       /* Clean up file */
      __ReturnError__();
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}
```

## Memory Management

### Allocation Pattern
```c
Byte_t *buffer = null;

if(OK(xMemAlloc((volatile Addr_t **)&buffer, size))) {
  /* Use buffer */

  /* Always free when done */
  xMemFree(buffer);
} else {
  __ReturnError__();
  __AssertOnElse__();
}
```

### Initialization
Always initialize pointers to null:
```c
void function(void) {
  Byte_t *buffer = null;  /* Always initialize to null */
  Word_t count = 0x0u;    /* Always initialize variables */

  /* ... */
}
```

### Double-Free Prevention
```c
if(__PointerIsNotNull__(ptr)) {
  xMemFree(ptr);
  ptr = null;  /* Set to null after free */
}
```

## Documentation Standards

### File Headers
Every source file must have a Doxygen header:
```c
/*UNCRUSTIFY-OFF*/
/**
 * @file filename.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Brief description of the file's purpose
 * @version 0.5.0
 * @date 2025-01-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
```

### Function Documentation
All public and kernel functions must be documented:
```c
/**
 * @brief Allocate memory from the heap
 * @param addr_ Double pointer to store allocated address
 * @param size_ Size in bytes to allocate
 * @return Return_t OK on success, ERROR on failure
 *
 * @note This function is thread-safe
 * @warning Size must be non-zero
 *
 * @code
 * Byte_t *buffer = null;
 * if(OK(xMemAlloc((volatile Addr_t **)&buffer, 256))) {
 *   // Use buffer
 *   xMemFree(buffer);
 * }
 * @endcode
 */
Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_) {
  /* Implementation */
}
```

### Inline Comments
```c
/* Explain complex logic */
if((state == STATE_READY) && (count > threshold)) {
  /* Transition to active state when threshold exceeded
   * This prevents spurious activations from noise */
  state = STATE_ACTIVE;
}

/* TODO: Add support for feature X */
/* FIXME: Handle edge case Y */
/* NOTE: This assumes Z is already initialized */
```

### Section Separators
```c
/* ============================================================================
 * PUBLIC API FUNCTIONS
 * ============================================================================
 */

/* ============================================================================
 * KERNEL INTERNAL FUNCTIONS
 * ============================================================================
 */

/* ============================================================================
 * MODULE PRIVATE FUNCTIONS
 * ============================================================================
 */
```

## File Organization

### Header Files (.h)
```c
#ifndef MODULE_H_
  #define MODULE_H_

  /* Includes */
  #include "config.h"
  #include "defines.h"
  #include "types.h"

  /* Constants */
  #define MODULE_CONSTANT 0x100u

  /* Type definitions */
  typedef struct ModuleState_s {
    /* members */
  } ModuleState_t;

  /* C++ compatibility */
  #ifdef __cplusplus
    extern "C" {
  #endif

  /* Public function declarations */
  Return_t xModuleInit(void);
  Return_t xModuleProcess(void *data_);

  /* Kernel internal function declarations */
  Return_t __ModuleInternalFunction__(void);

  #ifdef __cplusplus
    }
  #endif

#endif /* ifndef MODULE_H_ */
```

### Source Files (.c)
```c
/* File header */

/* Includes - system headers first, then project headers */
#include "module.h"
#include "mem.h"
#include "device.h"

/* Module constants */
#define INTERNAL_BUFFER_SIZE 0x80u

/* Module state */
static ModuleState_t moduleState;

/* Forward declarations for private functions */
static Return_t __InitializeState__(void);
static Return_t __ProcessData__(const Byte_t *data_, Size_t size_);

/* ============================================================================
 * PUBLIC API FUNCTIONS
 * ============================================================================
 */

Return_t xModuleInit(void) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}

/* ============================================================================
 * KERNEL INTERNAL FUNCTIONS
 * ============================================================================
 */

Return_t __ModuleInternalFunction__(void) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}

/* ============================================================================
 * MODULE PRIVATE FUNCTIONS
 * ============================================================================
 */

static Return_t __InitializeState__(void) {
  FUNCTION_ENTER;
  /* Implementation */
  FUNCTION_EXIT;
}
```

## Testing Standards

### Test Coverage Requirements

#### Minimum Coverage Targets
All HeliOS code must meet the following test coverage requirements:

- **Line Coverage**: Minimum 80% of executable lines
- **Branch Coverage**: Minimum 75% of decision branches
- **Function Coverage**: 100% of public API functions
- **MC/DC Coverage**: For safety-critical modules, Modified Condition/Decision Coverage required

#### Coverage Measurement
```bash
# Generate coverage report during test build
gcc --coverage -fprofile-arcs -ftest-coverage -o test

# Run tests and generate report
./test
gcov source.c
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

#### Required Test Categories

##### 1. Functional Tests
Every public function must have tests for:
- **Normal operation** with valid inputs
- **Boundary conditions** at min/max values
- **Error conditions** with invalid inputs
- **State transitions** if applicable

##### 2. Negative Tests
- **Null pointer handling**
- **Zero size/length handling**
- **Buffer overflow attempts**
- **Invalid state operations**
- **Out-of-range parameters**

##### 3. Edge Case Tests
- **Maximum allocation size**
- **Empty data structures**
- **Single element operations**
- **Concurrent access (if applicable)**

##### 4. Integration Tests
- **Module interaction**
- **Resource sharing**
- **Error propagation between modules**

#### Test Documentation
Each test must document:
```c
/**
 * @test Test_MemoryAllocation_NullPointer
 * @brief Verify xMemAlloc handles null pointer correctly
 * @details Tests that passing null pointer to xMemAlloc returns ERROR
 *          and triggers appropriate assertion
 * @expected Function returns ERROR, assertion logged
 * @coverage xMemAlloc: null pointer path
 */
```

### Unit Test Structure
```c
void test_module_functionality(void) {
  unit_print("--- Section: Module Functionality ---");

  /* Test 1: Normal operation */
  unit_begin("Module initializes successfully");
  unit_assert_ok(xModuleInit());
  unit_end();

  /* Test 2: Error handling */
  unit_begin("Module handles null pointer");
  unit_assert_error(xModuleProcess(null));
  unit_end();

  /* Test 3: Edge cases */
  unit_begin("Module handles maximum size");
  unit_assert_ok(xModuleProcess(data, MAX_SIZE));
  unit_end();
}
```

### Test Helpers
```c
/* Mock device for testing */
static void __SetupTestEnvironment__(void) {
  /* Initialize test state */
  __ConsoleStateClear__();
  xMockDeviceReset();

  /* Configure test environment */
  /* ... */
}
```

### Assertions in Tests
```c
unit_assert_true(condition);
unit_assert_false(condition);
unit_assert_ok(function());
unit_assert_error(function());
unit_assert_equals(actual, expected);
unit_assert_not_null(pointer);
```

### Test-Driven Development (TDD)
HeliOS encourages Test-Driven Development:

1. **Write the test first** - Define expected behavior
2. **Run test and verify it fails** - Ensure test is valid
3. **Write minimal code to pass** - Implement functionality
4. **Refactor** - Improve code while keeping tests green
5. **Repeat** - Continue cycle for next requirement

#### Example TDD Workflow
```c
/* Step 1: Write failing test */
void test_new_feature(void) {
  unit_begin("New feature returns correct value");
  unit_assert_equals(xNewFeature(5), 10);  /* Will fail - not implemented */
  unit_end();
}

/* Step 2: Implement minimal code */
Return_t xNewFeature(Word_t input_) {
  return input_ * 2;  /* Minimal implementation */
}

/* Step 3: Refactor with confidence */
Return_t xNewFeature(Word_t input_) {
  FUNCTION_ENTER;

  /* Add validation and error handling */
  if(input_ > MAX_INPUT) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  return input_ * 2;

  FUNCTION_EXIT;
}
```

### Coverage Exceptions
Certain code paths may be excluded from coverage requirements:
- **Hardware-specific code** that cannot run in test environment
- **Catastrophic error handlers** (e.g., xSystemHalt)
- **Platform-specific assembly code**

These exceptions must be documented with:
```c
/* COVERAGE_EXCLUDE_START - Hardware-specific code */
void hardware_specific_function(void) {
  /* Code that cannot be tested in POSIX environment */
}
/* COVERAGE_EXCLUDE_END */
```

## Build and Compilation

### Debug Build
```bash
gcc -O0 -ggdb -DPOSIX_ARCH_OTHER -DCONFIG_ENABLE_SYSTEM_ASSERT
```

### Release Build
```bash
gcc -O2 -DNDEBUG
```

### Required Defines
```c
/* Platform-specific */
-DPOSIX_ARCH_OTHER      /* POSIX testing environment */
-DARM_ARCH_CORTEX_M     /* ARM Cortex-M target */

/* Configuration */
-DCONFIG_MEMORY_REGION_SIZE_IN_BLOCKS=0x1C20u
-DCONFIG_ENABLE_SYSTEM_ASSERT

/* Testing */
-DUNIT_TEST_COLORIZE    /* Colored test output */
```

## Code Review Checklist

Before submitting code for review, ensure:

### Compilation and Standards
- [ ] Code compiles without warnings with `-Wall -Wextra -ansi -pedantic`
- [ ] C90/ANSI C compliant (no C99/C11 features)
- [ ] Code formatted with uncrustify

### MISRA C Compliance
- [ ] No goto statements used (Rule 15.1)
- [ ] No recursion used (Rule 17.2)
- [ ] All switch statements have default case (Rule 15.3)
- [ ] All function return values are checked (Rule 16.10)
- [ ] No direct pointer arithmetic (Rule 17.4)
- [ ] Loop counters not modified in loop body (Rule 13.6)
- [ ] Macros properly parenthesized (Rule 19.10)

### Defensive Programming
- [ ] All pointers validated with `__PointerIsNull__()` before use
- [ ] All array bounds checked
- [ ] All size parameters validated as non-zero
- [ ] Buffer overflow protection in place
- [ ] State validation before operations

### Code Structure
- [ ] All functions have `FUNCTION_ENTER`/`FUNCTION_EXIT` macros
- [ ] Single point of exit for all functions
- [ ] Function naming follows scope conventions (x, __, static, etc.)
- [ ] All magic numbers defined as constants
- [ ] Variable declarations at beginning of blocks (C90)

### Error Handling
- [ ] All error paths clean up resources properly
- [ ] Error conditions use `__AssertOnElse__()`
- [ ] Return values follow `Return_t` convention
- [ ] No ignored return values

### Documentation
- [ ] All public functions documented with Doxygen
- [ ] File header with copyright and SPDX identifier
- [ ] Complex algorithms have explanatory comments
- [ ] Comments explain "why" not "what"
- [ ] TODO/FIXME items clearly marked

### Testing
- [ ] Unit tests written for all new functions
- [ ] Line coverage ≥ 80%
- [ ] Branch coverage ≥ 75%
- [ ] Function coverage = 100% for public APIs
- [ ] All error paths tested
- [ ] Boundary conditions tested
- [ ] Null pointer cases tested
- [ ] Integration with existing modules tested

### Memory Management
- [ ] No memory leaks (verified with valgrind if applicable)
- [ ] All allocated memory freed on all paths
- [ ] Pointers set to null after free
- [ ] No use of dynamic allocation (malloc/free)

### Performance
- [ ] No unnecessary copying of large structures
- [ ] Efficient algorithms used (document O(n) complexity if >O(n))
- [ ] Resource usage within defined limits

## Examples

### Complete Function Example - MISRA Compliant Single Exit Point

```c
/**
 * @brief Process data buffer with validation (MISRA compliant)
 * @param buffer_ Input data buffer
 * @param size_ Size of buffer in bytes
 * @param result_ Pointer to store result
 * @return Return_t OK on success, ERROR on failure
 *
 * @note This implementation follows MISRA Rule 14.7 with single exit point
 */
Return_t xProcessData(const Byte_t *buffer_, Size_t size_, Word_t *result_) {
  FUNCTION_ENTER;

  Return_t retVal = RETURN_ERROR;  /* Default to error for safety */
  Word_t checksum = 0x0u;
  Size_t i = 0x0u;

  /* Validate input parameters */
  if(__PointerIsNull__(buffer_) || __PointerIsNull__(result_) || (0x0u == size_)) {
    retVal = RETURN_ERROR;
    __AssertOnElse__();
  }
  /* Validate size is within limits */
  else if(size_ > MAX_BUFFER_SIZE) {
    retVal = RETURN_ERROR;
    __AssertOnElse__();
  }
  else {
    /* Process data */
    for(i = 0x0u; i < size_; i++) {
      checksum ^= buffer_[i];
    }

    /* Store result */
    *result_ = checksum;
    retVal = RETURN_OK;
  }

  /* Single exit point - MISRA Rule 14.7 compliant */
  FUNCTION_EXIT;
  return retVal;
}

/**
 * Note: While HeliOS uses __ReturnOk__() and __ReturnError__() macros
 * in many places for convenience, strict MISRA compliance requires
 * the single exit point pattern shown above. Choose the approach based
 * on your project's safety requirements:
 *
 * - For safety-critical code: Use single exit point pattern
 * - For non-critical code: HeliOS macros may be acceptable
 *
 * Example with HeliOS macros (multiple exit points - not MISRA compliant):
 *
 *   if(error_condition) {
 *     __ReturnError__();
 *     __AssertOnElse__();
 *     FUNCTION_EXIT;
 *   }
 *
 *   __ReturnOk__();
 *   FUNCTION_EXIT;
 */
```

### Module Template
See `/templates/module_template.c` and `/templates/module_template.h` for complete examples.

## Enforcement

This coding standard is enforced through:

### 1. Automated Tools
- **Uncrustify**: Automatic code formatting
- **GCC Warnings**: `-Wall -Wextra -ansi -pedantic`
- **Static Analysis**:
  - `cppcheck --enable=all --std=c90`
  - `splint` for additional static checking
  - `pc-lint` for MISRA C compliance checking
- **Coverage Tools**:
  - `gcov` for coverage measurement
  - `lcov` for HTML report generation

### 2. Continuous Integration
- All commits trigger automated checks:
  - Compilation with strict warnings
  - Unit test execution
  - Coverage report generation
  - Static analysis scan

### 3. Code Review Process
- Peer review required for all changes
- Checklist must be completed
- MISRA C compliance verified
- Test coverage reviewed

### 4. Quality Gates
Code must pass quality gates before merge:
- Zero compiler warnings
- Unit tests pass
- Coverage thresholds met (80% line, 75% branch)
- No critical static analysis issues
- MISRA C mandatory rules compliance

## Revision History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0.0 | 2025-01-19 | HeliOS Team | Initial coding standard |
| 1.1.0 | 2025-01-19 | HeliOS Team | Added MISRA C compliance guidelines and test coverage requirements |

## References

### Standards and Guidelines
- [ISO/IEC 9899:1990](https://www.iso.org/standard/17782.html) - C90 Standard
- [MISRA C:2012](https://www.misra.org.uk/) - Guidelines for the use of C in critical systems
- [DO-178C](https://www.rtca.org/) - Software Considerations in Airborne Systems
- [NASA C Style Guide](https://ntrs.nasa.gov/citations/20080039927) - JPL C Coding Standard
- [CERT C Coding Standard](https://wiki.sei.cmu.edu/confluence/display/c) - SEI CERT C Coding Standard

### Tools and Documentation
- [Doxygen Manual](https://www.doxygen.nl/manual/) - Documentation generation
- [GCC Documentation](https://gcc.gnu.org/onlinedocs/) - Compiler options and warnings
- [Gcov Documentation](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html) - Code coverage tool
- [Uncrustify](http://uncrustify.sourceforge.net/) - Source code beautifier
- [CppCheck](http://cppcheck.sourceforge.net/) - Static analysis tool

### Testing Resources
- [Test-Driven Development](https://www.amazon.com/Test-Driven-Development-Kent-Beck/dp/0321146530) - Kent Beck
- [Code Coverage Best Practices](https://testing.googleblog.com/2020/08/code-coverage-best-practices.html) - Google Testing Blog
- [MC/DC Coverage](https://www.mathworks.com/help/slcoverage/ug/modified-condition-decision-coverage.html) - Modified Condition/Decision Coverage

---

*This document is part of the HeliOS Embedded Operating System project.*
*For questions or clarifications, contact the HeliOS development team.*