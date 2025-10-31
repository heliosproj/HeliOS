/*UNCRUSTIFY-OFF*/
/**
 * @file timer.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Timer management implementation
 * @details
 * Implements software timers for task scheduling and timing operations. Provides timer creation, deletion, period management, and expiration checking.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#include "timer.h"
/**
* @brief Creates a new software timer
* @details Allocates memory for a timer structure and initializes it with the
* specified period. The timer starts in suspended state and must be started
* explicitly.
*
* @param[out] timer_  Pointer to store the created timer handle
* @param[in]  period_ Timer period in system ticks
*
* @return             ReturnOK if timer was created successfully
* @return             ReturnError if allocation failed or invalid parameters
*
* @note The timer must be started with xTimerStart() before it becomes active
* @warning Caller is responsible for deleting the timer with xTimerDelete()
*/
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


/**
* @brief Deletes a software timer
* @details Frees the memory allocated for the timer structure. The timer must
* be valid.
*
* @param[in] timer_ Pointer to the timer to delete
*
* @return           ReturnOK if timer was deleted successfully
* @return           ReturnError if timer is invalid or deallocation failed
*
* @warning Using the timer handle after deletion results in undefined
* behavior
*/
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


/**
* @brief Changes the period of an existing timer
* @details Updates the timer period without affecting its running state or
* start time.
*
* @param[in,out] timer_  Pointer to the timer to modify
* @param[in]     period_ New timer period in system ticks
*
* @return                ReturnOK if period was changed successfully
* @return                ReturnError if timer is invalid
*/
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


/**
* @brief Gets the current period of a timer
* @details Retrieves the configured period without affecting the timer state.
*
* @param[in]  timer_  Pointer to the timer to query
* @param[out] period_ Pointer to store the timer period in system ticks
*
* @return             ReturnOK if period was retrieved successfully
* @return             ReturnError if timer is invalid or period pointer is
*                     NULL
*/
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


/**
* @brief Checks if a timer is currently active
* @details Returns true if the timer is in running state, false if suspended
* or stopped.
*
* @param[in]  timer_ Pointer to the timer to check
* @param[out] res_   Pointer to store the result (true if active, false
*                    otherwise)
*
* @return            ReturnOK if check was successful
* @return            ReturnError if timer is invalid or result pointer is
*                    NULL
*/
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


/**
* @brief Checks if a timer has expired
* @details Returns true if the elapsed time since timer start exceeds the
* configured period.
*
* @param[in]  timer_ Pointer to the timer to check
* @param[out] res_   Pointer to store the result (true if expired, false
*                    otherwise)
*
* @return            ReturnOK if check was successful
* @return            ReturnError if timer is invalid or result pointer is
*                    NULL
*/
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


/**
* @brief Resets a timer to its starting state
* @details Resets the timer start time to the current system ticks without
* changing its period.
*
* @param[in,out] timer_ Pointer to the timer to reset
*
* @return               ReturnOK if timer was reset successfully
* @return               ReturnError if timer is invalid
*/
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


/**
* @brief Starts a timer
* @details Changes the timer state to running and sets the start time to
* current system ticks.
*
* @param[in,out] timer_ Pointer to the timer to start
*
* @return               ReturnOK if timer was started successfully
* @return               ReturnError if timer is invalid
*/
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


/**
* @brief Stops a timer
* @details Changes the timer state to suspended, stopping expiration checks.
*
* @param[in,out] timer_ Pointer to the timer to stop
*
* @return               ReturnOK if timer was stopped successfully
* @return               ReturnError if timer is invalid
*/
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