/*UNCRUSTIFY-OFF*/
/**
 * @file timer_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.5.0
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
  Ticks_t timer02 = nil;
  Ticks_t timer03 = nil;
  Base_t timer04 = nil;
  Base_t timer05 = nil;
  Base_t timer06 = nil;
  Base_t timer07 = nil;
  Base_t timer08 = nil;
  Base_t timer09 = nil;
  Ticks_t timer10 = nil;


  unit_begin("Timer creation with specified period succeeds");
  timer01 = null;
  unit_assert_ok(xTimerCreate(&timer01, 0x3E8));
  unit_assert_not_null(timer01);
  unit_end();
  unit_begin("Timer period retrieval returns configured value");
  unit_assert_ok(xTimerGetPeriod(timer01, &timer02));
  unit_assert_equal(timer02, 0x3E8);
  unit_end();
  unit_begin("Timer period change updates period value");
  unit_assert_ok(xTimerChangePeriod(timer01, 0x7D0));
  unit_assert_ok(xTimerGetPeriod(timer01, &timer03));
  unit_assert_equal(timer03, 0x7D0);
  unit_end();
  unit_begin("Timer active check returns false for inactive timer");
  unit_assert_ok(xTimerIsTimerActive(timer01, &timer04));
  unit_assert_false(timer04);
  unit_end();
  unit_begin("Timer expiration check detects timeout after period elapses");
  unit_assert_not_ok(xTimerHasTimerExpired(timer01, &timer05));
  unit_assert_false(timer05);
  unit_assert_ok(xTimerStart(timer01));
  sleep(3);
  unit_assert_ok(xTimerHasTimerExpired(timer01, &timer06));
  unit_assert_true(timer06);
  unit_end();
  unit_begin("Timer stop deactivates running timer");
  unit_assert_ok(xTimerStop(timer01));
  unit_assert_ok(xTimerIsTimerActive(timer01, &timer07));
  unit_assert_false(timer07);
  unit_end();
  unit_begin("Timer start activates timer");
  unit_assert_ok(xTimerStart(timer01));
  unit_assert_ok(xTimerIsTimerActive(timer01, &timer08));
  unit_assert_true(timer08);
  unit_end();
  unit_begin("Timer reset clears expiration status");
  sleep(3);
  unit_assert_ok(xTimerHasTimerExpired(timer01, &timer09));
  unit_assert_true(timer09);
  unit_assert_ok(xTimerReset(timer01));
  unit_assert_ok(xTimerHasTimerExpired(timer01, &timer09));
  unit_assert_false(timer09);
  unit_end();
  unit_begin("Timer delete invalidates timer handle");
  unit_assert_ok(xTimerDelete(timer01));
  unit_assert_not_ok(xTimerGetPeriod(timer01, &timer10));
  unit_end();

  return;
}