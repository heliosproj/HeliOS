/**
 * @file timer.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for timers and managing timers in HeliOS
 * @version 0.3.0
 * @date 2022-01-31
 * 
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
#ifndef TIMER_H_
#define TIMER_H_

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "defines.h"
#include "types.h"
#include "mem.h"
#include "queue.h"
#include "sched.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

Timer_t *xTimerCreate(Time_t);
void xTimerDelete(Timer_t *);
void xTimerChangePeriod(Timer_t *, Time_t);
Time_t xTimerGetPeriod(Timer_t *);
Base_t xTimerIsTimerActive(Timer_t *);
Base_t xTimerHasTimerExpired(Timer_t *);
void xTimerReset(Timer_t *);
void xTimerStart(Timer_t *);
void xTimerStop(Timer_t *);

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif