/**
 * @file test.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for HeliOS unit testing
 * @version 0.3.5
 * @date 2022-08-23
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
#include "posix.h"
#include "HeliOS.h"
#include "unit.h"

#include <unistd.h>



void task_main(xTask task_, xTaskParm parm_);

int main(int argc, char **argv) {



  unit_init();



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

  mem04 = xTaskCreate("NONE", task_main, NULL);

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



  unit_begin("xQueueCreate()");

  xQueue queue01 = NULL;

  queue01 = xQueueCreate(0x4);

  unit_try(NULL == queue01);

  queue01 = xQueueCreate(0x7);

  unit_try(NULL != queue01);

  unit_end();



  unit_begin("xQueueSend()");

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, "MESSAGE1"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, "MESSAGE2"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, "MESSAGE3"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, "MESSAGE4"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, "MESSAGE5"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, "MESSAGE6"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, "MESSAGE7"));

  unit_try(RETURN_FAILURE == xQueueSend(queue01, 0x8, "MESSAGE8"));

  unit_end();



  unit_begin("xQueueGetLength()");

  unit_try(0x7u == xQueueGetLength(queue01));

  unit_end();



  unit_begin("xQueueIsQueueEmpty()");

  unit_try(false == xQueueIsQueueEmpty(queue01));

  unit_end();



  unit_begin("xQueueIsQueueFull()");

  unit_try(true == xQueueIsQueueFull(queue01));

  unit_end();



  unit_begin("xQueueMessagesWaiting()");

  unit_try(true == xQueueMessagesWaiting(queue01));

  unit_end();



  unit_begin("xQueuePeek()");

  xQueueMessage queue02 = NULL;

  queue02 = xQueuePeek(queue01);

  unit_try(NULL != queue02);

  unit_try(0x8u == queue02->messageBytes);

  unit_try(0x0u == strncmp("MESSAGE1", queue02->messageValue, 0x8));

  xMemFree(queue02);

  unit_end();



  unit_begin("xQueueReceive()");

  queue02 = xQueueReceive(queue01);

  unit_try(NULL != queue02);

  unit_try(0x8u == queue02->messageBytes);

  unit_try(0x0u == strncmp("MESSAGE1", queue02->messageValue, 0x8));

  xMemFree(queue02);

  unit_end();



  unit_begin("xQueueDropMessage()");

  xQueueDropMessage(queue01);

  unit_try(0x6u != xQueueGetLength(queue01));

  xQueueDelete(queue01);

  unit_end();



  unit_begin("xQueueLockQueue()");

  queue01 = xQueueCreate(5);

  unit_try(NULL != queue01);

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, "MESSAGE1"));

  xQueueLockQueue(queue01);

  unit_try(RETURN_FAILURE == xQueueSend(queue01, 0x8, "MESSAGE2"));

  unit_end();



  unit_begin("xQueueUnlockQueue()");

  xQueueUnLockQueue(queue01);

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, "MESSAGE3"));

  unit_try(0x2u == xQueueGetLength(queue01));

  unit_end();



  unit_begin("xQueueDelete()");

  xQueueDelete(queue01);

  unit_try(RETURN_FAILURE == xQueueSend(queue01, 0x8, "MESSAGE4"));

  unit_end();



  unit_begin("xTimerCreate()");

  xTimer timer01 = NULL;

  timer01 = xTimerCreate(0x3E8);

  unit_try(NULL != timer01);

  unit_end();



  unit_begin("xTimerGetPeriod()");

  unit_try(0x3E8 == xTimerGetPeriod(timer01));

  unit_end();



  unit_begin("xTimerChangePeriod()");

  xTimerChangePeriod(timer01, 0x7D0);

  unit_try(0x7D0 == xTimerGetPeriod(timer01));

  unit_end();



  unit_begin("xTimerIsTimerActive()");

  unit_try(false == xTimerIsTimerActive(timer01));

  unit_end();



  unit_begin("xTimerHasTimerExpired()");

  unit_try(false == xTimerHasTimerExpired(timer01));

  xTimerStart(timer01);

  sleep(3);

  unit_try(true == xTimerHasTimerExpired(timer01));

  unit_end();



  unit_begin("xTimerStop()");

  xTimerStop(timer01);

  unit_try(false == xTimerIsTimerActive(timer01));

  unit_end();



  unit_begin("xTimerStart()");

  xTimerStart(timer01);

  unit_try(true == xTimerIsTimerActive(timer01));

  unit_end();



  unit_begin("xTimerReset()");

  unit_try(true == xTimerHasTimerExpired(timer01));

  xTimerReset(timer01);

  unit_try(false == xTimerHasTimerExpired(timer01));

  unit_end();



  unit_begin("xTimerDelete()");

  xTimerDelete(timer01);

  unit_try(0x0 == xTimerGetPeriod(timer01));

  unit_end();



  unit_begin("xTaskCreate()");

  xTask task01 = NULL;

  task01 = xTaskCreate("TASK01", task_main, NULL);

  unit_try(NULL != task01);

  unit_end();



  unit_begin("xTaskGetHandleByName()");

  xTask task02 = NULL;

  task02 = xTaskGetHandleByName("TASK01");

  unit_try(task02 == task01);

  unit_end();



  unit_begin("xTaskGetHandleById()");

  task02 = NULL;

  xBase task03 = 0x0;

  task03 = xTaskGetId(task01);

  task02 = xTaskGetHandleById(task03);

  unit_try(task02 == task01);

  unit_end();



  unit_begin("xTaskGetAllRunTimeStats()");

  xTaskRunTimeStats task04 = NULL;
  xBase task05 = 0;

  task04 = xTaskGetAllRunTimeStats(&task05);

  unit_try(NULL != task04);

  unit_try(0x1 == task05);

  unit_try(0x2 == task04[0].id);

  xMemFree(task04);

  unit_end();



  unit_begin("xTaskGetTaskRunTimeStats()");

  task04 = NULL;

  task04 = xTaskGetTaskRunTimeStats(task01);

  unit_try(NULL != task04);

  unit_try(0x2 == task04->id);

  xMemFree(task04);

  unit_end();



  unit_begin("xTaskGetNumberOfTasks()");

  xBase task06 = 0;

  task06 = xTaskGetNumberOfTasks();

  unit_try(0x1 == task06);

  unit_end();



  unit_begin("xTaskGetTaskInfo()");

  xTaskInfo task07 = NULL;

  task07 = xTaskGetTaskInfo(task01);

  unit_try(NULL != task07);

  unit_try(0x2 == task07->id);

  unit_try(0x0 == strncmp("TASK01", task07->name, 0x6));

  unit_try(TaskStateSuspended == task07->state);

  xMemFree(task07);

  unit_end();



  unit_begin("xTaskGetAllTaskInfo()");

  task07 = NULL;

  task07 = xTaskGetAllTaskInfo(&task06);

  unit_try(NULL != task07);

  unit_try(0x1 == task06);

  unit_try(0x2 == task07->id);

  unit_try(0x0 == strncmp("TASK01", task07->name, 0x6));

  unit_try(TaskStateSuspended == task07->state);

  xMemFree(task07);

  unit_end();



  unit_begin("xTaskGetTaskState()");

  unit_try(TaskStateSuspended == xTaskGetTaskState(task01));

  unit_end();



  unit_begin("xTaskGetName()");

  char *task08;

  task08 = xTaskGetName(task01);

  unit_try(NULL != task08);

  unit_try(0x0 == strncmp("TASK01", task08, 0x6));

  unit_end();



  unit_begin("xTaskGetId()");

  unit_try(0x2 == xTaskGetId(task01));

  unit_end();



  unit_begin("xTaskNotifyGive()");

  unit_try(RETURN_SUCCESS == xTaskNotifyGive(task01, 0x7, "MESSAGE"));

  unit_end();



  unit_begin("xTaskNotificationIsWaiting()");

  unit_try(true == xTaskNotificationIsWaiting(task01));

  unit_end();



  unit_begin("xTaskNotifyStateClear()");

  xTaskNotifyStateClear(task01);

  unit_try(false == xTaskNotificationIsWaiting(task01));

  unit_end();



  unit_begin("xTaskNotifyTake()");

  xTaskNotification task09 = NULL;

  unit_try(RETURN_SUCCESS == xTaskNotifyGive(task01, 0x7, "MESSAGE"));

  task09 = xTaskNotifyTake(task01);

  unit_try(NULL != task09);

  unit_try(0x7 == task09->notificationBytes);

  unit_try(0x0 == strncmp("MESSAGE", task09->notificationValue, 0x7));

  xMemFree(task09);

  unit_end();



  unit_begin("xTaskResume()");

  xTaskResume(task01);

  unit_try(TaskStateRunning == xTaskGetTaskState(task01));

  unit_end();



  unit_begin("xTaskSuspend()");

  xTaskSuspend(task01);

  unit_try(TaskStateSuspended == xTaskGetTaskState(task01));

  unit_end();



  unit_begin("xTaskWait()");

  xTaskWait(task01);

  unit_try(TaskStateWaiting == xTaskGetTaskState(task01));

  unit_end();



  unit_begin("xTaskChangePeriod()");

  xTaskChangePeriod(task01, 0xD05);

  unit_try(0xD05 == xTaskGetPeriod(task01));

  unit_end();



  unit_begin("xTaskGetPeriod()");

  xTaskChangePeriod(task01, 0x1E61);

  unit_try(0x1E61 == xTaskGetPeriod(task01));

  unit_end();



  unit_begin("xTaskResetTimer()");

  xTaskResetTimer(task01);

  unit_end();



  unit_begin("xTaskGetSchedulerState()");

  xTaskSuspendAll();

  unit_try(SchedulerStateSuspended == xTaskGetSchedulerState());

  xTaskResumeAll();

  unit_try(SchedulerStateRunning == xTaskGetSchedulerState());

  unit_end();



  unit_begin("Unit test for task timer event");

  xTaskResumeAll();

  xTaskDelete(task01);

  xTask task10 = NULL;

  task10 = xTaskCreate("TASK10", task_main, NULL);

  unit_try(NULL != task10);

  xTaskChangePeriod(task10, 0xBB8);

  xTaskWait(task10);

  xTaskResetTimer(task10);

  xTaskStartScheduler();

  xTaskDelete(task10);

  unit_end();



  unit_begin("Unit test for direct to task notification event");

  xTaskResumeAll();

  xTask task11 = NULL;

  task11 = xTaskCreate("TASK11", task_main, NULL);

  unit_try(NULL != task11);

  xTaskWait(task11);

  xTaskNotifyGive(task11, 0x7, "MESSAGE");

  unit_try(true == xTaskNotificationIsWaiting(task11));

  xTaskResumeAll();

  xTaskStartScheduler();

  unit_try(false == xTaskNotificationIsWaiting(task11));

  xTaskDelete(task11);

  unit_end();




  unit_exit();

  return 0;
}



void task_main(xTask task_, xTaskParm parm_) {

  xTaskNotifyStateClear(task_);

  xTaskSuspendAll();

  return;
}
