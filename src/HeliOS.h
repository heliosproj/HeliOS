/**
 * @file HeliOS.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Header file to be included in end-user code and contains all of the public data types and functions
 * @version 0.3.0
 * @date 2022-01-31
 *
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
#ifndef HELIOS_H_
#define HELIOS_H_

#include <stdint.h>

#include "config.h"
#include "defines.h"

/* TO-DO: Look at some sort of printk() functionally. */

/**
 * @brief Enumerated type for the four possible states a task
 * can be in. A task's state can be set by xTaskResume(),
 * xTaskSuspend() and xTaskWait(). It is also returned by
 * xTaskGetTaskState().
 *
 */
typedef enum {
  TaskStateNone,      /**< No task. */
  TaskStateSuspended, /**< State a task is in when it is first created by xTaskCreate() or suspended by xTaskSuspend(). */
  TaskStateRunning,   /**< State a task is in after xTaskResume() is called. */
  TaskStateWaiting    /**< State a task is in after xTaskWait() is called. */
} TaskState_t;

/**
 * @brief Type definition for the task identifier in the task
 * data structure.
 *
 */
typedef int16_t TaskId_t;

/**
 * @brief Type definition for the base data type. This data type is
 * used if no other data types are relevent.
 *
 */
typedef int16_t Base_t;

/**
 * @brief The data structure for task runtime statistics. Should be declared as xTaskRunTimeStats. This type
 * is returned by xTaskGetTaskRunTimeStats() and xTaskGetAllRunTimeStats().
 *
 */
typedef struct TaskRunTimeStats_s {
  Time_t lastRunTime;  /**< The runtime duration in microseconds the last time the task was executed by the scheduler. */
  Time_t totalRunTime; /**< The total runtime duration in microseconds the task has been executed by the scheduler. */
} TaskRunTimeStats_t;

/**
 * @brief The data structure for task information. Should be declared as xTaskInfo. This type is returned by
 * xTaskGetTaskInfo().
 *
 */
typedef struct TaskInfo_s {
  TaskId_t id;                       /**< The task identifier which is used by xTaskGetHandleById() to return the task handle. */
  char name[CONFIG_TASK_NAME_BYTES]; /**< The name of the task which is used by xTaskGetHandleByName() to return the task handle. */
  TaskState_t state;                 /**< The state the task is in which is one of four states specified in the TaskState_t enumerated data type. */
  Time_t lastRunTime;                /**< The runtime duration in microseconds the last time the task was executed by the scheduler. */
  Time_t totalRunTime;               /**< The total runtime duration in microseconds the task has been executed by the scheduler. */
} TaskInfo_t;

/**
 * @brief The data structure for task notifications. Should be declared as xTaskNotification. This type is
 * returned by xTaskNotifyTake().
 *
 */
typedef struct TaskNotification_s {
  Base_t notificationBytes;                                /**< The number of bytes in the notificationValue member that makes up the notification value. This cannot exceed CONFIG_NOTIFICATION_VALUE_BYTES. */
  char notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES]; /**< The char array that contains the actual notification value. */
} TaskNotification_t;

/**
 * @brief The data structure for queue messages. Should be declared as xQueueMessage. This type is
 * returned by xQueueReceive() and xQueuePeek().
 *
 */
typedef struct QueueMessage_s {
  Base_t messageBytes;                           /**< The number of bytes in the messageValue member that makes up the message value. This cannot exceed CONFIG_MESSAGE_VALUE_BYTES. */
  char messageValue[CONFIG_MESSAGE_VALUE_BYTES]; /**< the char array that contains the actual message value. */
} QueueMessage_t;

/**
 * @brief The data structure for system information. Should be declared as xSystemInfo. This type is returned
 * by xSystemGetSystemInfo().
 *
 */
typedef struct SystemInfo_s {
  char productName[PRODUCTNAME_SIZE]; /**< The name of the operating system or product. This is always HeliOS. */
  Base_t majorVersion;                /**< The major version number of HeliOS and is Symantec Versioning Specification (SemVer) compliant. */
  Base_t minorVersion;                /**< The minor version number of HeliOS and is Symantec Versioning Specification (SemVer) compliant. */
  Base_t patchVersion;                /**< The patch version number of HeliOS and is Symantec Versioning Specification (SemVer) compliant. */
  Base_t numberOfTasks;               /**< The number of tasks presently in a suspended, running or waiting state. */
} SystemInfo_t;

/* Stub type defines for private (internal) data structures. Should be declared using the *x* types
(e.g., xTask, xQueue, etc). */
typedef void Task_t;
typedef void TaskParm_t;
typedef void Queue_t;
typedef void Timer_t;

/* Type defines for the HeliOS API. */
typedef Base_t xBase;
typedef TaskId_t xTaskId;
typedef Timer_t *xTimer;
typedef Queue_t *xQueue;
typedef QueueMessage_t *xQueueMessage;
typedef TaskNotification_t *xTaskNotification;
typedef TaskInfo_t *xTaskInfo;
typedef TaskRunTimeStats_t *xTaskRunTimeStats;
typedef Task_t *xTask;
typedef TaskParm_t *xTaskParm;
typedef Time_t xTime;
typedef TaskState_t xTaskState;
typedef SystemInfo_t *xSystemInfo;

#ifdef __cplusplus
extern "C" {
#endif

/* System calls for the HeliOS API. */
void xTaskStartScheduler();
xTask xTaskCreate(const char *, void (*)(xTask, xTaskParm), xTaskParm);
void xTaskDelete(xTask);
xTask xTaskGetHandleByName(const char *);
xTask xTaskGetHandleById(xTaskId);
xTaskRunTimeStats xTaskGetAllRunTimeStats();
xTaskRunTimeStats xTaskGetTaskRunTimeStats(xTask);
xBase xTaskGetNumberOfTasks();
xTaskState xTaskGetTaskState(xTask);
xTaskInfo xTaskGetTaskInfo(xTask);
char *xTaskGetName(xTask);
xTaskId xTaskGetId(xTask);
char *xTaskList();
xBase xTaskNotificationIsWaiting(xTask);
void xTaskNotifyGive(xTask, xBase, const char *);
xTaskNotification xTaskNotifyTake(xTask);
void xTaskNotifyStateClear(xTask);
void xTaskResume(xTask);
void xTaskResumeAll();
void xTaskSuspendAll();
void xTaskSuspend(xTask);
void xTaskWait(xTask);
void xTaskChangePeriod(xTask, xTime);
xTime xTaskGetPeriod(xTask);
void xTaskResetTimer(xTask);
xQueue xQueueCreate(xBase);
void xQueueDelete(xQueue);
xBase xQueueGetLength(xQueue);
xBase xQueueIsQueueEmpty(xQueue);
xBase xQueueIsQueueFull(xQueue);
xBase xQueueMessagesWaiting(xQueue);
xBase xQueueSend(xQueue, xBase, const char *);
xQueueMessage xQueuePeek(xQueue);
void xQueueDropMessage(xQueue);
xQueueMessage xQueueReceive(xQueue);
xTimer xTimerCreate(xTime);
void xTimerDelete(xTimer);
void xTimerChangePeriod(xTimer, xTime);
xTime xTimerGetPeriod(xTimer);
xBase xTimerIsTimerActive(xTimer);
xBase xTimerHasTimerExpired(xTimer);
void xTimerReset(xTimer);
void xTimerStart(xTimer);
void xTimerStop(xTimer);
xSystemInfo xSystemGetSystemInfo();
void *xMemAlloc(size_t);
void xMemFree(void *);
size_t xMemGetUsed();
size_t xMemGetSize(void *);

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif