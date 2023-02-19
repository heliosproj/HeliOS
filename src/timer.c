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


static TimerList_t *timerList = null;
static Return_t __TimerListFindTimer__(const Timer_t *timer_);


Return_t xTimerCreate(Timer_t **timer_, const Ticks_t period_) {
  RET_DEFINE;


  Timer_t *cursor = null;


  if((ISNOTNULLPTR(timer_) && ISNOTNULLPTR(timerList)) || (ISNOTNULLPTR(timer_) && (ISNULLPTR(timerList) && ISOK(__KernelAllocateMemory__((volatile
    Addr_t **) &timerList, sizeof(TimerList_t)))))) {
    if(ISOK(__KernelAllocateMemory__((volatile Addr_t **) timer_, sizeof(Task_t)))) {
      if(ISNOTNULLPTR(*timer_)) {
        (*timer_)->state = TimerStateSuspended;
        (*timer_)->timerPeriod = period_;
        (*timer_)->timerStartTime = __PortGetSysTicks__();
        (*timer_)->next = null;
        cursor = timerList->head;

        if(ISNOTNULLPTR(timerList->head)) {
          while(ISNOTNULLPTR(cursor->next)) {
            cursor = cursor->next;
          }

          cursor->next = *timer_;
        } else {
          timerList->head = *timer_;
        }

        timerList->length++;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTimerDelete(const Timer_t *timer_) {
  RET_DEFINE;


  Timer_t *cursor = null;
  Timer_t *previous = null;


  if(ISNOTNULLPTR(timer_) && ISNOTNULLPTR(timerList)) {
    if(ISOK(__TimerListFindTimer__(timer_))) {
      cursor = timerList->head;

      if((ISNOTNULLPTR(cursor)) && (cursor == timer_)) {
        timerList->head = cursor->next;

        if(ISOK(__KernelFreeMemory__(cursor))) {
          timerList->length--;
          RET_OK;
        } else {
          ASSERT;
        }
      } else if((ISNOTNULLPTR(cursor)) && (cursor != timer_)) {
        while((ISNOTNULLPTR(cursor)) && (cursor != timer_)) {
          previous = cursor;
          cursor = cursor->next;
        }

        if(ISNOTNULLPTR(cursor)) {
          previous->next = cursor->next;

          if(ISOK(__KernelFreeMemory__(cursor))) {
            timerList->length--;
            RET_OK;
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTimerChangePeriod(Timer_t *timer_, const Ticks_t period_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(timer_) && ISNOTNULLPTR(timerList)) {
    if(ISOK(__TimerListFindTimer__(timer_))) {
      timer_->timerPeriod = period_;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTimerGetPeriod(const Timer_t *timer_, Ticks_t *period_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(timer_) && ISNOTNULLPTR(period_) && ISNOTNULLPTR(timerList)) {
    if(ISOK(__TimerListFindTimer__(timer_))) {
      *period_ = timer_->timerPeriod;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTimerIsTimerActive(const Timer_t *timer_, Base_t *res_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(timer_) && ISNOTNULLPTR(res_) && ISNOTNULLPTR(timerList)) {
    if(ISOK(__TimerListFindTimer__(timer_))) {
      if(TimerStateRunning == timer_->state) {
        *res_ = true;
        RET_OK;
      } else {
        *res_ = false;
        RET_OK;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTimerHasTimerExpired(const Timer_t *timer_, Base_t *res_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(timer_) && ISNOTNULLPTR(res_) && ISNOTNULLPTR(timerList)) {
    if(ISOK(__TimerListFindTimer__(timer_))) {
      if((zero < timer_->timerPeriod) && ((__PortGetSysTicks__() - timer_->timerStartTime) > timer_->timerPeriod)) {
        *res_ = true;
        RET_OK;
      } else {
        *res_ = false;
        RET_OK;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTimerReset(Timer_t *timer_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(timer_) && ISNOTNULLPTR(timerList)) {
    if(ISOK(__TimerListFindTimer__(timer_))) {
      timer_->timerStartTime = __PortGetSysTicks__();
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTimerStart(Timer_t *timer_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(timer_) && ISNOTNULLPTR(timerList)) {
    if(ISOK(__TimerListFindTimer__(timer_))) {
      timer_->state = TimerStateRunning;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTimerStop(Timer_t *timer_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(timer_) && ISNOTNULLPTR(timerList)) {
    if(ISOK(__TimerListFindTimer__(timer_))) {
      timer_->state = TimerStateSuspended;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __TimerListFindTimer__(const Timer_t *timer_) {
  RET_DEFINE;


  Timer_t *cursor = null;


  if((ISNOTNULLPTR(timer_)) && (ISNOTNULLPTR(timerList))) {
    if(ISOK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      cursor = timerList->head;

      while((ISNOTNULLPTR(cursor)) && (cursor != timer_)) {
        cursor = cursor->next;
      }

      if(ISNOTNULLPTR(cursor)) {
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


#if defined(POSIX_ARCH_OTHER)


  void __TimerStateClear__(void) {
    timerList = null;

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */