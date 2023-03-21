/*UNCRUSTIFY-OFF*/
/**
 * @file sys.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for system control
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
Flags_t flags;


Return_t xSystemAssert(const char *file_, const int line_) {
  RET_DEFINE;

#if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR)
    CONFIG_SYSTEM_ASSERT_BEHAVIOR(file_, line_);
    RET_OK;
#endif /* if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR) */
  RET_RETURN;
}


Return_t xSystemInit(void) {
  RET_DEFINE;

  if(OK(__MemoryInit__())) {
    if(OK(__PortInit__())) {
      FLAG_MEMFAULT = false;
      FLAG_OVERFLOW = false;
      FLAG_RUNNING = false;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xSystemHalt(void) {
  RET_DEFINE;
  DISABLE_INTERRUPTS();

  for(;;) {
    /* Do nothing - literally. */
  }

  RET_RETURN;
}


Return_t xSystemGetSystemInfo(SystemInfo_t **info_) {
  RET_DEFINE;

  if(NOTNULLPTR(info_)) {
    if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, sizeof(SystemInfo_t)))) {
      if(NOTNULLPTR(*info_)) {
        if(OK(__memcpy__((*info_)->productName, OS_PRODUCT_NAME, OS_PRODUCT_NAME_SIZE))) {
          (*info_)->majorVersion = OS_MAJOR_VERSION_NO;
          (*info_)->minorVersion = OS_MINOR_VERSION_NO;
          (*info_)->patchVersion = OS_PATCH_VERSION_NO;

          if(OK(xTaskGetNumberOfTasks(&(*info_)->numberOfTasks))) {
            RET_OK;
          } else {
            ASSERT;


            /* Free heap memory because xTaskGetNumberOfTasks() failed. */
            __HeapFreeMemory__(*info_);
          }
        } else {
          ASSERT;


          /* Free heap memory because __memcpy__() failed. */
          __HeapFreeMemory__(*info_);
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


#if defined(POSIX_ARCH_OTHER)


  /* For unit testing only! */
  void __SysStateClear__(void) {
    __memset__(&flags, 0x0, sizeof(Flags_t));

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */