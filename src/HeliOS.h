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
  TaskStateSuspended,
  TaskStateRunning,
  TaskStateWaiting
} TaskState_t;

typedef struct TaskRunTimeStats_s {
  Time_t lastRunTime;
  Time_t totalRunTime;
} TaskRunTimeStats_t;

typedef struct TaskInfo_s {
  int16_t id;
  char name[TASKNAME_SIZE];
  TaskState_t state;
  Time_t lastRunTime;
  Time_t totalRunTime;
} TaskInfo_t;

typedef struct TaskNotification_s {
  int16_t notificationBytes;
  char notificationValue[TNOTIFYVALUE_SIZE];
} TaskNotification_t;

typedef struct Timer_s {
  Time_t timerPeriod;
  Time_t timerStartTime;
  struct void *next;
} Timer_t;

typedef struct QueueMessage_s {
  int16_t messageBytes;
  char messageValue[TNOTIFYVALUE_SIZE];
  struct void *next;
} QueueMessage_t;

typedef void Task_t;
typedef void TaskParm_t;
typedef void Queue_t;

typedef Timer_t *xTimer;
typedef Queue_t *xQueue;
typedef QueueMessage_t *xQueueMessage;
typedef TaskNotification_t *xTaskNotification;
typedef TaskInfo_t *xTaskInfo;
typedef TaskRunTimeStats_t *xTaskRunTimeStats;
typedef Task_t *xTask;
typedef TaskParm_t *xTaskParm;

#ifdef __cplusplus
extern "C" {
#endif

void xTaskStartScheduler();
Task_t *xTaskCreate(const char *, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *);
void xTaskDelete(Task_t *);
Task_t *xTaskGetHandleByName(const char *);
Task_t *xTaskGetHandleById(Task_t *);
TaskRunTimeStats_t *xTaskGetAllRunTimeStats();
TaskRunTimeStats_t *xTaskGetTaskRunTimeStats(Task_t *);
int16_t xTaskGetNumberOfTasks();
TaskState_t xTaskGetTaskState(Task_t *task_);
TaskInfo_t *xTaskGetTaskInfo(Task_t *);
char * xTaskGetName(Task_t *); 
char *xTaskList();
void xTaskNotifyGive(Task_t *task_, int16_t, char *);
TaskNotification_t *xTaskNotifyTake(Task_t *);
void xTaskNotifyStateClear(Task_t *);
void xTaskResume(Task_t *);
void xTaskResumeAll();
void xTaskSuspendAll();
void xTaskSuspend(Task_t *);
void xTaskWait(Task_t *);
void xTaskSetTimer(Task_t *, Time_t);
void xTaskSetTimerWOReset(Task_t *, Time_t);
void xTaskResetTimer(Task_t *);
Queue_t *xQueueCreate(int16_t);
void xQueueDelete(Queue_t *);
int16_t xQueueGetLength(Queue_t *);
int16_t xQueueIsQueueEmpty(Queue_t *);
int16_t xQueueIsQueueFull(Queue_t *);
int16_t xQueueMessagesWaiting(Queue_t *);
int16_t xQueueSend(Queue_t *, int16_t, char *);
QueueMessage_t *xQueuePeek(Queue_t *);
void xQueueDropMessage(Queue_t *);
QueueMessage_t *xQueueReceive(Queue_t *); 

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif