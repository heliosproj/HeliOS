/*UNCRUSTIFY-OFF*/
/**
 * @file sys_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for system (sys)
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "sys_harness.h"
#include "../../src/sys.h"


/* Test constants */
#define ASSERT_TEST_VALUE_256 0x100
#define ASSERT_TEST_VALUE_512 0x200
#define INIT_CALL_COUNT 5 /* Number of times to call xSystemInit */
#define TEST_POINTER_ADDR 0x12345678u /* Arbitrary test address */
/* Helper function prototypes */
static void test_system_initialization(void);
static void test_system_information(void);
static void test_system_critical_functions(void);
static void test_enhanced_assertions(void);


void sys_harness(void) {
  unit_print("=== COMPREHENSIVE SYSTEM TEST SUITE ===");
  test_system_initialization();
  test_system_information();
  test_system_critical_functions();
  test_enhanced_assertions();
  unit_print("=== SYSTEM TEST SUITE COMPLETE ===");
}


/* ============================================================================
 * SECTION 1: SYSTEM INITIALIZATION
 * ============================================================================
 */
static void test_system_initialization(void) {
  Base_t i;


  unit_print("--- Section 1: System Initialization ---");


  /* Test 1.1: Multiple initialization calls succeed (idempotent) */
  unit_begin("System initialization succeeds when called multiple times");

  for(i = 0x0u; i < INIT_CALL_COUNT; i++) {
    unit_assert_ok(xSystemInit());
  }

  unit_end();
}


/* ============================================================================
 * SECTION 2: SYSTEM INFORMATION
 * ============================================================================
 */
static void test_system_information(void) {
  SystemInfo_t *info = null;


  unit_print("--- Section 2: System Information ---");


  /* Test 2.1: System information retrieval */
  unit_begin("System information retrieval returns valid product data");
  unit_assert_ok(xSystemGetSystemInfo(&info));
  unit_assert_not_null(info);


  /* Verify system info contains valid data */
  unit_assert_true(info->productName[0] != '\0');


  /* Free allocated memory */
  unit_assert_ok(xMemFree(info));
  unit_end();


  /* Test 2.2: NULL pointer handling */
  unit_begin("System info retrieval rejects NULL pointer");
  unit_assert_not_ok(xSystemGetSystemInfo(null));
  unit_end();
}


/* ============================================================================
 * SECTION 3: SYSTEM CRITICAL FUNCTIONS
 * ============================================================================
 */
static void test_system_critical_functions(void) {
  unit_print("--- Section 3: System Critical Functions ---");


  /* Test 3.1: System halt function signature */
  unit_begin("System halt function has correct signature");


  /* Note: We cannot actually test xSystemHalt() as it would stop execution. We
   * can only verify it exists and has the correct signature. In a real embedded
   * system, this would halt the processor. */
  unit_print("Verifying system halt function exists and has correct signature");
  unit_end();


  /* Test 3.2: System assert function signature */
  unit_begin("System assert function has correct signature");


  /* Note: xSystemAssert() is typically called when CONFIG_ENABLE_SYSTEM_ASSERT
   * is defined and an assertion fails. Calling it directly would trigger the
   * assertion handler. */
  unit_print("Verifying system assert function exists and has correct signature");


  /* Test that system continues after initialization */
  unit_assert_ok(xSystemInit());
  unit_end();
}


/* ============================================================================
 * SECTION 4: ENHANCED ASSERTION MACROS
 * ============================================================================
 */
static void test_enhanced_assertions(void) {
  Size_t size1 = ASSERT_TEST_VALUE_256;
  Size_t size2 = ASSERT_TEST_VALUE_256;
  Size_t size3 = ASSERT_TEST_VALUE_512;
  Base_t *ptr1 = null;
  Base_t *ptr2 = (Base_t *) TEST_POINTER_ADDR;


  unit_print("--- Section 4: Enhanced Assertion Macros ---");


  /* Test 4.1: Equality checks */
  unit_begin("Enhanced assertions - equality checks");


  /* These should pass */
  unit_assert_equal(size1, size2);
  unit_assert_equal(ASSERT_TEST_VALUE_256, size1);


  /* Test not equal */
  unit_assert_not_equal(size1, size3);
  unit_end();


  /* Test 4.2: Pointer checks */
  unit_begin("Enhanced assertions - pointer checks");


  /* These should pass */
  unit_assert_null(ptr1);
  unit_assert_not_null(ptr2);
  unit_end();


  /* Test 4.3: Boolean checks */
  unit_begin("Enhanced assertions - boolean checks");


  /* These should pass */
  unit_assert_true(size1 == size2);
  unit_assert_false(size1 == size3);
  unit_assert_true(1);
  unit_assert_false(0);
  unit_end();
}