/*UNCRUSTIFY-OFF*/
/**
 * @file timer.c
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
#include "timer.h"


/* Declare and initialize the task list to null. */
static TimerList_t *timerList = null;
static Base_t __TimerListFindTimer__(const Timer_t *timer_);


/* The xTimerCreate() system call will create a new timer. Timers differ from
 * task timers in that they do not create events that effect the scheduling of a
 * task. Timers can be used by tasks to initiate various task activities based
 * on a specified time period represented in ticks. The memory allocated by
 * xTimerCreate() must be freed by xTimerDelete(). Unlike tasks, timers may be
 * created and deleted within tasks. */
Timer_t *xTimerCreate(const Ticks_t timerPeriod_) {
  Timer_t *ret = null;
  Timer_t *cursor = null;


  /* Check if xMemAlloc() did its job. */
  if(ISNOTNULLPTR(timerList) || (ISNULLPTR(timerList) && ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &timerList, sizeof(TimerList_t))))) {
    if(ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &ret, sizeof(Task_t)))) {
      /* Check if xMemAlloc() successfully allocated the memory for the timer.
       */
      if(ISNOTNULLPTR(ret)) {
        ret->state = TimerStateSuspended;
        ret->timerPeriod = timerPeriod_;
        __PortGetSysTicks__(&ret->timerStartTime);
        ret->next = null;
        cursor = timerList->head;


        /* Check if the head of the timer list is null. If so, iterate through
         * the timer list to find the end otherwise just append the timer to the
         * timer list. */
        if(ISNOTNULLPTR(timerList->head)) {
          /* While the next timer is not null. */
          while(ISNOTNULLPTR(cursor->next)) {
            cursor = cursor->next;
          }

          cursor->next = ret;
        } else {
          timerList->head = ret;
        }

        timerList->length++;
      }
    }
  }

  return(ret);
}


/* The xTimerDelete() system call will delete a timer. For more information on
 * timers see the xTaskTimerCreate() system call. */
void xTimerDelete(const Timer_t *timer_) {
  Timer_t *cursor = null;
  Timer_t *timerPrevious = null;


  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == __TimerListFindTimer__(timer_));


  /* Check if the timer was found in the timer list. */
  if(RETURN_SUCCESS == __TimerListFindTimer__(timer_)) {
    cursor = timerList->head;
    timerPrevious = null;


    /* Check if the timer cursor is not null a if the timer cursor equals the
     * timer parameter. */
    if((ISNOTNULLPTR(cursor)) && (cursor == timer_)) {
      timerList->head = cursor->next;
      __KernelFreeMemory__(cursor);
      timerList->length--;
    } else {
      /* While the timer cursor is not null and the timer cursor is not equal to
       * the timer parameter, continue to scan the timer list. */
      while((ISNOTNULLPTR(cursor)) && (cursor != timer_)) {
        timerPrevious = cursor;
        cursor = cursor->next;
      }


      /* Assert if the timer is not found though this shouldn't ever happen. */
      SYSASSERT(ISNOTNULLPTR(cursor));


      /* Check if the timer was found, if so drop it from the timer list and
       * free its memory. */
      if(ISNOTNULLPTR(cursor)) {
        timerPrevious->next = cursor->next;
        __KernelFreeMemory__(cursor);
        timerList->length--;
      }
    }
  }

  return;
}


/* The xTimerChangePeriod() system call will change the period of the specified
 * timer. The timer period is measured in ticks. If the timer period is zero,
 * the xTimerHasTimerExpired() system call will always return false. */
void xTimerChangePeriod(Timer_t *timer_, const Ticks_t timerPeriod_) {
  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == __TimerListFindTimer__(timer_));


  /* Check if the timer was found in the timer list. */
  if(RETURN_SUCCESS == __TimerListFindTimer__(timer_)) {
    timer_->timerPeriod = timerPeriod_;
  }

  return;
}


/* The xTimerGetPeriod() system call will return the current timer period for
 * the specified timer. */
Ticks_t xTimerGetPeriod(const Timer_t *timer_) {
  Ticks_t ret = zero;


  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == __TimerListFindTimer__(timer_));


  /* Check if the timer was found in the timer list. */
  if(RETURN_SUCCESS == __TimerListFindTimer__(timer_)) {
    ret = timer_->timerPeriod;
  }

  return(ret);
}


/* The xTimerIsTimerActive() system call will return true of the timer has been
 * started with xTimerStart(). */
Base_t xTimerIsTimerActive(const Timer_t *timer_) {
  Base_t ret = false;


  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == __TimerListFindTimer__(timer_));


  /* Check if the timer was found in the timer list. */
  if(RETURN_SUCCESS == __TimerListFindTimer__(timer_)) {
    /* Check if the timer state is running, if so return true. */
    if(TimerStateRunning == timer_->state) {
      ret = true;
    }
  }

  return(ret);
}


/* The xTimerHasTimerExpired() system call will return true or false dependent
 * on whether the timer period for the specified timer has elapsed.
 * xTimerHasTimerExpired() will NOT reset the timer. Timers must be reset with
 * xTimerReset(). */
Base_t xTimerHasTimerExpired(const Timer_t *timer_) {
  Base_t ret = false;
  Ticks_t currSysTicks = zero;


  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == __TimerListFindTimer__(timer_));


  /* Check if the timer was found in the timer list. */
  if(RETURN_SUCCESS == __TimerListFindTimer__(timer_)) {
    /* Assert if the timer isn't running, it must be for the timer to expire. */
    SYSASSERT(TimerStateRunning == timer_->state);


    /* Assert if the timer period is zero, it must be greater than zero for the
     * timer to expire. */
    SYSASSERT(zero < timer_->timerPeriod);
    __PortGetSysTicks__(&currSysTicks);


    /* The timer should be running, the timer period should be greater than zero
     * and the elapsed time is greater than the timer period. If so, then return
     * true. */
    if((TimerStateRunning == timer_->state) && (zero < timer_->timerPeriod) && ((currSysTicks - timer_->timerStartTime) > timer_->timerPeriod)) {
      ret = true;
    }
  }

  return(ret);
}


/* The xTimerReset() system call will reset the start time of the timer to zero.
 */
void xTimerReset(Timer_t *timer_) {
  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == __TimerListFindTimer__(timer_));


  /* Check if the timer was found in the timer list. */
  if(RETURN_SUCCESS == __TimerListFindTimer__(timer_)) {
    __PortGetSysTicks__(&timer_->timerStartTime);
  }

  return;
}


/* The xTimerStart() system call will place the timer in the running state.
 * Neither xTaskStart() nor xTaskStop() will reset the timer. Timers can only be
 * reset with xTimerReset(). */
void xTimerStart(Timer_t *timer_) {
  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == __TimerListFindTimer__(timer_));


  /* Check if the timer was found in the timer list. */
  if(RETURN_SUCCESS == __TimerListFindTimer__(timer_)) {
    timer_->state = TimerStateRunning;
  }

  return;
}


/* The xTimerStop() system call will place the timer in the stopped state.
 * Neither xTaskStart() nor xTaskStop() will reset the timer. Timers can only be
 * reset with xTimerReset(). */
void xTimerStop(Timer_t *timer_) {
  /* Assert if the timer cannot be found in the timer list. */
  SYSASSERT(RETURN_SUCCESS == __TimerListFindTimer__(timer_));


  /* Check if the timer was found in the timer list. */
  if(RETURN_SUCCESS == __TimerListFindTimer__(timer_)) {
    timer_->state = TimerStateSuspended;
  }

  return;
}


/* __TimerListFindTimer__() is used to search the timer list for a timer and
 * returns RETURN_SUCCESS if the timer is found. It also always checks the
 * health of the heap by calling __MemoryRegionCheckKernel__(). */
static Base_t __TimerListFindTimer__(const Timer_t *timer_) {
  Base_t ret = RETURN_FAILURE;
  Timer_t *cursor = null;


  /* Assert if the timer list is not initialized. */
  SYSASSERT(ISNOTNULLPTR(timerList));


  /* Assert if the timer paramater is null. */
  SYSASSERT(ISNOTNULLPTR(timer_));


  /* Check if the timer list is initialized and the timer pointer is not null.
   */
  if((ISNOTNULLPTR(timerList)) && (ISNOTNULLPTR(timer_))) {
    /* Assert if the __MemoryRegionCheckKernel__() fails on the health check or
     * is unable to find the entry for the heap pointer. */
    SYSASSERT(ISSUCCESSFUL(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR)));


    /* Check if __MemoryRegionCheckKernel__() was successful. */
    if(ISSUCCESSFUL(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      cursor = timerList->head;


      /* Traverse the timer list while there is a timer and the timer is not the
       * timer we are looking for. */
      while((ISNOTNULLPTR(cursor)) && (cursor != timer_)) {
        cursor = cursor->next;
      }


      /* Assert if the timer was never found. */
      SYSASSERT(ISNOTNULLPTR(cursor));


      /* Check if the timer was found. */
      if(ISNOTNULLPTR(cursor)) {
        ret = RETURN_SUCCESS;
      }
    }
  }

  return(ret);
}


#if defined(POSIX_ARCH_OTHER)


  void __TimerStateClear__(void) {
    timerList = null;

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */