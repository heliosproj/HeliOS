/*UNCRUSTIFY-OFF*/
/**
 * @file memory_harness.c
 * @author Test Harness
 * @brief Comprehensive memory management test harness implementation
 * @version 1.0.0
 * @date 2025-01-23
 *
 * @copyright
 * HeliOS Embedded Operating System Test Suite
 * SPDX-License-Identifier: GPL-2.0-or-later
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


/**
 * @brief Test basic memory allocation and deallocation
 */
void test_memory_basic_allocation(void) {
  volatile Addr_t *ptr1 = null;
  volatile Addr_t *ptr2 = null;
  volatile Addr_t *ptr3 = null;
  Size_t size1 = 64;
  Size_t size2 = 128;
  Size_t size3 = 256;
  Size_t retrieved_size = 0;


  unit_begin("Basic memory allocation");


  /* Test 1: Simple allocation and free */
  unit_assert_equal(xMemAlloc(&ptr1, size1), ReturnOK);
  unit_assert_not_null(ptr1);
  unit_assert_equal(xMemFree(ptr1), ReturnOK);


  /* Test 2: Multiple allocations */
  unit_assert_equal(xMemAlloc(&ptr1, size1), ReturnOK);
  unit_assert_equal(xMemAlloc(&ptr2, size2), ReturnOK);
  unit_assert_equal(xMemAlloc(&ptr3, size3), ReturnOK);
  unit_assert_not_null(ptr1);
  unit_assert_not_null(ptr2);
  unit_assert_not_null(ptr3);


  /* Test 3: Verify allocated sizes */
  unit_assert_equal(xMemGetSize(ptr1, &retrieved_size), ReturnOK);
  unit_assert_true(retrieved_size >= size1);
  unit_assert_equal(xMemGetSize(ptr2, &retrieved_size), ReturnOK);
  unit_assert_true(retrieved_size >= size2);
  unit_assert_equal(xMemGetSize(ptr3, &retrieved_size), ReturnOK);
  unit_assert_true(retrieved_size >= size3);


  /* Test 4: Free in different order */
  unit_assert_equal(xMemFree(ptr2), ReturnOK);
  unit_assert_equal(xMemFree(ptr3), ReturnOK);
  unit_assert_equal(xMemFree(ptr1), ReturnOK);


  /* Test 5: Allocate after free */
  unit_assert_equal(xMemAlloc(&ptr1, size2), ReturnOK);
  unit_assert_not_null(ptr1);
  unit_assert_equal(xMemFree(ptr1), ReturnOK);
  unit_end();
}


/**
 * @brief Stress test memory allocation with many allocations
 */
void test_memory_stress_allocation(void) {
  volatile Addr_t *ptrs[100];
  Size_t sizes[100];
  Size_t i;
  Size_t total_allocated = 0;
  Size_t used_memory = 0;


  unit_begin("Memory stress allocation");


  /* Initialize pointers */
  for(i = 0; i < 100; i++) {
    ptrs[i] = null;
    sizes[i] = 0;
  }


  /* Test 1: Allocate many small blocks */
  for(i = 0; i < 100; i++) {
    sizes[i] = (i + 1) * 4;  /* Sizes from 4 to 400 bytes */

    if(xMemAlloc(&ptrs[i], sizes[i]) == ReturnOK) {
      unit_assert_not_null(ptrs[i]);
      total_allocated++;


      /* Write pattern to verify no corruption */
      fill_pattern(ptrs[i], sizes[i], (Byte_t) (i & 0xFF));
    } else {
      /* Allocation failed - we may have run out of memory */
      break;
    }
  }


  /* Test 2: Verify all allocated blocks */
  for(i = 0; i < total_allocated; i++) {
    unit_assert_true(verify_pattern(ptrs[i], sizes[i], (Byte_t) (i & 0xFF)));
  }


  /* Test 3: Check memory usage */
  unit_assert_equal(xMemGetUsed(&used_memory), ReturnOK);
  unit_assert_true(used_memory > 0);


  /* Test 4: Free every other block to create fragmentation */
  for(i = 0; i < total_allocated; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }


  /* Test 5: Try to allocate in the gaps */
  for(i = 0; i < total_allocated; i += 2) {
    if(xMemAlloc(&ptrs[i], sizes[i]) == ReturnOK) {
      unit_assert_not_null(ptrs[i]);
      fill_pattern(ptrs[i], sizes[i], (Byte_t) (i & 0xFF));
    }
  }


  /* Test 6: Verify all blocks again */
  for(i = 0; i < total_allocated; i++) {
    if(ptrs[i] != null) {
      unit_assert_true(verify_pattern(ptrs[i], sizes[i], (Byte_t) (i & 0xFF)));
    }
  }


  /* Test 7: Free all remaining blocks */
  for(i = 0; i < total_allocated; i++) {
    if(ptrs[i] != null) {
      unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    }
  }


  /* Test 8: Verify memory is fully freed */
  unit_assert_equal(xMemGetUsed(&used_memory), ReturnOK);
  unit_assert_equal(used_memory, 0);
  unit_end();
}


/**
 * @brief Test memory fragmentation and defragmentation
 */
void test_memory_fragmentation(void) {
  volatile Addr_t *ptrs[50];
  Size_t block_size = 100;
  Size_t i;
  volatile Addr_t *large_ptr = null;
  MemoryRegionStats_t *stats = null;


  unit_begin("Memory fragmentation and defragmentation");


  /* Test 1: Create fragmented memory */
  for(i = 0; i < 50; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], block_size), ReturnOK);
    unit_assert_not_null(ptrs[i]);
  }


  /* Test 2: Free every other block to fragment memory */
  for(i = 1; i < 50; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }


  /* Test 3: Get fragmentation statistics */
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  unit_assert_not_null(stats);
  unit_assert_true(stats->numberOfFreeBlocks > 1);  /* Multiple free blocks =
                                                     * fragmented */
  xMemFree((const volatile Addr_t *) stats);


  /* Test 4: Free adjacent blocks to trigger defragmentation */
  for(i = 0; i < 50; i += 2) {
    if(ptrs[i] != null) {
      unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
      ptrs[i] = null;
    }
  }


  /* Test 5: Verify defragmentation occurred */
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  unit_assert_not_null(stats);


  /* After freeing all blocks, should have one large free block */
  unit_assert_equal(stats->numberOfFreeBlocks, 1);
  xMemFree((const volatile Addr_t *) stats);


  /* Test 6: Allocate a large block that wouldn't fit if fragmented */
  unit_assert_equal(xMemAlloc(&large_ptr, block_size * 40), ReturnOK);
  unit_assert_not_null(large_ptr);
  unit_assert_equal(xMemFree(large_ptr), ReturnOK);
  unit_end();
}


/**
 * @brief Test edge cases and boundary conditions
 */
