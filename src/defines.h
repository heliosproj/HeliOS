/*UNCRUSTIFY-OFF*/
/**
 * @file defines.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel header for macros and definitions
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
#ifndef DEFINES_H_
  #define DEFINES_H_

  #include "posix.h"

  #include <limits.h>
  #include <stdint.h>
  #include <stddef.h>


/** TEMPORARY!!!!!! */

  #define RETURN_SUCCESS 1
  #define RETURN_FAILURE 0


/** TEMPORARY!!!!!! */
/* Check that the system HeliOS is being targeted for has an 8-bit wide byte. */
  #if !defined(CHAR_BIT)
    #pragma message("WARNING: Unable to determine if system has an 8-bit wide byte. CHAR_BIT not defined?")
  #else  /* if !defined(CHAR_BIT) */
    #if CHAR_BIT != 8
      #pragma message("WARNING: System may not have an 8-bit wide byte!")
    #endif /* if CHAR_BIT != 8 */
  #endif /* if !defined(CHAR_BIT) */


/* Define "true". We are not using "true" for boolean logic, we are using "true"
 * as an integer so we do not use the (1=1) and (!TRUE) constructs. */
  #if defined(true)
    #undef true
  #endif /* if defined(true) */
  #define true 0x1u /* 1 */


/* Define "false". We are not using "false" for boolean logic, we are using
 * "false" as an integer so we do not use the (1=1) and (!TRUE) constructs. */
  #if defined(false)
    #undef false
  #endif /* if defined(false) */
  #define false 0x0u /* 0 */
/* Define "null". */
  #if defined(null)
    #undef null
  #endif /* if defined(null) */
  #if defined(__cplusplus)
    #define null 0x0 /* 0 */
  #else  /* if defined(__cplusplus) */
    #define null ((void *) 0x0) /* 0 */
  #endif /* if defined(__cplusplus) */
/* Define "zero". */
  #if defined(zero)
    #undef zero
  #endif /* if defined(zero) */
  #define zero 0x0u /* 0 */


/* Define the size in bytes of the OS product name which is accessible through
 * xSystemGetSystemInfo(). */
  #if defined(OS_PRODUCT_NAME_SIZE)
    #undef OS_PRODUCT_NAME_SIZE
  #endif /* if defined(OS_PRODUCT_NAME_SIZE) */
  #define OS_PRODUCT_NAME_SIZE 0x6u /* 6 */


/* Define the OS product name which is accessible through
 * xSystemGetSystemInfo(). */
  #if defined(OS_PRODUCT_NAME)
    #undef OS_PRODUCT_NAME
  #endif /* if defined(OS_PRODUCT_NAME) */
  #define OS_PRODUCT_NAME "HeliOS"


/* Define the OS product major version number which is accessible through
 * xSystemGetSystemInfo(). */
  #if defined(OS_MAJOR_VERSION_NO)
    #undef OS_MAJOR_VERSION_NO
  #endif /* if defined(OS_MAJOR_VERSION_NO) */
  #define OS_MAJOR_VERSION_NO 0x0u /* 0 */


/* Define the OS product minor version number which is accessible through
 * xSystemGetSystemInfo(). */
  #if defined(OS_MINOR_VERSION_NO)
    #undef OS_MINOR_VERSION_NO
  #endif /* if defined(OS_MINOR_VERSION_NO) */
  #define OS_MINOR_VERSION_NO 0x4u /* 4 */


/* Define the OS product patch version number which is accessible through
 * xSystemGetSystemInfo(). */
  #if defined(OS_PATCH_VERSION_NO)
    #undef OS_PATCH_VERSION_NO
  #endif /* if defined(OS_PATCH_VERSION_NO) */
  #define OS_PATCH_VERSION_NO 0x0u /* 0 */


/* Define the raw size of the heap in bytes based on the number of blocks the
 * heap contains and the size of each block in bytes. */
  #if defined(MEMORY_REGION_SIZE_IN_BYTES)
    #undef MEMORY_REGION_SIZE_IN_BYTES
  #endif /* if defined(MEMORY_REGION_SIZE_IN_BYTES) */
  #define MEMORY_REGION_SIZE_IN_BYTES CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS *CONFIG_MEMORY_REGION_BLOCK_SIZE


/* Define a macro to access the running system flag which is used by
 * xTaskStartScheduler() to indicate whether the scheduler is running. */
  #if defined(SYSFLAG_RUNNING)
    #undef SYSFLAG_RUNNING
  #endif /* if defined(SYSFLAG_RUNNING) */
  #define SYSFLAG_RUNNING() sysFlags.running


/* Define a macro to access the overflow system flag which is used by the
 * scheduler to determine when a task's runtime has overflowed and all runtimes
 * need to be reset. */
  #if defined(SYSFLAG_OVERFLOW)
    #undef SYSFLAG_OVERFLOW
  #endif /* if defined(SYSFLAG_OVERFLOW) */
  #define SYSFLAG_OVERFLOW() sysFlags.overflow


