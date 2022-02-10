/**
 * @file timer.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for timers and managing timers in HeliOS
 * @version 0.3.0
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
extern TaskList_t *taskList;

/* Declare and initialize the task list to null. */
TimerList_t *timerList = null;

/* The xTimerCreate() system call will create a new timer. Timers differ from
 task timers in that they do not create events that effect the scheduling of a task.
 Timers can be used by tasks to initiate various task activities based on a specified
 time period represented in microseconds. The memory allocated by xTimerCreate() must
 be freed by xTimerDelete(). Unlike tasks, timers may be created and deleted within
 tasks. */
Timer_t *xTimerCreate(Time_t timerPeriod_) {

  Timer_t *timer = null;

  Timer_t *timerCursor = null;

  /* Check if the timer period paramater is greater or equal to zero. */
  if (timerPeriod_ >= 0) {
    /* Check if the timer list is null, if it is create it. */
    if (ISNULLPTR(timerList)) {
      timerList = (TimerList_t *)xMemAlloc(sizeof(TimerList_t));

      /* Check if xMemAlloc() successfully allocated the memory for the timer list, if not
      enable interrupts and return null. */
      if (ISNULLPTR(timerList)) {

        return null;
      }
    }

    timer = (Timer_t *)xMemAlloc(sizeof(Task_t));

    /* Check if xMemAlloc() successfully allocated the memory for the timer. */
    if (ISNOTNULLPTR(timer)) {
      timer->state = TimerStateStopped;

      timer->timerPeriod = timerPeriod_;

      timer->timerStartTime = CURRENTTIME();

      timer->next = null;

      timerCursor = timerList->head;

      /* Check if the head of the timer list is null. If so, iterate through the
      timer list to find the end otherwise just append the timer to the timer list. */
      if (ISNOTNULLPTR(timerList->head)) {
        /* While the next timer is not null. */
        while (ISNOTNULLPTR(timerCursor->next)) {
          timerCursor = timerCursor->next;
        }

        timerCursor->next = timer;
      } else {
        timerList->head = timer;
      }

      timerList->length++;

      return timer;
    }
  }

  return null;
}

/* The xTimerDelete() system call will delete a timer. For more information on timers see the
xTaskTimerCreate() system call. */
void xTimerDelete(Timer_t *timer_) {

  Timer_t *timerCursor = null;

  Timer_t *timerPrevious = null;

  /* Check if the timer list is not null and the timer parameter is not null. */
  if (ISNOTNULLPTR(timerList) && ISNOTNULLPTR(timer_)) {
    timerCursor = timerList->head;

    timerPrevious = null;

    /* Check if the timer cursor is not null a if the timer cursor equals
    the timer parameter. */
    if (ISNOTNULLPTR(timerCursor) && timerCursor == timer_) {
      timerList->head = timerCursor->next;

      xMemFree(timerCursor);

      timerList->length--;

      timer_ = null;
    } else {
      /* While the timer cursor is not null and the timer cursor is not
      equal to the timer parameter, continue to scan the timer list. */
      while (ISNOTNULLPTR(timerCursor) && timerCursor != timer_) {
        timerPrevious = timerCursor;

        timerCursor = timerCursor->next;
      }

      /* If the timer cursor is null, then the timer could not be found so
      enable interrupts and return. */
      if (ISNULLPTR(timerCursor)) {

        return;
      }

      timerPrevious->next = timerCursor->next;

      xMemFree(timerCursor);

      timerList->length--;
    }
  }

  return;
}

/* The xTimerChangePeriod() system call will change the period of the specified timer.
The timer period is measured in microseconds. If the timer period is zero, the xTimerHasTimerExpired()
system call will always return false. */
void xTimerChangePeriod(Timer_t *timer_, Time_t timerPeriod_) {
  Timer_t *timerCursor = null;

  /* Check if the timer list is not null, the timer parameter is not null and the timer period
  is zero or greater. */
  if (ISNOTNULLPTR(timerList) && ISNOTNULLPTR(timer_) && timerPeriod_ >= 0) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while (ISNOTNULLPTR(timerCursor) && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }

    /* If the timer cursor is null, the timer could not be found in the timer list
    so just return. */
    if (ISNULLPTR(timerCursor)) {
      return;
    }

    timerCursor->timerPeriod = timerPeriod_;
  }

  return;
}

