/*UNCRUSTIFY-OFF*/
/**
 * @file timer.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for application timers
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
      if(TimerStateRunning == timer_->state) {
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
      if(TimerStateSuspended == timer_->state) {
        timer_->state = TimerStateRunning;
        timer_->timerStartTime = __PortGetSysTicks__();
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


Return_t xTimerStop(Timer_t *timer_) {
  RET_DEFINE;

  if(NOTNULLPTR(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(TimerStateRunning == timer_->state) {
        timer_->state = TimerStateSuspended;
        timer_->timerStartTime = __PortGetSysTicks__();
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