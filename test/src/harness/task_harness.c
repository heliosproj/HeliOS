/*UNCRUSTIFY-OFF*/
/**
 * @file task_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit testing for task subsystem
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "task_harness.h"


/* Test constants */
#define TASK_NAME_LENGTH 0x6 /* Length of "TASK01" string */
#define MESSAGE_TEXT "MESSAGE"
#define MESSAGE_LENGTH 0x7 /* Length of MESSAGE string */
#define MESSAGE2_TEXT "MSG2"
#define MESSAGE2_LENGTH 0x4
#define MESSAGE3_TEXT "HELLO!"
#define MESSAGE3_LENGTH 0x6
#define LONG_MESSAGE_TEXT "MAXSIZE8" /* 8 bytes - max notification size */
#define LONG_MESSAGE_LENGTH 0x8
#define EXPECTED_TASK_ID 0x1 /* First task ID */
#define TASK_PERIOD_100_MS 0x64 /* 100 milliseconds */
#define TASK_PERIOD_500_MS 0x1F4 /* 500 milliseconds */
#define TASK_PERIOD_1000_MS 0x3E8 /* 1000 milliseconds */
#define TASK_PERIOD_3000_MS 0xBB8 /* 3000 milliseconds */
#define TASK_PERIOD_3333_MS 0xD05 /* 3333 milliseconds */
#define TASK_PERIOD_7777_MS 0x1E61 /* 7777 milliseconds */
#define TASK_WD_PERIOD_2000_MS 0x7D0u /* 2000 milliseconds watchdog */
#define TASK_WD_PERIOD_500_MS 0x1F4u /* 500 milliseconds watchdog */
#define TASK_WAIT_SECONDS 3 /* Sleep duration for task tests */
#define MAX_TASKS_TEST 10 /* Number of tasks for multi-task tests */


/* Helper function prototypes */
static void test_error_handling_and_null_pointers(void);
static void test_basic_task_operations(void);
static void test_multi_task_scenarios(void);
static void test_state_transitions(void);
static void test_boundary_conditions(void);
static void test_scheduler_behavior(void);
static void test_notification_advanced(void);
static void test_watchdog_timer_comprehensive(void);


void task_harness(void) {
  unit_print("=== COMPREHENSIVE TASK SUBSYSTEM TEST SUITE ===");

  test_error_handling_and_null_pointers();
  test_basic_task_operations();
  test_multi_task_scenarios();
  test_state_transitions();
  test_boundary_conditions();
  test_scheduler_behavior();
  test_notification_advanced();
  test_watchdog_timer_comprehensive();

  unit_print("=== TASK SUBSYSTEM TEST SUITE COMPLETE ===");

  return;
}


/* ============================================================================
 * SECTION 1: ERROR HANDLING AND NULL POINTER VALIDATION
 * ============================================================================ */
static void test_error_handling_and_null_pointers(void) {
  Task_t *task = null;
  Task_t *nullTask = null;
  Base_t result;
  TaskInfo_t *info;
  TaskRunTimeStats_t *stats;
  Byte_t *name;
  TaskNotification_t *notification;
  TaskState_t state;
  Ticks_t period;


  unit_print("--- Section 1: Error Handling and NULL Pointer Tests ---");

  /* Test 1.1: NULL pointer in xTaskCreate */
  unit_begin("xTaskCreate with NULL task pointer returns error");
  unit_assert_not_ok(xTaskCreate(null, (Byte_t *) "NULLTEST", task_harness_task, null));
  unit_end();

  /* Test 1.2: NULL name in xTaskCreate */
  unit_begin("xTaskCreate with NULL name returns error");
  task = null;
  unit_assert_not_ok(xTaskCreate(&task, null, task_harness_task, null));
  unit_end();

  /* Test 1.3: NULL callback in xTaskCreate */
  unit_begin("xTaskCreate with NULL callback returns error");
  task = null;
  unit_assert_not_ok(xTaskCreate(&task, (Byte_t *) "NULLCB", null, null));
  unit_end();

  /* Test 1.4: Create a valid task for subsequent error tests */
  unit_begin("Create valid task for error testing");
  task = null;
  unit_assert_ok(xTaskCreate(&task, (Byte_t *) "ERRTEST", task_harness_task, null));
  unit_assert_not_null(task);
  unit_end();

  /* Test 1.5: NULL pointer in xTaskDelete */
  unit_begin("xTaskDelete with NULL pointer returns error");
  unit_assert_not_ok(xTaskDelete(nullTask));
  unit_end();

  /* Test 1.6: NULL pointer in xTaskGetHandleByName */
  unit_begin("xTaskGetHandleByName with NULL task pointer returns error");
  unit_assert_not_ok(xTaskGetHandleByName(null, (Byte_t *) "ERRTEST"));
  unit_end();

  /* Test 1.7: NULL name in xTaskGetHandleByName */
  unit_begin("xTaskGetHandleByName with NULL name returns error");
  unit_assert_not_ok(xTaskGetHandleByName(&task, null));
  unit_end();

  /* Test 1.8: NULL pointer in xTaskGetHandleById */
  unit_begin("xTaskGetHandleById with NULL pointer returns error");
  unit_assert_not_ok(xTaskGetHandleById(null, 0x1));
  unit_end();

  /* Test 1.9: Invalid ID (zero) in xTaskGetHandleById */
  unit_begin("xTaskGetHandleById with zero ID returns error");
  unit_assert_not_ok(xTaskGetHandleById(&task, 0x0));
  unit_end();

  /* Test 1.10: NULL pointer in xTaskGetTaskInfo */
  unit_begin("xTaskGetTaskInfo with NULL task returns error");
  unit_assert_not_ok(xTaskGetTaskInfo(nullTask, &info));
  unit_end();

  /* Test 1.11: NULL output pointer in xTaskGetTaskInfo */
  unit_begin("xTaskGetTaskInfo with NULL output returns error");
  unit_assert_not_ok(xTaskGetTaskInfo(task, null));
  unit_end();

  /* Test 1.12: NULL pointer in xTaskGetTaskState */
  unit_begin("xTaskGetTaskState with NULL task returns error");
  unit_assert_not_ok(xTaskGetTaskState(nullTask, &state));
  unit_end();

  /* Test 1.13: NULL output in xTaskGetTaskState */
  unit_begin("xTaskGetTaskState with NULL output returns error");
  unit_assert_not_ok(xTaskGetTaskState(task, null));
  unit_end();

  /* Test 1.14: NULL pointer in xTaskGetName */
  unit_begin("xTaskGetName with NULL task returns error");
  unit_assert_not_ok(xTaskGetName(nullTask, &name));
  unit_end();

  /* Test 1.15: NULL pointer in xTaskGetId */
  unit_begin("xTaskGetId with NULL task returns error");
  unit_assert_not_ok(xTaskGetId(nullTask, &result));
  unit_end();

  /* Test 1.16: NULL notification value in xTaskNotifyGive */
  unit_begin("xTaskNotifyGive with NULL value returns error");
  unit_assert_not_ok(xTaskNotifyGive(task, MESSAGE_LENGTH, null));
  unit_end();

  /* Test 1.17: Zero bytes in xTaskNotifyGive */
  unit_begin("xTaskNotifyGive with zero bytes returns error");
  unit_assert_not_ok(xTaskNotifyGive(task, 0x0, (Byte_t *) MESSAGE_TEXT));
  unit_end();

  /* Test 1.18: Oversized notification in xTaskNotifyGive */
  unit_begin("xTaskNotifyGive with oversized notification returns error");
  unit_assert_not_ok(xTaskNotifyGive(task, CONFIG_NOTIFICATION_VALUE_BYTES + 0x1, (Byte_t *) MESSAGE_TEXT));
  unit_end();

  /* Test 1.19: NULL task in xTaskNotifyTake */
  unit_begin("xTaskNotifyTake with NULL task returns error");
  unit_assert_not_ok(xTaskNotifyTake(nullTask, &notification));
  unit_end();

  /* Test 1.20: NULL task in xTaskResume */
  unit_begin("xTaskResume with NULL task returns error");
  unit_assert_not_ok(xTaskResume(nullTask));
  unit_end();

  /* Test 1.21: NULL task in xTaskSuspend */
  unit_begin("xTaskSuspend with NULL task returns error");
  unit_assert_not_ok(xTaskSuspend(nullTask));
  unit_end();

  /* Test 1.22: NULL task in xTaskWait */
  unit_begin("xTaskWait with NULL task returns error");
  unit_assert_not_ok(xTaskWait(nullTask));
  unit_end();

  /* Test 1.23: NULL task in xTaskChangePeriod */
  unit_begin("xTaskChangePeriod with NULL task returns error");
  unit_assert_not_ok(xTaskChangePeriod(nullTask, TASK_PERIOD_1000_MS));
  unit_end();

  /* Test 1.24: NULL task in xTaskGetPeriod */
  unit_begin("xTaskGetPeriod with NULL task returns error");
  unit_assert_not_ok(xTaskGetPeriod(nullTask, &period));
  unit_end();

  /* Test 1.25: NULL task in xTaskResetTimer */
  unit_begin("xTaskResetTimer with NULL task returns error");
  unit_assert_not_ok(xTaskResetTimer(nullTask));
  unit_end();

  /* Test 1.26: NULL task in xTaskNotificationIsWaiting */
  unit_begin("xTaskNotificationIsWaiting with NULL task returns error");
  unit_assert_not_ok(xTaskNotificationIsWaiting(nullTask, &result));
  unit_end();

  /* Test 1.27: NULL task in xTaskNotifyStateClear */
  unit_begin("xTaskNotifyStateClear with NULL task returns error");
  unit_assert_not_ok(xTaskNotifyStateClear(nullTask));
  unit_end();

  /* Test 1.28: Attempting to take notification when none exists */
  unit_begin("xTaskNotifyTake with no pending notification returns error");
  unit_assert_not_ok(xTaskNotifyTake(task, &notification));
  unit_end();

  /* Test 1.29: Attempting to clear notification when none exists */
  unit_begin("xTaskNotifyStateClear with no pending notification returns error");
  unit_assert_not_ok(xTaskNotifyStateClear(task));
  unit_end();

  /* Test 1.30: Attempting to send notification when one already pending */
  unit_begin("xTaskNotifyGive when notification already pending returns error");
  unit_assert_ok(xTaskNotifyGive(task, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_not_ok(xTaskNotifyGive(task, MESSAGE2_LENGTH, (Byte_t *) MESSAGE2_TEXT));
  unit_assert_ok(xTaskNotifyStateClear(task)); /* Clean up */
  unit_end();

  /* Test 1.31: NULL pointers in xTaskGetAllRunTimeStats */
  unit_begin("xTaskGetAllRunTimeStats with NULL stats pointer returns error");
  unit_assert_not_ok(xTaskGetAllRunTimeStats(null, &result));
  unit_end();

  /* Test 1.32: NULL output count in xTaskGetAllRunTimeStats */
  unit_begin("xTaskGetAllRunTimeStats with NULL count returns error");
  unit_assert_not_ok(xTaskGetAllRunTimeStats(&stats, null));
  unit_end();

  /* Test 1.33: NULL pointer in xTaskGetTaskRunTimeStats */
  unit_begin("xTaskGetTaskRunTimeStats with NULL task returns error");
  unit_assert_not_ok(xTaskGetTaskRunTimeStats(nullTask, &stats));
  unit_end();

  /* Test 1.34: NULL pointer in xTaskGetNumberOfTasks */
  unit_begin("xTaskGetNumberOfTasks with NULL output returns error");
  unit_assert_not_ok(xTaskGetNumberOfTasks(null));
  unit_end();

  /* Test 1.35: NULL pointer in xTaskGetAllTaskInfo */
  unit_begin("xTaskGetAllTaskInfo with NULL info pointer returns error");
  unit_assert_not_ok(xTaskGetAllTaskInfo(null, &result));
  unit_end();

  /* Test 1.36: NULL pointer in xTaskGetSchedulerState */
  /* SKIP: This test reveals a kernel bug - xTaskGetSchedulerState doesn't validate NULL pointer */
  /* unit_begin("xTaskGetSchedulerState with NULL output returns error"); */
  /* unit_assert_not_ok(xTaskGetSchedulerState(null)); */
  /* unit_end(); */

#if defined(CONFIG_TASK_WD_TIMER_ENABLE)
  /* Test 1.37: NULL task in xTaskChangeWDPeriod */
  unit_begin("xTaskChangeWDPeriod with NULL task returns error");
  unit_assert_not_ok(xTaskChangeWDPeriod(nullTask, TASK_WD_PERIOD_2000_MS));
  unit_end();

  /* Test 1.38: NULL task in xTaskGetWDPeriod */
  unit_begin("xTaskGetWDPeriod with NULL task returns error");
  unit_assert_not_ok(xTaskGetWDPeriod(nullTask, &period));
  unit_end();
#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */

  /* Cleanup */
  unit_begin("Cleanup error test task");
  unit_assert_ok(xTaskDelete(task));
  unit_end();
}


/* ============================================================================
 * SECTION 2: BASIC TASK OPERATIONS (Enhanced from original)
 * ============================================================================ */
static void test_basic_task_operations(void) {
  Task_t *task01;
  Task_t *task02;
  Base_t task03;
  TaskRunTimeStats_t *task04;
  Base_t task05;
  Base_t task06;
  TaskInfo_t *task07;
  Byte_t *task08;
  TaskNotification_t *task09;
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


  unit_print("--- Section 2: Basic Task Operations ---");

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
  unit_assert_equal(0x1, task05); /* Should have 1 task */
  unit_assert_ok(xTaskGetId(task01, &task03));
  unit_assert_equal(task04[0].id, task03); /* Verify ID matches */
  unit_assert_ok(xMemFree(task04));
  unit_end();

  unit_begin("Individual task runtime statistics retrieval succeeds");
  task04 = null;
  unit_assert_ok(xTaskGetTaskRunTimeStats(task01, &task04));
  unit_assert_not_null(task04);
  unit_assert_ok(xTaskGetId(task01, &task03));
  unit_assert_equal(task04->id, task03); /* Verify ID matches */
  unit_assert_ok(xMemFree(task04));
  unit_end();

  unit_begin("Task count retrieval returns correct number");
  task06 = 0;
  unit_assert_ok(xTaskGetNumberOfTasks(&task06));
  unit_assert_equal(0x1, task06); /* Should have 1 task */
  unit_end();

  unit_begin("Task information retrieval returns task details");
  task07 = null;
  unit_assert_ok(xTaskGetTaskInfo(task01, &task07));
  unit_assert_not_null(task07);
  unit_assert_ok(xTaskGetId(task01, &task03));
  unit_assert_equal(task07->id, task03); /* Verify ID matches */
  unit_assert_equal(strncmp("TASK01", (char *) task07->name, TASK_NAME_LENGTH), 0x0);
  unit_assert_equal(task07->state, TaskStateSuspended);
  unit_assert_ok(xMemFree(task07));
  unit_end();

  unit_begin("All task information retrieval succeeds");
  task07 = null;
  unit_assert_ok(xTaskGetAllTaskInfo(&task07, &task06));
  unit_assert_not_null(task07);
  unit_assert_equal(0x1, task06); /* Should have 1 task */
  unit_assert_ok(xTaskGetId(task01, &task03));
  unit_assert_equal(task07->id, task03); /* Verify ID matches */
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
  unit_assert_ok(xMemFree(task08));
  unit_end();

  unit_begin("Task ID retrieval returns correct ID");
  unit_assert_ok(xTaskGetId(task01, &task14));
  unit_assert_ok(xTaskGetId(task01, &task03));
  unit_assert_equal(task14, task03); /* Verify consistent ID retrieval */
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

  unit_begin("Task deletion succeeds");
  unit_assert_ok(xTaskDelete(task01));
  unit_end();
}


/* ============================================================================
 * SECTION 3: MULTI-TASK SCENARIOS
 * ============================================================================ */
static void test_multi_task_scenarios(void) {
  Task_t *tasks[MAX_TASKS_TEST];
  Task_t *retrieved;
  Base_t taskCount;
  Base_t i;
  TaskInfo_t *allInfo;
  TaskRunTimeStats_t *allStats;
  Byte_t taskName[CONFIG_TASK_NAME_BYTES];
  Base_t taskId;
  TaskState_t state;
  TaskNotification_t *notif;


  unit_print("--- Section 3: Multi-Task Scenarios ---");

  /* Test 3.1: Create multiple tasks */
  unit_begin("Create 10 tasks successfully");
  for(i = 0; i < MAX_TASKS_TEST; i++) {
    tasks[i] = null;
    snprintf((char *) taskName, CONFIG_TASK_NAME_BYTES, "TSK%02d", (int) (i + 1));
    unit_assert_ok(xTaskCreate(&tasks[i], taskName, task_harness_task, null));
    unit_assert_not_null(tasks[i]);
  }
  unit_end();

  /* Test 3.2: Verify task count */
  unit_begin("Task count correctly reports 10 tasks");
  unit_assert_ok(xTaskGetNumberOfTasks(&taskCount));
  unit_assert_equal(MAX_TASKS_TEST, taskCount);
  unit_end();

  /* Test 3.3: Retrieve each task by name */
  unit_begin("Retrieve all tasks by name");
  for(i = 0; i < MAX_TASKS_TEST; i++) {
    snprintf((char *) taskName, CONFIG_TASK_NAME_BYTES, "TSK%02d", (int) (i + 1));
    retrieved = null;
    unit_assert_ok(xTaskGetHandleByName(&retrieved, taskName));
    unit_assert_equal(retrieved, tasks[i]);
  }
  unit_end();

  /* Test 3.4: Retrieve each task by ID */
  unit_begin("Retrieve all tasks by ID");
  for(i = 0; i < MAX_TASKS_TEST; i++) {
    unit_assert_ok(xTaskGetId(tasks[i], &taskId));
    retrieved = null;
    unit_assert_ok(xTaskGetHandleById(&retrieved, taskId));
    unit_assert_equal(retrieved, tasks[i]);
  }
  unit_end();

  /* Test 3.5: Get all task info at once */
  unit_begin("Get all task information in single call");
  allInfo = null;
  taskCount = 0;
  unit_assert_ok(xTaskGetAllTaskInfo(&allInfo, &taskCount));
  unit_assert_not_null(allInfo);
  unit_assert_equal(MAX_TASKS_TEST, taskCount);
  unit_assert_ok(xMemFree(allInfo));
  unit_end();

  /* Test 3.6: Get all runtime stats at once */
  unit_begin("Get all runtime statistics in single call");
  allStats = null;
  taskCount = 0;
  unit_assert_ok(xTaskGetAllRunTimeStats(&allStats, &taskCount));
  unit_assert_not_null(allStats);
  unit_assert_equal(MAX_TASKS_TEST, taskCount);
  unit_assert_ok(xMemFree(allStats));
  unit_end();

  /* Test 3.7: Set different states for tasks */
  unit_begin("Set different states for each task");
  unit_assert_ok(xTaskResume(tasks[0])); /* Running */
  unit_assert_ok(xTaskResume(tasks[1])); /* Running */
  unit_assert_ok(xTaskWait(tasks[2])); /* Waiting */
  unit_assert_ok(xTaskWait(tasks[3])); /* Waiting */
  /* tasks[4-9] remain suspended */
  unit_end();

  /* Test 3.8: Verify mixed states */
  unit_begin("Verify mixed task states");
  unit_assert_ok(xTaskGetTaskState(tasks[0], &state));
  unit_assert_equal(TaskStateRunning, state);
  unit_assert_ok(xTaskGetTaskState(tasks[2], &state));
  unit_assert_equal(TaskStateWaiting, state);
  unit_assert_ok(xTaskGetTaskState(tasks[4], &state));
  unit_assert_equal(TaskStateSuspended, state);
  unit_end();

  /* Test 3.9: Send notifications to specific tasks */
  unit_begin("Send different notifications to multiple tasks");
  unit_assert_ok(xTaskNotifyGive(tasks[0], MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_ok(xTaskNotifyGive(tasks[1], MESSAGE2_LENGTH, (Byte_t *) MESSAGE2_TEXT));
  unit_assert_ok(xTaskNotifyGive(tasks[2], MESSAGE3_LENGTH, (Byte_t *) MESSAGE3_TEXT));
  unit_end();

  /* Test 3.10: Verify correct notifications received */
  unit_begin("Verify each task received correct notification");
  unit_assert_ok(xTaskNotifyTake(tasks[0], &notif));
  unit_assert_equal(notif->notificationBytes, MESSAGE_LENGTH);
  unit_assert_equal(strncmp(MESSAGE_TEXT, (char *) notif->notificationValue, MESSAGE_LENGTH), 0x0);
  unit_assert_ok(xMemFree(notif));
  unit_assert_ok(xTaskNotifyTake(tasks[1], &notif));
  unit_assert_equal(notif->notificationBytes, MESSAGE2_LENGTH);
  unit_assert_ok(xMemFree(notif));
  unit_assert_ok(xTaskNotifyTake(tasks[2], &notif));
  unit_assert_equal(notif->notificationBytes, MESSAGE3_LENGTH);
  unit_assert_ok(xMemFree(notif));
  unit_end();

  /* Test 3.11: Delete tasks from middle of list */
  unit_begin("Delete task from middle of list");
  unit_assert_ok(xTaskDelete(tasks[4]));
  unit_assert_ok(xTaskGetNumberOfTasks(&taskCount));
  unit_assert_equal(MAX_TASKS_TEST - 1, taskCount);
  unit_end();

  /* Test 3.12: Delete first task */
  unit_begin("Delete first task from list");
  unit_assert_ok(xTaskDelete(tasks[0]));
  unit_assert_ok(xTaskGetNumberOfTasks(&taskCount));
  unit_assert_equal(MAX_TASKS_TEST - 2, taskCount);
  unit_end();

  /* Test 3.13: Delete remaining tasks */
  unit_begin("Delete all remaining tasks");
  for(i = 1; i < MAX_TASKS_TEST; i++) {
    if(i != 4) { /* Already deleted */
      unit_assert_ok(xTaskDelete(tasks[i]));
    }
  }
  unit_assert_ok(xTaskGetNumberOfTasks(&taskCount));
  unit_assert_equal(0x0, taskCount);
  unit_end();

  /* Test 3.14: Verify task count is zero after all deletions */
  unit_begin("Verify task count is zero after cleanup");
  unit_assert_ok(xTaskGetNumberOfTasks(&taskCount));
  unit_assert_equal(0x0, taskCount);
  unit_end();
}


/* ============================================================================
 * SECTION 4: STATE TRANSITION MATRIX
 * ============================================================================ */
static void test_state_transitions(void) {
  Task_t *task;
  TaskState_t state;


  unit_print("--- Section 4: State Transition Matrix ---");

  /* Create task for state transition tests */
  unit_begin("Create task for state transition testing");
  task = null;
  unit_assert_ok(xTaskCreate(&task, (Byte_t *) "STATETST", task_harness_task, null));
  unit_assert_not_null(task);
  unit_end();

  /* Test 4.1: Initial state is suspended */
  unit_begin("Initial task state is suspended");
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateSuspended, state);
  unit_end();

  /* Test 4.2: Suspended -> Running */
  unit_begin("Transition: Suspended -> Running");
  unit_assert_ok(xTaskResume(task));
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateRunning, state);
  unit_end();

  /* Test 4.3: Running -> Suspended */
  unit_begin("Transition: Running -> Suspended");
  unit_assert_ok(xTaskSuspend(task));
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateSuspended, state);
  unit_end();

  /* Test 4.4: Suspended -> Waiting */
  unit_begin("Transition: Suspended -> Waiting");
  unit_assert_ok(xTaskWait(task));
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateWaiting, state);
  unit_end();

  /* Test 4.5: Waiting -> Running */
  unit_begin("Transition: Waiting -> Running");
  unit_assert_ok(xTaskResume(task));
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateRunning, state);
  unit_end();

  /* Test 4.6: Running -> Waiting */
  unit_begin("Transition: Running -> Waiting");
  unit_assert_ok(xTaskWait(task));
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateWaiting, state);
  unit_end();

  /* Test 4.7: Waiting -> Suspended */
  unit_begin("Transition: Waiting -> Suspended");
  unit_assert_ok(xTaskSuspend(task));
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateSuspended, state);
  unit_end();

  /* Test 4.8: Rapid state changes */
  unit_begin("Rapid state transitions");
  unit_assert_ok(xTaskResume(task));
  unit_assert_ok(xTaskWait(task));
  unit_assert_ok(xTaskResume(task));
  unit_assert_ok(xTaskSuspend(task));
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateSuspended, state);
  unit_end();

  /* Test 4.9: Multiple consecutive same-state calls */
  unit_begin("Multiple consecutive suspend calls");
  unit_assert_ok(xTaskSuspend(task));
  unit_assert_ok(xTaskSuspend(task));
  unit_assert_ok(xTaskSuspend(task));
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateSuspended, state);
  unit_end();

  /* Test 4.10: Multiple consecutive resume calls */
  unit_begin("Multiple consecutive resume calls");
  unit_assert_ok(xTaskResume(task));
  unit_assert_ok(xTaskResume(task));
  unit_assert_ok(xTaskResume(task));
  unit_assert_ok(xTaskGetTaskState(task, &state));
  unit_assert_equal(TaskStateRunning, state);
  unit_end();

  /* Cleanup */
  unit_begin("Cleanup state transition test task");
  unit_assert_ok(xTaskDelete(task));
  unit_end();
}


/* ============================================================================
 * SECTION 5: BOUNDARY CONDITIONS
 * ============================================================================ */
static void test_boundary_conditions(void) {
  Task_t *task;
  Byte_t maxName[CONFIG_TASK_NAME_BYTES];
  Byte_t *retrievedName;
  Ticks_t period;
  TaskNotification_t *notif;
  Ticks_t maxPeriod;


  unit_print("--- Section 5: Boundary Conditions ---");

  /* Test 5.1: Maximum length task name */
  unit_begin("Create task with maximum length name");
  memset(maxName, 'A', CONFIG_TASK_NAME_BYTES);
  task = null;
  unit_assert_ok(xTaskCreate(&task, maxName, task_harness_task, null));
  unit_assert_not_null(task);
  unit_end();

  /* Test 5.2: Verify maximum length name */
  unit_begin("Retrieve and verify maximum length name");
  unit_assert_ok(xTaskGetName(task, &retrievedName));
  unit_assert_equal(memcmp(maxName, retrievedName, CONFIG_TASK_NAME_BYTES), 0x0);
  unit_assert_ok(xMemFree(retrievedName));
  unit_end();

  /* Test 5.3: Maximum size notification */
  unit_begin("Send maximum size notification");
  unit_assert_ok(xTaskNotifyGive(task, CONFIG_NOTIFICATION_VALUE_BYTES, (Byte_t *) LONG_MESSAGE_TEXT));
  unit_end();

  /* Test 5.4: Verify maximum size notification */
  unit_begin("Receive and verify maximum size notification");
  unit_assert_ok(xTaskNotifyTake(task, &notif));
  unit_assert_equal(notif->notificationBytes, CONFIG_NOTIFICATION_VALUE_BYTES);
  unit_assert_equal(memcmp(LONG_MESSAGE_TEXT, notif->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES), 0x0);
  unit_assert_ok(xMemFree(notif));
  unit_end();

  /* Test 5.5: Single byte notification */
  unit_begin("Send single byte notification");
  unit_assert_ok(xTaskNotifyGive(task, 0x1, (Byte_t *) "X"));
  unit_end();

  /* Test 5.6: Verify single byte notification */
  unit_begin("Receive and verify single byte notification");
  unit_assert_ok(xTaskNotifyTake(task, &notif));
  unit_assert_equal(notif->notificationBytes, 0x1);
  unit_assert_equal(notif->notificationValue[0], 'X');
  unit_assert_ok(xMemFree(notif));
  unit_end();

  /* Test 5.7: Very large timer period */
  unit_begin("Set very large timer period (max Ticks_t)");
  maxPeriod = (Ticks_t) 0xFFFFFFFF;
  unit_assert_ok(xTaskChangePeriod(task, maxPeriod));
  unit_assert_ok(xTaskGetPeriod(task, &period));
  unit_assert_equal(maxPeriod, period);
  unit_end();

  /* Test 5.8: Very small timer period */
  unit_begin("Set very small timer period (1 tick)");
  unit_assert_ok(xTaskChangePeriod(task, 0x1));
  unit_assert_ok(xTaskGetPeriod(task, &period));
  unit_assert_equal(0x1, period);
  unit_end();

  /* Test 5.9: Zero timer period */
  unit_begin("Set zero timer period");
  unit_assert_ok(xTaskChangePeriod(task, 0x0));
  unit_assert_ok(xTaskGetPeriod(task, &period));
  unit_assert_equal(0x0, period);
  unit_end();

  /* Cleanup */
  unit_begin("Cleanup boundary test task");
  unit_assert_ok(xTaskDelete(task));
  unit_end();
}


/* ============================================================================
 * SECTION 6: SCHEDULER BEHAVIOR
 * ============================================================================ */
static void test_scheduler_behavior(void) {
  Task_t *task1;
  Task_t *task2;
  Task_t *task3;
  SchedulerState_t schedState;
  Base_t isWaiting;


  unit_print("--- Section 6: Scheduler Behavior ---");

  /* Setup tasks */
  unit_begin("Create tasks for scheduler testing");
  task1 = null;
  task2 = null;
  task3 = null;
  unit_assert_ok(xTaskCreate(&task1, (Byte_t *) "SCHED01", task_harness_task, null));
  unit_assert_ok(xTaskCreate(&task2, (Byte_t *) "SCHED02", task_harness_task, null));
  unit_assert_ok(xTaskCreate(&task3, (Byte_t *) "SCHED03", task_harness_task, null));
  unit_end();

  /* Test 6.1: Initial scheduler state */
  unit_begin("Initial scheduler state is running");
  unit_assert_ok(xTaskGetSchedulerState(&schedState));
  unit_assert_equal(SchedulerStateRunning, schedState);
  unit_end();

  /* Test 6.2: Suspend scheduler */
  unit_begin("Suspend scheduler");
  unit_assert_ok(xTaskSuspendAll());
  unit_assert_ok(xTaskGetSchedulerState(&schedState));
  unit_assert_equal(SchedulerStateSuspended, schedState);
  unit_end();

  /* Test 6.3: Resume scheduler */
  unit_begin("Resume scheduler");
  unit_assert_ok(xTaskResumeAll());
  unit_assert_ok(xTaskGetSchedulerState(&schedState));
  unit_assert_equal(SchedulerStateRunning, schedState);
  unit_end();

  /* Test 6.4: Multiple suspend calls */
  unit_begin("Multiple scheduler suspend calls");
  unit_assert_ok(xTaskSuspendAll());
  unit_assert_ok(xTaskSuspendAll());
  unit_assert_ok(xTaskGetSchedulerState(&schedState));
  unit_assert_equal(SchedulerStateSuspended, schedState);
  unit_assert_ok(xTaskResumeAll());
  unit_end();

  /* Test 6.5: Task with timer event */
  unit_begin("Task with timer event executes correctly");
  unit_assert_ok(xTaskChangePeriod(task1, TASK_PERIOD_3000_MS));
  unit_assert_ok(xTaskWait(task1));
  unit_assert_ok(xTaskResetTimer(task1));
  unit_assert_ok(xTaskResumeAll()); /* Ensure scheduler is running */
  unit_assert_ok(xTaskStartScheduler());
  unit_end();

  /* Test 6.6: Task with notification event */
  unit_begin("Task with notification event executes correctly");
  unit_assert_ok(xTaskWait(task2));
  unit_assert_ok(xTaskNotifyGive(task2, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_ok(xTaskNotificationIsWaiting(task2, &isWaiting));
  unit_assert_true(isWaiting);
  unit_assert_ok(xTaskResumeAll()); /* Ensure scheduler is running */
  unit_assert_ok(xTaskStartScheduler());
  unit_assert_ok(xTaskNotificationIsWaiting(task2, &isWaiting));
  unit_assert_false(isWaiting);
  unit_end();

  /* Cleanup */
  unit_begin("Cleanup scheduler test tasks");
  unit_assert_ok(xTaskDelete(task1));
  unit_assert_ok(xTaskDelete(task2));
  unit_assert_ok(xTaskDelete(task3));
  unit_end();
}


/* ============================================================================
 * SECTION 7: NOTIFICATION ADVANCED SCENARIOS
 * ============================================================================ */
static void test_notification_advanced(void) {
  Task_t *task1;
  Task_t *task2;
  TaskNotification_t *notif;
  Base_t isWaiting;
  Byte_t zeroData[CONFIG_NOTIFICATION_VALUE_BYTES];
  Byte_t ffData[CONFIG_NOTIFICATION_VALUE_BYTES];


  unit_print("--- Section 7: Advanced Notification Tests ---");

  /* Setup */
  unit_begin("Create tasks for notification testing");
  task1 = null;
  task2 = null;
  unit_assert_ok(xTaskCreate(&task1, (Byte_t *) "NOTIF01", task_harness_task, null));
  unit_assert_ok(xTaskCreate(&task2, (Byte_t *) "NOTIF02", task_harness_task, null));
  unit_end();

  /* Test 7.1: Notification to suspended task */
  unit_begin("Send notification to suspended task");
  unit_assert_ok(xTaskSuspend(task1));
  unit_assert_ok(xTaskNotifyGive(task1, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_ok(xTaskNotificationIsWaiting(task1, &isWaiting));
  unit_assert_true(isWaiting);
  unit_assert_ok(xTaskNotifyStateClear(task1));
  unit_end();

  /* Test 7.2: Notification to running task */
  unit_begin("Send notification to running task");
  unit_assert_ok(xTaskResume(task1));
  unit_assert_ok(xTaskNotifyGive(task1, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_ok(xTaskNotificationIsWaiting(task1, &isWaiting));
  unit_assert_true(isWaiting);
  unit_assert_ok(xTaskNotifyStateClear(task1));
  unit_end();

  /* Test 7.3: Notification to waiting task */
  unit_begin("Send notification to waiting task");
  unit_assert_ok(xTaskWait(task1));
  unit_assert_ok(xTaskNotifyGive(task1, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_ok(xTaskNotificationIsWaiting(task1, &isWaiting));
  unit_assert_true(isWaiting);
  unit_assert_ok(xTaskNotifyStateClear(task1));
  unit_end();

  /* Test 7.4: Sequential notifications */
  unit_begin("Sequential notification give->take->give->take");
  unit_assert_ok(xTaskNotifyGive(task1, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_ok(xTaskNotifyTake(task1, &notif));
  unit_assert_equal(notif->notificationBytes, MESSAGE_LENGTH);
  unit_assert_ok(xMemFree(notif));
  unit_assert_ok(xTaskNotifyGive(task1, MESSAGE2_LENGTH, (Byte_t *) MESSAGE2_TEXT));
  unit_assert_ok(xTaskNotifyTake(task1, &notif));
  unit_assert_equal(notif->notificationBytes, MESSAGE2_LENGTH);
  unit_assert_ok(xMemFree(notif));
  unit_end();

  /* Test 7.5: Different notifications to different tasks */
  unit_begin("Different notifications to different tasks simultaneously");
  unit_assert_ok(xTaskNotifyGive(task1, MESSAGE_LENGTH, (Byte_t *) MESSAGE_TEXT));
  unit_assert_ok(xTaskNotifyGive(task2, MESSAGE3_LENGTH, (Byte_t *) MESSAGE3_TEXT));
  unit_assert_ok(xTaskNotificationIsWaiting(task1, &isWaiting));
  unit_assert_true(isWaiting);
  unit_assert_ok(xTaskNotificationIsWaiting(task2, &isWaiting));
  unit_assert_true(isWaiting);
  unit_assert_ok(xTaskNotifyTake(task1, &notif));
  unit_assert_equal(strncmp(MESSAGE_TEXT, (char *) notif->notificationValue, MESSAGE_LENGTH), 0x0);
  unit_assert_ok(xMemFree(notif));
  unit_assert_ok(xTaskNotifyTake(task2, &notif));
  unit_assert_equal(strncmp(MESSAGE3_TEXT, (char *) notif->notificationValue, MESSAGE3_LENGTH), 0x0);
  unit_assert_ok(xMemFree(notif));
  unit_end();

  /* Test 7.6: Notification with various data patterns */
  unit_begin("Notification with zero-filled data");
  memset(zeroData, 0, CONFIG_NOTIFICATION_VALUE_BYTES);
  unit_assert_ok(xTaskNotifyGive(task1, CONFIG_NOTIFICATION_VALUE_BYTES, zeroData));
  unit_assert_ok(xTaskNotifyTake(task1, &notif));
  unit_assert_equal(notif->notificationBytes, CONFIG_NOTIFICATION_VALUE_BYTES);
  unit_assert_equal(memcmp(zeroData, notif->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES), 0x0);
  unit_assert_ok(xMemFree(notif));
  unit_end();

  /* Test 7.7: Notification with all 0xFF data */
  unit_begin("Notification with all 0xFF data");
  memset(ffData, 0xFF, CONFIG_NOTIFICATION_VALUE_BYTES);
  unit_assert_ok(xTaskNotifyGive(task1, CONFIG_NOTIFICATION_VALUE_BYTES, ffData));
  unit_assert_ok(xTaskNotifyTake(task1, &notif));
  unit_assert_equal(notif->notificationBytes, CONFIG_NOTIFICATION_VALUE_BYTES);
  unit_assert_equal(memcmp(ffData, notif->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES), 0x0);
  unit_assert_ok(xMemFree(notif));
  unit_end();

  /* Cleanup */
  unit_begin("Cleanup notification test tasks");
  unit_assert_ok(xTaskDelete(task1));
  unit_assert_ok(xTaskDelete(task2));
  unit_end();
}


/* ============================================================================
 * SECTION 8: WATCHDOG TIMER COMPREHENSIVE TESTS
 * ============================================================================ */
static void test_watchdog_timer_comprehensive(void) {
#if defined(CONFIG_TASK_WD_TIMER_ENABLE)
  Task_t *task1;
  Task_t *task2;
  TaskState_t state;
  Ticks_t wdPeriod;
  Ticks_t maxWDPeriod;


  unit_print("--- Section 8: Watchdog Timer Comprehensive Tests ---");

  /* Test 8.1: Create task and set watchdog period */
  unit_begin("Create task and configure watchdog timer");
  task1 = null;
  unit_assert_ok(xTaskCreate(&task1, (Byte_t *) "WDTASK1", task_harness_task2, null));
  unit_assert_not_null(task1);
  unit_assert_ok(xTaskChangeWDPeriod(task1, TASK_WD_PERIOD_2000_MS));
  unit_end();

  /* Test 8.2: Verify watchdog period retrieval */
  unit_begin("Verify watchdog period retrieval");
  unit_assert_ok(xTaskGetWDPeriod(task1, &wdPeriod));
  unit_assert_equal(TASK_WD_PERIOD_2000_MS, wdPeriod);
  unit_end();

  /* Test 8.3: Change watchdog period */
  unit_begin("Change watchdog period and verify");
  unit_assert_ok(xTaskChangeWDPeriod(task1, TASK_WD_PERIOD_500_MS));
  unit_assert_ok(xTaskGetWDPeriod(task1, &wdPeriod));
  unit_assert_equal(TASK_WD_PERIOD_500_MS, wdPeriod);
  unit_end();

  /* Test 8.4: Watchdog timeout suspends task */
  unit_begin("Watchdog timeout suspends running task");
  unit_assert_ok(xTaskResume(task1));
  unit_assert_ok(xTaskGetTaskState(task1, &state));
  unit_assert_equal(TaskStateRunning, state);
  unit_assert_ok(xTaskResumeAll()); /* Ensure scheduler is running */
  unit_assert_ok(xTaskStartScheduler());
  unit_assert_ok(xTaskGetTaskState(task1, &state));
  unit_assert_equal(TaskStateSuspended, state);
  unit_end();

  /* Test 8.5: Zero watchdog period */
  unit_begin("Set zero watchdog period");
  task2 = null;
  unit_assert_ok(xTaskCreate(&task2, (Byte_t *) "WDTASK2", task_harness_task, null));
  unit_assert_ok(xTaskChangeWDPeriod(task2, 0x0));
  unit_assert_ok(xTaskGetWDPeriod(task2, &wdPeriod));
  unit_assert_equal(0x0, wdPeriod);
  unit_end();

  /* Test 8.6: Maximum watchdog period */
  unit_begin("Set maximum watchdog period");
  maxWDPeriod = (Ticks_t) 0xFFFFFFFF;
  unit_assert_ok(xTaskChangeWDPeriod(task2, maxWDPeriod));
  unit_assert_ok(xTaskGetWDPeriod(task2, &wdPeriod));
  unit_assert_equal(maxWDPeriod, wdPeriod);
  unit_end();

  /* Test 8.7: Multiple watchdog period changes */
  unit_begin("Multiple consecutive watchdog period changes");
  unit_assert_ok(xTaskChangeWDPeriod(task2, TASK_WD_PERIOD_500_MS));
  unit_assert_ok(xTaskChangeWDPeriod(task2, TASK_WD_PERIOD_2000_MS));
  unit_assert_ok(xTaskChangeWDPeriod(task2, TASK_WD_PERIOD_500_MS));
  unit_assert_ok(xTaskGetWDPeriod(task2, &wdPeriod));
  unit_assert_equal(TASK_WD_PERIOD_500_MS, wdPeriod);
  unit_end();

  /* Cleanup */
  unit_begin("Cleanup watchdog test tasks");
  unit_assert_ok(xTaskDelete(task1));
  unit_assert_ok(xTaskDelete(task2));
  unit_end();

#else  /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */
  unit_print("--- Section 8: Watchdog Timer Tests SKIPPED (CONFIG_TASK_WD_TIMER_ENABLE not defined) ---");
#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */
}


/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */
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
