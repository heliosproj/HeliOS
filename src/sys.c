/*UNCRUSTIFY-OFF*/
/**
 * @file sys.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources system related calls
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
#include "sys.h"


/*UNCRUSTIFY-OFF*/
/*
 * If the new process paused because it was
 * swapped out, set the stack level to the last call
 * to savu(u_ssav).  This means that the return
 * which is executed immediately after the call to aretu
 * actually returns from the last routine which did
 * the savu.
 *
 * You are not expected to understand this.
 */

/*
 * Thank you Ken Thompson and Dennis Ritchie for
 * UNIX and your inspiration.
 */
/*UNCRUSTIFY-ON*/
SysFlags_t sysFlags;


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
      sysFlags.fault = false;
      sysFlags.overflow = false;
      sysFlags.running = false;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


void xSystemHalt(void) {
  DISABLE_INTERRUPTS();

  for(;;) {
    /* Do nothing - literally. */
  }
}


Return_t xSystemGetSystemInfo(SystemInfo_t **info_) {
  RET_DEFINE;

  if(NOTNULLPTR(info_) && OK(__HeapAllocateMemory__((volatile Addr_t **) info_, sizeof(SystemInfo_t)))) {
    if(NOTNULLPTR(*info_)) {
      if(OK(__memcpy__((*info_)->productName, OS_PRODUCT_NAME, OS_PRODUCT_NAME_SIZE))) {
        (*info_)->majorVersion = OS_MAJOR_VERSION_NO;
        (*info_)->minorVersion = OS_MINOR_VERSION_NO;
        (*info_)->patchVersion = OS_PATCH_VERSION_NO;

        if(OK(xTaskGetNumberOfTasks(&(*info_)->numberOfTasks))) {
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

  RET_RETURN;
}


#if defined(POSIX_ARCH_OTHER)


  void __SysStateClear__(void) {
    __memset__(&sysFlags, 0x0, sizeof(SysFlags_t));

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */