/*UNCRUSTIFY-OFF*/
/**
 * @file memory_1_harness.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#ifndef MEMORY_1_HARNESS_H_
  #define MEMORY_1_HARNESS_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"

  #include "unit.h"

  typedef struct MemoryTest_s {
    Size_t size;
    Size_t blocks;
    void *ptr;
  } MemoryTest_t;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  void memory_1_harness(void);
  void memory_1_harness_task(Task_t *task_, TaskParm_t *parm_);
  void test_memory_edge_cases(void);
  void test_memcpy_memcmp(void);
  void test_freed_pointer_operations(void);
  void test_boundary_allocations(void);
  void test_statistics_accuracy(void);
  void test_data_integrity(void);
  void test_fragmentation_stress(void);
  void test_invalid_pointers(void);
  void test_state_consistency(void);
  void test_memfreeall_idempotency(void);
  void test_kernel_memory(void);
  void test_performance_stress(void);
  void test_randomized_patterns(void);
  void test_alignment_verification(void);
  void test_cross_region_protection(void);
  void test_memory_corruption_detection(void);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef MEMORY_1_HARNESS_H_ */