# Enhanced Assertion Macros Guide

## Overview

The HeliOS unit testing framework now includes enhanced assertion macros that provide detailed diagnostic information when tests fail. These new assertions complement the existing `unit_try()` function by showing expected vs actual values, file locations, and line numbers.

## Available Assertion Macros

### Equality Checks

**`unit_assert_equal(actual, expected)`**
- Verifies that two values are equal
- On failure: Shows both values in hexadecimal and the expression that failed
- Example:
  ```c
  Size_t size = 0x100;
  unit_assert_equal(size, 0x100);  // Passes
  unit_assert_equal(size, 0x200);  // Fails with diagnostic
  ```

**`unit_assert_not_equal(actual, expected)`**
- Verifies that two values are not equal
- On failure: Shows that both values are unexpectedly equal
- Example:
  ```c
  Size_t size1 = 0x100;
  Size_t size2 = 0x200;
  unit_assert_not_equal(size1, size2);  // Passes
  ```

### Pointer Checks

**`unit_assert_null(ptr)`**
- Verifies that a pointer is NULL
- On failure: Shows the pointer address
- Example:
  ```c
  Base_t *ptr = null;
  unit_assert_null(ptr);  // Passes
  ```

**`unit_assert_not_null(ptr)`**
- Verifies that a pointer is not NULL
- On failure: Indicates pointer was unexpectedly NULL
- Example:
  ```c
  Base_t *ptr;
  xMemAlloc((volatile Addr_t **)&ptr, 128);
  unit_assert_not_null(ptr);  // Passes if allocation succeeded
  ```

### Boolean/Condition Checks

**`unit_assert_true(condition)`**
- Verifies that a condition evaluates to true
- On failure: Shows the condition expression
- Example:
  ```c
  Size_t size = 0x100;
  unit_assert_true(size > 0);  // Passes
  unit_assert_true(size == 0x100);  // Passes
  ```

**`unit_assert_false(condition)`**
- Verifies that a condition evaluates to false
- On failure: Shows the condition expression
- Example:
  ```c
  Size_t size = 0x100;
  unit_assert_false(size == 0);  // Passes
  ```

### Return Value Checks

**`unit_assert_ok(result)`**
- Verifies that a function returns OK status
- Convenience wrapper around `unit_assert_true(OK(result))`
- Example:
  ```c
  unit_assert_ok(xSystemInit());
  unit_assert_ok(xMemAlloc(&ptr, 128));
  ```

**`unit_assert_not_ok(result)`**
- Verifies that a function does NOT return OK status
- Convenience wrapper around `unit_assert_false(OK(result))`
- Example:
  ```c
  unit_assert_not_ok(xMemAlloc(null, 128));  // Should fail with NULL pointer
  ```

## Diagnostic Output Examples

### When unit_assert_equal() Fails:

```
unit: FAILED at memory_1_harness.c:84
unit:    Expected: size == 0x32020
unit:    Actual:   0x32000 != 0x32020
```

### When unit_assert_not_null() Fails:

```
unit: FAILED at task_harness.c:53
unit:    Expected: task != NULL
unit:    Actual:   NULL
```

### When unit_assert_true() Fails:

```
unit: FAILED at sys_harness.c:50
unit:    Expected: info->productName[0] != '\0' == true
unit:    Actual:   false
```

## Migration Guide

### Old Style (using unit_try):
```c
unit_begin("Memory allocation test");
Base_t *ptr = null;
unit_try(OK(xMemAlloc((volatile Addr_t **)&ptr, 128)));
unit_try(null != ptr);
unit_try(OK(xMemFree(ptr)));
unit_end();
```

### New Style (using enhanced assertions):
```c
unit_begin("Memory allocation succeeds for 128 bytes");
Base_t *ptr = null;
unit_assert_ok(xMemAlloc((volatile Addr_t **)&ptr, 128));
unit_assert_not_null(ptr);
unit_assert_ok(xMemFree(ptr));
unit_end();
```

## Benefits

1. **Immediate Diagnostics**: See exactly which value was wrong without adding debug prints
2. **File and Line Numbers**: Quickly locate the failing assertion
3. **Expression Context**: See the actual expression that was evaluated
4. **Better Debugging**: No need to re-run with added logging to understand failures

## Backward Compatibility

The existing `unit_try()` function is still fully supported. You can mix old and new style assertions in the same test. The enhanced assertions are simply additional tools in your testing toolkit.

## Example: sys_harness.c

The sys_harness.c file demonstrates the enhanced assertions in action:

```c
void test_system_info(void) {
  SystemInfo_t *info = null;

  unit_begin("System information retrieval returns valid product data");

  /* Using enhanced assertions */
  unit_assert_ok(xSystemGetSystemInfo(&info));
  unit_assert_not_null(info);
  unit_assert_true(info->productName[0] != '\0');
  unit_assert_ok(xMemFree(info));

  unit_end();

  /* Test NULL pointer handling */
  unit_begin("System info retrieval rejects NULL pointer");
  unit_assert_not_ok(xSystemGetSystemInfo(null));
  unit_end();
}
```

## Implementation Details

- All enhanced assertions are implemented as macros that capture `__FILE__` and `__LINE__`
- Values are cast to `unsigned long` for uniform handling (C90 compliant)
- Colorized output uses ANSI escape codes (when `UNIT_TEST_COLORIZE` is defined)
- Failed assertions mark the test as failed but allow the test to continue (soft assertions)
- Can be used anywhere between `unit_begin()` and `unit_end()`
- Fully compliant with ISO C90 standard (compiles with `-ansi -pedantic`)
