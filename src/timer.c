/**
 * @file timer.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for timers
 * @version 0.3.1
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

#include "timer.h"

extern SysFlags_t sysFlags;

/* Declare and initialize the task list to null. */
TimerList_t *timerList = NULL;

/* The xTimerCreate() system call will create a new timer. Timers differ from
 task timers in that they do not create events that effect the scheduling of a task.
 Timers can be used by tasks to initiate various task activities based on a specified
 time period represented in microseconds. The memory allocated by xTimerCreate() must
 be freed by xTimerDelete(). Unlike tasks, timers may be created and deleted within
 tasks. */
Timer_t *xTimerCreate(Time_t timerPeriod_) {
  Timer_t *ret = NULL;

  Timer_t *timerCursor = NULL;

  /* Check if the timer list is null, if it is create it. */
  if (ISNULLPTR(timerList)) {
  
    ENTER_PRIVILEGED();

    timerList = (TimerList_t *)xMemAlloc(sizeof(TimerList_t));
  }

  SYSASSERT(ISNOTNULLPTR(timerList));

  if (ISNOTNULLPTR(timerList)) {

    ENTER_PRIVILEGED();

    ret = (Timer_t *)xMemAlloc(sizeof(Task_t));

    SYSASSERT(ISNOTNULLPTR(ret));

    /* Check if xMemAlloc() successfully allocated the memory for the timer. */
    if (ISNOTNULLPTR(ret)) {
      ret->state = TimerStateSuspended;

      ret->timerPeriod = timerPeriod_;

      ret->timerStartTime = CURRENTTIME();

      ret->next = NULL;

      timerCursor = timerList->head;

      /* Check if the head of the timer list is null. If so, iterate through the
      timer list to find the end otherwise just append the timer to the timer list. */
      if (ISNOTNULLPTR(timerList->head)) {
        /* While the next timer is not null. */
        while (ISNOTNULLPTR(timerCursor->next)) {
          timerCursor = timerCursor->next;
        }

        timerCursor->next = ret;
      } else {
        timerList->head = ret;
      }

      timerList->length++;
    }
  }

  return ret;
}