/* The xTimerGetPeriod() system call will return the current timer period
for the specified timer. */
Time_t xTimerGetPeriod(Timer_t *timer_) {
  Timer_t *timerCursor = null;

  /* Check if the timer list is not null and the timer parameter is not null. */
  if (ISNOTNULLPTR(timerList) && ISNOTNULLPTR(timer_)) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while (ISNOTNULLPTR(timerCursor) && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }

    /* If the timer cursor is null, the timer could not be found in the timer list
    so just return zero. */
    if (ISNULLPTR(timerCursor)) {
      return 0;
    }
    return timerCursor->timerPeriod;
  }
  return 0;
}

/* The xTimerIsTimerActive() system call will return true of the timer has been
started with xTimerStart(). */
Base_t xTimerIsTimerActive(Timer_t *timer_) {
  Timer_t *timerCursor = null;

  /* Check if the timer list is not null and the timer parameter is not null. */
  if (ISNOTNULLPTR(timerList) && ISNOTNULLPTR(timer_)) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while (ISNOTNULLPTR(timerCursor) && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }

    /* If the timer cursor is null, the timer could not be found in the timer list
    so just return. */
    if (ISNULLPTR(timerCursor)) {
      return false;
    }
    if (timerCursor->state == TimerStateRunning) {
      return true;
    }
  }

  return false;
}

/* The xTimerHasTimerExpired() system call will return true or false dependent on whether
the timer period for the specified timer has elapsed. xTimerHasTimerExpired() will NOT
reset the timer. Timers must be reset with xTimerReset(). */
Base_t xTimerHasTimerExpired(Timer_t *timer_) {
  Timer_t *timerCursor = null;

  /* Check if the timer list is not null and the timer parameter is not null. */
  if (ISNOTNULLPTR(timerList) && ISNOTNULLPTR(timer_)) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while (ISNOTNULLPTR(timerCursor) && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }

    /* If the timer cursor is null, the timer could not be found in the timer list
    so just return false. */
    if (ISNULLPTR(timerCursor)) {
      return false;
    }

    /* If the state is running, timer period is greater than zero and if the elapsed time
    is equal to or greater than the timer period, return true. */
    if (timerCursor->state == TimerStateRunning && timerCursor->timerPeriod > 0 && CURRENTTIME() - timerCursor->timerStartTime > timerCursor->timerPeriod) {
      return true;
    }
  }

  return false;
}

/* The xTimerReset() system call will reset the start time of the timer to zero. */
void xTimerReset(Timer_t *timer_) {
  Timer_t *timerCursor = null;

  /* Check if the timer list is not null and the timer parameter is not null. */
  if (ISNOTNULLPTR(timerList) && ISNOTNULLPTR(timer_)) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while (ISNOTNULLPTR(timerCursor) && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }

    /* If the timer cursor is null, the timer could not be found in the timer list
    so just return. */
    if (ISNULLPTR(timerCursor)) {
      return;
    }

    timerCursor->timerStartTime = CURRENTTIME();
  }

  return;
}

/* The xTimerStart() system call will place the timer in the running state. Neither xTaskStart() nor
xTaskStop() will reset the timer. Timers can only be reset with xTimerReset(). */
void xTimerStart(Timer_t *timer_) {
  Timer_t *timerCursor = null;

  /* Check if the timer list is not null and the timer parameter is not null. */
  if (ISNOTNULLPTR(timerList) && ISNOTNULLPTR(timer_)) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while (ISNOTNULLPTR(timerCursor) && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }

    /* If the timer cursor is null, the timer could not be found in the timer list
    so just return. */
    if (ISNULLPTR(timerCursor)) {
      return;
    }

    timerCursor->state = TimerStateRunning;
  }

  return;
}

/* The xTimerStop() system call will place the timer in the stopped state. Neither xTaskStart() nor
xTaskStop() will reset the timer. Timers can only be reset with xTimerReset(). */
void xTimerStop(Timer_t *timer_) {
  Timer_t *timerCursor = null;

  /* Check if the timer list is not null and the timer parameter is not null. */
  if (ISNOTNULLPTR(timerList) && ISNOTNULLPTR(timer_)) {
    timerCursor = timerList->head;

    /* While timer cursor is not null and the timer cursor does not equal the timer
    parameter, keep scanning the timer list. */
    while (ISNOTNULLPTR(timerCursor) && timerCursor != timer_) {
      timerCursor = timerCursor->next;
    }

    /* If the timer cursor is null, the timer could not be found in the timer list
    so just return. */
    if (ISNULLPTR(timerCursor)) {
      return;
    }

    timerCursor->state = TimerStateStopped;
  }

  return;
}