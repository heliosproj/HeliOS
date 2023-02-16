/*UNCRUSTIFY-OFF*/
/**
 * @file task_harness.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.4.0
 * @date 2022-08-27
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
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
/*UNCRUSTIFY-ON*/

#include "task_harness.h"


void task_harness(void) {
  Task_t *task01;
  Task_t *task02;
  Base_t task03;
  TaskRunTimeStats_t *task04;
  Base_t task05;
  Base_t task06;
  TaskInfo_t *task07;
  Byte_t *task08;
  TaskNotification_t *task09;
  Task_t *task10 = null;
  Task_t *task11 = null;
  Task_t *task12 = null;


  unit_begin("xTaskCreate()");
  task01 = null;
  task01 = xTaskCreate((Byte_t *) "TASK01", task_harness_task, null);
  unit_try(null != task01);
  unit_end();
  unit_begin("xTaskGetHandleByName()");
  task02 = null;
  task02 = xTaskGetHandleByName((Byte_t *) "TASK01");
  unit_try(task02 == task01);
  unit_end();
  unit_begin("xTaskGetHandleById()");
  task02 = null;
  task03 = 0x0;
  task03 = xTaskGetId(task01);
  task02 = xTaskGetHandleById(task03);
  unit_try(task02 == task01);
  unit_end();
  unit_begin("xTaskGetAllRunTimeStats()");
  task04 = null;
  task05 = 0;
  task04 = xTaskGetAllRunTimeStats(&task05);
  unit_try(null != task04);
  unit_try(0x1 == task05);
  unit_try(0x1 == task04[0].id);
  unit_try(ISSUCCESSFUL(xMemFree(task04)));
  unit_end();
  unit_begin("xTaskGetTaskRunTimeStats()");
  task04 = null;
  task04 = xTaskGetTaskRunTimeStats(task01);
  unit_try(null != task04);
  unit_try(0x1 == task04->id);
  unit_try(ISSUCCESSFUL(xMemFree(task04)));
  unit_end();
  unit_begin("xTaskGetNumberOfTasks()");
  task06 = 0;
  task06 = xTaskGetNumberOfTasks();
  unit_try(0x1 == task06);
  unit_end();
  unit_begin("xTaskGetTaskInfo()");
  task07 = null;
  task07 = xTaskGetTaskInfo(task01);
  unit_try(null != task07);
  unit_try(0x1 == task07->id);
  unit_try(0x0 == strncmp("TASK01", (char *) task07->name, 0x6));
  unit_try(TaskStateSuspended == task07->state);
  unit_try(ISSUCCESSFUL(xMemFree(task07)));
  unit_end();
  unit_begin("xTaskGetAllTaskInfo()");
  task07 = null;
  task07 = xTaskGetAllTaskInfo(&task06);
  unit_try(null != task07);
  unit_try(0x1 == task06);
  unit_try(0x1 == task07->id);
  unit_try(0x0 == strncmp("TASK01", (char *) task07->name, 0x6));
  unit_try(TaskStateSuspended == task07->state);
  unit_try(ISSUCCESSFUL(xMemFree(task07)));
  unit_end();
  unit_begin("xTaskGetTaskState()");
  unit_try(TaskStateSuspended == xTaskGetTaskState(task01));
  unit_end();
  unit_begin("xTaskGetName()");
  task08 = xTaskGetName(task01);
  unit_try(null != task08);
  unit_try(0x0 == strncmp("TASK01", (char *) task08, 0x6));
  unit_end();
  unit_begin("xTaskGetId()");
  unit_try(0x1 == xTaskGetId(task01));
  unit_end();
  unit_begin("xTaskNotifyGive()");
  unit_try(RETURN_SUCCESS == xTaskNotifyGive(task01, 0x7, (Byte_t *) "MESSAGE"));
  unit_end();
  unit_begin("xTaskNotificationIsWaiting()");
  unit_try(true == xTaskNotificationIsWaiting(task01));
  unit_end();
  unit_begin("xTaskNotifyStateClear()");
  xTaskNotifyStateClear(task01);
  unit_try(false == xTaskNotificationIsWaiting(task01));
  unit_end();
  unit_begin("xTaskNotifyTake()");
  task09 = null;
  unit_try(RETURN_SUCCESS == xTaskNotifyGive(task01, 0x7, (Byte_t *) "MESSAGE"));
  task09 = xTaskNotifyTake(task01);
  unit_try(null != task09);
  unit_try(0x7 == task09->notificationBytes);
  unit_try(0x0 == strncmp("MESSAGE", (char *) task09->notificationValue, 0x7));
  unit_try(ISSUCCESSFUL(xMemFree(task09)));
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
  task10 = null;
  task10 = xTaskCreate((Byte_t *) "TASK10", task_harness_task, null);
  unit_try(null != task10);
  xTaskChangePeriod(task10, 0xBB8);
  xTaskWait(task10);
  xTaskResetTimer(task10);
  xTaskStartScheduler();
  xTaskDelete(task10);
  unit_end();
  unit_begin("Unit test for direct to task notification event");
  xTaskResumeAll();
  task11 = null;
  task11 = xTaskCreate((Byte_t *) "TASK11", task_harness_task, null);
  unit_try(null != task11);
  xTaskWait(task11);
  xTaskNotifyGive(task11, 0x7, (Byte_t *) "MESSAGE");
  unit_try(true == xTaskNotificationIsWaiting(task11));
  xTaskResumeAll();
  xTaskStartScheduler();
  unit_try(false == xTaskNotificationIsWaiting(task11));
  xTaskDelete(task11);
  unit_end();
  unit_begin("Unit test for task watchdog timer");
  xTaskResumeAll();
  task12 = null;
  task12 = xTaskCreate((Byte_t *) "TASK12", task_harness_task2, null);
  unit_try(null != task12);
  xTaskChangeWDPeriod(task12, 0x7D0u);
  xTaskResume(task12);
  unit_try(TaskStateRunning == xTaskGetTaskState(task12));
  xTaskStartScheduler();
  unit_try(TaskStateSuspended == xTaskGetTaskState(task12));
  unit_end();
  unit_begin("xTaskGetWDPeriod()");
  unit_try(0x7D0u == xTaskGetWDPeriod(task12));
  unit_end();

  return;
}


void task_harness_task(Task_t *task_, TaskParm_t *parm_) {
  xTaskNotifyStateClear(task_);
  xTaskSuspendAll();

  return;
}


void task_harness_task2(Task_t *task_, TaskParm_t *parm_) {
  sleep(3);
  xTaskSuspendAll();

  return;
}