/* Define a macro to access the corrupt system flag which is used by the memory
 * management system calls to flag if corruption of the heap has been detected.
 */
  #if defined(SYSFLAG_FAULT)
    #undef SYSFLAG_FAULT
  #endif /* if defined(SYSFLAG_FAULT) */
  #define SYSFLAG_FAULT() sysFlags.fault


/* Define a marco which makes null pointer checks more readable and concise */
  #if defined(ISNOTNULLPTR)
    #undef ISNOTNULLPTR
  #endif /* if defined(ISNOTNULLPTR) */
  #define ISNOTNULLPTR(addr_) ((null) != (addr_))


/* Define a marco which makes null pointer checks more readable and concise */
  #if defined(ISNULLPTR)
    #undef ISNULLPTR
  #endif /* if defined(ISNULLPTR) */
  #define ISNULLPTR(addr_) ((null) == (addr_))


  #if defined(RET_DEFINE)
    #undef RET_DEFINE
  #endif /* if defined(RET_DEFINE) */
  #define RET_DEFINE Return_t ret = ReturnFailure


  #if defined(RET_RETURN)
    #undef RET_RETURN
  #endif /* if defined(RET_RETURN) */
  #define RET_RETURN return(ret)


  #if defined(RET_SUCCESS)
    #undef RET_SUCCESS
  #endif /* if defined(RET_SUCCESS) */
  #define RET_SUCCESS ret = ReturnSuccess


  #if defined(ISSUCCESSFUL)
    #undef ISSUCCESSFUL
  #endif /* if defined(ISSUCCESSFUL) */
  #define ISSUCCESSFUL(expr_) (ReturnSuccess == (expr_))


  #if defined(SYSASSERT)
    #undef SYSASSERT
  #endif /* if defined(SYSASSERT) */
  #if defined(CONFIG_ENABLE_SYSTEM_ASSERT)
    #define SYSASSERT(expr_)  \
            if((false) == (expr_)) \
            __SystemAssert__(__FILE__, __LINE__)
  #else  /* if defined(CONFIG_ENABLE_SYSTEM_ASSERT) */
    #define SYSASSERT(expr_)
  #endif /* if defined(CONFIG_ENABLE_SYSTEM_ASSERT) */


  #if defined(MEMORY_REGION_CHECK_OPTION_WO_ADDR)
    #undef MEMORY_REGION_CHECK_OPTION_WO_ADDR
  #endif /* if defined(MEMORY_REGION_CHECK_OPTION_WO_ADDR) */
  #define MEMORY_REGION_CHECK_OPTION_WO_ADDR 0x1u /* 1 */


  #if defined(MEMORY_REGION_CHECK_OPTION_W_ADDR)
    #undef MEMORY_REGION_CHECK_OPTION_W_ADDR
  #endif /* if defined(MEMORY_REGION_CHECK_OPTION_W_ADDR) */
  #define MEMORY_REGION_CHECK_OPTION_W_ADDR 0x2u /* 2 */


/* Define a macro to convert a heap memory address to it's corresponding heap
 * entry. */
  #if defined(ADDR2ENTRY)
    #undef ADDR2ENTRY
  #endif /* if defined(ADDR2ENTRY) */
  #define ADDR2ENTRY(addr_, region_) ((MemoryEntry_t *) ((Byte_t *) (addr_) - ((region_)->entrySize * CONFIG_MEMORY_REGION_BLOCK_SIZE)))


/* Define a macro to convert a heap entry to it's corresponding heap memory
 * address. */
  #if defined(ENTRY2ADDR)
    #undef ENTRY2ADDR
  #endif /* if defined(ENTRY2ADDR) */
  #define ENTRY2ADDR(addr_, region_) ((Addr_t *) ((Byte_t *) (addr_) + ((region_)->entrySize * CONFIG_MEMORY_REGION_BLOCK_SIZE)))


  #if defined(UCHAR_TYPE)
    #undef UCHAR_TYPE
  #endif /* if defined(UCHAR_TYPE) */
  #define UCHAR_TYPE unsigned char


  #if defined(UINT8_TYPE)
    #undef UINT8_TYPE
  #endif /* if defined(UINT8_TYPE) */
  #define UINT8_TYPE uint8_t


  #if defined(UINT16_TYPE)
    #undef UINT16_TYPE
  #endif /* if defined(UINT16_TYPE) */
  #define UINT16_TYPE uint16_t


  #if defined(UINT32_TYPE)
    #undef UINT32_TYPE
  #endif /* if defined(UINT32_TYPE) */
  #define UINT32_TYPE uint32_t


  #if defined(SIZE_TYPE)
    #undef SIZE_TYPE
  #endif /* if defined(SIZE_TYPE) */
  #define SIZE_TYPE size_t


  #if defined(VOID_TYPE)
    #undef VOID_TYPE
  #endif /* if defined(VOID_TYPE) */
  #define VOID_TYPE void


  #if defined(DEREF_TASKPARM)
    #undef DEREF_TASKPARM
  #endif /* if defined(DEREF_TASKPARM) */
  #define DEREF_TASKPARM(type_, ptr_) (*((type_ *) ptr_))


#endif /* ifndef DEFINES_H_ */