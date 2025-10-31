/*UNCRUSTIFY-OFF*/
/**
 * @file defines.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel macro definitions and utilities
 * @details
 * Defines utility macros for pointer validation, object validation, return codes, and common kernel operations used throughout the codebase.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

  #else /* if defined(CHAR_BIT) */

    #pragma message("WARNING: Unable to determine if system has an 8-bit wide byte. CHAR_BIT not defined?")

  #endif /* if defined(CHAR_BIT) */

  #if !defined(true)

    #define true 0xFFu

  #endif /* if !defined(true) */

  #if !defined(false)

    #define false 0x00u

  #endif /* if !defined(false) */

  #if !defined(null)

    #if defined(__cplusplus)

      #define null 0x0u

    #else /* if defined(__cplusplus) */

      #define null ((void *) 0x0)

    #endif /* if defined(__cplusplus) */

  #endif /* if !defined(null) */

  #if !defined(UINT8_TYPE)

    #define UINT8_TYPE uint8_t

  #endif /* if !defined(UINT8_TYPE) */

  #if !defined(UINT16_TYPE)

    #define UINT16_TYPE uint16_t

  #endif /* if !defined(UINT16_TYPE) */

  #if !defined(UINT32_TYPE)

    #define UINT32_TYPE uint32_t

  #endif /* if !defined(UINT32_TYPE) */

  #if !defined(SIZE_TYPE)

    #define SIZE_TYPE size_t

  #endif /* if !defined(SIZE_TYPE) */

  #if !defined(VOID_TYPE)

    #define VOID_TYPE void

  #endif /* if !defined(VOID_TYPE) */

  #if !defined(OS_PRODUCT_NAME_SIZE)

    #define OS_PRODUCT_NAME_SIZE 0x6u

  #endif /* if !defined(OS_PRODUCT_NAME_SIZE) */

  #if defined(OS_MAJOR_VERSION_NO)

    #undef OS_MAJOR_VERSION_NO

  #endif /* if defined(OS_MAJOR_VERSION_NO) */

  #define OS_MAJOR_VERSION_NO 0x0u

  #if defined(OS_MINOR_VERSION_NO)

    #undef OS_MINOR_VERSION_NO

  #endif /* if defined(OS_MINOR_VERSION_NO) */

  #define OS_MINOR_VERSION_NO 0x5u

  #if defined(OS_PATCH_VERSION_NO)

    #undef OS_PATCH_VERSION_NO

  #endif /* if defined(OS_PATCH_VERSION_NO) */

  #define OS_PATCH_VERSION_NO 0x0u

  #if defined(__STRINGIFY__)

    #undef __STRINGIFY__

  #endif /* if defined(__STRINGIFY__) */

  #define __STRINGIFY__(x) #x

  #if defined(__TOSTRING__)

    #undef __TOSTRING__

  #endif /* if defined(__TOSTRING__) */

  #define __TOSTRING__(x) __STRINGIFY__(x)

  #if defined(OS_VERSION_STRING)

    #undef OS_VERSION_STRING

  #endif /* if defined(OS_VERSION_STRING) */

  #define OS_VERSION_STRING __TOSTRING__(OS_MAJOR_VERSION_NO) "." __TOSTRING__(OS_MINOR_VERSION_NO) "." __TOSTRING__(OS_PATCH_VERSION_NO)

  #if !defined(MEMORY_REGION_SIZE)

    #define MEMORY_REGION_SIZE CONFIG_MEMORY_REGION_SIZE

  #endif /* if !defined(MEMORY_REGION_SIZE) */

  #if !defined(OK)

    #define OK(expr_) (ReturnOK == (expr_))

  #endif /* if !defined(OK) */

  #if !defined(ERROR)

    #define ERROR(expr_) (ReturnError == (expr_))

  #endif /* if !defined(ERROR) */

  #if !defined(FUNCTION_ENTER)

    #define FUNCTION_ENTER Return_t ret = ReturnError

  #endif /* if !defined(FUNCTION_ENTER) */

  #if !defined(FUNCTION_EXIT)

    #define FUNCTION_EXIT return(ret)

  #endif /* if !defined(FUNCTION_EXIT) */

  #if !defined(__PointerIsNotNull__)

    #define __PointerIsNotNull__(addr_) (null != (addr_))

  #endif /* if !defined(__PointerIsNotNull__) */

  #if !defined(__PointerIsNull__)

    #define __PointerIsNull__(addr_) (null == (addr_))

  #endif /* if !defined(__PointerIsNull__) */

  #if !defined(__ReturnOk__)

    #define __ReturnOk__() ret = ReturnOK

  #endif /* if !defined(__ReturnOk__) */

  #if defined(VALID)

    #undef VALID

  #endif /* if defined(VALID) */

  #define VALID 0xAAu

  #if defined(INVALID)

    #undef INVALID

  #endif /* if defined(INVALID) */

  #define INVALID 0x55u

  #if !defined(__ObjectIsValid__)

    #define __ObjectIsValid__(obj_) (__PointerIsNotNull__(obj_) && (VALID == (obj_)->valid))

  #endif /* if !defined(__ObjectIsValid__) */

  #if !defined(__ObjectIsNotValid__)

    #define __ObjectIsNotValid__(obj_) (!__ObjectIsValid__(obj_))

  #endif /* if !defined(__ObjectIsNotValid__) */

  #if !defined(__AssertOnElse__)

    #if defined(CONFIG_ENABLE_SYSTEM_ASSERT)

      #define __AssertOnElse__() xSystemAssert(__FILE__, __LINE__)

    #else /* if defined(CONFIG_ENABLE_SYSTEM_ASSERT) */

      #define __AssertOnElse__()

    #endif /* if defined(CONFIG_ENABLE_SYSTEM_ASSERT) */

  #endif /* if !defined(__AssertOnElse__) */

#endif /* ifndef DEFINES_H_ */