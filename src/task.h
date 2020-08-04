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

void TaskInit();
int xTaskAdd(const char*, void (*)(int));
void xTaskRemove(int);
void xTaskClear();
void xTaskStart(int);
void xTaskStop(int);
void xTaskWait(int);
int xTaskGetId(const char*);
void xTaskNotify(int, int, char*);
void xTaskNotifyClear(int);
struct xTaskGetNotifResult* xTaskGetNotif(int); 
struct xTaskGetInfoResult* xTaskGetInfo(int);
int TaskListSeek(int);
struct xTaskGetListResult* xTaskGetList(int*);
void xTaskSetTimer(int, unsigned long);
void xTaskResetTimer(int);

#ifdef __cplusplus
} // extern "C" {
#endif
