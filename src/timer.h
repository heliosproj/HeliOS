/*UNCRUSTIFY-OFF*/
/**
 * @file timer.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for application timers
 * @version 0.4.1
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
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


  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef TIMER_H_ */