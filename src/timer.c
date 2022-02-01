/**
 * @file timer.c
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

#include "timer.h"

TimerList_t *timerList = null;

Timer_t *xTimerCreate(Time_t timerPeriod_) {
  DISABLE_INTERRUPTS();
  Timer_t *timer = null;
  Timer_t *timerCursor = null;
  if (IsNotCritBlocking() && timerPeriod_ >= 0) {
    if (!timerList) {
      timerList = (TimerList_t *)xMemAlloc(sizeof(TimerList_t));
      if (!timerList) {
        ENABLE_INTERRUPTS();
        return null;
      }
    }
    timer = (Timer_t *)xMemAlloc(sizeof(Task_t));
    if (timer) {
      timer->state = TimerStateStopped;
      timer->timerPeriod = timerPeriod_;
      timer->timerStartTime = CURRENTTIME();
      timer->next = null;
      timerCursor = timerList->head;
      if (timerList->head) {
        while (timerCursor->next) {
          timerCursor = timerCursor->next;
        }
        timerCursor->next = timer;
      } else {
        timerList->head = timer;
      }
      timerList->length++;
      ENABLE_INTERRUPTS();
      return timer;
    }
  }
  ENABLE_INTERRUPTS();
  return null;
}

void xTimerDelete(Timer_t *timer_) {
  DISABLE_INTERRUPTS();
  Timer_t *timerCursor = null;
  Timer_t *timerPrevious = null;
  if (IsNotCritBlocking() && timerList && timer_) {
    timerCursor = timerList->head;
    timerPrevious = null;
    if (timerCursor && timerCursor == timer_) {
      timerList->head = timerCursor->next;
      xMemFree(timerCursor);
      timerList->length--;
      timer_ = null;
    } else {
      while (timerCursor && timerCursor != timer_) {
        timerPrevious = timerCursor;
        timerCursor = timerCursor->next;
      }
      if (!timerCursor) {
        ENABLE_INTERRUPTS();
        return;
      }
      timerPrevious->next = timerCursor->next;
      xMemFree(timerCursor);
      timerList->length--;
    }
  }
  ENABLE_INTERRUPTS();
  return;
}

void xTimerChangePeriod(Timer_t *timer_, Time_t timerPeriod_) {
  Timer_t *timerCursor = null;
  if (timerList && timer_ && timerPeriod_ >= 0) {
    timerCursor = timerList->head;
    while (timerCursor && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }
    if (!timerCursor)
      return;
    timerCursor->timerPeriod = timerPeriod_;
  }
  return;
}

Time_t xTimerGetPeriod(Timer_t *timer_) {
  Timer_t *timerCursor = null;
  if (timerList && timer_) {
    timerCursor = timerList->head;
    while (timerCursor && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }
    if (!timerCursor)
      return 0;
    return timerCursor->timerPeriod;
  }
  return 0;
}

Base_t xTimerIsTimerActive(Timer_t *timer_) {
  Timer_t *timerCursor = null;
  if (timerList && timer_) {
    timerCursor = timerList->head;
    while (timerCursor && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }
    if (!timerCursor)
      return false;
    if (timerCursor->state == TimerStateRunning) {
      return true;
    }
  }
  return false;
}

Base_t xTimerHasTimerExpired(Timer_t *timer_) {
  Timer_t *timerCursor = null;
  if (timerList && timer_) {
    timerCursor = timerList->head;
    while (timerCursor && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }
    if (!timerCursor)
      return false;
    if (timerCursor->state == TimerStateRunning && CURRENTTIME() - timerCursor->timerStartTime > timerCursor->timerPeriod) {
      return true;
    }
  }
  return false;
}

void xTimerReset(Timer_t *timer_) {
  Timer_t *timerCursor = null;
  if (timerList && timer_) {
    timerCursor = timerList->head;
    while (timerCursor && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }
    if (!timerCursor)
      return;
    timerCursor->timerStartTime = CURRENTTIME();
  }
  return;
}

void xTimerStart(Timer_t *timer_) {
  Timer_t *timerCursor = null;
  if (timerList && timer_) {
    timerCursor = timerList->head;
    while (timerCursor && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }
    if (!timerCursor)
      return;
    timerCursor->state = TimerStateRunning;
  }
  return;
}

void xTimerStop(Timer_t *timer_) {
  Timer_t *timerCursor = null;
  if (timerList && timer_) {
    timerCursor = timerList->head;
    while (timerCursor && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }
    if (!timerCursor)
      return;
    timerCursor->state = TimerStateStopped;
  }
  return;
}