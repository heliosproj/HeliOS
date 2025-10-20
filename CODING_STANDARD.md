# HeliOS Coding Standard

Version 2.0.0 | 2025-01-20

## Overview

HeliOS follows strict C90/ANSI C standards for maximum portability across embedded platforms. This document defines the coding conventions used throughout the project.

### Key Requirements
- **C90/ANSI C** strict compliance (no C99/C11 features)
- **No dynamic memory allocation** (static pools only)
- **Defensive programming** with comprehensive validation
- **Test coverage**: 80% line, 75% branch, 100% public API
- **MISRA C** guidelines where applicable

## Code Formatting

### Uncrustify
All code must be formatted with uncrustify using the project configuration:
```bash
uncrustify -c .uncrustify.cfg --no-backup src/*.c src/*.h
```

Files should include uncrustify markers:
```c
/*UNCRUSTIFY-OFF*/
/**
 * @file mem.c
 * @brief Memory management module
 */
/*UNCRUSTIFY-ON*/
```

### Indentation and Spacing
- 2 spaces for indentation (no tabs)
- 120 character line limit
- Opening braces on same line for control structures
- Function opening braces on new line

## Naming Conventions

### Constants and Macros
```c
#define CONFIG_MEMORY_REGION_SIZE 0x1000u  /* Configuration constants */
#define MAX_BUFFER_SIZE 0x100u              /* General constants */
#define CHAR_NULL 0x00u                     /* Character constants */
```

### Type Names
All custom types end with `_t`:

#### Structures
```c
typedef struct DeviceName_s {    /* Tag: PascalCase_s */
  HalfWord_t uid;                /* Members: camelCase */
  Byte_t name[CONFIG_SIZE];
  DeviceState_t state;
  Word_t bytesWritten;
  struct DeviceName_s *next;
} DeviceName_t;                  /* Type: PascalCase_t */
```

#### Enumerations
```c
typedef enum TaskState_e {       /* Tag: PascalCase_e */
  TaskStateSuspended,           /* Values: TypePrefix + Description */
  TaskStateRunning,
  TaskStateWaiting
} TaskState_t;                   /* Type: PascalCase_t */

typedef enum Return_e {
  ReturnOK,                     /* Short prefix for common types */
  ReturnError
} Return_t;
```

#### Include Guards
```c
#ifndef TASKSTATE_T_
  #define TASKSTATE_T_
  typedef enum TaskState_e {
    /* ... */
  } TaskState_t;
#endif /* ifndef TASKSTATE_T_ */
```

### Function Names

#### Public API (User-facing)
Prefix with 'x':
```c
Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);
Return_t xTaskCreate(Task_t **task_, const Byte_t *name_);
```

#### Kernel Internal
Wrapped with double underscores:
```c
Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
Return_t __MemoryInit__(void);
```

#### Module Internal
Static functions with single underscore prefix:
```c
static Return_t _ValidateBuffer(const Byte_t *buffer_, Size_t size_);
static void _InitializeDevice(Device_t *device_);
```

#### Device Drivers
Use `TO_FUNCTION` macro with `DEVICE_NAME`:
```c
#define DEVICE_NAME CHARDEV0  /* Define once at file top */

Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);
Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);
```

### Variable Names

#### Parameters
All parameters end with underscore:
```c
Return_t function(const Byte_t *buffer_, Size_t size_, Word_t *result_);
```

#### Local Variables
Simple names without underscores:
```c
Base_t found = false;
HalfWord_t blocks = nil;
MemoryEntry_t *cursor = region_->start;
Size_t i = 0x0u;
```

#### Global/Static Variables
Descriptive names in camelCase:
```c
static CharDeviceState_t deviceState = {0};
static HalfWord_t cachedDeviceUID = 0x0u;
```

## Function Structure

### Return Pattern
Use HeliOS return macros:
```c
Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(addr_) && (nil < size_)) {
    if(OK(__calloc__(&heap, addr_, size_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}
```

### Non-Return_t Functions
Direct return for value-returning functions:
```c
Size_t __strlen__(const Byte_t *str_) {
  Size_t len = 0x0u;

  if(__PointerIsNotNull__(str_)) {
    while(CHAR_NULL != str_[len]) {
      len++;
    }
  }

  return len;
}
```

## Defensive Programming

### Pointer Validation
```c
if(__PointerIsNull__(ptr_)) {
  __ReturnError__();
  __AssertOnElse__();
  FUNCTION_EXIT;
}
```

### Bounds Checking
```c
if(index_ >= MAX_INDEX) {
  __ReturnError__();
  __AssertOnElse__();
  FUNCTION_EXIT;
}
```

### Buffer Safety
```c
/* Always use bounded operations */
Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_) {
  FUNCTION_ENTER;

  Size_t i = 0x0u;

  if(__PointerIsNull__(dest_) || __PointerIsNull__(src_) || (0x0u == destSize_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  /* Copy with bounds check */
  while((CHAR_NULL != src_[i]) && (i < (destSize_ - 0x1u))) {
    dest_[i] = src_[i];
    i++;
  }

  dest_[i] = CHAR_NULL;  /* Always null terminate */
  __ReturnOk__();

  FUNCTION_EXIT;
}
```

