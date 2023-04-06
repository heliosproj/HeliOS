/*UNCRUSTIFY-OFF*/
/**
 * @file timer.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for application timers
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
#include "timer.h"


Return_t xTimerCreate(Timer_t **timer_, const Ticks_t period_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_)) {
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) timer_, sizeof(Task_t)))) {
      if(__PointerIsNotNull__(*timer_)) {
        (*timer_)->state = TimerStateSuspended;
        (*timer_)->timerPeriod = period_;
        (*timer_)->timerStartTime = __PortGetSysTicks__();
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTimerDelete(const Timer_t *timer_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(OK(__KernelFreeMemory__(timer_))) {
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTimerChangePeriod(Timer_t *timer_, const Ticks_t period_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      timer_->timerPeriod = period_;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTimerGetPeriod(const Timer_t *timer_, Ticks_t *period_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_) && __PointerIsNotNull__(period_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      *period_ = timer_->timerPeriod;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTimerIsTimerActive(const Timer_t *timer_, Base_t *res_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_) && __PointerIsNotNull__(res_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(TimerStateRunning == timer_->state) {
        *res_ = true;
        __ReturnOk__();
      } else {
        *res_ = false;
        __ReturnOk__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTimerHasTimerExpired(const Timer_t *timer_, Base_t *res_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_) && __PointerIsNotNull__(res_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(TimerStateRunning == timer_->state) {
        if((nil < timer_->timerPeriod) && ((__PortGetSysTicks__() - timer_->timerStartTime) > timer_->timerPeriod)) {
          *res_ = true;
          __ReturnOk__();
        } else {
          *res_ = false;
          __ReturnOk__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTimerReset(Timer_t *timer_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      timer_->timerStartTime = __PortGetSysTicks__();
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTimerStart(Timer_t *timer_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(TimerStateSuspended == timer_->state) {
        timer_->state = TimerStateRunning;
        timer_->timerStartTime = __PortGetSysTicks__();
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTimerStop(Timer_t *timer_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_)) {
    if(OK(__MemoryRegionCheckKernel__(timer_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(TimerStateRunning == timer_->state) {
        timer_->state = TimerStateSuspended;
        timer_->timerStartTime = __PortGetSysTicks__();
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}