/* The xTimerDelete() system call will delete a timer. For more information on timers see the
xTaskTimerCreate() system call. */
void xTimerDelete(Timer_t *timer_) {
  Timer_t *timerCursor = NULL;

  Timer_t *timerPrevious = NULL;

  SYSASSERT(ISNOTNULLPTR(timerList));

  SYSASSERT(ISNOTNULLPTR(timer_));

  /* Check if the timer list is not null and the timer parameter is not null. */
  if ((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {
    timerCursor = timerList->head;

    timerPrevious = NULL;

    /* Check if the timer cursor is not null a if the timer cursor equals
    the timer parameter. */
    if ((ISNOTNULLPTR(timerCursor)) && (timerCursor == timer_)) {
      timerList->head = timerCursor->next;

      ENTER_PRIVILEGED();

      xMemFree(timerCursor);

      timerList->length--;

    } else {
      /* While the timer cursor is not null and the timer cursor is not
      equal to the timer parameter, continue to scan the timer list. */
      while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {
        timerPrevious = timerCursor;

        timerCursor = timerCursor->next;
      }

      SYSASSERT(ISNOTNULLPTR(timerCursor));

      /* If the timer cursor is not null, then remove the timer
      from the list and free its memory. */
      if (ISNOTNULLPTR(timerCursor)) {
        timerPrevious->next = timerCursor->next;

        ENTER_PRIVILEGED();

        xMemFree(timerCursor);

        timerList->length--;
      }
    }
  }

  return;
}

/* The xTimerChangePeriod() system call will change the period of the specified timer.
The timer period is measured in microseconds. If the timer period is zero, the xTimerHasTimerExpired()
system call will always return false. */
void xTimerChangePeriod(Timer_t *timer_, Time_t timerPeriod_) {
  Timer_t *timerCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(timerList));

  SYSASSERT(ISNOTNULLPTR(timer_));

  /* Check if the timer list is not null, the timer parameter is not null and the timer period
  is zero or greater. */
  if ((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {
      timerCursor = timerCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(timerCursor));

    /* If the timer cursor is not null, then set the timer's period. */
    if (ISNOTNULLPTR(timerCursor)) {
      timerCursor->timerPeriod = timerPeriod_;
    }
  }

  return;
}

/* The xTimerGetPeriod() system call will return the current timer period
for the specified timer. */
Time_t xTimerGetPeriod(Timer_t *timer_) {
  Time_t ret = zero;

  Timer_t *timerCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(timerList));

  SYSASSERT(ISNOTNULLPTR(timer_));

  /* Check if the timer list is not null and the timer parameter is not null. */
  if ((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {
      timerCursor = timerCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(timerCursor));

    /* If the timer cursor is not null, then set the return value to the timer's
    period. */
    if (ISNOTNULLPTR(timerCursor)) {
      ret = timerCursor->timerPeriod;
    }
  }
  return ret;
}

/* The xTimerIsTimerActive() system call will return true of the timer has been
started with xTimerStart(). */
Base_t xTimerIsTimerActive(Timer_t *timer_) {
  Base_t ret = false;
  Timer_t *timerCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(timerList));

  SYSASSERT(ISNOTNULLPTR(timer_));

  /* Check if the timer list is not null and the timer parameter is not null. */
  if ((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {
      timerCursor = timerCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(timerCursor));

    /* If the timer cursor is not null, then check if the timer is running. */
    if (ISNOTNULLPTR(timerCursor)) {
      if (timerCursor->state == TimerStateRunning) {
        ret = true;
      }
    }
  }

  return ret;
}

/* The xTimerHasTimerExpired() system call will return true or false dependent on whether
the timer period for the specified timer has elapsed. xTimerHasTimerExpired() will NOT
reset the timer. Timers must be reset with xTimerReset(). */
Base_t xTimerHasTimerExpired(Timer_t *timer_) {
  Base_t ret = false;

  Timer_t *timerCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(timerList));

  SYSASSERT(ISNOTNULLPTR(timer_));

  /* Check if the timer list is not null and the timer parameter is not null. */
  if ((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {
      timerCursor = timerCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(timerCursor));

    /* If the timer cursor is not null, check to see if the timer has expired. */
    if (ISNOTNULLPTR(timerCursor)) {
      /* If the state is running, timer period is greater than zero and if the elapsed time
      is equal to or greater than the timer period, return true. */
      if ((timerCursor->state == TimerStateRunning) && (timerCursor->timerPeriod > zero) && ((CURRENTTIME() - timerCursor->timerStartTime) > timerCursor->timerPeriod)) {
        ret = true;
      }
    }
  }

  return ret;
}

/* The xTimerReset() system call will reset the start time of the timer to zero. */
void xTimerReset(Timer_t *timer_) {
  Timer_t *timerCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(timerList));

  SYSASSERT(ISNOTNULLPTR(timer_));

  /* Check if the timer list is not null and the timer parameter is not null. */
  if ((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {
      timerCursor = timerCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(timerCursor));

    /* If the timer cursor is not null, then set the timer to the current time. */
    if (ISNOTNULLPTR(timerCursor)) {
      timerCursor->timerStartTime = CURRENTTIME();
    }
  }

  return;
}

/* The xTimerStart() system call will place the timer in the running state. Neither xTaskStart() nor
xTaskStop() will reset the timer. Timers can only be reset with xTimerReset(). */
void xTimerStart(Timer_t *timer_) {
  Timer_t *timerCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(timerList));

  SYSASSERT(ISNOTNULLPTR(timer_));

  /* Check if the timer list is not null and the timer parameter is not null. */
  if ((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {
      timerCursor = timerCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(timerCursor));

    /* If the timer cursor is not null, then set the state of the timer to running. */
    if (ISNOTNULLPTR(timerCursor)) {
      timerCursor->state = TimerStateRunning;
    }
  }

  return;
}

/* The xTimerStop() system call will place the timer in the stopped state. Neither xTaskStart() nor
xTaskStop() will reset the timer. Timers can only be reset with xTimerReset(). */
void xTimerStop(Timer_t *timer_) {
  Timer_t *timerCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(timerList));

  SYSASSERT(ISNOTNULLPTR(timer_));

  /* Check if the timer list is not null and the timer parameter is not null. */
  if ((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {
      timerCursor = timerCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(timerCursor));

    /* If the timer cursor is not null, set the state of the timer to stopped. */
    if (ISNOTNULLPTR(timerCursor)) {
      timerCursor->state = TimerStateSuspended;
    }
  }

  return;
}