## Memory Management

### Static Allocation
```c
/* Use static pools, never malloc/free */
static Byte_t memoryPool[CONFIG_MEMORY_POOL_SIZE];
static Task_t taskPool[CONFIG_MAX_TASKS];
```

### Memory Functions
```c
/* Use HeliOS memory functions */
xMemAlloc(&buffer, size);      /* Allocate from heap */
xMemFree(buffer);              /* Free to heap */
__memset__(buffer, 0x00u, size);  /* Set memory */
__memcpy__(dest, src, size);      /* Copy memory */
```

## Special Values

### Null Pointers
Use `null` (lowercase):
```c
MemoryEntry_t *entry = null;  /* NOT NULL */
if(null == ptr) { /* ... */ }
```

### Zero Values
Use `nil`:
```c
HalfWord_t count = nil;
if(nil < size) { /* ... */ }
```

### Boolean Values
Use `true`/`false`:
```c
Base_t initialized = false;
while(true) { /* ... */ }
```

## Magic Numbers

Define all magic numbers as constants:
```c
/* INCORRECT */
if(size > 256) { /* ... */ }

/* CORRECT */
#define MAX_BUFFER_SIZE 0x100u
if(size > MAX_BUFFER_SIZE) { /* ... */ }
```

## Documentation

### File Headers
```c
/**
 * @file module.c
 * @author Name <email@domain.org>
 * @brief Brief description
 * @version 0.5.0
 * @date 2025-01-20
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
```

### Function Documentation
```c
/**
 * @brief Process data buffer with validation
 * @param buffer_ Input data buffer
 * @param size_ Size in bytes
 * @param result_ Output result pointer
 * @return Return_t OK on success, ERROR on failure
 */
```

### Inline Comments
```c
/* Check if memory entry is valid */
if(__MemEntryMagicOk__(entry)) {
  /* Process the entry... */
}
```

## Testing

### Unit Tests
All public API functions must have tests:
```c
static Return_t TestMemAlloc(void) {
  FUNCTION_ENTER;

  Addr_t *buffer = null;

  /* Test normal allocation */
  __TestAssertTrue__(OK(xMemAlloc(&buffer, 0x100u)));
  __TestAssertNotNull__(buffer);

  /* Test cleanup */
  __TestAssertTrue__(OK(xMemFree(buffer)));

  __ReturnOk__();
  FUNCTION_EXIT;
}
```

### Coverage Requirements
- Line coverage: ≥80%
- Branch coverage: ≥75%
- Public API coverage: 100%

## Build System

### Compiler Flags
```makefile
CFLAGS = -ansi -pedantic -Wall -Wextra -Werror
CFLAGS += -Wno-unused-parameter -Wno-pointer-to-int-cast
```

### Static Analysis
- Run `cppcheck --enable=all --std=c90`
- Address all warnings before commit

## Code Review Checklist

- [ ] C90 compliance verified
- [ ] No dynamic memory allocation
- [ ] All pointers validated
- [ ] Magic numbers defined as constants
- [ ] Functions use FUNCTION_ENTER/EXIT
- [ ] Parameters end with underscore
- [ ] Types end with _t
- [ ] Public APIs start with x
- [ ] Kernel functions wrapped with __
- [ ] Code formatted with uncrustify
- [ ] Doxygen comments complete
- [ ] Unit tests written
- [ ] Coverage thresholds met

## Examples

### Complete Module Example
```c
/*UNCRUSTIFY-OFF*/
/**
 * @file example.c
 * @brief Example module implementation
 */
/*UNCRUSTIFY-ON*/
#include "example.h"

#define BUFFER_SIZE 0x100u
#define MAX_RETRIES 0x03u

static Byte_t internalBuffer[BUFFER_SIZE];

/**
 * @brief Initialize example module
 * @return Return_t OK on success
 */
Return_t xExampleInit(void) {
  FUNCTION_ENTER;

  __memset__(internalBuffer, nil, BUFFER_SIZE);
  __ReturnOk__();

  FUNCTION_EXIT;
}

/**
 * @brief Process example data
 * @param data_ Input data
 * @param size_ Data size
 * @return Return_t OK on success, ERROR on invalid input
 */
Return_t xExampleProcess(const Byte_t *data_, Size_t size_) {
  FUNCTION_ENTER;

  Size_t i = nil;

  if(__PointerIsNull__(data_) || (nil == size_) || (size_ > BUFFER_SIZE)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  /* Process the data */
  for(i = nil; i < size_; i++) {
    internalBuffer[i] = data_[i] ^ 0xAAu;
  }

  __ReturnOk__();

  FUNCTION_EXIT;
}
```

## Version History

| Version | Date | Description |
|---------|------|-------------|
| 2.0.0 | 2025-01-20 | Streamlined and aligned with actual HeliOS code |
| 1.1.0 | 2025-01-19 | Added MISRA C and test coverage |
| 1.0.0 | 2025-01-19 | Initial standard |