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

  /* Test framework limits */
  #define MAX_TESTS 1000
  #define MAX_ASSERTIONS_PER_TEST 100
  #define MAX_EXPRESSION_LENGTH 128
  #define MAX_FILE_PATH_LENGTH 256

  typedef enum {
    ASSERT_EQUAL,
    ASSERT_NOT_EQUAL,
    ASSERT_NULL,
    ASSERT_NOT_NULL,
    ASSERT_TRUE,
    ASSERT_FALSE
  } assertion_type_t;

  typedef struct test_assertion_s {
    char file[MAX_FILE_PATH_LENGTH];
    int line;
    assertion_type_t type;
    int32_t passed;
    /* For value comparison assertions (EQUAL, NOT_EQUAL, NULL, NOT_NULL) */
    char actual_expr[MAX_EXPRESSION_LENGTH];
    char expected_expr[MAX_EXPRESSION_LENGTH];
    unsigned long actual_value;
    unsigned long expected_value;
    /* For boolean assertions (TRUE, FALSE) */
    char condition_expr[MAX_EXPRESSION_LENGTH];
    int condition_value;
  } test_assertion_t;

  typedef struct test_result_s {
    char name[UNIT_NAME_LENGTH];
    int32_t passed;
    test_assertion_t assertions[MAX_ASSERTIONS_PER_TEST];
    int32_t assertion_count;
  } test_result_t;

  typedef struct unit_s {
    char name[UNIT_NAME_LENGTH];
    int32_t begun;
    int32_t failed;
    int32_t pass;
    int32_t fail;
    /* JSON output fields */
    int32_t json_enabled;
    const char *json_file;
    test_result_t tests[MAX_TESTS];
    int32_t test_count;
    int32_t current_test_assertions;
  } unit_t;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */

  /* Test framework initialization and control */
  void unit_init(void);
  void unit_init_with_options(int32_t json_enabled_, const char *json_file_);
  void unit_begin(const char *name_);
  void unit_end(void);
  void unit_exit(void);

  /* Legacy test function */
  void unit_try(int expr_);

  /* Output function */
  void unit_print(const char *msg_);

  /* Assertion implementation functions (called by macros) */
  void unit_assert_equal_impl(const char *file_, int line_, unsigned long actual_,
                              unsigned long expected_, const char *actual_expr_,
                              const char *expected_expr_);
  void unit_assert_not_equal_impl(const char *file_, int line_, unsigned long actual_,
                                  unsigned long expected_, const char *actual_expr_,
                                  const char *expected_expr_);
  void unit_assert_null_impl(const char *file_, int line_, const void *ptr_,
                             const char *ptr_expr_);
  void unit_assert_not_null_impl(const char *file_, int line_, const void *ptr_,
                                 const char *ptr_expr_);
  void unit_assert_true_impl(const char *file_, int line_, int condition_,
                             const char *condition_expr_);
  void unit_assert_false_impl(const char *file_, int line_, int condition_,
                              const char *condition_expr_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
  /* Assertion macros - these are the main interface for tests */
  #define unit_assert_equal(actual, expected) \
          unit_assert_equal_impl(__FILE__, __LINE__, \
            (unsigned long) (actual), \
            (unsigned long) (expected), \
            #actual, #expected)

  #define unit_assert_not_equal(actual, expected) \
          unit_assert_not_equal_impl(__FILE__, __LINE__, \
            (unsigned long) (actual), \
            (unsigned long) (expected), \
            #actual, #expected)

  #define unit_assert_null(ptr) \
          unit_assert_null_impl(__FILE__, __LINE__, (const void *) (ptr), #ptr)

  #define unit_assert_not_null(ptr) \
          unit_assert_not_null_impl(__FILE__, __LINE__, (const void *) (ptr), #ptr)

  #define unit_assert_true(condition) \
          unit_assert_true_impl(__FILE__, __LINE__, (condition), #condition)

  #define unit_assert_false(condition) \
          unit_assert_false_impl(__FILE__, __LINE__, (condition), #condition)

  #define unit_assert_ok(result) \
          unit_assert_true_impl(__FILE__, __LINE__, OK(result), #result)

  #define unit_assert_not_ok(result) \
          unit_assert_false_impl(__FILE__, __LINE__, OK(result), #result)


#endif /* ifndef UNIT_H_ */