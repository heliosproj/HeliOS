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

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

void TaskInit();
int16_t xTaskAdd(const char *, void (*)(int16_t));
void xTaskRemove(int16_t);
void xTaskClear();
void xTaskStart(int16_t);
void xTaskStop(int16_t);
void xTaskWait(int16_t);
int16_t xTaskGetId(const char *);
void xTaskNotify(int16_t, int16_t, char *);
void xTaskNotifyClear(int16_t);
xTaskGetNotifResult *xTaskGetNotif(int16_t);
xTaskGetInfoResult *xTaskGetInfo(int16_t);
bool TaskListSeek(int16_t);
xTaskGetListResult *xTaskGetList(int16_t *);
void xTaskSetTimer(int16_t, Time_t);
void xTaskResetTimer(int16_t);

#ifdef __cplusplus
} // extern "C" {
#endif
