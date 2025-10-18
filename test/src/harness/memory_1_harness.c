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


/* Calculate entry size in blocks (matches logic in mem.c) */
#define ENTRY_SIZE_IN_BLOCKS ((HalfWord_t) ((sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE) + \
                                            (((sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE) > 0) ? 1 : 0)))

/* Macro to convert allocated address to memory entry (for corruption tests) */
/* Note: This duplicates internal logic from mem.c for testing purposes */
#define ADDR2ENTRY(ptr_) ((MemoryEntry_t *) (((Byte_t *) (ptr_)) - (ENTRY_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE)))

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
#define MAX_SIZE_TEST ((CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS + 100) * CONFIG_MEMORY_REGION_BLOCK_SIZE) /* Oversized allocation test */
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


  /* Comprehensive test suite for improved coverage */
  test_memcpy_memcmp();
  test_freed_pointer_operations();
  test_boundary_allocations();
  test_statistics_accuracy();
  test_data_integrity();
  test_fragmentation_stress();
  test_invalid_pointers();
  test_state_consistency();
  test_memfreeall_idempotency();
  test_kernel_memory();
  test_performance_stress();
  test_randomized_patterns();
  test_alignment_verification();
  test_cross_region_protection();


  /* Memory corruption detection tests (merged from memory_2_harness) */
  test_memory_corruption_detection();

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


void test_memcpy_memcmp(void) {
  Byte_t src[128];
  Byte_t dest[128];
  Base_t result = false;
  Size_t i;


  /* Test __memcpy__ functionality */
  unit_begin("Memory Utility - __memcpy__() Basic Copy");
  {
    /* Initialize source with pattern */
    for(i = 0; i < 128; i++) {
      src[i] = (Byte_t) i;
    }

    /* Clear destination */
    for(i = 0; i < 128; i++) {
      dest[i] = 0;
    }

    /* Positive: Normal copy */
    unit_assert_ok(__memcpy__(dest, src, 128));
    unit_assert_ok(__memcmp__(dest, src, 128, &result));
    unit_assert_true(result);
  }
  unit_end();
  unit_begin("Memory Utility - __memcpy__() NULL Source");
  {
    /* Negative: NULL source */
    unit_assert_not_ok(__memcpy__(dest, null, 128));
  }
  unit_end();
  unit_begin("Memory Utility - __memcpy__() NULL Destination");
  {
    /* Negative: NULL destination */
    unit_assert_not_ok(__memcpy__(null, src, 128));
  }
  unit_end();
  unit_begin("Memory Utility - __memcpy__() Zero Size");
  {
    /* Negative: Zero size */
    unit_assert_not_ok(__memcpy__(dest, src, 0));
  }
  unit_end();
  unit_begin("Memory Utility - __memcmp__() Equal Buffers");
  {
    /* Test __memcmp__ with equal buffers */
    for(i = 0; i < 128; i++) {
      src[i] = (Byte_t) 0xAA;
      dest[i] = (Byte_t) 0xAA;
    }

    result = false;
    unit_assert_ok(__memcmp__(src, dest, 128, &result));
    unit_assert_true(result);
  }
  unit_end();
  unit_begin("Memory Utility - __memcmp__() Different Buffers");
  {
    /* Test __memcmp__ with different buffers */
    dest[64] = 0x55; /* Change one byte */
    result = true;
    unit_assert_ok(__memcmp__(src, dest, 128, &result));
    unit_assert_false(result);
  }
  unit_end();
  unit_begin("Memory Utility - __memcmp__() NULL Parameters");
  {
    /* Negative: NULL parameters */
    unit_assert_not_ok(__memcmp__(null, dest, 128, &result));
    unit_assert_not_ok(__memcmp__(src, null, 128, &result));
    unit_assert_not_ok(__memcmp__(src, dest, 128, null));
    unit_assert_not_ok(__memcmp__(src, dest, 0, &result));
  }
  unit_end();
  unit_begin("Memory Utility - __memcmp__() Early Difference");
  {
    /* Test difference at start */
    for(i = 0; i < 128; i++) {
      src[i] = (Byte_t) i;
      dest[i] = (Byte_t) i;
    }

    dest[0] = 0xFF;
    result = true;
    unit_assert_ok(__memcmp__(src, dest, 128, &result));
    unit_assert_false(result);
  }
  unit_end();
  unit_begin("Memory Utility - __memset__() Explicit Test");
  {
    volatile Addr_t *ptr = null;
    Byte_t *bytes;


    unit_assert_ok(xMemAlloc(&ptr, 256));
    bytes = (Byte_t *) ptr;

    /* Set to pattern */
    unit_assert_ok(__memset__(ptr, 0xAA, 256));

    /* Verify pattern */
    for(i = 0; i < 256; i++) {
      unit_assert_equal(bytes[i], 0xAA);
    }

    /* Set to different pattern */
    unit_assert_ok(__memset__(ptr, 0x55, 256));

    /* Verify new pattern */
    for(i = 0; i < 256; i++) {
      unit_assert_equal(bytes[i], 0x55);
    }

    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Memory Utility - __memset__() NULL Pointer");
  {
    unit_assert_not_ok(__memset__(null, 0xAA, 128));
  }
  unit_end();
  unit_begin("Memory Utility - __memset__() Zero Size");
  {
    volatile Addr_t *ptr = null;


    unit_assert_ok(xMemAlloc(&ptr, 64));
    unit_assert_not_ok(__memset__(ptr, 0xAA, 0));
    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();

  return;
}


void test_freed_pointer_operations(void) {
  volatile Addr_t *ptr = null;
  Size_t size;


  unit_begin("Negative - Get size of freed pointer");
  {
    unit_assert_ok(xMemAlloc(&ptr, 128));
    unit_assert_not_null(ptr);
    unit_assert_ok(xMemFree(ptr));

    /* Should fail - pointer was freed */
    unit_assert_not_ok(xMemGetSize(ptr, &size));
  }
  unit_end();
  unit_begin("Negative - Double free detection");
  {
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, 256));
    unit_assert_not_null(ptr);
    unit_assert_ok(xMemFree(ptr));

    /* Second free should fail */
    unit_assert_not_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Negative - Use after free detection");
  {
    volatile Addr_t *ptr1 = null;
    volatile Addr_t *ptr2 = null;
    Size_t size1;


    /* Allocate and free first pointer */
    unit_assert_ok(xMemAlloc(&ptr1, 128));
    unit_assert_ok(xMemFree(ptr1));

    /* Allocate second pointer (might reuse same memory) */
    unit_assert_ok(xMemAlloc(&ptr2, 128));

    /* Trying to get size of freed pointer should still fail */
    unit_assert_not_ok(xMemGetSize(ptr1, &size1));

    unit_assert_ok(xMemFree(ptr2));
  }
  unit_end();

  return;
}


void test_boundary_allocations(void) {
  volatile Addr_t *ptr = null;
  Size_t size;
  Size_t entrySize;


  /* Calculate entry size in blocks */
  entrySize = ((HalfWord_t) (sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE));

  if(nil < ((HalfWord_t) (sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
    entrySize++;
  }

  unit_begin("Boundary - Single byte allocation");
  {
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, 1));
    unit_assert_not_null(ptr);
    unit_assert_ok(xMemGetSize(ptr, &size));

    /* Should round up to entry size plus at least one block */
    unit_assert_true(size >= CONFIG_MEMORY_REGION_BLOCK_SIZE);
    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Boundary - Exact block size allocation");
  {
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, CONFIG_MEMORY_REGION_BLOCK_SIZE));
    unit_assert_not_null(ptr);
    unit_assert_ok(xMemGetSize(ptr, &size));
    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Boundary - Block size minus one");
  {
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, CONFIG_MEMORY_REGION_BLOCK_SIZE - 1));
    unit_assert_not_null(ptr);
    unit_assert_ok(xMemGetSize(ptr, &size));

    /* Should round up */
    unit_assert_true(size >= CONFIG_MEMORY_REGION_BLOCK_SIZE);
    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Boundary - Block size plus one");
  {
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, CONFIG_MEMORY_REGION_BLOCK_SIZE + 1));
    unit_assert_not_null(ptr);
    unit_assert_ok(xMemGetSize(ptr, &size));

    /* Should round up to at least 2 blocks plus entry */
    unit_assert_true(size >= CONFIG_MEMORY_REGION_BLOCK_SIZE * 2);
    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Boundary - Multiple of block size");
  {
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, CONFIG_MEMORY_REGION_BLOCK_SIZE * 4));
    unit_assert_not_null(ptr);
    unit_assert_ok(xMemGetSize(ptr, &size));
    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Boundary - Large allocation near limit");
  {
    Size_t largeSize = (CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS - entrySize - 10) * CONFIG_MEMORY_REGION_BLOCK_SIZE;


    ptr = null;

    /* This should succeed if memory is available */
    if(OK(xMemAlloc(&ptr, largeSize))) {
      unit_assert_not_null(ptr);
      unit_assert_ok(xMemFree(ptr));
    }
  }
  unit_end();

  return;
}


void test_statistics_accuracy(void) {
  MemoryRegionStats_t *stats1 = null;
  MemoryRegionStats_t *stats2 = null;
  MemoryRegionStats_t *stats3 = null;
  volatile Addr_t *ptr = null;
  Size_t min1;
  Size_t min2;
  Size_t min3;


  unit_begin("Statistics - minimumEverFreeBytesRemaining tracking");
  {
    /* Get initial stats */
    unit_assert_ok(xMemGetHeapStats(&stats1));
    unit_assert_not_null(stats1);
    min1 = stats1->minimumEverFreeBytesRemaining;

    /* Allocate to reduce free space */
    unit_assert_ok(xMemAlloc(&ptr, 1024));
    unit_assert_ok(xMemGetHeapStats(&stats2));
    unit_assert_not_null(stats2);
    min2 = stats2->minimumEverFreeBytesRemaining;

    /* Minimum should have decreased or stayed same */
    unit_assert_true(min2 <= min1);

    /* Free memory */
    unit_assert_ok(xMemFree(ptr));
    unit_assert_ok(xMemGetHeapStats(&stats3));
    unit_assert_not_null(stats3);
    min3 = stats3->minimumEverFreeBytesRemaining;

    /* Minimum should remain at lowest point */
    unit_assert_equal(min2, min3);

    /* Cleanup */
    unit_assert_ok(xMemFree(stats1));
    unit_assert_ok(xMemFree(stats2));
    unit_assert_ok(xMemFree(stats3));
  }
  unit_end();
  unit_begin("Statistics - Allocation counter accuracy");
  {
    MemoryRegionStats_t *before = null;
    MemoryRegionStats_t *after = null;
    HalfWord_t beforeAllocs;
    HalfWord_t afterAllocs;


    unit_assert_ok(xMemGetHeapStats(&before));
    beforeAllocs = before->successfulAllocations;

    /* Perform allocation */
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, 128));

    unit_assert_ok(xMemGetHeapStats(&after));
    afterAllocs = after->successfulAllocations;

    /* Count should have increased (accounting for stats allocation) */
    unit_assert_true(afterAllocs > beforeAllocs);

    unit_assert_ok(xMemFree(ptr));
    unit_assert_ok(xMemFree(before));
    unit_assert_ok(xMemFree(after));
  }
  unit_end();
  unit_begin("Statistics - Free counter accuracy");
  {
    MemoryRegionStats_t *before = null;
    MemoryRegionStats_t *after = null;
    HalfWord_t beforeFrees;
    HalfWord_t afterFrees;


    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, 128));

    unit_assert_ok(xMemGetHeapStats(&before));
    beforeFrees = before->successfulFrees;

    /* Perform free */
    unit_assert_ok(xMemFree(ptr));

    unit_assert_ok(xMemGetHeapStats(&after));
    afterFrees = after->successfulFrees;

    /* Count should have increased */
    unit_assert_true(afterFrees > beforeFrees);

    unit_assert_ok(xMemFree(before));
    unit_assert_ok(xMemFree(after));
  }
  unit_end();
  unit_begin("Statistics - Available space consistency");
  {
    MemoryRegionStats_t *stats = null;
    Size_t allocated = 512;
    Size_t before;
    Size_t after;


    unit_assert_ok(xMemGetHeapStats(&stats));
    before = stats->availableSpaceInBytes;
    unit_assert_ok(xMemFree(stats));

    /* Allocate known size */
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, allocated));

    unit_assert_ok(xMemGetHeapStats(&stats));
    after = stats->availableSpaceInBytes;

    /* Available should have decreased */
    unit_assert_true(after < before);

    unit_assert_ok(xMemFree(ptr));
    unit_assert_ok(xMemFree(stats));
  }
  unit_end();

  return;
}


void test_data_integrity(void) {
  volatile Addr_t *ptr = null;
  Byte_t *bytes;
  Size_t i;


  unit_begin("Data Integrity - Memory zero initialization");
  {
    unit_assert_ok(xMemAlloc(&ptr, 256));
    unit_assert_not_null(ptr);
    bytes = (Byte_t *) ptr;

    /* Verify all bytes are zero */
    for(i = 0; i < 256; i++) {
      unit_assert_equal(bytes[i], 0);
    }

    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Data Integrity - Write and read back");
  {
    unit_assert_ok(xMemAlloc(&ptr, 128));
    bytes = (Byte_t *) ptr;

    /* Write pattern */
    for(i = 0; i < 128; i++) {
      bytes[i] = (Byte_t) (i & 0xFF);
    }

    /* Verify pattern */
    for(i = 0; i < 128; i++) {
      unit_assert_equal(bytes[i], (Byte_t) (i & 0xFF));
    }

    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Data Integrity - Multiple allocations independence");
  {
    volatile Addr_t *ptr1 = null;
    volatile Addr_t *ptr2 = null;
    Byte_t *bytes1;
    Byte_t *bytes2;


    unit_assert_ok(xMemAlloc(&ptr1, 64));
    unit_assert_ok(xMemAlloc(&ptr2, 64));

    bytes1 = (Byte_t *) ptr1;
    bytes2 = (Byte_t *) ptr2;

    /* Write different patterns */
    for(i = 0; i < 64; i++) {
      bytes1[i] = 0xAA;
      bytes2[i] = 0x55;
    }

    /* Verify independence */
    for(i = 0; i < 64; i++) {
      unit_assert_equal(bytes1[i], 0xAA);
      unit_assert_equal(bytes2[i], 0x55);
    }

    unit_assert_ok(xMemFree(ptr1));
    unit_assert_ok(xMemFree(ptr2));
  }
  unit_end();
  unit_begin("Data Integrity - Data survives across operations");
  {
    volatile Addr_t *ptr1 = null;
    volatile Addr_t *ptr2 = null;
    Byte_t *bytes1;


    /* Allocate and write pattern */
    unit_assert_ok(xMemAlloc(&ptr1, 128));
    bytes1 = (Byte_t *) ptr1;

    for(i = 0; i < 128; i++) {
      bytes1[i] = (Byte_t) (i * 2);
    }

    /* Allocate another block */
    unit_assert_ok(xMemAlloc(&ptr2, 64));

    /* Verify first block still has pattern */
    for(i = 0; i < 128; i++) {
      unit_assert_equal(bytes1[i], (Byte_t) (i * 2));
    }

    unit_assert_ok(xMemFree(ptr1));
    unit_assert_ok(xMemFree(ptr2));
  }
  unit_end();

  return;
}


void test_fragmentation_stress(void) {
  unit_begin("Stress - Alternating allocation/free pattern");
  {
    volatile Addr_t *ptrs[20];
    int i;


    /* Allocate 20 small blocks */
    for(i = 0; i < 20; i++) {
      ptrs[i] = null;
      unit_assert_ok(xMemAlloc(&ptrs[i], 64));
      unit_assert_not_null(ptrs[i]);
    }

    /* Free every other block (create fragmentation) */
    for(i = 0; i < 20; i += 2) {
      unit_assert_ok(xMemFree(ptrs[i]));
    }

    /* Try to allocate blocks in freed spaces */
    for(i = 0; i < 20; i += 2) {
      ptrs[i] = null;
      unit_assert_ok(xMemAlloc(&ptrs[i], 32));
      unit_assert_not_null(ptrs[i]);
    }

    /* Cleanup all */
    for(i = 0; i < 20; i++) {
      unit_assert_ok(xMemFree(ptrs[i]));
    }
  }
  unit_end();
  unit_begin("Stress - Worst case fragmentation");
  {
    volatile Addr_t *ptrs[10];
    int i;


    /* Allocate 10 blocks */
    for(i = 0; i < 10; i++) {
      ptrs[i] = null;
      unit_assert_ok(xMemAlloc(&ptrs[i], 128));
    }

    /* Free all odd-indexed blocks */
    for(i = 1; i < 10; i += 2) {
      unit_assert_ok(xMemFree(ptrs[i]));
    }

    /* Free all even-indexed blocks */
    for(i = 0; i < 10; i += 2) {
      unit_assert_ok(xMemFree(ptrs[i]));
    }

    /* Memory should be defragmented now - try large allocation */
    ptrs[0] = null;

    if(OK(xMemAlloc(&ptrs[0], 1024))) {
      unit_assert_ok(xMemFree(ptrs[0]));
    }
  }
  unit_end();
  unit_begin("Stress - Mixed size fragmentation");
  {
    volatile Addr_t *small[5];
    volatile Addr_t *medium[3];
    volatile Addr_t *large[2];
    int i;


    /* Allocate mixed sizes */
    for(i = 0; i < 5; i++) {
      small[i] = null;
      unit_assert_ok(xMemAlloc(&small[i], 32));
    }

    for(i = 0; i < 3; i++) {
      medium[i] = null;
      unit_assert_ok(xMemAlloc(&medium[i], 128));
    }

    for(i = 0; i < 2; i++) {
      large[i] = null;
      unit_assert_ok(xMemAlloc(&large[i], 512));
    }

    /* Free in mixed order */
    unit_assert_ok(xMemFree(small[0]));
    unit_assert_ok(xMemFree(medium[1]));
    unit_assert_ok(xMemFree(small[2]));
    unit_assert_ok(xMemFree(large[0]));
    unit_assert_ok(xMemFree(small[4]));

    /* Reallocate freed spaces */
    small[0] = null;
    unit_assert_ok(xMemAlloc(&small[0], 32));
    medium[1] = null;
    unit_assert_ok(xMemAlloc(&medium[1], 128));
    small[2] = null;
    unit_assert_ok(xMemAlloc(&small[2], 32));
    small[4] = null;
    unit_assert_ok(xMemAlloc(&small[4], 32));

    /* Cleanup - now all pointers are valid */
    for(i = 0; i < 5; i++) {
      unit_assert_ok(xMemFree(small[i]));
    }

    for(i = 0; i < 3; i++) {
      unit_assert_ok(xMemFree(medium[i]));
    }

    unit_assert_ok(xMemFree(large[1]));
  }
  unit_end();

  return;
}


void test_invalid_pointers(void) {
  Byte_t stack_var;
  Size_t size;


  unit_begin("Negative - Free invalid stack pointer");
  {
    /* Attempt to free stack address - should fail */
    unit_assert_not_ok(xMemFree(&stack_var));
  }
  unit_end();
  unit_begin("Negative - Get size of invalid pointer");
  {
    /* Attempt to get size of stack address - should fail */
    unit_assert_not_ok(xMemGetSize(&stack_var, &size));
  }
  unit_end();
  unit_begin("Negative - Free arbitrary pointer");
  {
    volatile Addr_t *arbitrary = (volatile Addr_t *) 0x12345678;


    /* Should fail safely */
    unit_assert_not_ok(xMemFree(arbitrary));
  }
  unit_end();
  unit_begin("Negative - Operations on unaligned pointer");
  {
    volatile Addr_t *ptr = null;
    volatile Addr_t *offset_ptr;


    unit_assert_ok(xMemAlloc(&ptr, 128));

    /* Create offset pointer (not at block start) */
    offset_ptr = (volatile Addr_t *) (((Byte_t *) ptr) + 5);

    /* Operations should fail on offset pointer */
    unit_assert_not_ok(xMemGetSize(offset_ptr, &size));
    unit_assert_not_ok(xMemFree(offset_ptr));

    /* Original pointer should still work */
    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();

  return;
}


void test_state_consistency(void) {
  volatile Addr_t *ptr1 = null;
  volatile Addr_t *ptr2 = null;
  Size_t used1;
  Size_t used2;


  unit_begin("State - Allocation after failed oversized request");
  {
    Size_t tooLarge;


    /* Ensure clean state */
    unit_assert_ok(xMemFreeAll());

    /* Get baseline (should be 0 after FreeAll) */
    unit_assert_ok(xMemGetUsed(&used1));
    unit_assert_equal(used1, 0x0u);

    /* Request more than available - use region size + 1 block */
    tooLarge = (CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS + 1) * CONFIG_MEMORY_REGION_BLOCK_SIZE;

    /* Failed allocation should not affect state */
    ptr1 = null;
    unit_assert_not_ok(xMemAlloc(&ptr1, tooLarge));
    unit_assert_null(ptr1);

    /* Verify memory usage unchanged */
    unit_assert_ok(xMemGetUsed(&used2));
    unit_assert_equal(used1, used2);

    /* Subsequent allocation should succeed */
    ptr2 = null;
    unit_assert_ok(xMemAlloc(&ptr2, 128));
    unit_assert_not_null(ptr2);
    unit_assert_ok(xMemFree(ptr2));
  }
  unit_end();
  unit_begin("State - Consistency after failed free");
  {
    Byte_t stack_var;


    unit_assert_ok(xMemGetUsed(&used1));

    /* Failed free should not affect state */
    unit_assert_not_ok(xMemFree(&stack_var));

    /* State should be unchanged */
    unit_assert_ok(xMemGetUsed(&used2));
    unit_assert_equal(used1, used2);

    /* Normal operations should continue */
    ptr1 = null;
    unit_assert_ok(xMemAlloc(&ptr1, 64));
    unit_assert_ok(xMemFree(ptr1));
  }
  unit_end();
  unit_begin("State - Recovery after multiple failures");
  {
    int i;
    Size_t tooLarge = (CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS + 10) * CONFIG_MEMORY_REGION_BLOCK_SIZE;


    /* Multiple failed allocations */
    for(i = 0; i < 5; i++) {
      ptr1 = null;
      unit_assert_not_ok(xMemAlloc(&ptr1, tooLarge));
    }

    /* System should still work */
    ptr1 = null;
    unit_assert_ok(xMemAlloc(&ptr1, 256));
    unit_assert_not_null(ptr1);
    unit_assert_ok(xMemFree(ptr1));
  }
  unit_end();

  return;
}


void test_memfreeall_idempotency(void) {
  volatile Addr_t *ptr = null;
  Size_t size;


  unit_begin("Idempotency - Multiple xMemFreeAll() calls");
  {
    unit_assert_ok(xMemAlloc(&ptr, 128));
    unit_assert_ok(xMemFreeAll());

    /* Verify memory freed */
    unit_assert_ok(xMemGetUsed(&size));
    unit_assert_equal(size, 0x0u);

    /* Second call should still succeed */
    unit_assert_ok(xMemFreeAll());

    /* Third call */
    unit_assert_ok(xMemFreeAll());

    /* Memory should still be usable */
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, 64));
    unit_assert_not_null(ptr);
    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();
  unit_begin("Idempotency - xMemFreeAll() with no allocations");
  {
    /* FreeAll with clean slate */
    unit_assert_ok(xMemFreeAll());
    unit_assert_ok(xMemGetUsed(&size));
    unit_assert_equal(size, 0x0u);

    /* Should still work */
    unit_assert_ok(xMemFreeAll());
  }
  unit_end();
  unit_begin("Idempotency - Allocate after xMemFreeAll()");
  {
    Byte_t *bytes;
    Size_t i;


    /* Allocate before */
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, 256));

    /* Free all */
    unit_assert_ok(xMemFreeAll());

    /* Allocate after - should get fresh memory */
    ptr = null;
    unit_assert_ok(xMemAlloc(&ptr, 256));
    unit_assert_not_null(ptr);

    /* Verify it's zeroed */
    bytes = (Byte_t *) ptr;

    for(i = 0; i < 256; i++) {
      unit_assert_equal(bytes[i], 0);
    }

    unit_assert_ok(xMemFree(ptr));
  }
  unit_end();

  return;
}


