/*UNCRUSTIFY-OFF*/
/**
 * @file unit.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#ifndef UNIT_H_
  #define UNIT_H_

  #include "posix.h"
  #include "defines.h"

  #include <stdint.h>
  #include <inttypes.h>
  #include <string.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <unistd.h>

  #if defined(UNIT_NAME_LENGTH)
    #undef UNIT_NAME_LENGTH
  #endif /* if defined(UNIT_NAME_LENGTH) */
  #define UNIT_NAME_LENGTH 0x40 /* 64 */
  typedef struct unit_s {
    char name[UNIT_NAME_LENGTH];
    int32_t begun;
    int32_t failed;
    int32_t pass;
    int32_t fail;
  } unit_t;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  void unit_init(void);
  void unit_begin(const char *name_);
  void unit_try(int expr_);
  void unit_end(void);
  void unit_exit(void);
  void unit_print(const char *msg_);

  /* Enhanced assertion functions */
  void unit_assert_equal_impl(const char *file_, int line_,
                              unsigned long actual_, unsigned long expected_,
                              const char *actual_expr_, const char *expected_expr_);
  void unit_assert_not_equal_impl(const char *file_, int line_,
                                  unsigned long actual_, unsigned long expected_,
                                  const char *actual_expr_, const char *expected_expr_);
  void unit_assert_null_impl(const char *file_, int line_,
                             const void *ptr_, const char *ptr_expr_);
  void unit_assert_not_null_impl(const char *file_, int line_,
                                 const void *ptr_, const char *ptr_expr_);
  void unit_assert_true_impl(const char *file_, int line_,
                             int condition_, const char *condition_expr_);
  void unit_assert_false_impl(const char *file_, int line_,
                              int condition_, const char *condition_expr_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */

  /* Enhanced assertion macros */
  #define unit_assert_equal(actual, expected) \
    unit_assert_equal_impl(__FILE__, __LINE__, \
                           (unsigned long)(actual), \
                           (unsigned long)(expected), \
                           #actual, #expected)

  #define unit_assert_not_equal(actual, expected) \
    unit_assert_not_equal_impl(__FILE__, __LINE__, \
                               (unsigned long)(actual), \
                               (unsigned long)(expected), \
                               #actual, #expected)

  #define unit_assert_null(ptr) \
    unit_assert_null_impl(__FILE__, __LINE__, (const void *)(ptr), #ptr)

  #define unit_assert_not_null(ptr) \
    unit_assert_not_null_impl(__FILE__, __LINE__, (const void *)(ptr), #ptr)

  #define unit_assert_true(condition) \
    unit_assert_true_impl(__FILE__, __LINE__, (condition), #condition)

  #define unit_assert_false(condition) \
    unit_assert_false_impl(__FILE__, __LINE__, (condition), #condition)

  #define unit_assert_ok(result) \
    unit_assert_true_impl(__FILE__, __LINE__, OK(result), #result)

  #define unit_assert_not_ok(result) \
    unit_assert_false_impl(__FILE__, __LINE__, OK(result), #result)


#endif /* ifndef UNIT_H_ */