/*UNCRUSTIFY-OFF*/
/**
 * @file memory_2_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#include "memory_2_harness.h"

/* Test constants */
#define TEST_ALLOC_SIZE         128         /* Allocation size for consistency tests */
#define EXPECTED_USED_SIZE      160         /* Expected memory used (128 + overhead) */
#define CORRUPT_VALUE           123         /* Arbitrary value for corruption tests */
#define CORRUPT_POINTER         823829342   /* Arbitrary invalid pointer value */


void memory_2_harness(void) {
  /*UNCRUSTIFY-OFF*/
  /*
    1) Create something in the heap
    2) Call a memory syscall like xMemGetUsed() which should return ReturnOK
    3) Check the memfault flag (should be "false" at this point)
    4) Modify some part of the memory entry
    5) Call a memory syscall like xMemGetUsed() which should return ReturnError
    6) Check the memfault flag (should be "true" at this point)
    7) Call __MemoryClear__() and __SysStateClear__() and __

    Things we can screw with in the memory entry
    1) magic
    2) free
    3) blocks
    4) next
  */
  /*UNCRUSTIFY-ON*/
  unit_begin("Unit test for memory consistency checking");
  test_magic();
  test_free();
  test_blocks();
  test_next();
  unit_end();

  return;
}


void test_magic(void) {
  volatile Addr_t *ptr = null;
  MemoryEntry_t *entry = null;
  Size_t size = nil;


  /* 1) Create something in the heap*/
  unit_assert_ok(xMemAlloc(&ptr, TEST_ALLOC_SIZE));


  /* 2) Call a memory syscall like xMemGetUsed() which should return ReturnOK */
  unit_assert_ok(xMemGetUsed(&size));
  unit_try(EXPECTED_USED_SIZE == size);


  /* 3) Check the memfault flag (should be "false" at this point) */
  unit_try(__FlagIsNotSet__(MEMFAULT));


  /* 4) Modify some part of the memory entry */
  entry = ADDR2ENTRY(ptr);
  entry->magic = MAGIC_CONST;


  /* 5) Call a memory syscall like xMemGetUsed() which should return ReturnError
   */
  unit_assert_not_ok(xMemGetUsed(&size));


  /* 6) Check the memfault flag (should be "true" at this point) */
  unit_try(__FlagIsSet__(MEMFAULT));


  /* 7) Call __MemoryClear__() and __SysStateClear__() */
  __MemoryClear__();
  __SysStateClear__();

  return;
}


void test_free(void) {
  volatile Addr_t *ptr = null;
  MemoryEntry_t *entry = null;
  Size_t size = nil;


  /* 1) Create something in the heap*/
  unit_assert_ok(xMemAlloc(&ptr, TEST_ALLOC_SIZE));


  /* 2) Call a memory syscall like xMemGetUsed() which should return ReturnOK */
  unit_assert_ok(xMemGetUsed(&size));
  unit_try(EXPECTED_USED_SIZE == size);


  /* 3) Check the memfault flag (should be "false" at this point) */
  unit_try(__FlagIsNotSet__(MEMFAULT));


  /* 4) Modify some part of the memory entry */
  entry = ADDR2ENTRY(ptr);
  entry->free = CORRUPT_VALUE;
  /* 5) Call a memory syscall like xMemGetUsed() which should return ReturnError
   */
  unit_assert_not_ok(xMemGetUsed(&size));


  /* 6) Check the memfault flag (should be "true" at this point) */
  unit_try(__FlagIsSet__(MEMFAULT));


  /* 7) Call __MemoryClear__() and __SysStateClear__() */
  __MemoryClear__();
  __SysStateClear__();

  return;
}


void test_blocks(void) {
  volatile Addr_t *ptr = null;
  MemoryEntry_t *entry = null;
  Size_t size = nil;


  /* 1) Create something in the heap*/
  unit_assert_ok(xMemAlloc(&ptr, TEST_ALLOC_SIZE));


  /* 2) Call a memory syscall like xMemGetUsed() which should return ReturnOK */
  unit_assert_ok(xMemGetUsed(&size));
  unit_try(EXPECTED_USED_SIZE == size);


  /* 3) Check the memfault flag (should be "false" at this point) */
  unit_try(__FlagIsNotSet__(MEMFAULT));


  /* 4) Modify some part of the memory entry */
  entry = ADDR2ENTRY(ptr);
  entry->blocks = CORRUPT_VALUE;
  /* 5) Call a memory syscall like xMemGetUsed() which should return ReturnError
   */
  unit_assert_not_ok(xMemGetUsed(&size));


  /* 6) Check the memfault flag (should be "true" at this point) */
  unit_try(__FlagIsSet__(MEMFAULT));


  /* 7) Call __MemoryClear__() and __SysStateClear__() */
  __MemoryClear__();
  __SysStateClear__();

  return;
}


void test_next(void) {
  volatile Addr_t *ptr = null;
  MemoryEntry_t *entry = null;
  Size_t size = nil;


  /* 1) Create something in the heap*/
  unit_assert_ok(xMemAlloc(&ptr, TEST_ALLOC_SIZE));


  /* 2) Call a memory syscall like xMemGetUsed() which should return ReturnOK */
  unit_assert_ok(xMemGetUsed(&size));
  unit_try(EXPECTED_USED_SIZE == size);


  /* 3) Check the memfault flag (should be "false" at this point) */
  unit_try(__FlagIsNotSet__(MEMFAULT));


  /* 4) Modify some part of the memory entry */
  entry = ADDR2ENTRY(ptr);
  entry->next = (MemoryEntry_t *) CORRUPT_POINTER;
  /* 5) Call a memory syscall like xMemGetUsed() which should return ReturnError
   */
  unit_assert_not_ok(xMemGetUsed(&size));


  /* 6) Check the memfault flag (should be "true" at this point) */
  unit_try(__FlagIsSet__(MEMFAULT));


  /* 7) Call __MemoryClear__() and __SysStateClear__() */
  __MemoryClear__();
  __SysStateClear__();

  return;
}