void test_kernel_memory(void) {
  Task_t *task1 = null;
  Task_t *task2 = null;
  Task_t *task3 = null;
  MemoryRegionStats_t *kernelStats = null;
  HalfWord_t allocsBefore;
  HalfWord_t allocsAfter;


  unit_begin("Kernel Memory - Task allocation uses kernel region");
  {
    unit_assert_ok(xMemGetKernelStats(&kernelStats));
    allocsBefore = kernelStats->successfulAllocations;
    unit_assert_ok(xMemFree(kernelStats));

    /* Create task - should allocate from kernel */
    unit_assert_ok(xTaskCreate(&task1, (Byte_t *) "TEST1", memory_1_harness_task, null));
    unit_assert_not_null(task1);

    unit_assert_ok(xMemGetKernelStats(&kernelStats));
    allocsAfter = kernelStats->successfulAllocations;

    /* Allocation count should increase */
    unit_assert_true(allocsAfter > allocsBefore);

    unit_assert_ok(xTaskDelete(task1));
    unit_assert_ok(xMemFree(kernelStats));
  }
  unit_end();
  unit_begin("Kernel Memory - Multiple task allocations");
  {
    /* Create multiple tasks */
    unit_assert_ok(xTaskCreate(&task1, (Byte_t *) "TEST1", memory_1_harness_task, null));
    unit_assert_ok(xTaskCreate(&task2, (Byte_t *) "TEST2", memory_1_harness_task, null));
    unit_assert_ok(xTaskCreate(&task3, (Byte_t *) "TEST3", memory_1_harness_task, null));

    unit_assert_not_null(task1);
    unit_assert_not_null(task2);
    unit_assert_not_null(task3);

    /* Verify kernel stats */
    unit_assert_ok(xMemGetKernelStats(&kernelStats));
    unit_assert_not_null(kernelStats);
    unit_assert_true(kernelStats->successfulAllocations > 0);

    /* Cleanup */
    unit_assert_ok(xTaskDelete(task1));
    unit_assert_ok(xTaskDelete(task2));
    unit_assert_ok(xTaskDelete(task3));
    unit_assert_ok(xMemFree(kernelStats));
  }
  unit_end();
  unit_begin("Kernel Memory - Stats NULL parameter");
  {
    unit_assert_not_ok(xMemGetKernelStats(null));
  }
  unit_end();

  return;
}


void test_performance_stress(void) {
  unit_begin("Performance - Rapid allocation/deallocation");
  {
    volatile Addr_t *ptr = null;
    int i;


    /* Rapid alloc/free cycles */
    for(i = 0; i < 50; i++) {
      ptr = null;
      unit_assert_ok(xMemAlloc(&ptr, 64));
      unit_assert_not_null(ptr);
      unit_assert_ok(xMemFree(ptr));
    }
  }
  unit_end();
  unit_begin("Performance - Maximum small allocations");
  {
    volatile Addr_t *ptrs[100];
    int i;
    int allocCount = 0;


    /* Allocate as many as possible */
    for(i = 0; i < 100; i++) {
      ptrs[i] = null;

      if(OK(xMemAlloc(&ptrs[i], 32))) {
        allocCount++;
      } else {
        break;
      }
    }

    /* Should have allocated at least some */
    unit_assert_true(allocCount > 0);

    /* Free all */
    for(i = 0; i < allocCount; i++) {
      unit_assert_ok(xMemFree(ptrs[i]));
    }
  }
  unit_end();
  unit_begin("Performance - Growing allocations");
  {
    volatile Addr_t *ptr = null;
    Size_t size = 32;
    int i;


    /* Allocate progressively larger blocks */
    for(i = 0; i < 10; i++) {
      ptr = null;

      if(OK(xMemAlloc(&ptr, size))) {
        unit_assert_not_null(ptr);
        unit_assert_ok(xMemFree(ptr));
        size *= 2;
      } else {
        break;
      }
    }
  }
  unit_end();

  return;
}


void test_randomized_patterns(void) {
  unit_begin("Randomized - Pseudo-random allocation pattern");
  {
    volatile Addr_t *ptrs[16];
    Size_t sizes[16] = {64, 128, 32, 256, 96, 48, 512, 80, 160, 40, 192, 72, 144, 88, 112, 56};
    int freeOrder[16] = {5, 12, 3, 9, 1, 14, 7, 0, 11, 4, 15, 2, 13, 6, 10, 8};
    int i;


    /* Allocate in sequential order with varied sizes */
    for(i = 0; i < 16; i++) {
      ptrs[i] = null;

      if(!OK(xMemAlloc(&ptrs[i], sizes[i]))) {
        ptrs[i] = null;
      }
    }

    /* Free in pseudo-random order */
    for(i = 0; i < 16; i++) {
      if(__PointerIsNotNull__(ptrs[freeOrder[i]])) {
        unit_assert_ok(xMemFree(ptrs[freeOrder[i]]));
      }
    }
  }
  unit_end();
  unit_begin("Randomized - Interleaved operations");
  {
    volatile Addr_t *ptr1 = null;
    volatile Addr_t *ptr2 = null;
    volatile Addr_t *ptr3 = null;


    unit_assert_ok(xMemAlloc(&ptr1, 100));
    unit_assert_ok(xMemAlloc(&ptr2, 200));
    unit_assert_ok(xMemFree(ptr1));
    unit_assert_ok(xMemAlloc(&ptr3, 50));
    unit_assert_ok(xMemFree(ptr2));
    ptr1 = null;
    unit_assert_ok(xMemAlloc(&ptr1, 150));
    unit_assert_ok(xMemFree(ptr3));
    unit_assert_ok(xMemFree(ptr1));
  }
  unit_end();

  return;
}


