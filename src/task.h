/**
 * @file task.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for tasks and managing tasks in HeliOS
 * @version 0.3.0
 * @date 2022-01-31
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
#ifndef TASK_H_
#define TASK_H_

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "defines.h"
#include "types.h"
#include "mem.h"
#include "queue.h"
#include "sched.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

Task_t *xTaskCreate(const char *, void (*)(Task_t *, TaskParm_t *), TaskParm_t *);
void xTaskDelete(Task_t *);
Task_t *xTaskGetHandleByName(const char *);
Task_t *xTaskGetHandleById(TaskId_t);
TaskRunTimeStats_t *xTaskGetAllRunTimeStats();
TaskRunTimeStats_t *xTaskGetTaskRunTimeStats(Task_t *);
Base_t xTaskGetNumberOfTasks();
TaskState_t xTaskGetTaskState(Task_t *);
TaskInfo_t *xTaskGetTaskInfo(Task_t *);
char * xTaskGetName(Task_t *); 
TaskId_t xTaskGetId(Task_t *);
char *xTaskList();
Base_t xTaskNotificationIsWaiting(Task_t *);
void xTaskNotifyGive(Task_t *task_, Base_t, const char *); 
TaskNotification_t *xTaskNotifyTake(Task_t *);
TaskList_t *TaskListGet();
void xTaskNotifyStateClear(Task_t *);
void xTaskResume(Task_t *);
void xTaskSuspend(Task_t *);
void xTaskWait(Task_t *);
void xTaskChangePeriod(Task_t *, Time_t);
Time_t xTaskGetPeriod(Task_t *); 
void xTaskResetTimer(Task_t *);

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif