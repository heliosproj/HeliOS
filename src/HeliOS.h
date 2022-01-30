/*
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
 */
#ifndef HELIOS_H_
#define HELIOS_H_

#include <stdint.h>

#include "config.h"
#include "defines.h"

typedef enum {
  TaskStateNone,
  TaskStateSuspended,
  TaskStateRunning,
  TaskStateWaiting
} TaskState_t;

typedef enum {
  TimerStateNone,
  TimerStateStopped,
  TimerStateRunning
} TimerState_t;

typedef int16_t TaskId_t;
typedef int16_t Base_t;

typedef struct TaskRunTimeStats_s {
  Time_t lastRunTime;
  Time_t totalRunTime;
} TaskRunTimeStats_t;

typedef struct TaskInfo_s {
  TaskId_t id;
  char name[TASKNAME_SIZE];
  TaskState_t state;
  Time_t lastRunTime;
  Time_t totalRunTime;
} TaskInfo_t;

typedef struct TaskNotification_s {
  Base_t notificationBytes;
  char notificationValue[TNOTIFYVALUE_SIZE];
} TaskNotification_t;

typedef struct QueueMessage_s {
  Base_t messageBytes;
  char messageValue[TNOTIFYVALUE_SIZE];
} QueueMessage_t;

typedef void Task_t;
typedef void TaskParm_t;
typedef void Queue_t;
typedef void Timer_t;

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

#ifdef __cplusplus
extern "C" {
#endif

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
Base_t xTaskNotificationIsWaiting(xTask);
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
xBase xTimerIsActive(xTimer);
xBase xTimerHasElapsed(xTimer);
void xTimerReset(xTimer);
void xTimerStart(xTimer);
void xTimerStop(xTimer);

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif