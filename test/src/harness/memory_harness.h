/*UNCRUSTIFY-OFF*/
/**
 * @file memory_harness.h
 * @author Test Harness
 * @brief Comprehensive memory management test harness
 * @version 1.0.0
 * @date 2025-01-23
 *
 * @copyright
 * HeliOS Embedded Operating System Test Suite
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
/*UNCRUSTIFY-ON*/
#ifndef MEMORY_HARNESS_H_
  #define MEMORY_HARNESS_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "mem.h"
  #include "unit.h"

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  /* Test Suite Functions */
  void test_memory_basic_allocation(void);
  void test_memory_stress_allocation(void);
  void test_memory_fragmentation(void);
  void test_memory_edge_cases(void);
  void test_memory_utilities(void);
  void test_memory_statistics(void);
  void test_memory_corruption_detection(void);
  void test_memory_alignment(void);
  void test_memory_kernel_region(void);
  void test_memory_free_all(void);
  void test_memory_size_tracking(void);
  void test_memory_defragmentation(void);
  void test_memory_cycle_detection(void);
  void test_memory_large_allocations(void);
  void test_memory_pattern_verification(void);


  /* Main test entry point */
  void run_memory_tests(void);
  void memory_harness(void);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef MEMORY_HARNESS_H_ */