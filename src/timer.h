/*UNCRUSTIFY-OFF*/
/**
 * @file timer.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for timers
 * @version 0.4.0
 * @date 2022-01-31
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
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
/*UNCRUSTIFY-ON*/
#ifndef TIMER_H_
  #define TIMER_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "stream.h"
  #include "sys.h"
  #include "task.h"

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  Return_t xTimerCreate(Timer_t **timer_, const Ticks_t period_);
  Return_t xTimerDelete(const Timer_t *timer_);
  Return_t xTimerChangePeriod(Timer_t *timer_, const Ticks_t period_);
  Return_t xTimerGetPeriod(const Timer_t *timer_, Ticks_t *period_);
  Return_t xTimerIsTimerActive(const Timer_t *timer_, Base_t *res_);
  Return_t xTimerHasTimerExpired(const Timer_t *timer_, Base_t *res_);
  Return_t xTimerReset(Timer_t *timer_);
  Return_t xTimerStart(Timer_t *timer_);
  Return_t xTimerStop(Timer_t *timer_);


  #if defined(POSIX_ARCH_OTHER)
    void __TimerStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef TIMER_H_ */