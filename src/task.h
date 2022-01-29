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

#ifdef __cplusplus
extern "C" {
#endif

Task_t *xTaskCreate(const char *, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *);
void xTaskDelete(Task_t *);
Task_t *xTaskGetHandleByName(const char *);
Task_t *xTaskGetHandleById(TaskId_t id_);
TaskRunTimeStats_t *xTaskGetAllRunTimeStats();
TaskRunTimeStats_t *xTaskGetTaskRunTimeStats(Task_t *);
int16_t xTaskGetNumberOfTasks();
TaskState_t xTaskGetTaskState(Task_t *);
TaskInfo_t *xTaskGetTaskInfo(Task_t *);
char * xTaskGetName(Task_t *); 
char *xTaskList();

void xTaskChangeState(Task_t *, TaskState_t);
TaskList_t *TaskListGet();

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif