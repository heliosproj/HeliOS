#include "timer.h"

Return_t xTimerCreate(Timer_t **timer_, const Ticks_t period_) {

  FUNCTION_ENTER;

  if(__PointerIsNotNull__(timer_)) {

    if(OK(__KernelAllocateMemory__((volatile Addr_t **) timer_, sizeof(Timer_t)))) {

      if(__PointerIsNotNull__(*timer_)) {

        (*timer_)->valid = VALID;

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

  if(__ObjectIsValid__(timer_)) {

    ((Timer_t *) timer_)->valid = INVALID;

    if(OK(__KernelFreeMemory__(timer_))) {

      __ReturnOk__();

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

  if(__ObjectIsValid__(timer_)) {

    timer_->timerPeriod = period_;

    __ReturnOk__();

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}

Return_t xTimerGetPeriod(const Timer_t *timer_, Ticks_t *period_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(timer_) && __PointerIsNotNull__(period_)) {

    *period_ = timer_->timerPeriod;

    __ReturnOk__();

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}

Return_t xTimerIsTimerActive(const Timer_t *timer_, Base_t *res_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(timer_) && __PointerIsNotNull__(res_)) {

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

  FUNCTION_EXIT;

}

Return_t xTimerHasTimerExpired(const Timer_t *timer_, Base_t *res_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(timer_) && __PointerIsNotNull__(res_)) {

    if(TimerStateRunning == timer_->state) {

      if((0x0u < timer_->timerPeriod) && ((__PortGetSysTicks__() - timer_->timerStartTime) > timer_->timerPeriod)) {

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

Return_t xTimerReset(Timer_t *timer_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(timer_)) {

    timer_->timerStartTime = __PortGetSysTicks__();

    __ReturnOk__();

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}

Return_t xTimerStart(Timer_t *timer_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(timer_)) {

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

  FUNCTION_EXIT;

}

Return_t xTimerStop(Timer_t *timer_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(timer_)) {

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

  FUNCTION_EXIT;

}

