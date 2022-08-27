/**
 * @file timer.c
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

#include "timer.h"




/* Declare and initialize the task list to null. */
static TimerList_t *timerList = NULL;




/* The xTimerCreate() system call will create a new timer. Timers differ from
 task timers in that they do not create events that effect the scheduling of a task.
 Timers can be used by tasks to initiate various task activities based on a specified
 time period represented in ticks. The memory allocated by xTimerCreate() must
 be freed by xTimerDelete(). Unlike tasks, timers may be created and deleted within
 tasks. */
Timer_t *xTimerCreate(Ticks_t timerPeriod_) {


  Timer_t *ret = NULL;

  Timer_t *timerCursor = NULL;


  /* Check if the timer list has been initialized. */
  if (ISNULLPTR(timerList)) {


    timerList = (TimerList_t *)_KernelAllocateMemory_(sizeof(TimerList_t));
  }


  /* Assert if xMemAlloc() didn't do its job. */
  SYSASSERT(ISNOTNULLPTR(timerList));


  /* Check if xMemAlloc() did its job. */
  if (ISNOTNULLPTR(timerList)) {




    ret = (Timer_t *)_KernelAllocateMemory_(sizeof(Task_t));


    /* Assert if xMemAlloc() didn't do its job. */
    SYSASSERT(ISNOTNULLPTR(ret));


    /* Check if xMemAlloc() successfully allocated the memory for the timer. */
    if (ISNOTNULLPTR(ret)) {


      ret->state = TimerStateSuspended;

      ret->timerPeriod = timerPeriod_;

      ret->timerStartTime = _SysGetSysTicks_();

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


  /* Assert if the timer cannot be found in the timer
  list. */
  SYSASSERT(RETURN_SUCCESS == _TimerListFindTimer_(timer_));


  /* Check if the timer was found in the timer list. */
  if (RETURN_SUCCESS == _TimerListFindTimer_(timer_)) {



    timerCursor = timerList->head;

    timerPrevious = NULL;

    /* Check if the timer cursor is not null a if the timer cursor equals
    the timer parameter. */
    if ((ISNOTNULLPTR(timerCursor)) && (timerCursor == timer_)) {


      timerList->head = timerCursor->next;




      _KernelFreeMemory_(timerCursor);

      timerList->length--;

    } else {


      /* While the timer cursor is not null and the timer cursor is not
      equal to the timer parameter, continue to scan the timer list. */
      while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {


        timerPrevious = timerCursor;

        timerCursor = timerCursor->next;
      }


      /* Assert if the timer is not found though this
      shouldn't ever happen. */
      SYSASSERT(ISNOTNULLPTR(timerCursor));


      /* Check if the timer was found, if so drop it from
      the timer list and free its memory. */
      if (ISNOTNULLPTR(timerCursor)) {


        timerPrevious->next = timerCursor->next;




        _KernelFreeMemory_(timerCursor);

        timerList->length--;
      }
    }
  }


  return;
}




/* The xTimerChangePeriod() system call will change the period of the specified timer.
The timer period is measured in ticks. If the timer period is zero, the xTimerHasTimerExpired()
system call will always return false. */
void xTimerChangePeriod(Timer_t *timer_, Ticks_t timerPeriod_) {



  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == _TimerListFindTimer_(timer_));


  /* Check if the timer was found in the timer list. */
  if (RETURN_SUCCESS == _TimerListFindTimer_(timer_)) {

    timer_->timerPeriod = timerPeriod_;
  }


  return;
}




/* The xTimerGetPeriod() system call will return the current timer period
for the specified timer. */
Ticks_t xTimerGetPeriod(Timer_t *timer_) {

  Ticks_t ret = zero;


  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == _TimerListFindTimer_(timer_));

  /* Check if the timer was found in the timer list. */
  if (RETURN_SUCCESS == _TimerListFindTimer_(timer_)) {

    ret = timer_->timerPeriod;
  }


  return ret;
}




/* The xTimerIsTimerActive() system call will return true of the timer has been
started with xTimerStart(). */
Base_t xTimerIsTimerActive(Timer_t *timer_) {


  Base_t ret = false;


  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == _TimerListFindTimer_(timer_));

  /* Check if the timer was found in the timer list. */
  if (RETURN_SUCCESS == _TimerListFindTimer_(timer_)) {

    /* Check if the timer state is running, if so
    return true. */
    if (TimerStateRunning == timer_->state) {

      ret = true;
    }
  }


  return ret;
}




/* The xTimerHasTimerExpired() system call will return true or false dependent on whether
the timer period for the specified timer has elapsed. xTimerHasTimerExpired() will NOT
reset the timer. Timers must be reset with xTimerReset(). */
Base_t xTimerHasTimerExpired(Timer_t *timer_) {


  Base_t ret = false;


  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == _TimerListFindTimer_(timer_));


  /* Check if the timer was found in the timer list. */
  if (RETURN_SUCCESS == _TimerListFindTimer_(timer_)) {


    /* Assert if the timer isn't running, it must be for the
    timer to expire. */
    SYSASSERT(TimerStateRunning == timer_->state);



    /* Assert if the timer period is zero, it must be greater
    than zero for the timer to expire. */
    SYSASSERT(zero < timer_->timerPeriod);


    /* The timer should be running, the timer period should be
    greater than zero and the elapsed time is greater than
    the timer period. If so, then return true. */
    if ((TimerStateRunning == timer_->state) && (zero < timer_->timerPeriod) && ((_SysGetSysTicks_() - timer_->timerStartTime) > timer_->timerPeriod)) {

      ret = true;
    }
  }


  return ret;
}




/* The xTimerReset() system call will reset the start time of the timer to zero. */
void xTimerReset(Timer_t *timer_) {



  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == _TimerListFindTimer_(timer_));



  /* Check if the timer was found in the timer list. */
  if (RETURN_SUCCESS == _TimerListFindTimer_(timer_)) {

    timer_->timerStartTime = _SysGetSysTicks_();
  }

  return;
}




/* The xTimerStart() system call will place the timer in the running state. Neither xTaskStart() nor
xTaskStop() will reset the timer. Timers can only be reset with xTimerReset(). */
void xTimerStart(Timer_t *timer_) {



  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == _TimerListFindTimer_(timer_));



  /* Check if the timer was found in the timer list. */
  if (RETURN_SUCCESS == _TimerListFindTimer_(timer_)) {

    timer_->state = TimerStateRunning;
  }


  return;
}




/* The xTimerStop() system call will place the timer in the stopped state. Neither xTaskStart() nor
xTaskStop() will reset the timer. Timers can only be reset with xTimerReset(). */
void xTimerStop(Timer_t *timer_) {



  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == _TimerListFindTimer_(timer_));


  /* Check if the timer was found in the timer list. */
  if (RETURN_SUCCESS == _TimerListFindTimer_(timer_)) {

    timer_->state = TimerStateSuspended;
  }


  return;
}




/* _TimerListFindTimer_() is used to search the timer list for a
timer and returns RETURN_SUCCESS if the timer is found. It also
always checks the health of the heap by calling _MemoryRegionCheckKernel_(). */
Base_t _TimerListFindTimer_(const Timer_t *timer_) {


  Base_t ret = RETURN_FAILURE;


  Timer_t *timerCursor = NULL;


  /* Assert if the timer list is not initialized. */
  SYSASSERT(ISNOTNULLPTR(timerList));


  /* Assert if the timer paramater is null. */
  SYSASSERT(ISNOTNULLPTR(timer_));


  /* Check if the timer list is initialized and the timer pointer
  is not null. */
  if ((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {


    /* Assert if the _MemoryRegionCheckKernel_() fails on the health check or is unable
    to find the entry for the heap pointer. */
    SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR));


    /* Check if _MemoryRegionCheckKernel_() was successful. */
    if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {

      timerCursor = timerList->head;


      /* Traverse the timer list while there is a timer
      and the timer is not the timer we are looking for. */
      while ((ISNOTNULLPTR(timerCursor)) && (timerCursor != timer_)) {

        timerCursor = timerCursor->next;
      }


      /* Assert if the timer was never found. */
      SYSASSERT(ISNOTNULLPTR(timerCursor));

      /* Check if the timer was found. */
      if (ISNOTNULLPTR(timerCursor)) {

        ret = RETURN_SUCCESS;
      }
    }
  }

  return ret;
}



#if defined(POSIX_ARCH_OTHER)
void __TimerStateClear__(void) {

  timerList = NULL;

  return;
}
#endif