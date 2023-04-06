/*UNCRUSTIFY-OFF*/
/**
 * @file sys.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for system control
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
#include "sys.h"


/*UNCRUSTIFY-OFF*/
/*
 * 
 * "If the new process paused because it was
 *  swapped out, set the stack level to the last call
 *  to savu(u_ssav).  This means that the return
 *  which is executed immediately after the call to aretu
 *  actually returns from the last routine which did
 *  the savu.
 *
 *  You are not expected to understand this."
 *
 * 
 * Thank you Ken Thompson and Dennis Ritchie (R.I.P.)
 * for UNIX and inspiring the computer scientist in me.
 *
 */
/*UNCRUSTIFY-ON*/
Flags_t flag;


Return_t xSystemAssert(const char *file_, const int line_) {
  FUNCTION_ENTER;

#if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR)
    CONFIG_SYSTEM_ASSERT_BEHAVIOR(file_, line_);
    __ReturnOk__();
#endif /* if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR) */
  FUNCTION_EXIT;
}


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


Return_t xSystemHalt(void) {
  FUNCTION_ENTER;
  __DisableInterrupts__();

  for(;;) {
    /* Do nothing - literally. */
  }

  FUNCTION_EXIT;
}


Return_t xSystemGetSystemInfo(SystemInfo_t **info_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(info_)) {
    if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, sizeof(SystemInfo_t)))) {
      if(__PointerIsNotNull__(*info_)) {
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


            /* Free heap memory because xTaskGetNumberOfTasks() failed. */
            __HeapFreeMemory__(*info_);
          }
        } else {
          __AssertOnElse__();


          /* Free heap memory because __memcpy__() failed. */
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


  /* For unit testing only! */
  void __SysStateClear__(void) {
    __memset__(&flag, nil, sizeof(Flags_t));

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */