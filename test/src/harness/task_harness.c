/*UNCRUSTIFY-OFF*/
/**
 * @file task_harness.c
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
  TaskState_t task13;
  Base_t task14 = zero;
  Base_t task15 = zero;
  Base_t task16 = zero;
  TaskState_t task17;
  TaskState_t task18;
  TaskState_t task19;
  Ticks_t task20 = zero;
  Ticks_t task21 = zero;
  SchedulerState_t task22;
  SchedulerState_t task23;
  Base_t task24 = zero;
  Base_t task25 = zero;
  TaskState_t task26;
  TaskState_t task27;
  Ticks_t task28 = zero;


  unit_begin("xTaskCreate()");
  task01 = null;
  unit_try(OK(xTaskCreate(&task01, (Byte_t *) "TASK01", task_harness_task, null)));
  unit_try(null != task01);
  unit_end();
  unit_begin("xTaskGetHandleByName()");
  task02 = null;
  unit_try(OK(xTaskGetHandleByName(&task02, (Byte_t *) "TASK01")));
  unit_try(task02 == task01);
  unit_end();
  unit_begin("xTaskGetHandleById()");
  task02 = null;
  task03 = 0x0;
  unit_try(OK(xTaskGetId(task01, &task03)));
  unit_try(OK(xTaskGetHandleById(&task02, task03)));
  unit_try(task02 == task01);
  unit_end();
  unit_begin("xTaskGetAllRunTimeStats()");
  task04 = null;
  task05 = 0;
  unit_try(OK(xTaskGetAllRunTimeStats(&task04, &task05)));
  unit_try(null != task04);
  unit_try(0x1 == task05);
  unit_try(0x1 == task04[0].id);
  unit_try(OK(xMemFree(task04)));
  unit_end();
  unit_begin("xTaskGetTaskRunTimeStats()");
  task04 = null;
  unit_try(OK(xTaskGetTaskRunTimeStats(task01, &task04)));
  unit_try(null != task04);
  unit_try(0x1 == task04->id);
  unit_try(OK(xMemFree(task04)));
  unit_end();
  unit_begin("xTaskGetNumberOfTasks()");
  task06 = 0;
  unit_try(OK(xTaskGetNumberOfTasks(&task06)));
  unit_try(0x1 == task06);
  unit_end();
  unit_begin("xTaskGetTaskInfo()");
  task07 = null;
  unit_try(OK(xTaskGetTaskInfo(task01, &task07)));
  unit_try(null != task07);
  unit_try(0x1 == task07->id);
  unit_try(0x0 == strncmp("TASK01", (char *) task07->name, 0x6));
  unit_try(TaskStateSuspended == task07->state);
  unit_try(OK(xMemFree(task07)));
  unit_end();
  unit_begin("xTaskGetAllTaskInfo()");
  task07 = null;
  unit_try(OK(xTaskGetAllTaskInfo(&task07, &task06)));
  unit_try(null != task07);
  unit_try(0x1 == task06);
  unit_try(0x1 == task07->id);
  unit_try(0x0 == strncmp("TASK01", (char *) task07->name, 0x6));
  unit_try(TaskStateSuspended == task07->state);
  unit_try(OK(xMemFree(task07)));
  unit_end();
  unit_begin("xTaskGetTaskState()");
  unit_try(OK(xTaskGetTaskState(task01, &task13)));
  unit_try(TaskStateSuspended == task13);
  unit_end();
  unit_begin("xTaskGetName()");
  unit_try(OK(xTaskGetName(task01, &task08)));
  unit_try(null != task08);
  unit_try(0x0 == strncmp("TASK01", (char *) task08, 0x6));
  unit_end();
  unit_begin("xTaskGetId()");
  unit_try(OK(xTaskGetId(task01, &task14)));
  unit_try(0x1 == task14);
  unit_end();
  unit_begin("xTaskNotifyGive()");
  unit_try(OK(xTaskNotifyGive(task01, 0x7, (Byte_t *) "MESSAGE")));
  unit_end();
  unit_begin("xTaskNotificationIsWaiting()");
  unit_try(OK(xTaskNotificationIsWaiting(task01, &task15)));
  unit_try(true == task15);
  unit_end();
  unit_begin("xTaskNotifyStateClear()");
  unit_try(OK(xTaskNotifyStateClear(task01)));
  unit_try(OK(xTaskNotificationIsWaiting(task01, &task16)));
  unit_try(false == task16);
  unit_end();
  unit_begin("xTaskNotifyTake()");
  task09 = null;
  unit_try(OK(xTaskNotifyGive(task01, 0x7, (Byte_t *) "MESSAGE")));
  unit_try(OK(xTaskNotifyTake(task01, &task09)));
  unit_try(null != task09);
  unit_try(0x7 == task09->notificationBytes);
  unit_try(0x0 == strncmp("MESSAGE", (char *) task09->notificationValue, 0x7));
  unit_try(OK(xMemFree(task09)));
  unit_end();
  unit_begin("xTaskResume()");
  unit_try(OK(xTaskResume(task01)));
  unit_try(OK(xTaskGetTaskState(task01, &task17)));
  unit_try(TaskStateRunning == task17);
  unit_end();
  unit_begin("xTaskSuspend()");
  unit_try(OK(xTaskSuspend(task01)));
  unit_try(OK(xTaskGetTaskState(task01, &task18)));
  unit_try(TaskStateSuspended == task18);
  unit_end();
  unit_begin("xTaskWait()");
  unit_try(OK(xTaskWait(task01)));
  unit_try(OK(xTaskGetTaskState(task01, &task19)));
  unit_try(TaskStateWaiting == task19);
  unit_end();
  unit_begin("xTaskChangePeriod()");
  unit_try(OK(xTaskChangePeriod(task01, 0xD05)));
  unit_try(OK(xTaskGetPeriod(task01, &task20)));
  unit_try(0xD05 == task20);
  unit_end();
  unit_begin("xTaskGetPeriod()");
  unit_try(OK(xTaskChangePeriod(task01, 0x1E61)));
  unit_try(OK(xTaskGetPeriod(task01, &task21)));
  unit_try(0x1E61 == task21);
  unit_end();
  unit_begin("xTaskResetTimer()");
  unit_try(OK(xTaskResetTimer(task01)));
  unit_end();
  unit_begin("xTaskGetSchedulerState()");
  unit_try(OK(xTaskSuspendAll()));
  unit_try(OK(xTaskGetSchedulerState(&task22)));
  unit_try(SchedulerStateSuspended == task22);
  unit_try(OK(xTaskResumeAll()));
  unit_try(OK(xTaskGetSchedulerState(&task23)));
  unit_try(SchedulerStateRunning == task23);
  unit_end();
  unit_begin("Unit test for task timer event");
  unit_try(OK(xTaskResumeAll()));
  unit_try(OK(xTaskDelete(task01)));
  task10 = null;
  unit_try(OK(xTaskCreate(&task10, (Byte_t *) "TASK10", task_harness_task, null)));
  unit_try(null != task10);
  unit_try(OK(xTaskChangePeriod(task10, 0xBB8)));
  unit_try(OK(xTaskWait(task10)));
  unit_try(OK(xTaskResetTimer(task10)));
  unit_try(OK(xTaskStartScheduler()));
  unit_try(OK(xTaskDelete(task10)));
  unit_end();
  unit_begin("Unit test for direct to task notification event");
  unit_try(OK(xTaskResumeAll()));
  task11 = null;
  unit_try(OK(xTaskCreate(&task11, (Byte_t *) "TASK11", task_harness_task, null)));
  unit_try(null != task11);
  unit_try(OK(xTaskWait(task11)));
  unit_try(OK(xTaskNotifyGive(task11, 0x7, (Byte_t *) "MESSAGE")));
  unit_try(OK(xTaskNotificationIsWaiting(task11, &task24)));
  unit_try(true == task24);
  unit_try(OK(xTaskResumeAll()));
  unit_try(OK(xTaskStartScheduler()));
  unit_try(OK(xTaskNotificationIsWaiting(task11, &task25)));
  unit_try(false == task25);
  unit_try(OK(xTaskDelete(task11)));
  unit_end();
  unit_begin("Unit test for task watchdog timer");
  unit_try(OK(xTaskResumeAll()));
  task12 = null;
  unit_try(OK(xTaskCreate(&task12, (Byte_t *) "TASK12", task_harness_task2, null)));
  unit_try(null != task12);
  unit_try(OK(xTaskChangeWDPeriod(task12, 0x7D0u)));
  unit_try(OK(xTaskResume(task12)));
  unit_try(OK(xTaskGetTaskState(task12, &task26)));
  unit_try(TaskStateRunning == task26);
  unit_try(OK(xTaskStartScheduler()));
  unit_try(OK(xTaskGetTaskState(task12, &task27)));
  unit_try(TaskStateSuspended == task27);
  unit_end();
  unit_begin("xTaskGetWDPeriod()");
  unit_try(OK(xTaskGetWDPeriod(task12, &task28)));
  unit_try(0x7D0u == task28);
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