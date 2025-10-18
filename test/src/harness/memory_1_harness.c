/*UNCRUSTIFY-OFF*/
/**
 * @file memory_1_harness.c
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
#include "memory_1_harness.h"


/* Test constants */
#define NUM_TEST_ALLOCS 0x20u /* Number of allocation test iterations */
#define OVERSIZED_ALLOC 0x99999u /* Size that should fail allocation */
#define LARGE_BLOCK_SIZE 0x32000u /* 204,800 bytes - large allocation test */
#define LARGE_BLOCK_USED 0x32020u /* Expected memory used after large alloc */
#define HEAP_AVAILABLE_BYTES 0x63A0u /* Expected heap available space */
#define HEAP_FREE_BLOCKS 0x31Du /* Expected number of free blocks */
#define HEAP_ALLOC_COUNT 0x24u /* Expected successful allocations */
#define HEAP_FREE_COUNT 0x22u /* Expected successful frees */
#define KERNEL_AVAILABLE_BYTES 0x383C0u /* Expected kernel available space */
#define KERNEL_MIN_FREE 0x38340u /* Minimum ever free bytes remaining */
#define KERNEL_FREE_BLOCKS 0x1C1Eu /* Expected kernel free blocks */
#define KERNEL_ALLOC_COUNT 0x2u /* Expected kernel allocations */
#define KERNEL_FREE_COUNT 0x1u /* Expected kernel frees */
#define SMALL_ALLOC_SIZE 128 /* Small allocation test size */
#define TINY_ALLOC_SIZE 64 /* Tiny block for max allocs test */
#define MEDIUM_ALLOC_SIZE 256 /* Medium allocation size */
#define LARGE_ALLOC_SIZE 512 /* Large allocation size */
#define FRAG_BLOCK_SIZE 1024 /* Fragmentation test block size */
#define MAX_SIZE_TEST 0xFFFFFFFFu /* Maximum size boundary test */
#define MAX_TEST_ALLOCS 100 /* Maximum test allocations */
static Size_t sizes[NUM_TEST_ALLOCS] = {
  0x2532u, 0x1832u, 0x132u, 0x2932u, 0x332u, 0x1432u, 0x1332u, 0x532u, 0x1732u, 0x932u, 0x1432u, 0x2232u, 0x1432u, 0x3132u, 0x032u, 0x1132u, 0x632u, 0x932u,
    0x1532u, 0x632u, 0x1832u, 0x132u, 0x1332u, 0x3132u, 0x2732u, 0x1532u, 0x2432u, 0x2932u, 0x2432u, 0x2932u, 0x3032u, 0x2332u
};
static Size_t order[NUM_TEST_ALLOCS] = {
  0x02u, 0x16u, 0x07u, 0x0Cu, 0x06u, 0x00u, 0x0Du, 0x18u, 0x10u, 0x08u, 0x0Au, 0x1Eu, 0x0Bu, 0x0Eu, 0x03u, 0x09u, 0x19u, 0x05u, 0x1Cu, 0x1Du, 0x0Fu, 0x01u,
    0x1Au, 0x04u, 0x13u, 0x11u, 0x1Fu, 0x12u, 0x17u, 0x15u, 0x14u, 0x1Bu
};
static MemoryTest_t tests[NUM_TEST_ALLOCS];


void memory_1_harness(void) {
  Size_t i;
  Size_t used;
  Size_t actual;
  Base_t *mem01;
  MemoryRegionStats_t *mem02;
  MemoryRegionStats_t *mem03;
  Task_t *mem04 = null;
  Byte_t *mem05 = null;


  unit_begin("Unit test for memory region defragmentation routine");
  i = nil;
  used = nil;
  actual = nil;

  for(i = 0; i < NUM_TEST_ALLOCS; i++) {
    tests[i].size = sizes[i];
    tests[i].blocks = (sizes[i] / CONFIG_MEMORY_REGION_BLOCK_SIZE) + 1;

    if(nil < ((Size_t) (sizes[i] % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
      tests[i].blocks += 1;
    }

    unit_assert_ok(xMemAlloc((volatile Addr_t **) &tests[i].ptr, sizes[i]));
    unit_assert_not_null(tests[i].ptr);
    used += tests[i].blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
    unit_assert_ok(xMemGetUsed(&actual));
    unit_assert_equal(used, actual);
    unit_assert_ok(xMemGetSize(tests[i].ptr, &actual));
    unit_assert_equal(tests[i].blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE, actual);
  }

  unit_assert_not_ok(xMemAlloc((volatile Addr_t **) &mem05, OVERSIZED_ALLOC));

  for(i = 0; i < NUM_TEST_ALLOCS; i++) {
    unit_assert_ok(xMemFree(tests[order[i]].ptr));
  }

  unit_assert_ok(xMemGetUsed(&actual));
  unit_assert_equal(actual, 0x0u);
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &mem05, (CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS - 1) * CONFIG_MEMORY_REGION_BLOCK_SIZE));
  actual = nil;
  unit_assert_ok(xMemGetUsed(&actual));
  unit_assert_equal(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE, actual);
  unit_assert_ok(xMemFree(mem05));
  unit_end();
  unit_begin("Memory allocation succeeds for large block");
  mem01 = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &mem01, LARGE_BLOCK_SIZE));
  unit_assert_not_null(mem01);
  unit_end();
  unit_begin("Used memory tracking reflects allocations");
  unit_assert_ok(xMemGetUsed(&actual));
  unit_assert_equal(actual, LARGE_BLOCK_USED);
  unit_end();
  unit_begin("Allocated block size retrieval is accurate");
  unit_assert_ok(xMemGetSize(mem01, &actual));
  unit_assert_equal(actual, LARGE_BLOCK_USED);
  unit_end();
  unit_begin("Heap statistics reflect current memory state");
  mem02 = null;
  unit_assert_ok(xMemGetHeapStats(&mem02));
  unit_assert_not_null(mem02);
  unit_assert_equal(mem02->availableSpaceInBytes, HEAP_AVAILABLE_BYTES);
  unit_assert_equal(mem02->largestFreeEntryInBytes, HEAP_AVAILABLE_BYTES);
  unit_assert_equal(mem02->minimumEverFreeBytesRemaining, 0x0u);
  unit_assert_equal(mem02->numberOfFreeBlocks, HEAP_FREE_BLOCKS);
  unit_assert_equal(mem02->smallestFreeEntryInBytes, HEAP_AVAILABLE_BYTES);
  unit_assert_equal(mem02->successfulAllocations, HEAP_ALLOC_COUNT); /* +1 from
                                                                      * xSystemGetSystemInfo
                                                                      */
  unit_assert_equal(mem02->successfulFrees, HEAP_FREE_COUNT); /* +1 from
                                                               * xSystemGetSystemInfo
                                                               */
  unit_end();
  unit_begin("Kernel statistics track internal allocations");
  mem03 = null;
  mem04 = null;
  unit_assert_ok(xTaskCreate(&mem04, (Byte_t *) "NONE", memory_1_harness_task, null));
  unit_assert_not_null(mem04);
  unit_assert_ok(xTaskDelete(mem04));
  unit_assert_ok(xMemGetKernelStats(&mem03));
  unit_assert_not_null(mem03);
  unit_assert_equal(mem03->availableSpaceInBytes, KERNEL_AVAILABLE_BYTES);
  unit_assert_equal(mem03->largestFreeEntryInBytes, KERNEL_AVAILABLE_BYTES);
  unit_assert_equal(mem03->minimumEverFreeBytesRemaining, KERNEL_MIN_FREE);
  unit_assert_equal(mem03->numberOfFreeBlocks, KERNEL_FREE_BLOCKS);
  unit_assert_equal(mem03->smallestFreeEntryInBytes, KERNEL_AVAILABLE_BYTES);
  unit_assert_equal(mem03->successfulAllocations, KERNEL_ALLOC_COUNT);
  unit_assert_equal(mem03->successfulFrees, KERNEL_FREE_COUNT);
  unit_assert_ok(xMemFree(mem01));
  unit_assert_ok(xMemFree(mem02));
  unit_assert_ok(xMemFree(mem03));
  unit_end();


  /* Edge case tests */
  test_memory_edge_cases();

  return;
}


void test_memory_edge_cases(void) {
  volatile Addr_t *ptr1 = null;
  volatile Addr_t *ptr2 = null;
  Size_t size = nil;


  /* Test NULL pointer handling */
  unit_begin("Edge Case - xMemAlloc() NULL Pointer");
  unit_assert_not_ok(xMemAlloc(null, SMALL_ALLOC_SIZE));
  unit_end();


  /* Test zero size allocation */
  unit_begin("Edge Case - xMemAlloc() Zero Size");
  ptr1 = null;
  unit_assert_not_ok(xMemAlloc(&ptr1, 0));
  unit_assert_null(ptr1);
  unit_end();


  /* Test oversized allocation */
  unit_begin("Edge Case - xMemAlloc() Oversized");
  ptr1 = null;
  unit_assert_not_ok(xMemAlloc(&ptr1, MAX_SIZE_TEST));
  unit_assert_null(ptr1);
  unit_end();


  /* Test freeing NULL pointer (should succeed like standard C free()) */
  unit_begin("Edge Case - xMemFree() NULL Pointer");
  unit_assert_ok(xMemFree(null));
  unit_end();


  /* Test double free */
  unit_begin("Edge Case - xMemFree() Double Free");
  ptr1 = null;
  unit_assert_ok(xMemAlloc(&ptr1, SMALL_ALLOC_SIZE));
  unit_assert_not_null(ptr1);
  unit_assert_ok(xMemFree(ptr1));


  /* Attempting to free again should fail */
  unit_assert_not_ok(xMemFree(ptr1));
  unit_end();


  /* Test getting size of NULL pointer */
  unit_begin("Edge Case - xMemGetSize() NULL Pointer");
  size = nil;
  unit_assert_not_ok(xMemGetSize(null, &size));
  unit_end();


  /* Test getting size with NULL output parameter */
  unit_begin("Edge Case - xMemGetSize() NULL Output");
  ptr1 = null;
  unit_assert_ok(xMemAlloc(&ptr1, SMALL_ALLOC_SIZE));
  unit_assert_not_null(ptr1);
  unit_assert_not_ok(xMemGetSize(ptr1, null));
  unit_assert_ok(xMemFree(ptr1));
  unit_end();


  /* Test xMemGetUsed with NULL parameter */
  unit_begin("Edge Case - xMemGetUsed() NULL Pointer");
  unit_assert_not_ok(xMemGetUsed(null));
  unit_end();


  /* Test xMemGetHeapStats with NULL parameter */
  unit_begin("Edge Case - xMemGetHeapStats() NULL Pointer");
  unit_assert_not_ok(xMemGetHeapStats(null));
  unit_end();


  /* Test xMemGetKernelStats with NULL parameter */
  unit_begin("Edge Case - xMemGetKernelStats() NULL Pointer");
  unit_assert_not_ok(xMemGetKernelStats(null));
  unit_end();


  /* Test maximum number of allocations */
  unit_begin("Edge Case - Maximum Allocations");
  {
    volatile Addr_t *ptrs[MAX_TEST_ALLOCS];
    int i;
    int allocCount = 0;
    Size_t sizeBefore = 0;
    Size_t sizeAfter = 0;


    /* Get baseline memory usage */
    unit_assert_ok(xMemGetUsed(&sizeBefore));

    /* Allocate as many small blocks as possible */
    for(i = 0; i < MAX_TEST_ALLOCS; i++) {
      ptrs[i] = null;

      if(OK(xMemAlloc(&ptrs[i], TINY_ALLOC_SIZE))) {
        unit_assert_not_null(ptrs[i]);
        allocCount++;
      } else {
        break;
      }
    }

    /* Should have allocated at least some blocks */
    unit_assert_true(allocCount > 0);

    /* Free all allocated blocks */
    for(i = 0; i < allocCount; i++) {
      unit_assert_ok(xMemFree(ptrs[i]));
    }

    /* Verify memory returns to baseline */
    unit_assert_ok(xMemGetUsed(&sizeAfter));
    unit_assert_equal(sizeBefore, sizeAfter);
  } unit_end();


  /* Test fragmentation resilience */
  unit_begin("Edge Case - Memory Fragmentation");
  {
    volatile Addr_t *frag1 = null;
    volatile Addr_t *frag2 = null;
    volatile Addr_t *frag3 = null;


    /* Create fragmented memory pattern */
    unit_assert_ok(xMemAlloc(&frag1, FRAG_BLOCK_SIZE));
    unit_assert_ok(xMemAlloc(&frag2, FRAG_BLOCK_SIZE));
    unit_assert_ok(xMemAlloc(&frag3, FRAG_BLOCK_SIZE));


    /* Free middle block */
    unit_assert_ok(xMemFree(frag2));


    /* Try to allocate a block that fits in the freed space */
    frag2 = null;
    unit_assert_ok(xMemAlloc(&frag2, LARGE_ALLOC_SIZE));
    unit_assert_not_null(frag2);


    /* Cleanup */
    unit_assert_ok(xMemFree(frag1));
    unit_assert_ok(xMemFree(frag2));
    unit_assert_ok(xMemFree(frag3));
  } unit_end();


  /* Test allocation after xMemFreeAll */
  unit_begin("Edge Case - Allocation After xMemFreeAll()");
  ptr1 = null;
  ptr2 = null;
  unit_assert_ok(xMemAlloc(&ptr1, MEDIUM_ALLOC_SIZE));
  unit_assert_ok(xMemAlloc(&ptr2, LARGE_ALLOC_SIZE));


  /* Free all memory */
  unit_assert_ok(xMemFreeAll());


  /* Verify memory is freed */
  unit_assert_ok(xMemGetUsed(&size));
  unit_assert_equal(size, 0x0u);


  /* Allocate again - should succeed */
  ptr1 = null;
  unit_assert_ok(xMemAlloc(&ptr1, SMALL_ALLOC_SIZE));
  unit_assert_not_null(ptr1);
  unit_assert_ok(xMemFree(ptr1));
  unit_end();
}


void memory_1_harness_task(Task_t *task_, TaskParm_t *parm_) {
  xTaskSuspendAll();

  return;
}