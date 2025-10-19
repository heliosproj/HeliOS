/*UNCRUSTIFY-OFF*/
/**
 * @file unit.c
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
#include "unit.h"

static unit_t *unit = null;


void unit_init(void) {
  if(null == unit) {
    unit = (unit_t *) calloc(1, sizeof(unit_t));
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[92m initializing...\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: initializing...\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_begin(const char *name_) {
  if((null != unit) && (null != name_) && (false == unit->begun)) {
    strncpy(unit->name, name_, UNIT_NAME_LENGTH);
    unit->begun = true;
    unit->failed = false;
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[92m begin: %s\n\033[39m", unit->name);
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: begin: %s\n", unit->name);
#endif /* if defined(UNIT_TEST_COLORIZE) */
  } else if(true == unit->begun) {

#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_begin() called inside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_begin() called inside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}



void unit_end(void) {
  if((null != unit) && (true == unit->begun)) {
    if(true == unit->failed) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m end: %s failed\n\033[39m", unit->name);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: end: %s failed\n", unit->name);
#endif /* if defined(UNIT_TEST_COLORIZE) */
      unit->fail++;
    } else {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[92m end: %s\n\033[39m", unit->name);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: end: %s\n", unit->name);
#endif /* if defined(UNIT_TEST_COLORIZE) */
      unit->pass++;
    }

    memset(unit->name, 0x0, UNIT_NAME_LENGTH);
    unit->begun = false;
    unit->failed = false;
  } else if(false == unit->begun) {

#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_end() called outside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_end() called outside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_exit(void) {
  if(null != unit) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[92m failed:\033[95m %" PRId32 "\n\033[39m", unit->fail);
      printf("\033[95munit:\033[92m passed:\033[95m %" PRId32 "\n\033[39m", unit->pass);
      printf("\033[95munit:\033[92m total:\033[95m %" PRId32 "\n\033[39m", unit->fail + unit->pass);
      printf("\033[95munit:\033[92m exiting...\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: failed: %" PRId32 "\n", unit->fail);
      printf("unit: passed: %" PRId32 "\n", unit->pass);
      printf("unit: total: %" PRId32 "\n", unit->fail + unit->pass);
      printf("unit: exiting...\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
    free(unit);

    if(0x0 < unit->fail) {
      exit(0x1);
    } else {
      exit(0x0);
    }
  }

  return;
}


void unit_print(const char *msg_) {
  if((null != unit) && (true == unit->begun) && (null != msg_)) {

#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[96m %s\n\033[39m", msg_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: %s\n", msg_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_assert_equal_impl(const char *file_, int line_, unsigned long actual_, unsigned long expected_, const char *actual_expr_, const char *expected_expr_)
    {
  if((null != unit) && (true == unit->begun)) {
    if(actual_ != expected_) {
      unit->failed = true;
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m FAILED at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected: %s == %s\n", actual_expr_, expected_expr_);
        printf("\033[95munit:\033[91m   Actual:   0x%lX != 0x%lX\n\033[39m", actual_, expected_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: FAILED at %s:%d\n", file_, line_);
        printf("unit:    Expected: %s == %s\n", actual_expr_, expected_expr_);
        printf("unit:    Actual:   0x%lX != 0x%lX\n", actual_, expected_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  } else if(false == unit->begun) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_assert_equal() called outside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_assert_equal() called outside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_assert_not_equal_impl(const char *file_, int line_, unsigned long actual_, unsigned long expected_, const char *actual_expr_, const char *
  expected_expr_) {
  if((null != unit) && (true == unit->begun)) {
    if(actual_ == expected_) {
      unit->failed = true;
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m FAILED at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected: %s != %s\n", actual_expr_, expected_expr_);
        printf("\033[95munit:\033[91m   Actual:   0x%lX == 0x%lX\n\033[39m", actual_, expected_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: FAILED at %s:%d\n", file_, line_);
        printf("unit:    Expected: %s != %s\n", actual_expr_, expected_expr_);
        printf("unit:    Actual:   0x%lX == 0x%lX\n", actual_, expected_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  } else if(false == unit->begun) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_assert_not_equal() called outside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_assert_not_equal() called outside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_assert_null_impl(const char *file_, int line_, const void *ptr_, const char *ptr_expr_) {
  if((null != unit) && (true == unit->begun)) {
    if(null != ptr_) {
      unit->failed = true;
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m FAILED at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected: %s == NULL\n", ptr_expr_);
        printf("\033[95munit:\033[91m   Actual:   %p != NULL\n\033[39m", ptr_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: FAILED at %s:%d\n", file_, line_);
        printf("unit:    Expected: %s == NULL\n", ptr_expr_);
        printf("unit:    Actual:   %p != NULL\n", ptr_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  } else if(false == unit->begun) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_assert_null() called outside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_assert_null() called outside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_assert_not_null_impl(const char *file_, int line_, const void *ptr_, const char *ptr_expr_) {
  if((null != unit) && (true == unit->begun)) {
    if(null == ptr_) {
      unit->failed = true;
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m FAILED at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected: %s != NULL\n", ptr_expr_);
        printf("\033[95munit:\033[91m   Actual:   NULL\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: FAILED at %s:%d\n", file_, line_);
        printf("unit:    Expected: %s != NULL\n", ptr_expr_);
        printf("unit:    Actual:   NULL\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  } else if(false == unit->begun) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_assert_not_null() called outside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_assert_not_null() called outside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_assert_true_impl(const char *file_, int line_, int condition_, const char *condition_expr_) {
  if((null != unit) && (true == unit->begun)) {
    if(false == condition_) {
      unit->failed = true;
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m FAILED at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected: %s == true\n", condition_expr_);
        printf("\033[95munit:\033[91m   Actual:   false\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: FAILED at %s:%d\n", file_, line_);
        printf("unit:    Expected: %s == true\n", condition_expr_);
        printf("unit:    Actual:   false\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  } else if(false == unit->begun) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_assert_true() called outside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_assert_true() called outside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_assert_false_impl(const char *file_, int line_, int condition_, const char *condition_expr_) {
  if((null != unit) && (true == unit->begun)) {
    if(true == condition_) {
      unit->failed = true;
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m FAILED at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected: %s == false\n", condition_expr_);
        printf("\033[95munit:\033[91m   Actual:   true\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: FAILED at %s:%d\n", file_, line_);
        printf("unit:    Expected: %s == false\n", condition_expr_);
        printf("unit:    Actual:   true\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  } else if(false == unit->begun) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_assert_false() called outside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_assert_false() called outside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}