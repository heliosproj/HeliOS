/*UNCRUSTIFY-OFF*/
/**
 * @file memory_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive memory management test harness implementation
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "memory_harness.h"
#include <stdlib.h>


/* Test configuration */
#define MAX_STRESS_ALLOCS 1000
#define PATTERN_BYTE 0xAA
#define INVERSE_PATTERN_BYTE 0x55
#define MAX_ALLOCATION_SIZE (MEMORY_REGION_SIZE / 4)


/* Helper function to fill memory with a pattern */
static void fill_pattern(volatile Addr_t *addr, Size_t size, Byte_t pattern) {
  Size_t i;
  volatile Byte_t *ptr = (volatile Byte_t *) addr;


  for(i = 0; i < size; i++) {
    ptr[i] = pattern;
  }
}


/* Helper function to verify memory pattern */
static Base_t verify_pattern(volatile Addr_t *addr, Size_t size, Byte_t pattern) {
  Size_t i;
  volatile Byte_t *ptr = (volatile Byte_t *) addr;


  for(i = 0; i < size; i++) {
    if(ptr[i] != pattern) {
      return(false);
    }
  }

  return(true);
}


void memory_harness(void) {
  volatile Addr_t *ptr = null;
  volatile Addr_t *ptr1 = null;
  volatile Addr_t *ptr2 = null;
  volatile Addr_t *ptr3 = null;
  volatile Addr_t *ptrs[100];
  volatile Addr_t *kptr1 = null;
  volatile Addr_t *kptr2 = null;
  volatile Addr_t *hptr = null;
  volatile Addr_t *large_ptr = null;
  volatile Addr_t *small_ptr = null;
  volatile Addr_t *large = null;
  Size_t size = 0;
  Size_t size1 = 64;
  Size_t size2 = 128;
  Size_t size3 = 256;
  Size_t sizes[100];
  Size_t retrieved_size = 0;
  Size_t i, j;
  Size_t total_allocated = 0;
  Size_t used_memory = 0;
  Size_t used_before = 0;
  Size_t used_after = 0;
  Size_t frees_before;
  Size_t actual_size = 0;
  Size_t block_size = 100;
  Size_t huge_size = MEMORY_REGION_SIZE * 2;
  Size_t max_safe_size = MEMORY_REGION_SIZE - 1024;
  Size_t large_size;
  Size_t available;
  Size_t count = 0;
  Size_t total_requested = 0;
  Size_t total_used = 0;
  Size_t used_after_free;
  Byte_t src[256];
  Byte_t dest[256];
  Byte_t patterns[20];
  Base_t result = false;
  Word_t blocks_before, blocks_after;
  MemoryRegionStats_t *stats = null;
  MemoryRegionStats_t *heap_stats = null;
  MemoryRegionStats_t *kernel_stats = null;
  MemoryRegionStats_t *kstats = null;
  Size_t test_sizes[] = {
    1, 3, 5, 7, 9, 15, 17, 31, 33, 63, 65, 127, 129
  };
  Size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
  Size_t requested_sizes[10] = {
    1, 7, 15, 16, 17, 31, 32, 33, 63, 64
  };


  /* Clear memory before starting tests */
  xMemFreeAll();
  unit_print("=== COMPREHENSIVE MEMORY TEST SUITE ===");


  /*
   * ============================================================================
   * SECTION 1: BASIC MEMORY ALLOCATION
   * ============================================================================
   */
  unit_print("--- Section 1: Basic Memory Allocation ---");


  /* Test 1.1: Simple allocation and free */
  unit_begin("Basic memory allocation");
  unit_assert_equal(xMemAlloc(&ptr1, size1), ReturnOK);
  unit_assert_not_null(ptr1);
  unit_assert_equal(xMemFree(ptr1), ReturnOK);
  unit_end();


  /* Test 1.2: Multiple allocations */
  unit_begin("Multiple allocations");
  unit_assert_equal(xMemAlloc(&ptr1, size1), ReturnOK);
  unit_assert_equal(xMemAlloc(&ptr2, size2), ReturnOK);
  unit_assert_equal(xMemAlloc(&ptr3, size3), ReturnOK);
  unit_assert_not_null(ptr1);
  unit_assert_not_null(ptr2);
  unit_assert_not_null(ptr3);
  unit_end();


  /* Test 1.3: Verify allocated sizes */
  unit_begin("Verify allocated sizes");
  unit_assert_equal(xMemGetSize(ptr1, &retrieved_size), ReturnOK);
  unit_assert_true(retrieved_size >= size1);
  unit_assert_equal(xMemGetSize(ptr2, &retrieved_size), ReturnOK);
  unit_assert_true(retrieved_size >= size2);
  unit_assert_equal(xMemGetSize(ptr3, &retrieved_size), ReturnOK);
  unit_assert_true(retrieved_size >= size3);
  unit_end();


  /* Test 1.4: Free in different order */
  unit_begin("Free in different order");
  unit_assert_equal(xMemFree(ptr2), ReturnOK);
  unit_assert_equal(xMemFree(ptr3), ReturnOK);
  unit_assert_equal(xMemFree(ptr1), ReturnOK);
  unit_end();


  /* Test 1.5: Allocate after free */
  unit_begin("Allocate after free");
  unit_assert_equal(xMemAlloc(&ptr1, size2), ReturnOK);
  unit_assert_not_null(ptr1);
  unit_assert_equal(xMemFree(ptr1), ReturnOK);
  unit_end();


  /*
   * ============================================================================
   * SECTION 2: STRESS ALLOCATION
   * ============================================================================
   */
  unit_print("--- Section 2: Memory Stress Allocation ---");


  /* Test 2.1: Initialize pointers and allocate many small blocks */
  unit_begin("Memory stress allocation");

  for(i = 0; i < 100; i++) {
    ptrs[i] = null;
    sizes[i] = 0;
  }

  for(i = 0; i < 100; i++) {
    sizes[i] = (i + 1) * 4;

    if(xMemAlloc(&ptrs[i], sizes[i]) == ReturnOK) {
      unit_assert_not_null(ptrs[i]);
      total_allocated++;
      fill_pattern(ptrs[i], sizes[i], (Byte_t) (i & 0xFF));
    } else {
      break;
    }
  }

  unit_end();


  /* Test 2.2: Verify all allocated blocks */
  unit_begin("Verify all allocated blocks");

  for(i = 0; i < total_allocated; i++) {
    unit_assert_true(verify_pattern(ptrs[i], sizes[i], (Byte_t) (i & 0xFF)));
  }

  unit_end();


  /* Test 2.3: Check memory usage */
  unit_begin("Check memory usage");
  unit_assert_equal(xMemGetUsed(&used_memory), ReturnOK);
  unit_assert_true(used_memory > 0);
  unit_end();


  /* Test 2.4: Free every other block to create fragmentation */
  unit_begin("Free every other block");

  for(i = 0; i < total_allocated; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }

  unit_end();


  /* Test 2.5: Try to allocate in the gaps */
  unit_begin("Allocate in fragmented gaps");

  for(i = 0; i < total_allocated; i += 2) {
    if(xMemAlloc(&ptrs[i], sizes[i]) == ReturnOK) {
      unit_assert_not_null(ptrs[i]);
      fill_pattern(ptrs[i], sizes[i], (Byte_t) (i & 0xFF));
    }
  }

  unit_end();


  /* Test 2.6: Verify all blocks again */
  unit_begin("Verify all blocks after reallocation");

  for(i = 0; i < total_allocated; i++) {
    if(ptrs[i] != null) {
      unit_assert_true(verify_pattern(ptrs[i], sizes[i], (Byte_t) (i & 0xFF)));
    }
  }

  unit_end();


  /* Test 2.7: Free all remaining blocks */
  unit_begin("Free all remaining blocks");

  for(i = 0; i < total_allocated; i++) {
    if(ptrs[i] != null) {
      unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    }
  }

  unit_end();


  /* Test 2.8: Verify memory is fully freed */
  unit_begin("Verify memory fully freed");
  unit_assert_equal(xMemGetUsed(&used_memory), ReturnOK);
  unit_assert_equal(used_memory, 0);
  unit_end();


  /*
   * ============================================================================
   * SECTION 3: MEMORY FRAGMENTATION AND DEFRAGMENTATION
   * ============================================================================
   */
  unit_print("--- Section 3: Memory Fragmentation and Defragmentation ---");


  /* Test 3.1: Create fragmented memory */
  unit_begin("Memory fragmentation and defragmentation");

  for(i = 0; i < 50; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], block_size), ReturnOK);
    unit_assert_not_null(ptrs[i]);
  }

  unit_end();


  /* Test 3.2: Free every other block to fragment memory */
  unit_begin("Fragment memory");

  for(i = 1; i < 50; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }

  unit_end();


  /* Test 3.3: Get fragmentation statistics */
  unit_begin("Get fragmentation statistics");
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  unit_assert_not_null(stats);
  unit_assert_true(stats->numberOfFreeBlocks > 1);
  xMemFree((const volatile Addr_t *) stats);
  unit_end();


  /* Test 3.4: Free adjacent blocks to trigger defragmentation */
  unit_begin("Trigger defragmentation");

  for(i = 0; i < 50; i += 2) {
    if(ptrs[i] != null) {
      unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
      ptrs[i] = null;
    }
  }

  unit_end();


  /* Test 3.5: Verify defragmentation occurred */
  unit_begin("Verify defragmentation");
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  unit_assert_not_null(stats);
  unit_assert_equal(stats->numberOfFreeBlocks, 1);
  xMemFree((const volatile Addr_t *) stats);
  unit_end();


  /* Test 3.6: Allocate large block after defragmentation */
  unit_begin("Allocate large block after defragmentation");
  unit_assert_equal(xMemAlloc(&large_ptr, block_size * 40), ReturnOK);
  unit_assert_not_null(large_ptr);
  unit_assert_equal(xMemFree(large_ptr), ReturnOK);
  unit_end();


  /*
   * ============================================================================
   * SECTION 4: EDGE CASES AND BOUNDARY CONDITIONS
   * ============================================================================
   */
  unit_print("--- Section 4: Edge Cases and Boundaries ---");


  /* Test 4.1: Zero size allocation */
  unit_begin("Memory edge cases and boundaries");
  unit_assert_equal(xMemAlloc(&ptr, 0), ReturnError);
  unit_end();


  /* Test 4.2: null pointer allocation */
  unit_begin("null pointer allocation");
  unit_assert_equal(xMemAlloc(null, 100), ReturnError);
  unit_end();


  /* Test 4.3: Huge size allocation */
  unit_begin("Huge size allocation fails");
  unit_assert_equal(xMemAlloc(&ptr, huge_size), ReturnError);
  unit_end();


  /* Test 4.4: Free null pointer */
  unit_begin("Free null pointer");
  unit_assert_equal(xMemFree(null), ReturnOK);
  unit_end();


  /* Test 4.5: Double free */
  unit_begin("Double free detection");
  unit_assert_equal(xMemAlloc(&ptr, 100), ReturnOK);
  unit_assert_not_null(ptr);
  unit_assert_equal(xMemFree(ptr), ReturnOK);
  unit_assert_equal(xMemFree(ptr), ReturnError);
  unit_end();


  /* Test 4.6: Get size of null pointer */
  unit_begin("Get size of null pointer");
  unit_assert_equal(xMemGetSize(null, &size), ReturnError);
  unit_end();


  /* Test 4.7: Get size with null size pointer */
  unit_begin("Get size with null size pointer");
  unit_assert_equal(xMemAlloc(&ptr, 100), ReturnOK);
  unit_assert_equal(xMemGetSize(ptr, null), ReturnError);
  unit_assert_equal(xMemFree(ptr), ReturnOK);
  unit_end();


  /* Test 4.8: Minimum size allocation */
  unit_begin("Minimum size allocation");
  unit_assert_equal(xMemAlloc(&ptr, 1), ReturnOK);
  unit_assert_not_null(ptr);
  unit_assert_equal(xMemFree(ptr), ReturnOK);
  unit_end();


  /* Test 4.9: Maximum practical allocation */
  unit_begin("Maximum practical allocation");
  unit_assert_equal(xMemAlloc(&ptr, max_safe_size), ReturnOK);

  if(ptr != null) {
    fill_pattern(ptr, 100, PATTERN_BYTE);
    unit_assert_true(verify_pattern(ptr, 100, PATTERN_BYTE));
    unit_assert_equal(xMemFree(ptr), ReturnOK);
  }

  unit_end();


  /* Test 4.10: Allocation alignment */
  unit_begin("Allocation alignment");
  unit_assert_equal(xMemAlloc(&ptr, 17), ReturnOK);
  unit_assert_not_null(ptr);
  unit_assert_true(((Size_t) ptr % CONFIG_MEMORY_ALIGNMENT) == 0);
  unit_assert_equal(xMemFree(ptr), ReturnOK);
  unit_end();


  /*
   * ============================================================================
   * SECTION 5: MEMORY UTILITY FUNCTIONS
   * ============================================================================
   */
  unit_print("--- Section 5: Memory Utility Functions ---");


  /* Test 5.1: Initialize buffers and test memset */
  unit_begin("Memory utility functions");

  for(i = 0; i < 256; i++) {
    src[i] = (Byte_t) i;
    dest[i] = 0;
  }

  unit_assert_equal(__memset__((volatile Addr_t *) dest, PATTERN_BYTE, 256), ReturnOK);

  for(i = 0; i < 256; i++) {
    unit_assert_equal(dest[i], PATTERN_BYTE);
  }

  unit_end();


  /* Test 5.2: memcpy */
  unit_begin("memcpy test");
  unit_assert_equal(__memcpy__((volatile Addr_t *) dest, (volatile Addr_t *) src, 256), ReturnOK);

  for(i = 0; i < 256; i++) {
    unit_assert_equal(dest[i], src[i]);
  }

  unit_end();


  /* Test 5.3: memcmp equal */
  unit_begin("memcmp equal");
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, (volatile Addr_t *) dest, 256, &result), ReturnOK);
  unit_assert_true(result);
  unit_end();


  /* Test 5.4: memcmp not equal */
  unit_begin("memcmp not equal");
  dest[100] = 0xFF;
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, (volatile Addr_t *) dest, 256, &result), ReturnOK);
  unit_assert_false(result);
  unit_end();


  /* Test 5.5: memset with zero size */
  unit_begin("memset with zero size");
  unit_assert_equal(__memset__((volatile Addr_t *) dest, 0, 0), ReturnError);
  unit_end();


  /* Test 5.6: memcpy with zero size */
  unit_begin("memcpy with zero size");
  unit_assert_equal(__memcpy__((volatile Addr_t *) dest, (volatile Addr_t *) src, 0), ReturnError);
  unit_end();


  /* Test 5.7: memcmp with zero size */
  unit_begin("memcmp with zero size");
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, (volatile Addr_t *) dest, 0, &result), ReturnError);
  unit_end();


  /* Test 5.8: null pointer checks */
  unit_begin("Memory utility null pointer checks");
  unit_assert_equal(__memset__(null, 0, 100), ReturnError);
  unit_assert_equal(__memcpy__(null, (volatile Addr_t *) src, 100), ReturnError);
  unit_assert_equal(__memcpy__((volatile Addr_t *) dest, null, 100), ReturnError);
  unit_assert_equal(__memcmp__(null, (volatile Addr_t *) dest, 100, &result), ReturnError);
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, null, 100, &result), ReturnError);
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, (volatile Addr_t *) dest, 100, null), ReturnError);
  unit_end();


  /*
   * ============================================================================
   * SECTION 6: MEMORY STATISTICS
   * ============================================================================
   */
  unit_print("--- Section 6: Memory Statistics ---");


  /* Test 6.1: null pointer in xMemGetUsed */
  unit_begin("Memory statistics");
  unit_assert_equal(xMemGetUsed(null), ReturnError);
  unit_end();


  /* Test 6.2: null pointer in xMemGetHeapStats */
  unit_begin("null pointer in heap stats");
  unit_assert_equal(xMemGetHeapStats(null), ReturnError);
  unit_end();


  /* Test 6.3: null pointer in xMemGetKernelStats */
  unit_begin("null pointer in kernel stats");
  unit_assert_equal(xMemGetKernelStats(null), ReturnError);
  unit_end();


  /* Test 6.4: Get initial used memory */
  unit_begin("Get initial used memory");
  unit_assert_equal(xMemGetUsed(&used_before), ReturnOK);
  unit_end();


  /* Test 6.5: Allocate and check used memory increased */
  unit_begin("Check used memory increases");
  unit_assert_equal(xMemAlloc(&ptr1, 1000), ReturnOK);
  unit_assert_equal(xMemGetUsed(&used_after), ReturnOK);
  unit_assert_true(used_after > used_before);
  unit_end();


  /* Test 6.6: Get size of allocation */
  unit_begin("Get size of allocation");
  unit_assert_equal(xMemGetSize(ptr1, &size), ReturnOK);
  unit_assert_true(size >= 1000);
  unit_end();


  /* Test 6.7: Allocate more and check stats */
  unit_begin("Allocate more and check stats");
  unit_assert_equal(xMemAlloc(&ptr2, 2000), ReturnOK);
  unit_assert_equal(xMemGetUsed(&used_after), ReturnOK);
  unit_assert_true(used_after >= used_before + 3000);
  unit_end();


  /* Test 6.8: Get heap statistics */
  unit_begin("Get heap statistics");
  unit_assert_equal(xMemGetHeapStats(&heap_stats), ReturnOK);
  unit_assert_not_null(heap_stats);
  unit_assert_true(heap_stats->successfulAllocations >= 2);
  unit_assert_true(heap_stats->availableSpaceInBytes < MEMORY_REGION_SIZE);
  unit_end();


  /* Test 6.9: Get kernel statistics */
  unit_begin("Get kernel statistics");
  unit_assert_equal(xMemGetKernelStats(&kernel_stats), ReturnOK);
  unit_assert_not_null(kernel_stats);
  unit_end();


  /* Test 6.10: Free and check stats update */
  unit_begin("Free and check stats update");
  frees_before = heap_stats->successfulFrees;
  unit_assert_equal(xMemFree(ptr1), ReturnOK);
  unit_assert_equal(xMemFree(ptr2), ReturnOK);
  xMemFree((const volatile Addr_t *) heap_stats);
  xMemFree((const volatile Addr_t *) kernel_stats);
  unit_assert_equal(xMemGetHeapStats(&heap_stats), ReturnOK);
  unit_assert_true(heap_stats->successfulFrees > frees_before);
  xMemFree((const volatile Addr_t *) heap_stats);
  unit_end();


  /* Test 6.11: Check used memory decreased */
  unit_begin("Check used memory decreased");
  unit_assert_equal(xMemGetUsed(&used_after), ReturnOK);
  unit_assert_true(used_after <= used_before);
  unit_end();


  /*
   * ============================================================================
   * SECTION 7: MEMORY ALIGNMENT
   * ============================================================================
   */
  unit_print("--- Section 7: Memory Alignment ---");


  /* Test 7.1: Various sizes ensure proper alignment */
  unit_begin("Memory alignment");

  for(i = 0; i < num_sizes; i++) {
    unit_assert_equal(xMemAlloc(&ptr, test_sizes[i]), ReturnOK);
    unit_assert_not_null(ptr);
    unit_assert_true(((Size_t) ptr % CONFIG_MEMORY_ALIGNMENT) == 0);
    fill_pattern(ptr, test_sizes[i], (Byte_t) i);
    unit_assert_true(verify_pattern(ptr, test_sizes[i], (Byte_t) i));
    unit_assert_equal(xMemFree(ptr), ReturnOK);
  }

  unit_end();


  /* Test 7.2: Allocation sizes rounded up for alignment */
  unit_begin("Allocation sizes rounded up");
  unit_assert_equal(xMemAlloc(&ptr, 1), ReturnOK);
  unit_assert_equal(xMemGetSize(ptr, &actual_size), ReturnOK);
  unit_assert_true(actual_size >= CONFIG_MEMORY_ALIGNMENT);
  unit_assert_equal(xMemFree(ptr), ReturnOK);
  unit_end();


  /*
   * ============================================================================
   * SECTION 8: KERNEL MEMORY REGION
   * ============================================================================
   */
  unit_print("--- Section 8: Kernel Memory Region ---");


  /* Test 8.1: Allocate from kernel region */
  unit_begin("Kernel memory region");
  unit_assert_equal(__KernelAllocateMemory__(&kptr1, 500), ReturnOK);
  unit_assert_not_null(kptr1);
  unit_end();


  /* Test 8.2: Allocate from heap region */
  unit_begin("Allocate from heap region");
  unit_assert_equal(__HeapAllocateMemory__(&hptr, 500), ReturnOK);
  unit_assert_not_null(hptr);
  unit_end();


  /* Test 8.3: Verify different regions */
  unit_begin("Verify different regions");
  unit_assert_true(kptr1 != hptr);
  unit_end();


  /* Test 8.4: Allocate more from kernel */
  unit_begin("Allocate more from kernel");
  unit_assert_equal(__KernelAllocateMemory__(&kptr2, 1000), ReturnOK);
  unit_assert_not_null(kptr2);
  unit_end();


  /* Test 8.5: Get kernel stats */
  unit_begin("Get kernel stats");
  unit_assert_equal(xMemGetKernelStats(&kstats), ReturnOK);
  unit_assert_not_null(kstats);
  unit_assert_true(kstats->successfulAllocations >= 2);
  unit_end();


  /* Test 8.6: Free kernel allocations */
  unit_begin("Free kernel allocations");
  unit_assert_equal(__KernelFreeMemory__(kptr1), ReturnOK);
  unit_assert_equal(__KernelFreeMemory__(kptr2), ReturnOK);
  unit_end();


  /* Test 8.7: Free heap allocation */
  unit_begin("Free heap allocation");
  unit_assert_equal(__HeapFreeMemory__(hptr), ReturnOK);
  xMemFree((const volatile Addr_t *) kstats);
  unit_end();


  /*
   * ============================================================================
   * SECTION 9: MEMORY FREE ALL
   * ============================================================================
   */
  unit_print("--- Section 9: Memory Free All ---");


  /* Test 9.1: Allocate multiple blocks */
  unit_begin("Memory free all");

  for(i = 0; i < 10; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], (i + 1) * 100), ReturnOK);
    unit_assert_not_null(ptrs[i]);
    fill_pattern(ptrs[i], (i + 1) * 100, (Byte_t) i);
  }

  unit_end();


  /* Test 9.2: Verify memory is used */
  unit_begin("Verify memory is used");
  unit_assert_equal(xMemGetUsed(&used_memory), ReturnOK);
  unit_assert_true(used_memory > 0);
  unit_end();


  /* Test 9.3: Free all memory at once */
  unit_begin("Free all memory");
  unit_assert_equal(xMemFreeAll(), ReturnOK);
  unit_end();


  /* Test 9.4: Verify all memory is freed */
  unit_begin("Verify all freed");
  unit_assert_equal(xMemGetUsed(&used_memory), ReturnOK);
  unit_assert_equal(used_memory, 0);
  unit_end();


  /* Test 9.5: Allocate after free all */
  unit_begin("Allocate after free all");
  unit_assert_equal(xMemAlloc(&ptrs[0], 1000), ReturnOK);
  unit_assert_not_null(ptrs[0]);
  unit_assert_equal(xMemFree(ptrs[0]), ReturnOK);
  unit_end();


  /* Test 9.6: Multiple free all calls */
  unit_begin("Multiple free all calls");
  unit_assert_equal(xMemFreeAll(), ReturnOK);
  unit_assert_equal(xMemFreeAll(), ReturnOK);
  unit_end();


  /*
   * ============================================================================
   * SECTION 10: PATTERN VERIFICATION
   * ============================================================================
   */
  unit_print("--- Section 10: Memory Pattern Verification ---");


  /* Test 10.1: Generate unique patterns and allocate */
  unit_begin("Memory pattern verification");

  for(i = 0; i < 20; i++) {
    patterns[i] = (Byte_t) (0x10 + i);
    ptrs[i] = null;
  }

  for(i = 0; i < 20; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], 256), ReturnOK);
    unit_assert_not_null(ptrs[i]);
    fill_pattern(ptrs[i], 256, patterns[i]);
  }

  unit_end();


  /* Test 10.2: Verify all patterns intact */
  unit_begin("Verify patterns intact");

  for(i = 0; i < 20; i++) {
    unit_assert_true(verify_pattern(ptrs[i], 256, patterns[i]));
  }

  unit_end();


  /* Test 10.3: Free some blocks */
  unit_begin("Free some blocks");

  for(i = 0; i < 20; i += 3) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }

  unit_end();


  /* Test 10.4: Verify remaining patterns */
  unit_begin("Verify remaining patterns");

  for(i = 0; i < 20; i++) {
    if(ptrs[i] != null) {
      unit_assert_true(verify_pattern(ptrs[i], 256, patterns[i]));
    }
  }

  unit_end();


  /* Test 10.5: Reallocate with new patterns */
  unit_begin("Reallocate with new patterns");

  for(i = 0; i < 20; i += 3) {
    unit_assert_equal(xMemAlloc(&ptrs[i], 256), ReturnOK);
    unit_assert_not_null(ptrs[i]);
    patterns[i] = (Byte_t) (0x80 + i);
    fill_pattern(ptrs[i], 256, patterns[i]);
  }

  unit_end();


  /* Test 10.6: Final verification */
  unit_begin("Final pattern verification");

  for(i = 0; i < 20; i++) {
    unit_assert_true(verify_pattern(ptrs[i], 256, patterns[i]));
  }

  for(i = 0; i < 20; i++) {
    if(ptrs[i] != null) {
      unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    }
  }

  unit_end();


  /*
   * ============================================================================
   * SECTION 11: LARGE ALLOCATIONS
   * ============================================================================
   */
  unit_print("--- Section 11: Large Memory Allocations ---");


  /* Test 11.1: Get available memory */
  unit_begin("Large memory allocations");
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  unit_assert_not_null(stats);
  available = stats->largestFreeEntryInBytes;
  xMemFree((const volatile Addr_t *) stats);
  unit_end();


  /* Test 11.2: Allocate 50% of available */
  unit_begin("Allocate 50% of available");
  large_size = available / 2;

  if(large_size > 100) {
    unit_assert_equal(xMemAlloc(&large_ptr, large_size), ReturnOK);

    if(large_ptr != null) {
      fill_pattern(large_ptr, 100, PATTERN_BYTE);
      unit_assert_true(verify_pattern(large_ptr, 100, PATTERN_BYTE));
      unit_assert_equal(xMemAlloc(&small_ptr, 100), ReturnOK);

      if(small_ptr != null) {
        fill_pattern(small_ptr, 100, INVERSE_PATTERN_BYTE);
        unit_assert_true(verify_pattern(small_ptr, 100, INVERSE_PATTERN_BYTE));
        unit_assert_equal(xMemFree(small_ptr), ReturnOK);
      }

      unit_assert_equal(xMemFree(large_ptr), ReturnOK);
    }
  }

  unit_end();


  /* Test 11.3: Allocate 90% of total */
  unit_begin("Allocate 90% of total");
  large_size = (MEMORY_REGION_SIZE * 9) / 10;

  if(xMemAlloc(&large_ptr, large_size) == ReturnOK) {
    fill_pattern(large_ptr, 100, PATTERN_BYTE);
    unit_assert_true(verify_pattern(large_ptr, 100, PATTERN_BYTE));
    unit_assert_equal(xMemFree(large_ptr), ReturnOK);
  }

  unit_end();


  /*
   * ============================================================================
   * SECTION 12: CYCLE DETECTION
   * ============================================================================
   */
  unit_print("--- Section 12: Memory Cycle Detection ---");


  /* Test 12.1: Allocate many blocks */
  unit_begin("Memory cycle detection");
  count = 0;

  for(i = 0; i < 100; i++) {
    if(xMemAlloc(&ptrs[i], 50) == ReturnOK) {
      count++;
      fill_pattern(ptrs[i], 50, (Byte_t) i);
    } else {
      ptrs[i] = null;
      break;
    }
  }

  unit_end();


  /* Test 12.2: Free blocks to create fragmentation */
  unit_begin("Create fragmentation");

  for(i = 1; i < count; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }

  unit_end();


  /* Test 12.3: Trigger defragmentation */
  unit_begin("Trigger defragmentation");

  for(i = 0; i < count; i += 2) {
    if(ptrs[i] != null) {
      unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
      ptrs[i] = null;
    }
  }

  unit_end();


  /* Test 12.4: Allocate large block */
  unit_begin("Allocate after defragmentation");
  unit_assert_equal(xMemAlloc(&large, count * 25), ReturnOK);

  if(large != null) {
    unit_assert_equal(xMemFree(large), ReturnOK);
  }

  unit_end();


  /*
   * ============================================================================
   * SECTION 13: DEFRAGMENTATION BEHAVIOR
   * ============================================================================
   */
  unit_print("--- Section 13: Memory Defragmentation Behavior ---");


  /* Test 13.1: Create maximum fragmentation */
  unit_begin("Memory defragmentation behavior");

  for(i = 0; i < 30; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], 64), ReturnOK);
  }

  unit_end();


  /* Test 13.2: Free every third block */
  unit_begin("Free every third block");

  for(i = 0; i < 30; i += 3) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }

  unit_end();


  /* Test 13.3: Get fragmentation level */
  unit_begin("Get fragmentation level");
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  blocks_before = stats->numberOfFreeBlocks;
  unit_end();


  /* Test 13.4: Free adjacent blocks */
  unit_begin("Free adjacent blocks");

  for(i = 1; i < 30; i += 3) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }

  xMemFree((const volatile Addr_t *) stats);
  unit_end();


  /* Test 13.5: Check blocks were merged */
  unit_begin("Check blocks merged");
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  blocks_after = stats->numberOfFreeBlocks;
  unit_assert_true(blocks_after <= blocks_before + 1);
  xMemFree((const volatile Addr_t *) stats);
  unit_end();


  /* Test 13.6: Free remaining blocks */
  unit_begin("Free remaining blocks");

  for(i = 2; i < 30; i += 3) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
  }

  unit_end();


  /* Test 13.7: Verify one large free block */
  unit_begin("Verify one large free block");
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  unit_assert_equal(stats->numberOfFreeBlocks, 1);
  xMemFree((const volatile Addr_t *) stats);
  unit_end();


  /*
   * ============================================================================
   * SECTION 14: CORRUPTION DETECTION
   * ============================================================================
   */
  unit_print("--- Section 14: Memory Corruption Detection ---");


  /* Test 14.1: Allocate blocks */
  unit_begin("Memory corruption detection");
  unit_assert_equal(xMemAlloc(&ptr1, 100), ReturnOK);
  unit_assert_equal(xMemAlloc(&ptr2, 100), ReturnOK);
  unit_assert_equal(xMemAlloc(&ptr3, 100), ReturnOK);
  unit_end();


  /* Test 14.2: Use blocks normally */
  unit_begin("Use blocks normally");
  fill_pattern(ptr1, 100, 0x11);
  fill_pattern(ptr2, 100, 0x22);
  fill_pattern(ptr3, 100, 0x33);
  unit_end();


  /* Test 14.3: Verify patterns */
  unit_begin("Verify patterns");
  unit_assert_true(verify_pattern(ptr1, 100, 0x11));
  unit_assert_true(verify_pattern(ptr2, 100, 0x22));
  unit_assert_true(verify_pattern(ptr3, 100, 0x33));
  unit_end();


  /* Test 14.4: Free blocks normally */
  unit_begin("Free blocks normally");
  unit_assert_equal(xMemFree(ptr1), ReturnOK);
  unit_assert_equal(xMemFree(ptr2), ReturnOK);
  unit_assert_equal(xMemFree(ptr3), ReturnOK);
  unit_end();


  /*
   * ============================================================================
   * SECTION 15: SIZE TRACKING
   * ============================================================================
   */
  unit_print("--- Section 15: Memory Size Tracking ---");


  /* Test 15.1: Allocate various sizes */
  unit_begin("Memory size tracking");
  total_requested = 0;

  for(i = 0; i < 10; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], requested_sizes[i]), ReturnOK);
    unit_assert_not_null(ptrs[i]);
    unit_assert_equal(xMemGetSize(ptrs[i], &actual_size), ReturnOK);
    unit_assert_true(actual_size >= requested_sizes[i]);
    unit_assert_true((actual_size % CONFIG_MEMORY_ALIGNMENT) == 0);
    total_requested += requested_sizes[i];
  }

  unit_end();


  /* Test 15.2: Check total used memory */
  unit_begin("Check total used");
  unit_assert_equal(xMemGetUsed(&total_used), ReturnOK);
  unit_assert_true(total_used >= total_requested);
  unit_end();


  /* Test 15.3: Free half the blocks */
  unit_begin("Free half blocks");

  for(i = 0; i < 10; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
  }

  unit_end();


  /* Test 15.4: Check used memory decreased */
  unit_begin("Check memory decreased");
  unit_assert_equal(xMemGetUsed(&used_after_free), ReturnOK);
  unit_assert_true(used_after_free < total_used);
  unit_end();


  /* Test 15.5: Free remaining blocks */
  unit_begin("Free remaining blocks");

  for(i = 1; i < 10; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
  }

  unit_end();


  /* Test 15.6: Verify all memory freed */
  unit_begin("Verify all freed");
  unit_assert_equal(xMemGetUsed(&used_after_free), ReturnOK);
  unit_assert_equal(used_after_free, 0);
  unit_end();


  /* Final cleanup */
  xMemFreeAll();
  unit_print("=== MEMORY TEST SUITE COMPLETE ===");
}


/*UNCRUSTIFY-ON*/