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
#ifndef LIST_H_
#define LIST_H_
#include <stdbool.h>
#include <stdint.h>

#include "HeliOS.h"
#include "mem.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

void TaskListInit();
void TaskListClear();
void TaskListAdd(Task_t *);
void TaskListRemove();
Task_t *TaskListGet();
Task_t *TaskListGetPriv();
bool TaskListMoveNext();
bool TaskListMoveNextPriv();
void TaskListRewind();
void TaskListRewindPriv();

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif