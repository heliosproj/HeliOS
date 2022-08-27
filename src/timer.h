/**
 * @file timer.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for timers
 * @version 0.3.5
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
#ifndef TIMER_H_
#define TIMER_H_

#include "config.h"
#include "defines.h"
#include "types.h"
#include "port.h"
#include "mem.h"
#include "queue.h"
#include "sys.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

Timer_t *xTimerCreate(Ticks_t timerPeriod_);
void xTimerDelete(Timer_t *timer_);
void xTimerChangePeriod(Timer_t *timer_, Ticks_t timerPeriod_);
Ticks_t xTimerGetPeriod(Timer_t *timer_);
Base_t xTimerIsTimerActive(Timer_t *timer_);
Base_t xTimerHasTimerExpired(Timer_t *timer_);
void xTimerReset(Timer_t *timer_);
void xTimerStart(Timer_t *timer_);
void xTimerStop(Timer_t *timer_);
Base_t _TimerListFindTimer_(const Timer_t *timer_);

#if defined(POSIX_ARCH_OTHER)
void __TimerStateClear__(void);
#endif

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif