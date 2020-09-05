/*
 * HeliOS Embedded Operating System
 * Copyright (C) 2020 Manny Peterson <me@mannypeterson.com>
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

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif

void TaskInit();
TaskId_t xTaskAdd(const char *, void (*)(TaskId_t));
void xTaskRemove(TaskId_t);
void xTaskClear();
void xTaskStart(TaskId_t);
void xTaskStop(TaskId_t);
void xTaskWait(TaskId_t);
TaskId_t xTaskGetId(const char *);
void xTaskNotify(TaskId_t, int16_t, char *);
void xTaskNotifyClear(TaskId_t);
TaskGetNotifResult_t *xTaskGetNotif(TaskId_t);
TaskGetInfoResult_t *xTaskGetInfo(TaskId_t);
bool TaskListSeek(TaskId_t);
TaskGetListResult_t *xTaskGetList(int16_t *);
void xTaskSetTimer(TaskId_t, Time_t);
void xTaskResetTimer(TaskId_t);

#ifdef __cplusplus
} // extern "C" {
#endif
