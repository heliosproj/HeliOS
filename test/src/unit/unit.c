/*UNCRUSTIFY-OFF*/
/**
 * @file unit.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief A simple unit testing framework for HeliOS
 * @version 0.4.0
 * @date 2022-08-23
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
#include "unit.h"

static unit_t *unit = null;


void unit_init(void) {
  if(null == unit) {
    unit = (unit_t *) calloc(1, sizeof(unit_t));
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[92munit: initializing...\n\033[39m");
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
      printf("\033[92munit: begin: %s\n\033[39m", unit->name);
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: begin: %s\n", unit->name);
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_try(int expr_) {
  if((null != unit) && (true == unit->begun) && (false == expr_)) {
    unit->failed = true;
  }

  return;
}


void unit_end(void) {
  if((null != unit) && (true == unit->begun)) {
    if(true == unit->failed) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[91munit: end: %s failed\n\033[39m", unit->name);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: end: %s failed\n", unit->name);
#endif /* if defined(UNIT_TEST_COLORIZE) */
      unit->fail++;
    } else {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[92munit: end: %s\n\033[39m", unit->name);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: end: %s\n", unit->name);
#endif /* if defined(UNIT_TEST_COLORIZE) */
      unit->pass++;
    }

    memset(unit->name, 0x0, UNIT_NAME_LENGTH);
    unit->begun = false;
    unit->failed = false;
  }

  return;
}


void unit_exit(void) {
  if(null != unit) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[92munit: failed = %" PRId32 "\n\033[39m", unit->fail);
      printf("\033[92munit: passed = %" PRId32 "\n\033[39m", unit->pass);
      printf("\033[92munit: total = %" PRId32 "\n\033[39m", unit->fail + unit->pass);
      printf("\033[92munit: exiting...\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: failed = %" PRId32 "\n", unit->fail);
      printf("unit: passed = %" PRId32 "\n", unit->pass);
      printf("unit: total = %" PRId32 "\n", unit->fail + unit->pass);
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
    printf("unit: %s\n", msg_);
  }

  return;
}