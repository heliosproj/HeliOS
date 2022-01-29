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

typedef enum {
  TaskStateStopped,
  TaskStateRunning,
  TaskStateWaiting
} TaskState_t;

typedef void Task_t;
typedef Task_t *xTask;
typedef void TaskParm_t;
typedef TaskParm_t *xTaskParm;

#ifdef __cplusplus
extern "C" {
#endif

void xHeliOSLoop();
Task_t *xTaskCreate(const char *, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *);
Task_t *xTaskDestroy(Task_t *);
void xTaskChangeState(Task_t *, TaskState_t);

#ifdef __cplusplus
}  // extern "C" {
#endif

#endif