/**
 * @file sys.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources system related calls
 * @version 0.3.5
 * @date 2022-01-31
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
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

#include "sys.h"

/* "You are not expected to understand this."
Thank you for the best OS on Earth, Dennis. */



/* Declare and set the system flags to their default values. */
SysFlags_t sysFlags = {
    .running = false,
    .overflow = false,
    .corrupt = false};



/* The _SystemAssert_() system call will be called when
the SYSASSERT() macro evaluates false. In order for there
to be any effect, CONFIG_ENABLE_SYSTEM_ASSERT and
CONFIG_SYSTEM_ASSERT_BEHAVIOR must be defined.

_SystemAssert_() should NOT be called directly because it is an INTERNAL
function name and may change in future releases. Instead use the SYSASSERT() C macro. */
void _SystemAssert_(const char *file_, int line_) {

  /* Do not modify this system call directly. Define
  the behavior (code) through the CONFIG_SYSTEM_ASSERT_BEHAVIOR
  setting in the config.h header file. */

#if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR)
  CONFIG_SYSTEM_ASSERT_BEHAVIOR(file_, line_);
#endif



  return;
}


/* The xSystemInit() system call initializes the system. */
void xSystemInit(void) {

  _SysInit_();

  return;
}



/* The xSystemHalt() system call halts the system. */
void xSystemHalt(void) {


  /* Don't want to service interrupts anymore so disable them. */
  DISABLE_INTERRUPTS();



  /* Put the processor into an infinite loop. */
  for (;;) {
    /* Do nothing - literally. */
  }
}




/* The xSystemGetSystemInfo() system call will return the type xSystemInfo containing
information about the system including the OS (product) name, its version and how many tasks
are currently in the running, suspended or waiting states. */
SystemInfo_t *xSystemGetSystemInfo(void) {

  SystemInfo_t *ret = NULL;



  ret = (SystemInfo_t *)_HeapAllocateMemory_(sizeof(SystemInfo_t));



  /* Assert if xMemAlloc() failed to allocate the heap memory. */
  SYSASSERT(ISNOTNULLPTR(ret));



  /* Check if system info is not null to make sure xMemAlloc() successfully allocated
  the memory. */
  if (ISNOTNULLPTR(ret)) {


    _memcpy_(ret->productName, OS_PRODUCT_NAME, OS_PRODUCT_NAME_SIZE);

    ret->majorVersion = OS_MAJOR_VERSION_NO;

    ret->minorVersion = OS_MINOR_VERSION_NO;

    ret->patchVersion = OS_PATCH_VERSION_NO;

    ret->numberOfTasks = xTaskGetNumberOfTasks();
  }

  return ret;
}



#if defined(POSIX_ARCH_OTHER)
void __SysStateClear__(void) {

  _memset_(&sysFlags, 0x0, sizeof(SysFlags_t));

  return;
}
#endif