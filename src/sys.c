/*UNCRUSTIFY-OFF*/
/**
 * @file sys.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief System initialization and configuration
 * @details
 * Implements kernel initialization, system configuration management, and core system utility functions including version information and feature flags.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/


#include "sys.h"


Flags_t flag = {


  VALID,


  0,


  0,


  0,


  0


};
/**
 * @brief Triggers a system assertion failure
 * @details Records the file and line number of an assertion failure and halts the system.
 *
 * @param[in] file_ Source file name where assertion failed
 * @param[in] line_ Line number where assertion failed
 *
 * @return          ReturnError always (system will halt)
 *
 * @warning This function should only be called when a critical error is detected
 */
Return_t xSystemAssert(const char *file_, const int line_) {

  FUNCTION_ENTER;

#if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR)

    CONFIG_SYSTEM_ASSERT_BEHAVIOR(file_, line_);

    __ReturnOk__();

#endif /* if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR) */

  FUNCTION_EXIT;

}


/**
 * @brief Initializes the HeliOS kernel
 * @details Sets up memory management, scheduler, and other kernel subsystems.
 *
 * @return ReturnOK if initialization was successful
 * @return ReturnError if initialization failed
 *
 * @note This function must be called before any other HeliOS functions
 */
Return_t xSystemInit(void) {

  FUNCTION_ENTER;

  if(OK(__MemoryInit__())) {

    if(OK(__PortInit__())) {

      __UnsetFlag__(OVERFLOW);

      __UnsetFlag__(RUNNING);

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
 * @brief Halts the system
 * @details Stops the scheduler and enters an infinite loop, effectively halting execution.
 *
 * @return ReturnError always (function does not return normally)
 *
 * @warning This function does not return; use only for fatal errors
 */
Return_t xSystemHalt(void) {

  FUNCTION_ENTER;

  __DisableInterrupts__();

  for(;;) {

  }

  FUNCTION_EXIT;

}


/**
 * @brief Gets system information
 * @details Allocates and returns a structure containing OS version, configuration, and status.
 *
 * @param[out] info_ Pointer to store allocated system information structure
 *
 * @return           ReturnOK if information was retrieved successfully
 * @return           ReturnError if allocation failed or invalid parameter
 *
 * @warning Caller is responsible for freeing the allocated structure
 */
Return_t xSystemGetSystemInfo(SystemInfo_t **info_) {

  FUNCTION_ENTER;

  if(__PointerIsNotNull__(info_)) {

    if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, sizeof(SystemInfo_t)))) {

      if(__PointerIsNotNull__(*info_)) {

        (*info_)->valid = VALID;

        if(OK(__memcpy__((*info_)->productName, OS_PRODUCT_NAME, OS_PRODUCT_NAME_SIZE))) {

          (*info_)->majorVersion = OS_MAJOR_VERSION_NO;

          (*info_)->minorVersion = OS_MINOR_VERSION_NO;

          (*info_)->patchVersion = OS_PATCH_VERSION_NO;

          if(__FlagIsSet__(LITTLEEND)) {

            (*info_)->littleEndian = true;

          } else {

            (*info_)->littleEndian = false;

          }

          if(OK(xTaskGetNumberOfTasks(&(*info_)->numberOfTasks))) {

            __ReturnOk__();

          } else {

            __AssertOnElse__();

            (*info_)->valid = INVALID;

            __HeapFreeMemory__(*info_);

          }

        } else {

          __AssertOnElse__();

          (*info_)->valid = INVALID;

          __HeapFreeMemory__(*info_);

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


#if defined(POSIX_ARCH_OTHER)
  void __SysStateClear__(void) {

    __memset__(&flag, 0x0u, sizeof(Flags_t));

    return;

  }


#endif /* if defined(POSIX_ARCH_OTHER) */