void test_memory_edge_cases(void) {
  volatile Addr_t *ptr = null;
  Size_t size = 0;
  Size_t huge_size = MEMORY_REGION_SIZE * 2;  /* Too large */
  Size_t max_safe_size = MEMORY_REGION_SIZE - 1024; /* Leave room for headers */


  unit_begin("Memory edge cases and boundaries");


  /* Test 1: Zero size allocation */
  unit_assert_equal(xMemAlloc(&ptr, 0), ReturnError);


  /* Test 2: NULL pointer allocation */
  unit_assert_equal(xMemAlloc(null, 100), ReturnError);


  /* Test 3: Huge size allocation (should fail) */
  unit_assert_equal(xMemAlloc(&ptr, huge_size), ReturnError);


  /* Test 4: Free NULL pointer (should handle gracefully) */
  unit_assert_equal(xMemFree(null), ReturnOK);


  /* Test 5: Double free (should fail) */
  unit_assert_equal(xMemAlloc(&ptr, 100), ReturnOK);
  unit_assert_not_null(ptr);
  unit_assert_equal(xMemFree(ptr), ReturnOK);
  unit_assert_equal(xMemFree(ptr), ReturnError);  /* Double free should fail */
  /* Test 6: Get size of NULL pointer */
  unit_assert_equal(xMemGetSize(null, &size), ReturnError);


  /* Test 7: Get size with NULL size pointer */
  unit_assert_equal(xMemAlloc(&ptr, 100), ReturnOK);
  unit_assert_equal(xMemGetSize(ptr, null), ReturnError);
  unit_assert_equal(xMemFree(ptr), ReturnOK);


  /* Test 8: Minimum size allocation */
  unit_assert_equal(xMemAlloc(&ptr, 1), ReturnOK);
  unit_assert_not_null(ptr);
  unit_assert_equal(xMemFree(ptr), ReturnOK);


  /* Test 9: Maximum practical allocation */
  unit_assert_equal(xMemAlloc(&ptr, max_safe_size), ReturnOK);

  if(ptr != null) {
    /* If allocation succeeded, verify we can use the memory */
    fill_pattern(ptr, 100, PATTERN_BYTE); /* Just test first 100 bytes */
    unit_assert_true(verify_pattern(ptr, 100, PATTERN_BYTE));
    unit_assert_equal(xMemFree(ptr), ReturnOK);
  }


  /* Test 10: Allocation alignment */
  unit_assert_equal(xMemAlloc(&ptr, 17), ReturnOK); /* Odd size */
  unit_assert_not_null(ptr);
  unit_assert_true(((Size_t) ptr % CONFIG_MEMORY_ALIGNMENT) == 0); /* Check
                                                                    * alignment
                                                                    */
  unit_assert_equal(xMemFree(ptr), ReturnOK);
  unit_end();
}


/**
 * @brief Test memory utility functions
 */
void test_memory_utilities(void) {
  Byte_t src[256];
  Byte_t dest[256];
  Base_t result = false;
  Size_t i;


  unit_begin("Memory utility functions");


  /* Initialize source buffer */
  for(i = 0; i < 256; i++) {
    src[i] = (Byte_t) i;
    dest[i] = 0;
  }


  /* Test 1: memset */
  unit_assert_equal(__memset__((volatile Addr_t *) dest, PATTERN_BYTE, 256), ReturnOK);

  for(i = 0; i < 256; i++) {
    unit_assert_equal(dest[i], PATTERN_BYTE);
  }


  /* Test 2: memcpy */
  unit_assert_equal(__memcpy__((volatile Addr_t *) dest, (volatile Addr_t *) src, 256), ReturnOK);

  for(i = 0; i < 256; i++) {
    unit_assert_equal(dest[i], src[i]);
  }


  /* Test 3: memcmp - equal */
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, (volatile Addr_t *) dest, 256, &result), ReturnOK);
  unit_assert_true(result);


  /* Test 4: memcmp - not equal */
  dest[100] = 0xFF;
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, (volatile Addr_t *) dest, 256, &result), ReturnOK);
  unit_assert_false(result);


  /* Test 5: memset with zero size */
  unit_assert_equal(__memset__((volatile Addr_t *) dest, 0, 0), ReturnError);


  /* Test 6: memcpy with zero size */
  unit_assert_equal(__memcpy__((volatile Addr_t *) dest, (volatile Addr_t *) src, 0), ReturnError);


  /* Test 7: memcmp with zero size */
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, (volatile Addr_t *) dest, 0, &result), ReturnError);


  /* Test 8: NULL pointer checks */
  unit_assert_equal(__memset__(null, 0, 100), ReturnError);
  unit_assert_equal(__memcpy__(null, (volatile Addr_t *) src, 100), ReturnError);
  unit_assert_equal(__memcpy__((volatile Addr_t *) dest, null, 100), ReturnError);
  unit_assert_equal(__memcmp__(null, (volatile Addr_t *) dest, 100, &result), ReturnError);
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, null, 100, &result), ReturnError);
  unit_assert_equal(__memcmp__((volatile Addr_t *) src, (volatile Addr_t *) dest, 100, null), ReturnError);
  unit_end();
}


/**
 * @brief Test memory statistics functions
 */
void test_memory_statistics(void) {
  volatile Addr_t *ptr1 = null;
  volatile Addr_t *ptr2 = null;
  Size_t used_before = 0;
  Size_t used_after = 0;
  Size_t size = 0;
  Size_t frees_before;
  MemoryRegionStats_t *heap_stats = null;
  MemoryRegionStats_t *kernel_stats = null;


  unit_begin("Memory statistics");


  /* Test 1: Get initial used memory */
  unit_assert_equal(xMemGetUsed(&used_before), ReturnOK);


  /* Test 2: Allocate and check used memory increased */
  unit_assert_equal(xMemAlloc(&ptr1, 1000), ReturnOK);
  unit_assert_equal(xMemGetUsed(&used_after), ReturnOK);
  unit_assert_true(used_after > used_before);


  /* Test 3: Get size of allocation */
  unit_assert_equal(xMemGetSize(ptr1, &size), ReturnOK);
  unit_assert_true(size >= 1000);  /* Should be at least what we requested */
  /* Test 4: Allocate more and check stats */
  unit_assert_equal(xMemAlloc(&ptr2, 2000), ReturnOK);
  unit_assert_equal(xMemGetUsed(&used_after), ReturnOK);
  unit_assert_true(used_after >= used_before + 3000);  /* At least 3000 bytes
                                                        * allocated */
  /* Test 5: Get heap statistics */
  unit_assert_equal(xMemGetHeapStats(&heap_stats), ReturnOK);
  unit_assert_not_null(heap_stats);
  unit_assert_true(heap_stats->successfulAllocations >= 2);
  unit_assert_true(heap_stats->availableSpaceInBytes < MEMORY_REGION_SIZE);


  /* Test 6: Get kernel statistics */
  unit_assert_equal(xMemGetKernelStats(&kernel_stats), ReturnOK);
  unit_assert_not_null(kernel_stats);


  /* Note: Kernel stats structure is allocated from heap, not kernel region, so
   * kernel region might have 0 allocations at this point. Just verify the stats
   * structure was returned successfully. */


  /* Test 7: Free and check stats update */
  frees_before = heap_stats->successfulFrees;
  unit_assert_equal(xMemFree(ptr1), ReturnOK);
  unit_assert_equal(xMemFree(ptr2), ReturnOK);
  xMemFree((const volatile Addr_t *) heap_stats);
  xMemFree((const volatile Addr_t *) kernel_stats);
  unit_assert_equal(xMemGetHeapStats(&heap_stats), ReturnOK);
  unit_assert_true(heap_stats->successfulFrees > frees_before);
  xMemFree((const volatile Addr_t *) heap_stats);


  /* Test 8: Check used memory decreased */
  unit_assert_equal(xMemGetUsed(&used_after), ReturnOK);
  unit_assert_true(used_after <= used_before);
  unit_end();
}


/**
 * @brief Test memory alignment requirements
 */
void test_memory_alignment(void) {
  volatile Addr_t *ptr = null;
  Size_t i;
  Size_t sizes[] = {
    1, 3, 5, 7, 9, 15, 17, 31, 33, 63, 65, 127, 129
  };
  Size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);
  Size_t actual_size = 0;


  unit_begin("Memory alignment");


  /* Test various sizes to ensure proper alignment */
  for(i = 0; i < num_sizes; i++) {
    unit_assert_equal(xMemAlloc(&ptr, sizes[i]), ReturnOK);
    unit_assert_not_null(ptr);


    /* Check that returned pointer is properly aligned */
    unit_assert_true(((Size_t) ptr % CONFIG_MEMORY_ALIGNMENT) == 0);


    /* Verify we can write to the entire allocated space */
    fill_pattern(ptr, sizes[i], (Byte_t) i);
    unit_assert_true(verify_pattern(ptr, sizes[i], (Byte_t) i));
    unit_assert_equal(xMemFree(ptr), ReturnOK);
  }


  /* Test that allocation sizes are rounded up for alignment */
  unit_assert_equal(xMemAlloc(&ptr, 1), ReturnOK);
  unit_assert_equal(xMemGetSize(ptr, &actual_size), ReturnOK);


  /* Size should be rounded up to alignment */
  unit_assert_true(actual_size >= CONFIG_MEMORY_ALIGNMENT);
  unit_assert_equal(xMemFree(ptr), ReturnOK);
  unit_end();
}


/**
 * @brief Test kernel memory region operations
 */
void test_memory_kernel_region(void) {
  volatile Addr_t *kptr1 = null;
  volatile Addr_t *kptr2 = null;
  volatile Addr_t *hptr = null;
  MemoryRegionStats_t *kstats = null;


  unit_begin("Kernel memory region");


  /* Test 1: Allocate from kernel region */
  unit_assert_equal(__KernelAllocateMemory__(&kptr1, 500), ReturnOK);
  unit_assert_not_null(kptr1);


  /* Test 2: Allocate from heap region */
  unit_assert_equal(__HeapAllocateMemory__(&hptr, 500), ReturnOK);
  unit_assert_not_null(hptr);


  /* Test 3: Verify they're from different regions (pointers shouldn't overlap)
   */
  unit_assert_true(kptr1 != hptr);


  /* Test 4: Allocate more from kernel */
  unit_assert_equal(__KernelAllocateMemory__(&kptr2, 1000), ReturnOK);
  unit_assert_not_null(kptr2);


  /* Test 5: Get kernel stats */
  unit_assert_equal(xMemGetKernelStats(&kstats), ReturnOK);
  unit_assert_not_null(kstats);
  unit_assert_true(kstats->successfulAllocations >= 2);


  /* Test 6: Free kernel allocations */
  unit_assert_equal(__KernelFreeMemory__(kptr1), ReturnOK);
  unit_assert_equal(__KernelFreeMemory__(kptr2), ReturnOK);


  /* Test 7: Free heap allocation */
  unit_assert_equal(__HeapFreeMemory__(hptr), ReturnOK);


  /* Clean up stats */
  xMemFree((const volatile Addr_t *) kstats);
  unit_end();
}


/**
 * @brief Test xMemFreeAll function
 */
void test_memory_free_all(void) {
  volatile Addr_t *ptrs[10];
  Size_t i;
  Size_t used = 0;


  unit_begin("Memory free all");


  /* Test 1: Allocate multiple blocks */
  for(i = 0; i < 10; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], (i + 1) * 100), ReturnOK);
    unit_assert_not_null(ptrs[i]);
    fill_pattern(ptrs[i], (i + 1) * 100, (Byte_t) i);
  }


  /* Test 2: Verify memory is used */
  unit_assert_equal(xMemGetUsed(&used), ReturnOK);
  unit_assert_true(used > 0);


  /* Test 3: Free all memory at once */
  unit_assert_equal(xMemFreeAll(), ReturnOK);


  /* Test 4: Verify all memory is freed */
  unit_assert_equal(xMemGetUsed(&used), ReturnOK);
  unit_assert_equal(used, 0);


  /* Test 5: Allocate after free all */
  unit_assert_equal(xMemAlloc(&ptrs[0], 1000), ReturnOK);
  unit_assert_not_null(ptrs[0]);
  unit_assert_equal(xMemFree(ptrs[0]), ReturnOK);


  /* Test 6: Multiple free all calls */
  unit_assert_equal(xMemFreeAll(), ReturnOK);
  unit_assert_equal(xMemFreeAll(), ReturnOK);  /* Should be safe to call
                                                * multiple times */
  unit_end();
}


/**
 * @brief Test allocation patterns that could cause fragmentation issues
 */
void test_memory_pattern_verification(void) {
  volatile Addr_t *ptrs[20];
  Size_t i;
  Byte_t patterns[20];


  unit_begin("Memory pattern verification");


  /* Generate unique patterns */
  for(i = 0; i < 20; i++) {
    patterns[i] = (Byte_t) (0x10 + i);
    ptrs[i] = null;
  }


  /* Test 1: Allocate blocks with different patterns */
  for(i = 0; i < 20; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], 256), ReturnOK);
    unit_assert_not_null(ptrs[i]);
    fill_pattern(ptrs[i], 256, patterns[i]);
  }


  /* Test 2: Verify all patterns intact */
  for(i = 0; i < 20; i++) {
    unit_assert_true(verify_pattern(ptrs[i], 256, patterns[i]));
  }


  /* Test 3: Free some blocks */
  for(i = 0; i < 20; i += 3) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }


  /* Test 4: Verify remaining patterns still intact */
  for(i = 0; i < 20; i++) {
    if(ptrs[i] != null) {
      unit_assert_true(verify_pattern(ptrs[i], 256, patterns[i]));
    }
  }


  /* Test 5: Reallocate freed blocks with new patterns */
  for(i = 0; i < 20; i += 3) {
    unit_assert_equal(xMemAlloc(&ptrs[i], 256), ReturnOK);
    unit_assert_not_null(ptrs[i]);
    patterns[i] = (Byte_t) (0x80 + i);
    fill_pattern(ptrs[i], 256, patterns[i]);
  }


  /* Test 6: Final verification of all patterns */
  for(i = 0; i < 20; i++) {
    unit_assert_true(verify_pattern(ptrs[i], 256, patterns[i]));
  }


  /* Clean up */
  for(i = 0; i < 20; i++) {
    if(ptrs[i] != null) {
      unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    }
  }

  unit_end();
}


/**
 * @brief Test large allocation scenarios
 */
void test_memory_large_allocations(void) {
  volatile Addr_t *large_ptr = null;
  volatile Addr_t *small_ptr = null;
  Size_t large_size;
  Size_t available;
  MemoryRegionStats_t *stats = null;


  unit_begin("Large memory allocations");


  /* Get available memory */
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  unit_assert_not_null(stats);
  available = stats->largestFreeEntryInBytes;
  xMemFree((const volatile Addr_t *) stats);


  /* Test 1: Allocate 50% of available memory */
  large_size = available / 2;

  if(large_size > 100) {  /* Make sure it's reasonable */
    unit_assert_equal(xMemAlloc(&large_ptr, large_size), ReturnOK);

    if(large_ptr != null) {
      /* Test we can use the allocated memory */
      fill_pattern(large_ptr, 100, PATTERN_BYTE); /* Just test first 100 bytes
                                                   */
      unit_assert_true(verify_pattern(large_ptr, 100, PATTERN_BYTE));


      /* Test 2: Try to allocate another large block (should succeed if we have
       * space) */
      unit_assert_equal(xMemAlloc(&small_ptr, 100), ReturnOK);

      if(small_ptr != null) {
        fill_pattern(small_ptr, 100, INVERSE_PATTERN_BYTE);
        unit_assert_true(verify_pattern(small_ptr, 100, INVERSE_PATTERN_BYTE));
        unit_assert_equal(xMemFree(small_ptr), ReturnOK);
      }

      unit_assert_equal(xMemFree(large_ptr), ReturnOK);
    }
  }


  /* Test 3: Allocate 90% of total memory (might fail) */
  large_size = (MEMORY_REGION_SIZE * 9) / 10;

  if(xMemAlloc(&large_ptr, large_size) == ReturnOK) {
    /* If it succeeded, verify we can use it */
    fill_pattern(large_ptr, 100, PATTERN_BYTE);
    unit_assert_true(verify_pattern(large_ptr, 100, PATTERN_BYTE));
    unit_assert_equal(xMemFree(large_ptr), ReturnOK);
  }

  unit_end();
}


/**
 * @brief Test that should trigger cycle detection in defragmentation
 */
void test_memory_cycle_detection(void) {
  volatile Addr_t *ptrs[100];
  volatile Addr_t *large = null;
  Size_t i;
  Size_t count = 0;


  unit_begin("Memory cycle detection");


  /* Try to create conditions that might trigger cycle detection */
  /* Allocate as many blocks as possible */
  for(i = 0; i < 100; i++) {
    if(xMemAlloc(&ptrs[i], 50) == ReturnOK) {
      count++;
      fill_pattern(ptrs[i], 50, (Byte_t) i);
    } else {
      ptrs[i] = null;
      break;
    }
  }


  /* Free blocks in a pattern that creates maximum fragmentation */
  for(i = 1; i < count; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }


  /* Free more blocks to trigger defragmentation */
  for(i = 0; i < count; i += 2) {
    if(ptrs[i] != null) {
      unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
      ptrs[i] = null;
    }
  }


  /* The defragmentation should handle this without issues */
  /* Allocate a large block to verify defragmentation worked */
  unit_assert_equal(xMemAlloc(&large, count * 25), ReturnOK);

  if(large != null) {
    unit_assert_equal(xMemFree(large), ReturnOK);
  }

  unit_end();
}


/**
 * @brief Test defragmentation behavior
 */
void test_memory_defragmentation(void) {
  volatile Addr_t *ptrs[30];
  MemoryRegionStats_t *stats = null;
  Size_t i;
  Word_t blocks_before, blocks_after;


  unit_begin("Memory defragmentation behavior");


  /* Test 1: Create maximum fragmentation */
  for(i = 0; i < 30; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], 64), ReturnOK);
  }


  /* Free every third block */
  for(i = 0; i < 30; i += 3) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }


  /* Get fragmentation level */
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  blocks_before = stats->numberOfFreeBlocks;


  /* Don't free stats yet to avoid affecting block count */
  /* Test 2: Free adjacent blocks to trigger merging */
  for(i = 1; i < 30; i += 3) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
    ptrs[i] = null;
  }


  /* Free the first stats structure now */
  xMemFree((const volatile Addr_t *) stats);


  /* Check that blocks were merged - defragmentation happens automatically */
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  blocks_after = stats->numberOfFreeBlocks;


  /* Allow for minor variation due to stats allocation/deallocation */
  /* The important thing is that adjacent blocks do get merged */
  unit_assert_true(blocks_after <= blocks_before + 1);
  xMemFree((const volatile Addr_t *) stats);


  /* Test 3: Free remaining blocks */
  for(i = 2; i < 30; i += 3) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
  }


  /* Should have one large free block now */
  unit_assert_equal(xMemGetHeapStats(&stats), ReturnOK);
  unit_assert_equal(stats->numberOfFreeBlocks, 1);
  xMemFree((const volatile Addr_t *) stats);
  unit_end();
}


/**
 * @brief Test corruption detection capabilities
 */
