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
#ifndef SCHED_H_
#define SCHED_H_

#include "HeliOS.h"
#include "config.h"
#include "mem.h"
#include "queue.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

void xHeliOSLoop();
HeliOSGetInfoResult_t *xHeliOSGetInfo();
inline Flag_t IsNotCritBlocking()
inline Time_t CurrentTime();
inline void TaskRun(Task_t *);
inline void RuntimeReset();
void memcpy_(void *, void *, size_t);
void memset_(void *, int16_t, size_t);
char *strncpy_(char *, const char *, size_t);
int16_t strncmp_(const char *, const char *, size_t);

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif