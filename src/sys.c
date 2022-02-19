/**
 * @file sys.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources system related calls
 * @version 0.3.1
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
Thank you for the best OS on Earth, Dennis.
May you forever rest in peace. */

/* Declare and set the system flags to their default values. */
SysFlags_t sysFlags = {
    .running = true,
    .critical = false,
    .overflow = false,
    .privileged = false};

/* The xSystemAssert() system call will be called when
the SYSASSERT() macro evaluates true. In order for there
to be any effect, CONFIG_ENABLE_SYSTEM_ASSERT and
CONFIG_SYSTEM_ASSERT_BEHAVIOR must be defined. */
void xSystemAssert(const char *file_, int line_) {

#if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR)
  CONFIG_SYSTEM_ASSERT_BEHAVIOR(file_, line_);
#endif

  return;
}

/* The xSystemHalt() system call stops the system by putting HeliOS into an infinite loop. */
void xSystemHalt(void) {


  DISABLE_INTERRUPTS();


  for (;;) {
  }
}

/* The xSystemGetSystemInfo() system call will return the type xSystemInfo containing
information about the system including the OS (product) name, its version and how many tasks
are currently in the running, suspended or waiting states. */
SystemInfo_t *xSystemGetSystemInfo(void) {
  SystemInfo_t *ret = NULL;

  ret = (SystemInfo_t *)xMemAlloc(sizeof(SystemInfo_t));

  SYSASSERT(ISNOTNULLPTR(ret));

  /* Check if system info is not null to make sure xMemAlloc() successfully allocated
  the memory. */
  if (ISNOTNULLPTR(ret)) {
    memcpy_(ret->productName, OS_PRODUCT_NAME, OS_PRODUCT_NAME_SIZE);

    ret->majorVersion = OS_MAJOR_VERSION_NO;

    ret->minorVersion = OS_MINOR_VERSION_NO;

    ret->patchVersion = OS_PATCH_VERSION_NO;

    ret->numberOfTasks = xTaskGetNumberOfTasks();
  }

  return ret;
}