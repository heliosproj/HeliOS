/*UNCRUSTIFY-OFF*/
/**
 * @file timer_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
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
#include "timer_harness.h"


void timer_harness(void) {
  Timer_t *timer01 = null;
  Ticks_t timer02 = zero;
  Ticks_t timer03 = zero;
  Base_t timer04 = zero;
  Base_t timer05 = zero;
  Base_t timer06 = zero;
  Base_t timer07 = zero;
  Base_t timer08 = zero;
  Base_t timer09 = zero;
  Ticks_t timer10 = zero;


  unit_begin("xTimerCreate()");
  timer01 = null;
  unit_try(OK(xTimerCreate(&timer01, 0x3E8)));
  unit_try(null != timer01);
  unit_end();
  unit_begin("xTimerGetPeriod()");
  unit_try(OK(xTimerGetPeriod(timer01, &timer02)));
  unit_try(0x3E8 == timer02);
  unit_end();
  unit_begin("xTimerChangePeriod()");
  unit_try(OK(xTimerChangePeriod(timer01, 0x7D0)));
  unit_try(OK(xTimerGetPeriod(timer01, &timer03)));
  unit_try(0x7D0 == timer03);
  unit_end();
  unit_begin("xTimerIsTimerActive()");
  unit_try(OK(xTimerIsTimerActive(timer01, &timer04)));
  unit_try(false == timer04);
  unit_end();
  unit_begin("xTimerHasTimerExpired()");
  unit_try(!OK(xTimerHasTimerExpired(timer01, &timer05)));
  unit_try(false == timer05);
  unit_try(OK(xTimerStart(timer01)));
  sleep(3);
  unit_try(OK(xTimerHasTimerExpired(timer01, &timer06)));
  unit_try(true == timer06);
  unit_end();
  unit_begin("xTimerStop()");
  unit_try(OK(xTimerStop(timer01)));
  unit_try(OK(xTimerIsTimerActive(timer01, &timer07)));
  unit_try(false == timer07);
  unit_end();
  unit_begin("xTimerStart()");
  unit_try(OK(xTimerStart(timer01)));
  unit_try(OK(xTimerIsTimerActive(timer01, &timer08)));
  unit_try(true == timer08);
  unit_end();
  unit_begin("xTimerReset()");
  sleep(3);
  unit_try(OK(xTimerHasTimerExpired(timer01, &timer09)));
  unit_try(true == timer09);
  unit_try(OK(xTimerReset(timer01)));
  unit_try(OK(xTimerHasTimerExpired(timer01, &timer09)));
  unit_try(false == timer09);
  unit_end();
  unit_begin("xTimerDelete()");
  unit_try(OK(xTimerDelete(timer01)));
  unit_try(!OK(xTimerGetPeriod(timer01, &timer10)));
  unit_end();

  return;
}