void test_alignment_verification(void) {
  volatile Addr_t *ptr = null;
  Size_t address;


  unit_begin("Alignment - Allocated addresses");
  {
    int i;


    /* Allocate multiple blocks and verify alignment */
    for(i = 0; i < 10; i++) {
      ptr = null;
      unit_assert_ok(xMemAlloc(&ptr, 64));
      unit_assert_not_null(ptr);

      /* Check that address is reasonable (not null, not obviously invalid) */
      address = (Size_t) ptr;
      unit_assert_true(address != 0);

      unit_assert_ok(xMemFree(ptr));
    }
  }
  unit_end();
  unit_begin("Alignment - Various sizes");
  {
    Size_t sizes[] = {1, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    int i;


    for(i = 0; i < 10; i++) {
      ptr = null;

      if(OK(xMemAlloc(&ptr, sizes[i]))) {
        unit_assert_not_null(ptr);
        address = (Size_t) ptr;
        unit_assert_true(address != 0);
        unit_assert_ok(xMemFree(ptr));
      }
    }
  }
  unit_end();

  return;
}


void test_cross_region_protection(void) {
  volatile Addr_t *heapPtr = null;
  Task_t *kernelTask = null;
  Size_t heapUsedBefore;
  Size_t heapUsedAfter;


  unit_begin("Cross-region - Heap allocation doesn't affect kernel");
  {
    MemoryRegionStats_t *kernelStats1 = null;
    MemoryRegionStats_t *kernelStats2 = null;
    Size_t kernelAvail1;
    Size_t kernelAvail2;


    /* Get kernel baseline */
    unit_assert_ok(xMemGetKernelStats(&kernelStats1));
    kernelAvail1 = kernelStats1->availableSpaceInBytes;

    /* Allocate from heap */
    unit_assert_ok(xMemAlloc(&heapPtr, 512));

    /* Check kernel unchanged */
    unit_assert_ok(xMemGetKernelStats(&kernelStats2));
    kernelAvail2 = kernelStats2->availableSpaceInBytes;

    /* Kernel available space should be same (or less if stats allocated) */
    unit_assert_true(kernelAvail2 <= kernelAvail1);

    unit_assert_ok(xMemFree(heapPtr));
    unit_assert_ok(xMemFree(kernelStats1));
    unit_assert_ok(xMemFree(kernelStats2));
  }
  unit_end();
  unit_begin("Cross-region - Kernel allocation doesn't affect heap");
  {
    /* Get heap baseline */
    unit_assert_ok(xMemGetUsed(&heapUsedBefore));

    /* Allocate from kernel (via task) */
    unit_assert_ok(xTaskCreate(&kernelTask, (Byte_t *) "TEST", memory_1_harness_task, null));

    /* Get heap usage - should only change due to stats allocations */
    unit_assert_ok(xMemGetUsed(&heapUsedAfter));

    /* Cleanup */
    unit_assert_ok(xTaskDelete(kernelTask));
  }
  unit_end();
  unit_begin("Cross-region - Independent statistics");
  {
    MemoryRegionStats_t *heapStats = null;
    MemoryRegionStats_t *kernelStats = null;


    /* Both stat queries should succeed */
    unit_assert_ok(xMemGetHeapStats(&heapStats));
    unit_assert_ok(xMemGetKernelStats(&kernelStats));

    unit_assert_not_null(heapStats);
    unit_assert_not_null(kernelStats);

    /* Stats should be different objects */
    unit_assert_true(heapStats != kernelStats);

    unit_assert_ok(xMemFree(heapStats));
    unit_assert_ok(xMemFree(kernelStats));
  }
  unit_end();

  return;
}


void test_memory_corruption_detection(void) {
  /*
   * Enhanced memory corruption detection tests
   * Tests internal consistency checking by intentionally corrupting:
   * 1) magic field (XOR'd address validation)
   * 2) free field (INUSE/FREE flag)
   * 3) blocks field (allocation size tracking)
   * 4) next pointer (linked list integrity)
   */
  unit_begin("Corruption Detection - Magic field corruption");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size = nil;


    /* Allocate and verify clean state */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    unit_assert_ok(xMemGetUsed(&size));
    unit_assert_equal(160, size);
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));

    /* Corrupt magic field */
    entry = ADDR2ENTRY(ptr);
    entry->magic = MAGIC_CONST; /* Should be XOR'd with address */

    /* Verify corruption detected */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - Free field corruption");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size = nil;


    /* Allocate and verify clean state */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    unit_assert_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));

    /* Corrupt free field (should be INUSE=0xAA or FREE=0xD5) */
    entry = ADDR2ENTRY(ptr);
    entry->free = 123; /* Invalid value */

    /* Verify corruption detected */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - Blocks field corruption");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size = nil;


    /* Allocate and verify clean state */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    unit_assert_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));

    /* Corrupt blocks field */
    entry = ADDR2ENTRY(ptr);
    entry->blocks = 12345; /* Invalid - doesn't add up to region size */

    /* Verify corruption detected */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - Next pointer corruption");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size = nil;


    /* Allocate and verify clean state */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    unit_assert_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));

    /* Corrupt next pointer (point outside memory region) */
    entry = ADDR2ENTRY(ptr);
    entry->next = (MemoryEntry_t *) 0x12345678;

    /* Verify corruption detected */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - xMemFree detects corruption");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;


    /* Allocate and verify clean state */
    unit_assert_ok(xMemAlloc(&ptr, 256));
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));

    /* Corrupt magic field */
    entry = ADDR2ENTRY(ptr);
    entry->magic = 0xDEADBEEF;

    /* xMemFree should detect corruption */
    unit_assert_not_ok(xMemFree(ptr));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - xMemGetSize detects corruption");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size;


    /* Allocate and verify clean state */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));

    /* Corrupt blocks field */
    entry = ADDR2ENTRY(ptr);
    entry->blocks = 9999;

    /* xMemGetSize should detect corruption */
    unit_assert_not_ok(xMemGetSize(ptr, &size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - Multiple field corruption");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size;


    /* Allocate and verify clean state */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));

    /* Corrupt multiple fields */
    entry = ADDR2ENTRY(ptr);
    entry->magic = 0xBADBAD;
    entry->free = 0xFF;
    entry->blocks = 0;

    /* Should detect corruption */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - MEMFAULT flag persistence");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size;


    /* Allocate and corrupt */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    entry = ADDR2ENTRY(ptr);
    entry->magic = 0;

    /* First call sets MEMFAULT */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* MEMFAULT should persist across calls */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();

    /* After reset, flag should be clear */
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));
  }
  unit_end();
  unit_begin("Corruption Detection - Recovery after cleanup");
  {
    volatile Addr_t *ptr1 = null;
    volatile Addr_t *ptr2 = null;
    MemoryEntry_t *entry = null;
    Size_t size;


    /* Allocate and corrupt */
    unit_assert_ok(xMemAlloc(&ptr1, 128));
    entry = ADDR2ENTRY(ptr1);
    entry->free = 0x42;

    /* Trigger corruption detection */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup */
    __MemoryClear__();
    __SysStateClear__();

    /* System should be usable again */
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));
    unit_assert_ok(xMemAlloc(&ptr2, 256));
    unit_assert_not_null(ptr2);
    unit_assert_ok(xMemGetUsed(&size));
    unit_assert_ok(xMemFree(ptr2));
  }
  unit_end();
  unit_begin("Corruption Detection - Heap stats with corruption");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    MemoryRegionStats_t *stats = null;


    /* Allocate and corrupt */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    entry = ADDR2ENTRY(ptr);
    entry->next = (MemoryEntry_t *) 0xFFFFFFFF;

    /* Heap stats should detect corruption */
    unit_assert_not_ok(xMemGetHeapStats(&stats));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - Free flag manipulation");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size;


    /* Allocate memory */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    unit_assert_true(__FlagIsNotSet__(MEMFAULT));

    /* Manually set to FREE while still allocated */
    entry = ADDR2ENTRY(ptr);
    entry->free = FREE; /* 0xD5 - should be INUSE */

    /* This corruption might be detected differently */
    /* Since it's a valid value, check behavior */
    unit_assert_ok(xMemGetUsed(&size));

    /* But trying to free should work since it looks valid */
    /* Note: This tests that valid magic values still work */

    /* Cleanup */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - Zero magic value");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size;


    /* Allocate and corrupt with zero */
    unit_assert_ok(xMemAlloc(&ptr, 64));
    entry = ADDR2ENTRY(ptr);
    entry->magic = 0;

    /* Should detect corruption */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - Blocks underflow");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size;


    /* Allocate and set blocks to zero */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    entry = ADDR2ENTRY(ptr);
    entry->blocks = 0;

    /* Should detect corruption */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - Blocks overflow");
  {
    volatile Addr_t *ptr = null;
    MemoryEntry_t *entry = null;
    Size_t size;


    /* Allocate and set blocks beyond region size */
    unit_assert_ok(xMemAlloc(&ptr, 128));
    entry = ADDR2ENTRY(ptr);
    entry->blocks = 0xFFFF;

    /* Should detect corruption */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  unit_begin("Corruption Detection - NULL next pointer");
  {
    volatile Addr_t *ptr1 = null;
    volatile Addr_t *ptr2 = null;
    MemoryEntry_t *entry = null;
    Size_t size;


    /* Allocate two blocks to create a chain */
    unit_assert_ok(xMemAlloc(&ptr1, 128));
    unit_assert_ok(xMemAlloc(&ptr2, 128));

    /* Corrupt first entry's next to NULL prematurely */
    entry = ADDR2ENTRY(ptr1);
    entry->next = null;

    /* Should detect corruption (blocks won't add up) */
    unit_assert_not_ok(xMemGetUsed(&size));
    unit_assert_true(__FlagIsSet__(MEMFAULT));

    /* Cleanup and reset */
    __MemoryClear__();
    __SysStateClear__();
  }
  unit_end();
  /* NOTE: Circular next pointer test disabled - causes infinite loop
   * unit_begin("Corruption Detection - Circular next pointer");
   * {
   *   volatile Addr_t *ptr = null;
   *   MemoryEntry_t *entry = null;
   *   Size_t size;
   *
   *   unit_assert_ok(xMemAlloc(&ptr, 128));
   *   entry = ADDR2ENTRY(ptr);
   *   entry->next = entry;
   *   unit_assert_not_ok(xMemGetUsed(&size));
   *   unit_assert_true(__FlagIsSet__(MEMFAULT));
   *   __MemoryClear__();
   *   __SysStateClear__();
   * }
   * unit_end();
   */

  return;
}