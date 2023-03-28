/*UNCRUSTIFY-OFF*/
/**
 * @file unit.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
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


void unit_try(int expr_) {
  if((null != unit) && (true == unit->begun) && (false == expr_)) {
    unit->failed = true;
  } else if(false == unit->begun) {

#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_try() called outside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_try() called outside a unit test\n");
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
      printf("\033[92munit: %s\n\033[39m", msg_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: %s\n", msg_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}