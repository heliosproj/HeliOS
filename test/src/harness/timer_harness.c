/*UNCRUSTIFY-OFF*/
/**
 * @file timer_harness.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.4.0
 * @date 2022-08-27
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
#include "timer_harness.h"


void timer_harness(void) {
  Timer_t *timer01 = null;


  unit_begin("xTimerCreate()");
  timer01 = null;
  timer01 = xTimerCreate(0x3E8);
  unit_try(null != timer01);
  unit_end();
  unit_begin("xTimerGetPeriod()");
  unit_try(0x3E8 == xTimerGetPeriod(timer01));
  unit_end();
  unit_begin("xTimerChangePeriod()");
  xTimerChangePeriod(timer01, 0x7D0);
  unit_try(0x7D0 == xTimerGetPeriod(timer01));
  unit_end();
  unit_begin("xTimerIsTimerActive()");
  unit_try(false == xTimerIsTimerActive(timer01));
  unit_end();
  unit_begin("xTimerHasTimerExpired()");
  unit_try(false == xTimerHasTimerExpired(timer01));
  xTimerStart(timer01);
  sleep(3);
  unit_try(true == xTimerHasTimerExpired(timer01));
  unit_end();
  unit_begin("xTimerStop()");
  xTimerStop(timer01);
  unit_try(false == xTimerIsTimerActive(timer01));
  unit_end();
  unit_begin("xTimerStart()");
  xTimerStart(timer01);
  unit_try(true == xTimerIsTimerActive(timer01));
  unit_end();
  unit_begin("xTimerReset()");
  unit_try(true == xTimerHasTimerExpired(timer01));
  xTimerReset(timer01);
  unit_try(false == xTimerHasTimerExpired(timer01));
  unit_end();
  unit_begin("xTimerDelete()");
  xTimerDelete(timer01);
  unit_try(0x0u == xTimerGetPeriod(timer01));
  unit_end();

  return;
}