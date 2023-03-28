/*UNCRUSTIFY-OFF*/
/**
 * @file memory_1_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.4.1
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
  i = zero;
  used = zero;
  actual = zero;

  for(i = 0; i < 0x20u; i++) {
    tests[i].size = sizes[i];
    tests[i].blocks = (sizes[i] / CONFIG_MEMORY_REGION_BLOCK_SIZE) + 1;

    if(zero < ((Size_t) (sizes[i] % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
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
  actual = zero;
  unit_try(OK(xMemGetUsed(&actual)));
  unit_try((CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE) == actual);
  unit_try(OK(xMemFree(mem05)));
  unit_end();
  unit_begin("xMemAlloc()");
  mem01 = null;
  unit_try(OK(xMemAlloc((volatile Addr_t **) &mem01, 0x32000u)));
  unit_try(null != mem01);
  unit_end();
  unit_begin("xMemGetUsed()");
  unit_try(OK(xMemGetUsed(&actual)));
  unit_try(0x32020u == actual);
  unit_end();
  unit_begin("xMemGetSize()");
  unit_try(OK(xMemGetSize(mem01, &actual)));
  unit_try(0x32020u == actual);
  unit_end();
  unit_begin("xMemGetHeapStats()");
  mem02 = null;
  unit_try(OK(xMemGetHeapStats(&mem02)));
  unit_try(null != mem02);
  unit_try(0x63A0u == mem02->availableSpaceInBytes);
  unit_try(0x63A0u == mem02->largestFreeEntryInBytes);
  unit_try(0x0u == mem02->minimumEverFreeBytesRemaining);
  unit_try(0x31Du == mem02->numberOfFreeBlocks);
  unit_try(0x63A0u == mem02->smallestFreeEntryInBytes);
  unit_try(0x23u == mem02->successfulAllocations);
  unit_try(0x21u == mem02->successfulFrees);
  unit_end();
  unit_begin("xMemGetKernelStats()");
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

  return;
}


void memory_1_harness_task(Task_t *task_, TaskParm_t *parm_) {
  xTaskSuspendAll();

  return;
}