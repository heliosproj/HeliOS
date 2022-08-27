/**
 * @file memory_harness.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.3.5
 * @date 2022-08-27
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "memory_harness.h"

void memory_harness(void) {

  unit_begin("xMemAlloc()");

  xBase *mem01 = NULL;

  mem01 = (xBase *)xMemAlloc(0x32000u);

  unit_try(NULL != mem01);

  unit_end();



  unit_begin("xMemGetUsed()");

  unit_try(0x32020u == xMemGetUsed());

  unit_end();



  unit_begin("xMemGetSize()");

  unit_try(0x32020u == xMemGetSize(mem01));

  unit_end();



  unit_begin("xMemGetHeapStats()");

  xMemoryRegionStats mem02 = NULL;

  mem02 = xMemGetHeapStats();

  unit_try(NULL != mem02);

  unit_try(0x63A0u == mem02->availableSpaceInBytes);

  unit_try(0x63A0u == mem02->largestFreeEntryInBytes);

  unit_try(0x63A0u == mem02->minimumEverFreeBytesRemaining);

  unit_try(0x31Du == mem02->numberOfFreeBlocks);

  unit_try(0x63A0u == mem02->smallestFreeEntryInBytes);

  unit_try(0x2u == mem02->successfulAllocations);

  unit_try(0x0u == mem02->successfulFrees);

  unit_end();



  unit_begin("xMemGetKernelStats()");

  xMemoryRegionStats mem03 = NULL;

  xTask mem04 = NULL;

  mem04 = xTaskCreate("NONE", memory_harness_task, NULL);

  unit_try(NULL != mem04);

  xTaskDelete(mem04);

  mem03 = xMemGetKernelStats();

  unit_try(NULL != mem03);

  unit_try(0x383C0u == mem03->availableSpaceInBytes);

  unit_try(0x383C0u == mem03->largestFreeEntryInBytes);

  unit_try(0x38340u == mem03->minimumEverFreeBytesRemaining);

  unit_try(0x1C1Eu == mem03->numberOfFreeBlocks);

  unit_try(0x383C0u == mem03->smallestFreeEntryInBytes);

  unit_try(0x2u == mem03->successfulAllocations);

  unit_try(0x1u == mem03->successfulFrees);

  xMemFree(mem01);

  xMemFree(mem02);

  xMemFree(mem03);

  unit_end();

  return;
}

void memory_harness_task(xTask task_, xTaskParm parm_) {

  xTaskSuspendAll();

  return;
}