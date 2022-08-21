/**
 * @file test.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for unit testing
 * @version 0.3.4
 * @date 2022-08-19
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

#include <posix.h>

#include <HeliOS.h>

#include <stdio.h>

void task_main(xTask task_, xTaskParm parm_) {

  return;
}

int main(int argc, char **argv) {

  xBase ret = zero;

  /* BEGIN: MEMORY UNIT TESTS */

  xBase *mem01 = NULL;

  mem01 = (xBase *)xMemAlloc(0x32000u);
  if (ISNULLPTR(mem01)) {
    ret++;
  }

  if (0x32020u != xMemGetUsed()) {
    ret++;
  }

  if (0x32020u != xMemGetSize(mem01)) {
    ret++;
  }

  xMemoryRegionStats mem02 = NULL;
  xMemoryRegionStats mem03 = NULL;

  mem02 = xMemGetHeapStats();
  if (ISNULLPTR(mem02)) {
    ret++;
  }

  if (0x63A0u != mem02->availableSpaceInBytes) {
    ret++;
  }

  if (0x63A0u != mem02->largestFreeEntryInBytes) {
    ret++;
  }

  if (0x63A0u != mem02->minimumEverFreeBytesRemaining) {
    ret++;
  }

  if (0x31Du != mem02->numberOfFreeBlocks) {
    ret++;
  }

  if (0x63A0u != mem02->smallestFreeEntryInBytes) {
    ret++;
  }

  if (0x2u != mem02->successfulAllocations) {
    ret++;
  }

  if (0x0u != mem02->successfulFrees) {
    ret++;
  }

  xTask mem04 = NULL;

  mem04 = xTaskCreate("NONE", task_main, NULL);
  xTaskDelete(mem04);

  mem03 = xMemGetKernelStats();
  if (ISNULLPTR(mem03)) {
    ret++;
  }

  if (0x383C0u != mem03->availableSpaceInBytes) {
    ret++;
  }

  if (0x383C0u != mem03->largestFreeEntryInBytes) {
    ret++;
  }

  if (0x38340u != mem03->minimumEverFreeBytesRemaining) {
    ret++;
  }

  if (0x1C1Eu != mem03->numberOfFreeBlocks) {
    ret++;
  }

  if (0x383C0u != mem03->smallestFreeEntryInBytes) {
    ret++;
  }

  if (0x2u != mem03->successfulAllocations) {
    ret++;
  }

  if (0x1u != mem03->successfulFrees) {
    ret++;
  }

  xMemFree(mem01);
  xMemFree(mem02);
  xMemFree(mem03);

  /* END: MEMORY UNIT TESTS */


  /* BEGIN: QUEUE UNIT TESTS */

  xQueue queue01 = NULL;

  queue01 = xQueueCreate(1);
  if (ISNULLPTR(queue01)) {
    ret++;
  }

  xQueueDelete(queue01);


  /* END: QUEUE UNIT TESTS */

  SYSASSERT(zero == ret);

  return ret;
}