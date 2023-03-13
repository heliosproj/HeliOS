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


Return_t xTimerCreate(Timer_t **timer_, const Ticks_t period_) {
  RET_DEFINE;

  if(NOTNULLPTR(timer_)) {
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) timer_, sizeof(Task_t)))) {
      if(NOTNULLPTR(*timer_)) {
        (*timer_)->state = TimerStateSuspended;
        (*timer_)->timerPeriod = period_;
        (*timer_)->timerStartTime = __PortGetSysTicks__();
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

  if(NOTNULLPTR(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(OK(__KernelFreeMemory__(timer_))) {
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


Return_t xTimerChangePeriod(Timer_t *timer_, const Ticks_t period_) {
  RET_DEFINE;

  if(NOTNULLPTR(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
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

  if(NOTNULLPTR(timer_) && NOTNULLPTR(period_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
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

  if(NOTNULLPTR(timer_) && NOTNULLPTR(res_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
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

  if(NOTNULLPTR(timer_) && NOTNULLPTR(res_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
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

  if(NOTNULLPTR(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
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

  if(NOTNULLPTR(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
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

  if(NOTNULLPTR(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
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