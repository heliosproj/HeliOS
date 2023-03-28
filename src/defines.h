/*UNCRUSTIFY-OFF*/
/**
 * @file defines.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for macro definitions
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
#ifndef DEFINES_H_
  #define DEFINES_H_

  #include "posix.h"

  #include <limits.h>
  #include <stdint.h>
  #include <stddef.h>


  #if defined(CHAR_BIT)
    #if CHAR_BIT != 8
      #pragma message("WARNING: System may not have an 8-bit wide byte!")
    #endif /* if CHAR_BIT != 8 */
  #else  /* if !defined(CHAR_BIT) */
    #pragma message("WARNING: Unable to determine if system has an 8-bit wide byte. CHAR_BIT not defined?")
  #endif /* if !defined(CHAR_BIT) */


  #if defined(true)
    #undef true
  #endif /* if defined(true) */
  #define true 0xFFu /* 255*/


  #if defined(false)
    #undef false
  #endif /* if defined(false) */
  #define false 0x0u /* 0 */


  #if defined(null)
    #undef null
  #endif /* if defined(null) */
  #if defined(__cplusplus)
    #define null 0x0 /* 0 */
  #else  /* if defined(__cplusplus) */
    #define null ((void *) 0x0) /* 0 */
  #endif /* if defined(__cplusplus) */


  #if defined(zero)
    #undef zero
  #endif /* if defined(zero) */
  #define zero 0x0u /* 0 */


  #if defined(OS_PRODUCT_NAME_SIZE)
    #undef OS_PRODUCT_NAME_SIZE
  #endif /* if defined(OS_PRODUCT_NAME_SIZE) */
  #define OS_PRODUCT_NAME_SIZE 0x6u /* 6 */


  #if defined(OS_PRODUCT_NAME)
    #undef OS_PRODUCT_NAME
  #endif /* if defined(OS_PRODUCT_NAME) */
  #define OS_PRODUCT_NAME "HeliOS"


  #if defined(OS_MAJOR_VERSION_NO)
    #undef OS_MAJOR_VERSION_NO
  #endif /* if defined(OS_MAJOR_VERSION_NO) */
  #define OS_MAJOR_VERSION_NO 0x0u /* 0 */


  #if defined(OS_MINOR_VERSION_NO)
    #undef OS_MINOR_VERSION_NO
  #endif /* if defined(OS_MINOR_VERSION_NO) */
  #define OS_MINOR_VERSION_NO 0x4u /* 4 */


  #if defined(OS_PATCH_VERSION_NO)
    #undef OS_PATCH_VERSION_NO
  #endif /* if defined(OS_PATCH_VERSION_NO) */
  #define OS_PATCH_VERSION_NO 0x1u /* 1 */


  #if defined(MEMORY_REGION_SIZE_IN_BYTES)
    #undef MEMORY_REGION_SIZE_IN_BYTES
  #endif /* if defined(MEMORY_REGION_SIZE_IN_BYTES) */
  #define MEMORY_REGION_SIZE_IN_BYTES CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS *CONFIG_MEMORY_REGION_BLOCK_SIZE


  #if defined(FLAG_RUNNING)
    #undef FLAG_RUNNING
  #endif /* if defined(FLAG_RUNNING) */
  #define FLAG_RUNNING flags.running


  #if defined(FLAG_OVERFLOW)
    #undef FLAG_OVERFLOW
  #endif /* if defined(FLAG_OVERFLOW) */
  #define FLAG_OVERFLOW flags.overflow


  #if defined(FLAG_MEMFAULT)
    #undef FLAG_MEMFAULT
  #endif /* if defined(FLAG_MEMFAULT) */
  #define FLAG_MEMFAULT flags.memfault


  #if defined(NOTNULLPTR)
    #undef NOTNULLPTR
  #endif /* if defined(NOTNULLPTR) */
  #define NOTNULLPTR(addr_) ((null) != (addr_))


  #if defined(NULLPTR)
    #undef NULLPTR
  #endif /* if defined(NULLPTR) */
  #define NULLPTR(addr_) ((null) == (addr_))


  #if defined(RET_DEFINE)
    #undef RET_DEFINE
  #endif /* if defined(RET_DEFINE) */
  #define RET_DEFINE Return_t ret = ReturnError


  #if defined(RET_RETURN)
    #undef RET_RETURN
  #endif /* if defined(RET_RETURN) */
  #define RET_RETURN return(ret)


  #if defined(RET_OK)
    #undef RET_OK
  #endif /* if defined(RET_OK) */
  #define RET_OK ret = ReturnOK


  #if defined(OK)
    #undef OK
  #endif /* if defined(OK) */
  #define OK(expr_) ((ReturnOK) == (expr_))


  #if defined(ERROR)
    #undef ERROR
  #endif /* if defined(ERROR) */
  #define ERROR(expr_) ((ReturnError) == (expr_))


  #if defined(ASSERT)
    #undef ASSERT
  #endif /* if defined(ASSERT) */
  #if defined(CONFIG_ENABLE_SYSTEM_ASSERT)
    #define ASSERT xSystemAssert(__FILE__, __LINE__)
  #else  /* if defined(CONFIG_ENABLE_SYSTEM_ASSERT) */
    #define ASSERT
  #endif /* if defined(CONFIG_ENABLE_SYSTEM_ASSERT) */


  #if defined(MEMORY_REGION_CHECK_OPTION_WO_ADDR)
    #undef MEMORY_REGION_CHECK_OPTION_WO_ADDR
  #endif /* if defined(MEMORY_REGION_CHECK_OPTION_WO_ADDR) */
  #define MEMORY_REGION_CHECK_OPTION_WO_ADDR 0x1u /* 1 */


  #if defined(MEMORY_REGION_CHECK_OPTION_W_ADDR)
    #undef MEMORY_REGION_CHECK_OPTION_W_ADDR
  #endif /* if defined(MEMORY_REGION_CHECK_OPTION_W_ADDR) */
  #define MEMORY_REGION_CHECK_OPTION_W_ADDR 0x2u /* 2 */


  #if defined(ADDR2ENTRY)
    #undef ADDR2ENTRY
  #endif /* if defined(ADDR2ENTRY) */
  #define ADDR2ENTRY(addr_, region_) ((MemoryEntry_t *) (((Byte_t *) (addr_)) - ((region_)->entrySize * \
          CONFIG_MEMORY_REGION_BLOCK_SIZE)))


  #if defined(ENTRY2ADDR)
    #undef ENTRY2ADDR
  #endif /* if defined(ENTRY2ADDR) */
  #define ENTRY2ADDR(addr_, region_) ((Addr_t *) (((Byte_t *) (addr_)) + ((region_)->entrySize * \
          CONFIG_MEMORY_REGION_BLOCK_SIZE)))


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
  #define DEREF_TASKPARM(type_, ptr_) (*((type_ *) (ptr_)))


  #if defined(CONCAT)
    #undef CONCAT
  #endif /* if defined(CONCAT) */
  #define CONCAT(a_, b_) a_ ## b_


  #if defined(QUOTE)
    #undef QUOTE
  #endif /* if defined(QUOTE) */
  #define QUOTE(a_) #a_


  #if defined(TO_FUNCTION)
    #undef TO_FUNCTION
  #endif /* if defined(TO_FUNCTION) */
  #define TO_FUNCTION(a_, b_) CONCAT(a_, b_)


  #if defined(TO_LITERAL)
    #undef TO_LITERAL
  #endif /* if defined(TO_LITERAL) */
  #define TO_LITERAL(a_) QUOTE(a_)


  #if defined(MAGIC_CONST)
    #undef MAGIC_CONST
  #endif /* if defined(MAGIC_CONST) */
  #define MAGIC_CONST 0xB16B00B5u /* https://en.wikipedia.org/wiki/Hexspeak */


  #if defined(CALCMAGIC)
    #undef CALCMAGIC
  #endif /* if defined(CALCMAGIC) */
  #define CALCMAGIC(ptr_) (((Word_t) (ptr_)) ^ MAGIC_CONST)


  #if defined(OKMAGIC)
    #undef OKMAGIC
  #endif /* if defined(OKMAGIC) */
  #define OKMAGIC(ptr_) (CALCMAGIC(ptr_) == (ptr_)->magic)


  #if defined(OKADDR)
    #undef OKADDR
  #endif /* if defined(OKADDR) */
  #define OKADDR(region_, addr_) (((const volatile Addr_t *) (addr_) >= (Addr_t *) ((region_)->mem)) && ((const volatile Addr_t *) (addr_) < \
          (Addr_t *) ((region_)->mem + MEMORY_REGION_SIZE_IN_BYTES)))


  #if defined(INUSE)
    #undef INUSE
  #endif /* if defined(INUSE) */
  #define INUSE 0xAAu /* 170 */


  #if defined(FREE)
    #undef FREE
  #endif /* if defined(FREE) */
  #define FREE 0xD5u /* 213 */


#endif /* ifndef DEFINES_H_ */