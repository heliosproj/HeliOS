/**
 * @file task_harness.c
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


#include "task_harness.h"


void task_harness(void) {


  unit_begin("xTaskCreate()");

  xTask task01 = NULL;

  task01 = xTaskCreate("TASK01", task_harness_task, NULL);

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

  unit_try(0x1 == task04[0].id);

  xMemFree(task04);

  unit_end();



  unit_begin("xTaskGetTaskRunTimeStats()");

  task04 = NULL;

  task04 = xTaskGetTaskRunTimeStats(task01);

  unit_try(NULL != task04);

  unit_try(0x1 == task04->id);

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

  unit_try(0x1 == task07->id);

  unit_try(0x0 == strncmp("TASK01", task07->name, 0x6));

  unit_try(TaskStateSuspended == task07->state);

  xMemFree(task07);

  unit_end();



  unit_begin("xTaskGetAllTaskInfo()");

  task07 = NULL;

  task07 = xTaskGetAllTaskInfo(&task06);

  unit_try(NULL != task07);

  unit_try(0x1 == task06);

  unit_try(0x1 == task07->id);

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

  unit_try(0x1 == xTaskGetId(task01));

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

  task10 = xTaskCreate("TASK10", task_harness_task, NULL);

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

  task11 = xTaskCreate("TASK11", task_harness_task, NULL);

  unit_try(NULL != task11);

  xTaskWait(task11);

  xTaskNotifyGive(task11, 0x7, "MESSAGE");

  unit_try(true == xTaskNotificationIsWaiting(task11));

  xTaskResumeAll();

  xTaskStartScheduler();

  unit_try(false == xTaskNotificationIsWaiting(task11));

  xTaskDelete(task11);

  unit_end();

  return;
}

void task_harness_task(xTask task_, xTaskParm parm_) {

  xTaskNotifyStateClear(task_);

  xTaskSuspendAll();

  return;
}