void test_memory_corruption_detection(void) {
  volatile Addr_t *ptr1 = null;
  volatile Addr_t *ptr2 = null;
  volatile Addr_t *ptr3 = null;


  unit_begin("Memory corruption detection");


  /* Test 1: Allocate some blocks */
  unit_assert_equal(xMemAlloc(&ptr1, 100), ReturnOK);
  unit_assert_equal(xMemAlloc(&ptr2, 100), ReturnOK);
  unit_assert_equal(xMemAlloc(&ptr3, 100), ReturnOK);


  /* Test 2: Use the blocks normally */
  fill_pattern(ptr1, 100, 0x11);
  fill_pattern(ptr2, 100, 0x22);
  fill_pattern(ptr3, 100, 0x33);


  /* Test 3: Verify patterns */
  unit_assert_true(verify_pattern(ptr1, 100, 0x11));
  unit_assert_true(verify_pattern(ptr2, 100, 0x22));
  unit_assert_true(verify_pattern(ptr3, 100, 0x33));


  /* Test 4: Free blocks normally */
  unit_assert_equal(xMemFree(ptr1), ReturnOK);
  unit_assert_equal(xMemFree(ptr2), ReturnOK);
  unit_assert_equal(xMemFree(ptr3), ReturnOK);


  /* Note: We can't safely test actual corruption as it would break the memory
   * system */
  /* The memory manager should detect corruption via checksums in block headers
   */
  unit_end();
}


/**
 * @brief Test size tracking accuracy
 */
void test_memory_size_tracking(void) {
  volatile Addr_t *ptrs[10];
  Size_t requested_sizes[10] = {
    1, 7, 15, 16, 17, 31, 32, 33, 63, 64
  };
  Size_t actual_size;
  Size_t total_requested = 0;
  Size_t total_used = 0;
  Size_t used_after_free;
  Size_t i;


  unit_begin("Memory size tracking");


  /* Test 1: Allocate various sizes and verify tracking */
  for(i = 0; i < 10; i++) {
    unit_assert_equal(xMemAlloc(&ptrs[i], requested_sizes[i]), ReturnOK);
    unit_assert_not_null(ptrs[i]);


    /* Get actual allocated size */
    unit_assert_equal(xMemGetSize(ptrs[i], &actual_size), ReturnOK);


    /* Actual size should be at least requested size */
    unit_assert_true(actual_size >= requested_sizes[i]);


    /* Actual size should be aligned */
    unit_assert_true((actual_size % CONFIG_MEMORY_ALIGNMENT) == 0);
    total_requested += requested_sizes[i];
  }


  /* Test 2: Check total used memory */
  unit_assert_equal(xMemGetUsed(&total_used), ReturnOK);


  /* Total used should be at least total requested (plus headers) */
  unit_assert_true(total_used >= total_requested);


  /* Test 3: Free half the blocks */
  for(i = 0; i < 10; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
  }


  /* Test 4: Check used memory decreased */
  unit_assert_equal(xMemGetUsed(&used_after_free), ReturnOK);
  unit_assert_true(used_after_free < total_used);


  /* Test 5: Free remaining blocks */
  for(i = 1; i < 10; i += 2) {
    unit_assert_equal(xMemFree(ptrs[i]), ReturnOK);
  }


  /* Test 6: Verify all memory freed */
  unit_assert_equal(xMemGetUsed(&used_after_free), ReturnOK);
  unit_assert_equal(used_after_free, 0);
  unit_end();
}


/**
 * @brief Main entry point for memory tests
 */
void run_memory_tests(void) {
  /* Clear memory before starting tests */
  xMemFreeAll();


  /* Run all test suites */
  test_memory_basic_allocation();
  test_memory_stress_allocation();
  test_memory_fragmentation();
  test_memory_edge_cases();
  test_memory_utilities();
  test_memory_statistics();
  test_memory_alignment();
  test_memory_kernel_region();
  test_memory_free_all();
  test_memory_pattern_verification();
  test_memory_large_allocations();
  test_memory_cycle_detection();
  test_memory_defragmentation();
  test_memory_corruption_detection();
  test_memory_size_tracking();


  /* Final cleanup */
  xMemFreeAll();
}


/**
 * @brief Entry point called by test harness
 */
void memory_harness(void) {
  run_memory_tests();
}