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

int memcmp(const void *s1_, const void *s2_, size_t n_);
void task_main(xTask task_, xTaskParm parm_);

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

  queue01 = xQueueCreate(7);
  if (ISNULLPTR(queue01)) {
    ret++;
  }

  if (RETURN_FAILURE == xQueueSend(queue01, 8, "MESSAGE1")) {
    ret++;
  }

  if (RETURN_FAILURE == xQueueSend(queue01, 8, "MESSAGE2")) {
    ret++;
  }

  if (RETURN_FAILURE == xQueueSend(queue01, 8, "MESSAGE3")) {
    ret++;
  }

  if (RETURN_FAILURE == xQueueSend(queue01, 8, "MESSAGE4")) {
    ret++;
  }

  if (RETURN_FAILURE == xQueueSend(queue01, 8, "MESSAGE5")) {
    ret++;
  }

  if (RETURN_FAILURE == xQueueSend(queue01, 8, "MESSAGE6")) {
    ret++;
  }

  if (RETURN_FAILURE == xQueueSend(queue01, 8, "MESSAGE7")) {
    ret++;
  }

  if (0x7u != xQueueGetLength(queue01)) {
    ret++;
  }

  if (true == xQueueIsQueueEmpty(queue01)) {
    ret++;
  }

  if (false == xQueueIsQueueFull(queue01)) {
    ret++;
  }

  if (false == xQueueMessagesWaiting(queue01)) {
    ret++;
  }

  xQueueMessage queue02 = NULL;

  queue02 = xQueuePeek(queue01);
  if (ISNULLPTR(queue02)) {
    ret++;
  }

  if (0x8u != queue02->messageBytes) {
    ret++;
  }

  if (0x0u != memcmp("MESSAGE1", queue02->messageValue, 8)) {
    ret++;
  }

  xMemFree(queue02);

  queue02 = xQueueReceive(queue01);
  if (ISNULLPTR(queue02)) {
    ret++;
  }

  if (0x8u != queue02->messageBytes) {
    ret++;
  }

  if (0x0u != memcmp("MESSAGE1", queue02->messageValue, 8)) {
    ret++;
  }

  xMemFree(queue02);

  xQueueDropMessage(queue01);

  if (0x5u != xQueueGetLength(queue01)) {
    ret++;
  }

  xQueueDelete(queue01);

  queue01 = xQueueCreate(5);
  if (ISNULLPTR(queue01)) {
    ret++;
  }

  if (RETURN_FAILURE == xQueueSend(queue01, 8, "MESSAGE1")) {
    ret++;
  }

  xQueueLockQueue(queue01);

  if (RETURN_SUCCESS == xQueueSend(queue01, 8, "MESSAGE2")) {
    ret++;
  }

  xQueueUnLockQueue(queue01);

  if (RETURN_FAILURE == xQueueSend(queue01, 8, "MESSAGE3")) {
    ret++;
  }

  if (0x2u != xQueueGetLength(queue01)) {
    ret++;
  }

  xQueueDelete(queue01);

  if (RETURN_SUCCESS == xQueueSend(queue01, 8, "MESSAGE4")) {
    ret++;
  }

  /* END: QUEUE UNIT TESTS */

  /* BEGIN: TASK UNIT TESTS */


  /* END: TASK UNIT TESTS */

  SYSASSERT(zero == ret);

  return ret;
}



void task_main(xTask task_, xTaskParm parm_) {

  return;
}



int memcmp(const void *s1_, const void *s2_, size_t n_) {

  size_t i = zero;

  int ret = zero;

  char *s1 = (char *)s1_;

  char *s2 = (char *)s2_;

  for (i = zero; i < n_; i++) {

    if (*s1 != *s2) {

      ret = *s1 - *s2;

      break;
    }

    s1++;

    s2++;
  }


  return ret;
}