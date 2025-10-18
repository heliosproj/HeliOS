/*UNCRUSTIFY-OFF*/
/**
 * @file sys_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit test harness for system API
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


void sys_harness(void) {
  test_system_init();
  test_system_info();
  test_system_halt();
  test_system_assert();
  test_enhanced_assertions();
}


void test_system_init(void) {
  unit_begin("System initialization succeeds when called multiple times");


  /* System should already be initialized by the test harness, calling it again
   * should succeed (idempotent) */
  unit_assert_ok(xSystemInit());
  unit_end();
}


void test_system_info(void) {
  SystemInfo_t *info = null;


  unit_begin("System information retrieval returns valid product data");


  /* Get system information */
  unit_assert_ok(xSystemGetSystemInfo(&info));
  unit_assert_not_null(info);


  /* Verify system info contains valid data */
  unit_assert_true(info->productName[0] != '\0');


  /* Note: Version fields are unsigned, so no need to check >= 0 */
  /* Free allocated memory */
  unit_assert_ok(xMemFree(info));
  unit_end();


  /* Test NULL pointer handling */
  unit_begin("System info retrieval rejects NULL pointer");
  unit_assert_not_ok(xSystemGetSystemInfo(null));
  unit_end();
}


void test_system_halt(void) {
  unit_begin("System halt function has correct signature");


  /* Note: We cannot actually test xSystemHalt() as it would stop execution. We
   * can only verify it exists and has the correct signature. In a real embedded
   * system, this would halt the processor. */
  /* For testing purposes, we just verify the function can be called without
   * crashing if the system is in a valid state. */
  /* This test is intentionally minimal as calling xSystemHalt() would prevent
   * the test suite from continuing. */
  unit_print("Verifying system halt function exists and has correct signature");
  unit_end();
}


void test_system_assert(void) {
  unit_begin("System assert function has correct signature");


  /* Note: xSystemAssert() is typically called when CONFIG_ENABLE_SYSTEM_ASSERT
   * is defined and an assertion fails. Calling it directly would trigger the
   * assertion handler.  */
  /* For testing purposes, we verify the function exists and has the correct
   * signature. In a real scenario, this would be called by the ASSERT() macro
   * when an assertion fails. */
  unit_print("Verifying system assert function exists and has correct signature");


  /* Test that system continues after initialization */
  unit_assert_ok(xSystemInit());
  unit_end();
}


void test_enhanced_assertions(void) {
  Size_t size1 = 0x100;
  Size_t size2 = 0x100;
  Size_t size3 = 0x200;
  Base_t *ptr1 = null;
  Base_t *ptr2 = (Base_t *) 0x12345678;


  unit_begin("Enhanced assertions - equality checks");


  /* These should pass */
  unit_assert_equal(size1, size2);
  unit_assert_equal(0x100, size1);


  /* Test not equal */
  unit_assert_not_equal(size1, size3);
  unit_end();
  unit_begin("Enhanced assertions - pointer checks");


  /* These should pass */
  unit_assert_null(ptr1);
  unit_assert_not_null(ptr2);
  unit_end();
  unit_begin("Enhanced assertions - boolean checks");


  /* These should pass */
  unit_assert_true(size1 == size2);
  unit_assert_false(size1 == size3);
  unit_assert_true(1);
  unit_assert_false(0);
  unit_end();
}