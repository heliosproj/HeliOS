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

#if !defined(null)
#define null 0x0
#endif

#if !defined(true)
#define true 0x1
#endif

#if !defined(false)
#define false 0x0
#endif

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

typedef void Task_t;
typedef void TaskParm_t;

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
Task_t *xTaskGetHandleById(TaskId_t id_);
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

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif