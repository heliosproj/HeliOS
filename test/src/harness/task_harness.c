/*UNCRUSTIFY-OFF*/
/**
 * @file task_harness.c
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
#include "task_harness.h"

/* Test constants */
#define TASK_NAME_LENGTH        0x6     /* Length of "TASK01" string */
#define MESSAGE_TEXT            "MESSAGE"
#define MESSAGE_LENGTH          0x7     /* Length of MESSAGE string */
#define EXPECTED_TASK_ID        0x1     /* First task ID */
#define TASK_PERIOD_3333_MS     0xD05   /* 3333 milliseconds */
#define TASK_PERIOD_7777_MS     0x1E61  /* 7777 milliseconds */
#define TASK_PERIOD_3000_MS     0xBB8   /* 3000 milliseconds */
#define TASK_WD_PERIOD_2000_MS  0x7D0u  /* 2000 milliseconds watchdog */
#define TASK_WAIT_SECONDS       3       /* Sleep duration for task tests */


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
  Base_t task14 = nil;
  Base_t task15 = nil;
  Base_t task16 = nil;
  TaskState_t task17;
  TaskState_t task18;
  TaskState_t task19;
  Ticks_t task20 = nil;
  Ticks_t task21 = nil;
  SchedulerState_t task22;
  SchedulerState_t task23;
  Base_t task24 = nil;
  Base_t task25 = nil;
  TaskState_t task26;
  TaskState_t task27;
  Ticks_t task28 = nil;


  unit_begin("Task creation with name and function succeeds");
  task01 = null;
  unit_assert_ok(xTaskCreate(&task01, (Byte_t *) "TASK01", task_harness_task, null));
  unit_assert_not_null(task01);
  unit_end();
  unit_begin("Task handle retrieval by name succeeds");
  task02 = null;
  unit_assert_ok(xTaskGetHandleByName(&task02, (Byte_t *) "TASK01"));
  unit_assert_equal(task02, task01);
  unit_end();
  unit_begin("Task handle retrieval by ID succeeds");
  task02 = null;
  task03 = 0x0;
  unit_assert_ok(xTaskGetId(task01, &task03));
  unit_assert_ok(xTaskGetHandleById(&task02, task03));
  unit_assert_equal(task02, task01);
  unit_end();
  unit_begin("All task runtime statistics retrieval succeeds");
  task04 = null;
  task05 = 0;
  unit_assert_ok(xTaskGetAllRunTimeStats(&task04, &task05));
  unit_assert_not_null(task04);
  unit_assert_equal(EXPECTED_TASK_ID, task05);
  unit_assert_equal(task04[0].id, EXPECTED_TASK_ID);
  unit_assert_ok(xMemFree(task04));
  unit_end();
  unit_begin("Individual task runtime statistics retrieval succeeds");
  task04 = null;
  unit_assert_ok(xTaskGetTaskRunTimeStats(task01, &task04));
  unit_assert_not_null(task04);
  unit_assert_equal(task04->id, EXPECTED_TASK_ID);
  unit_assert_ok(xMemFree(task04));
  unit_end();
  unit_begin("Task count retrieval returns correct number");
  task06 = 0;
  unit_assert_ok(xTaskGetNumberOfTasks(&task06));
  unit_assert_equal(EXPECTED_TASK_ID, task06);
  unit_end();
  unit_begin("Task information retrieval returns task details");
  task07 = null;
  unit_assert_ok(xTaskGetTaskInfo(task01, &task07));
  unit_assert_not_null(task07);
  unit_assert_equal(task07->id, EXPECTED_TASK_ID);
  unit_assert_equal(strncmp("TASK01", (char *) task07->name, TASK_NAME_LENGTH), 0x0);
  unit_assert_equal(task07->state, TaskStateSuspended);
  unit_assert_ok(xMemFree(task07));
  unit_end();
  unit_begin("All task information retrieval succeeds");
  task07 = null;
  unit_assert_ok(xTaskGetAllTaskInfo(&task07, &task06));
  unit_assert_not_null(task07);
  unit_assert_equal(EXPECTED_TASK_ID, task06);
  unit_assert_equal(task07->id, EXPECTED_TASK_ID);
  unit_assert_equal(strncmp("TASK01", (char *) task07->name, TASK_NAME_LENGTH), 0x0);
  unit_assert_equal(task07->state, TaskStateSuspended);
  unit_assert_ok(xMemFree(task07));
  unit_end();
  unit_begin("Task state retrieval returns suspended state");
  unit_assert_ok(xTaskGetTaskState(task01, &task13));
  unit_assert_equal(TaskStateSuspended, task13);
  unit_end();
  unit_begin("Task name retrieval returns correct name");
  unit_assert_ok(xTaskGetName(task01, &task08));
  unit_assert_not_null(task08);
  unit_assert_equal(strncmp("TASK01", (char *) task08, TASK_NAME_LENGTH), 0x0);
  unit_end();
  unit_begin("Task ID retrieval returns correct ID");
  unit_assert_ok(xTaskGetId(task01, &task14));
  unit_assert_equal(EXPECTED_TASK_ID, task14);
  unit_end();
  unit_begin("Task notification delivery succeeds");
  unit_assert_ok(xTaskNotifyGive(task01, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_end();
  unit_begin("Task notification waiting check returns true");
  unit_assert_ok(xTaskNotificationIsWaiting(task01, &task15));
  unit_assert_true(task15);
  unit_end();
  unit_begin("Task notification state clear removes pending notification");
  unit_assert_ok(xTaskNotifyStateClear(task01));
  unit_assert_ok(xTaskNotificationIsWaiting(task01, &task16));
  unit_assert_false(task16);
  unit_end();
  unit_begin("Task notification retrieval returns message");
  task09 = null;
  unit_assert_ok(xTaskNotifyGive(task01, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_ok(xTaskNotifyTake(task01, &task09));
  unit_assert_not_null(task09);
  unit_assert_equal(task09->notificationBytes, MESSAGE_LENGTH);
  unit_assert_equal(strncmp(MESSAGE_TEXT, (char *) task09->notificationValue, MESSAGE_LENGTH), 0x0);
  unit_assert_ok(xMemFree(task09));
  unit_end();
  unit_begin("Task resume changes state to running");
  unit_assert_ok(xTaskResume(task01));
  unit_assert_ok(xTaskGetTaskState(task01, &task17));
  unit_assert_equal(TaskStateRunning, task17);
  unit_end();
  unit_begin("Task suspend changes state to suspended");
  unit_assert_ok(xTaskSuspend(task01));
  unit_assert_ok(xTaskGetTaskState(task01, &task18));
  unit_assert_equal(TaskStateSuspended, task18);
  unit_end();
  unit_begin("Task wait changes state to waiting");
  unit_assert_ok(xTaskWait(task01));
  unit_assert_ok(xTaskGetTaskState(task01, &task19));
  unit_assert_equal(TaskStateWaiting, task19);
  unit_end();
  unit_begin("Task period change updates period value");
  unit_assert_ok(xTaskChangePeriod(task01, TASK_PERIOD_3333_MS));
  unit_assert_ok(xTaskGetPeriod(task01, &task20));
  unit_assert_equal(TASK_PERIOD_3333_MS, task20);
  unit_end();
  unit_begin("Task period retrieval returns configured value");
  unit_assert_ok(xTaskChangePeriod(task01, TASK_PERIOD_7777_MS));
  unit_assert_ok(xTaskGetPeriod(task01, &task21));
  unit_assert_equal(TASK_PERIOD_7777_MS, task21);
  unit_end();
  unit_begin("Task timer reset succeeds");
  unit_assert_ok(xTaskResetTimer(task01));
  unit_end();
  unit_begin("Scheduler state retrieval reflects suspend and resume");
  unit_assert_ok(xTaskSuspendAll());
  unit_assert_ok(xTaskGetSchedulerState(&task22));
  unit_assert_equal(SchedulerStateSuspended, task22);
  unit_assert_ok(xTaskResumeAll());
  unit_assert_ok(xTaskGetSchedulerState(&task23));
  unit_assert_equal(SchedulerStateRunning, task23);
  unit_end();
  unit_begin("Unit test for task timer event");
  unit_assert_ok(xTaskResumeAll());
  unit_assert_ok(xTaskDelete(task01));
  task10 = null;
  unit_assert_ok(xTaskCreate(&task10, (Byte_t *) "TASK10", task_harness_task, null));
  unit_assert_not_null(task10);
  unit_assert_ok(xTaskChangePeriod(task10, TASK_PERIOD_3000_MS));
  unit_assert_ok(xTaskWait(task10));
  unit_assert_ok(xTaskResetTimer(task10));
  unit_assert_ok(xTaskStartScheduler());
  unit_assert_ok(xTaskDelete(task10));
  unit_end();
  unit_begin("Unit test for direct to task notification event");
  unit_assert_ok(xTaskResumeAll());
  task11 = null;
  unit_assert_ok(xTaskCreate(&task11, (Byte_t *) "TASK11", task_harness_task, null));
  unit_assert_not_null(task11);
  unit_assert_ok(xTaskWait(task11));
  unit_assert_ok(xTaskNotifyGive(task11, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_ok(xTaskNotificationIsWaiting(task11, &task24));
  unit_assert_true(task24);
  unit_assert_ok(xTaskResumeAll());
  unit_assert_ok(xTaskStartScheduler());
  unit_assert_ok(xTaskNotificationIsWaiting(task11, &task25));
  unit_assert_false(task25);
  unit_assert_ok(xTaskDelete(task11));
  unit_end();
  unit_begin("Unit test for task watchdog timer");
  unit_assert_ok(xTaskResumeAll());
  task12 = null;
  unit_assert_ok(xTaskCreate(&task12, (Byte_t *) "TASK12", task_harness_task2, null));
  unit_assert_not_null(task12);
  unit_assert_ok(xTaskChangeWDPeriod(task12, TASK_WD_PERIOD_2000_MS));
  unit_assert_ok(xTaskResume(task12));
  unit_assert_ok(xTaskGetTaskState(task12, &task26));
  unit_assert_equal(TaskStateRunning, task26);
  unit_assert_ok(xTaskStartScheduler());
  unit_assert_ok(xTaskGetTaskState(task12, &task27));
  unit_assert_equal(TaskStateSuspended, task27);
  unit_end();
  unit_begin("Task watchdog period retrieval returns configured value");
  unit_assert_ok(xTaskGetWDPeriod(task12, &task28));
  unit_assert_equal(TASK_WD_PERIOD_2000_MS, task28);
  unit_end();

  return;
}


void task_harness_task(Task_t *task_, TaskParm_t *parm_) {
  xTaskNotifyStateClear(task_);
  xTaskSuspendAll();

  return;
}


void task_harness_task2(Task_t *task_, TaskParm_t *parm_) {
  sleep(TASK_WAIT_SECONDS);
  xTaskSuspendAll();

  return;
}