/*UNCRUSTIFY-OFF*/
/**
 * @file memory_1_harness.c
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
#include "memory_1_harness.h"

static Size_t sizes[0x20u] = {
  0x2532u, 0x1832u, 0x132u, 0x2932u, 0x332u, 0x1432u, 0x1332u, 0x532u, 0x1732u, 0x932u, 0x1432u, 0x2232u, 0x1432u, 0x3132u, 0x032u, 0x1132u, 0x632u, 0x932u,
    0x1532u, 0x632u, 0x1832u, 0x132u, 0x1332u, 0x3132u, 0x2732u, 0x1532u, 0x2432u, 0x2932u, 0x2432u, 0x2932u, 0x3032u, 0x2332u
};
static Size_t order[0x20u] = {
  0x02u, 0x16u, 0x07u, 0x0Cu, 0x06u, 0x00u, 0x0Du, 0x18u, 0x10u, 0x08u, 0x0Au, 0x1Eu, 0x0Bu, 0x0Eu, 0x03u, 0x09u, 0x19u, 0x05u, 0x1Cu, 0x1Du, 0x0Fu, 0x01u,
    0x1Au, 0x04u, 0x13u, 0x11u, 0x1Fu, 0x12u, 0x17u, 0x15u, 0x14u, 0x1Bu
};
static MemoryTest_t tests[0x20u];


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

  for(i = 0; i < 0x20u; i++) {
    tests[i].size = sizes[i];
    tests[i].blocks = (sizes[i] / CONFIG_MEMORY_REGION_BLOCK_SIZE) + 1;

    if(nil < ((Size_t) (sizes[i] % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
      tests[i].blocks += 1;
    }

    unit_try(OK(xMemAlloc((volatile Addr_t **) &tests[i].ptr, sizes[i])));
    unit_try(null != tests[i].ptr);
    used += tests[i].blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
    unit_try(OK(xMemGetUsed(&actual)));
    unit_try(used == actual);
    unit_try(OK(xMemGetSize(tests[i].ptr, &actual)));
    unit_try((tests[i].blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE) == actual);
  }

  unit_try(!OK(xMemAlloc((volatile Addr_t **) &mem05, 0x99999u)));

  for(i = 0; i < 0x20u; i++) {
    unit_try(OK(xMemFree(tests[order[i]].ptr)));
  }

  unit_try(OK(xMemGetUsed(&actual)));
  unit_try(0x0u == actual);
  unit_try(OK(xMemAlloc((volatile Addr_t **) &mem05, (CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS - 1) * CONFIG_MEMORY_REGION_BLOCK_SIZE)));
  actual = nil;
  unit_try(OK(xMemGetUsed(&actual)));
  unit_try((CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE) == actual);
  unit_try(OK(xMemFree(mem05)));
  unit_end();
  unit_begin("Memory allocation succeeds for large block");
  mem01 = null;
  unit_try(OK(xMemAlloc((volatile Addr_t **) &mem01, 0x32000u)));
  unit_try(null != mem01);
  unit_end();
  unit_begin("Used memory tracking reflects allocations");
  unit_try(OK(xMemGetUsed(&actual)));
  unit_try(0x32020u == actual);
  unit_end();
  unit_begin("Allocated block size retrieval is accurate");
  unit_try(OK(xMemGetSize(mem01, &actual)));
  unit_try(0x32020u == actual);
  unit_end();
  unit_begin("Heap statistics reflect current memory state");
  mem02 = null;
  unit_try(OK(xMemGetHeapStats(&mem02)));
  unit_try(null != mem02);
  unit_try(0x63A0u == mem02->availableSpaceInBytes);
  unit_try(0x63A0u == mem02->largestFreeEntryInBytes);
  unit_try(0x0u == mem02->minimumEverFreeBytesRemaining);
  unit_try(0x31Du == mem02->numberOfFreeBlocks);
  unit_try(0x63A0u == mem02->smallestFreeEntryInBytes);
  unit_try(0x24u == mem02->successfulAllocations); /* +1 from xSystemGetSystemInfo */
  unit_try(0x22u == mem02->successfulFrees); /* +1 from xSystemGetSystemInfo */
  unit_end();
  unit_begin("Kernel statistics track internal allocations");
  mem03 = null;
  mem04 = null;
  unit_try(OK(xTaskCreate(&mem04, (Byte_t *) "NONE", memory_1_harness_task, null)));
  unit_try(null != mem04);
  unit_try(OK(xTaskDelete(mem04)));
  unit_try(OK(xMemGetKernelStats(&mem03)));
  unit_try(null != mem03);
  unit_try(0x383C0u == mem03->availableSpaceInBytes);
  unit_try(0x383C0u == mem03->largestFreeEntryInBytes);
  unit_try(0x38340u == mem03->minimumEverFreeBytesRemaining);
  unit_try(0x1C1Eu == mem03->numberOfFreeBlocks);
  unit_try(0x383C0u == mem03->smallestFreeEntryInBytes);
  unit_try(0x2u == mem03->successfulAllocations);
  unit_try(0x1u == mem03->successfulFrees);
  unit_try(OK(xMemFree(mem01)));
  unit_try(OK(xMemFree(mem02)));
  unit_try(OK(xMemFree(mem03)));
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
  unit_try(!OK(xMemAlloc(null, 128)));
  unit_end();

  /* Test zero size allocation */
  unit_begin("Edge Case - xMemAlloc() Zero Size");
  ptr1 = null;
  unit_try(!OK(xMemAlloc(&ptr1, 0)));
  unit_try(null == ptr1);
  unit_end();

  /* Test oversized allocation */
  unit_begin("Edge Case - xMemAlloc() Oversized");
  ptr1 = null;
  unit_try(!OK(xMemAlloc(&ptr1, 0xFFFFFFFFu)));
  unit_try(null == ptr1);
  unit_end();

  /* Test freeing NULL pointer (should succeed like standard C free()) */
  unit_begin("Edge Case - xMemFree() NULL Pointer");
  unit_try(OK(xMemFree(null)));
  unit_end();

  /* Test double free */
  unit_begin("Edge Case - xMemFree() Double Free");
  ptr1 = null;
  unit_try(OK(xMemAlloc(&ptr1, 128)));
  unit_try(null != ptr1);
  unit_try(OK(xMemFree(ptr1)));
  /* Attempting to free again should fail */
  unit_try(!OK(xMemFree(ptr1)));
  unit_end();

  /* Test getting size of NULL pointer */
  unit_begin("Edge Case - xMemGetSize() NULL Pointer");
  size = nil;
  unit_try(!OK(xMemGetSize(null, &size)));
  unit_end();

  /* Test getting size with NULL output parameter */
  unit_begin("Edge Case - xMemGetSize() NULL Output");
  ptr1 = null;
  unit_try(OK(xMemAlloc(&ptr1, 128)));
  unit_try(null != ptr1);
  unit_try(!OK(xMemGetSize(ptr1, null)));
  unit_try(OK(xMemFree(ptr1)));
  unit_end();

  /* Test xMemGetUsed with NULL parameter */
  unit_begin("Edge Case - xMemGetUsed() NULL Pointer");
  unit_try(!OK(xMemGetUsed(null)));
  unit_end();

  /* Test xMemGetHeapStats with NULL parameter */
  unit_begin("Edge Case - xMemGetHeapStats() NULL Pointer");
  unit_try(!OK(xMemGetHeapStats(null)));
  unit_end();

  /* Test xMemGetKernelStats with NULL parameter */
  unit_begin("Edge Case - xMemGetKernelStats() NULL Pointer");
  unit_try(!OK(xMemGetKernelStats(null)));
  unit_end();

  /* Test maximum number of allocations */
  unit_begin("Edge Case - Maximum Allocations");
  {
    #define MAX_TEST_ALLOCS 100
    volatile Addr_t *ptrs[MAX_TEST_ALLOCS];
    int i;
    int allocCount = 0;
    Size_t sizeBefore = 0;
    Size_t sizeAfter = 0;

    /* Get baseline memory usage */
    unit_try(OK(xMemGetUsed(&sizeBefore)));

    /* Allocate as many small blocks as possible */
    for(i = 0; i < MAX_TEST_ALLOCS; i++) {
      ptrs[i] = null;

      if(OK(xMemAlloc(&ptrs[i], 64))) {
        unit_try(null != ptrs[i]);
        allocCount++;
      } else {
        break;
      }
    }

    /* Should have allocated at least some blocks */
    printf("DEBUG: allocCount=%d, sizeBefore=%u\n", allocCount, (unsigned int)sizeBefore);
    unit_try(allocCount > 0);

    /* Free all allocated blocks */
    for(i = 0; i < allocCount; i++) {
      unit_try(OK(xMemFree(ptrs[i])));
    }

    /* Verify memory returns to baseline */
    unit_try(OK(xMemGetUsed(&sizeAfter)));
    printf("DEBUG: sizeAfter=%u\n", (unsigned int)sizeAfter);
    unit_try(sizeBefore == sizeAfter);
  }
  unit_end();

  /* Test fragmentation resilience */
  unit_begin("Edge Case - Memory Fragmentation");
  {
    volatile Addr_t *frag1 = null;
    volatile Addr_t *frag2 = null;
    volatile Addr_t *frag3 = null;

    /* Create fragmented memory pattern */
    unit_try(OK(xMemAlloc(&frag1, 1024)));
    unit_try(OK(xMemAlloc(&frag2, 1024)));
    unit_try(OK(xMemAlloc(&frag3, 1024)));

    /* Free middle block */
    unit_try(OK(xMemFree(frag2)));

    /* Try to allocate a block that fits in the freed space */
    frag2 = null;
    unit_try(OK(xMemAlloc(&frag2, 512)));
    unit_try(null != frag2);

    /* Cleanup */
    unit_try(OK(xMemFree(frag1)));
    unit_try(OK(xMemFree(frag2)));
    unit_try(OK(xMemFree(frag3)));
  }
  unit_end();

  /* Test allocation after xMemFreeAll */
  unit_begin("Edge Case - Allocation After xMemFreeAll()");
  ptr1 = null;
  ptr2 = null;
  unit_try(OK(xMemAlloc(&ptr1, 256)));
  unit_try(OK(xMemAlloc(&ptr2, 512)));

  /* Free all memory */
  unit_try(OK(xMemFreeAll()));

  /* Verify memory is freed */
  unit_try(OK(xMemGetUsed(&size)));
  unit_try(0x0u == size);

  /* Allocate again - should succeed */
  ptr1 = null;
  unit_try(OK(xMemAlloc(&ptr1, 128)));
  unit_try(null != ptr1);
  unit_try(OK(xMemFree(ptr1)));
  unit_end();
}


void memory_1_harness_task(Task_t *task_, TaskParm_t *parm_) {
  xTaskSuspendAll();

  return;
}