/**
 * @file defines.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel header for macros and definitions
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
#ifndef DEFINES_H_
#define DEFINES_H_

#include "posix.h"

#include <limits.h>
#include <stdint.h>
#include <stddef.h>


/* Check that the system HeliOS is being targeted for has an
8-bit wide byte. */
#if !defined(CHAR_BIT)
#pragma message("WARNING: Unable to determine if system has an 8-bit wide byte. CHAR_BIT not defined?")
#else
#if CHAR_BIT != 8
#pragma message("WARNING: System may not have an 8-bit wide byte!")
#endif
#endif




/* Define "true" if not defined. */
#if !defined(true)
#define true 0x1u /* 1 */
#endif




/* Define "false" if not defined. */
#if !defined(false)
#define false 0x0u /* 0 */
#endif




/* Define "NULL" if not defined. */
#if !defined(NULL)
#if !defined(__cplusplus)
#define NULL ((void *)0x0) /* 0 */
#else
#define NULL 0x0 /* 0 */
#endif
#endif




/* Define "zero" if not defined. */
#if !defined(zero)
#define zero 0x0u /* 0 */
#endif




/* Define a general return failure for
return values. */
#if !defined(RETURN_FAILURE)
#define RETURN_FAILURE 0x0u /* 0 */
#endif




/* Define a general return success
for return values. */
#if !defined(RETURN_SUCCESS)
#define RETURN_SUCCESS 0x1u /* 1 */
#endif




/* Define the size in bytes of the OS product name which is accessible through
xSystemGetSystemInfo(). */
#if !defined(OS_PRODUCT_NAME_SIZE)
#define OS_PRODUCT_NAME_SIZE 0x6u /* 6 */
#endif




/* Define the OS product name which is accessible through xSystemGetSystemInfo(). */
#if !defined(OS_PRODUCT_NAME)
#define OS_PRODUCT_NAME "HeliOS"
#endif




/* Define the OS product major version number which is accessible through
xSystemGetSystemInfo(). */
#if !defined(OS_MAJOR_VERSION_NO)
#define OS_MAJOR_VERSION_NO 0x0u /* 0 */
#endif




/* Define the OS product minor version number which is accessible through
xSystemGetSystemInfo(). */
#if !defined(OS_MINOR_VERSION_NO)
#define OS_MINOR_VERSION_NO 0x3u /* 3 */
#endif




/* Define the OS product patch version number which is accessible through
xSystemGetSystemInfo(). */
#if !defined(OS_PATCH_VERSION_NO)
#define OS_PATCH_VERSION_NO 0x5u /* 5 */
#endif



/* Define the raw size of the heap in bytes based on the number of blocks
the heap contains and the size of each block in bytes. */
#if !defined(MEMORY_REGION_SIZE_IN_BYTES)
#define MEMORY_REGION_SIZE_IN_BYTES CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE
#endif



/* Define a macro to access the running system flag which is used
by xTaskStartScheduler() to indicate whether the scheduler is
running. */
#if !defined(SYSFLAG_RUNNING)
#define SYSFLAG_RUNNING() sysFlags.running
#endif




/* Define a macro to access the overflow system flag which is used
by the scheduler to determine when a task's runtime has overflowed
and all runtimes need to be reset. */
#if !defined(SYSFLAG_OVERFLOW)
#define SYSFLAG_OVERFLOW() sysFlags.overflow
#endif




/* Define a macro to access the corrupt system flag which is used
by the memory management system calls to flag if corruption
of the heap has been detected. */
#if !defined(SYSFLAG_CORRUPT)
#define SYSFLAG_CORRUPT() sysFlags.corrupt
#endif



/* Define a marco which makes null pointer checks more readable and
concise */
#if !defined(ISNOTNULLPTR)
#define ISNOTNULLPTR(addr_) ((NULL) != ( addr_ ))
#endif




/* Define a marco which makes null pointer checks more readable and
concise */
#if !defined(ISNULLPTR)
#define ISNULLPTR(addr_) ((NULL) == ( addr_ ))
#endif




/* Define a macro to assert if assertions are enabled through
the CONFIG_ENABLE_SYSTEM_ASSERT setting. */
#if !defined(SYSASSERT)
#if !defined(CONFIG_ENABLE_SYSTEM_ASSERT)
#define SYSASSERT(expr_)
#else
#define SYSASSERT(expr_) if (false == ( expr_ )) _SystemAssert_( __FILE__ , __LINE__ )
#endif
#endif


#if !defined(MEMORY_REGION_CHECK_OPTION_WO_ADDR)
#define MEMORY_REGION_CHECK_OPTION_WO_ADDR 0x1u /* 1 */
#endif


#if !defined(MEMORY_REGION_CHECK_OPTION_W_ADDR)
#define MEMORY_REGION_CHECK_OPTION_W_ADDR 0x2u /* 2 */
#endif




/* Define a macro to convert a heap memory address to it's corresponding
heap entry. */
#if !defined(ADDR2ENTRY)
#define ADDR2ENTRY(addr_, region_) (MemoryEntry_t *)((Byte_t *)( addr_ ) - (( region_ )->entrySize * CONFIG_MEMORY_REGION_BLOCK_SIZE))
#endif



/* Define a macro to convert a heap entry to it's corresponding heap memory
address. */
#if !defined(ENTRY2ADDR)
#define ENTRY2ADDR(addr_, region_) (Addr_t *)((Byte_t *)( addr_ ) + (( region_ )->entrySize * CONFIG_MEMORY_REGION_BLOCK_SIZE))
#endif

#endif