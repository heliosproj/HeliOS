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
#include <time.h>

static unit_t *unit = null;

static void output_json(void);
static void record_assertion(const char *file_, int line_, const char *message_);


void unit_init(void) {
  unit_init_with_options(false, null);
  return;
}


void unit_init_with_options(int32_t json_enabled_, const char *json_file_) {
  if(null == unit) {
    unit = (unit_t *) calloc(1, sizeof(unit_t));
    unit->json_enabled = json_enabled_;
    unit->json_file = json_file_;
    unit->test_count = 0;
    unit->current_test_assertions = 0;

    if(!json_enabled_) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[92m initializing...\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: initializing...\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  }

  return;
}


void unit_begin(const char *name_) {
  if((null != unit) && (null != name_) && (false == unit->begun)) {
    strncpy(unit->name, name_, UNIT_NAME_LENGTH);
    unit->begun = true;
    unit->failed = false;
    unit->current_test_assertions = 0;

    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[92m begin: %s\n\033[39m", unit->name);
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: begin: %s\n", unit->name);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  } else if(true == unit->begun) {
    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_begin() called inside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_begin() called inside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  }

  return;
}


void unit_end(void) {
  if((null != unit) && (true == unit->begun)) {
    /* Store test result for JSON output */
    if(unit->test_count < MAX_TESTS) {
      test_result_t *test = &unit->tests[unit->test_count];
      strncpy(test->name, unit->name, UNIT_NAME_LENGTH);
      test->passed = !unit->failed;
      test->assertion_count = unit->current_test_assertions;
      unit->test_count++;
    }

    if(true == unit->failed) {
      if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m end: %s failed\n\033[39m", unit->name);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: end: %s failed\n", unit->name);
#endif /* if defined(UNIT_TEST_COLORIZE) */
      }
      unit->fail++;
    } else {
      if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[92m end: %s\n\033[39m", unit->name);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: end: %s\n", unit->name);
#endif /* if defined(UNIT_TEST_COLORIZE) */
      }
      unit->pass++;
    }

    memset(unit->name, 0x0u, UNIT_NAME_LENGTH);
    unit->begun = false;
    unit->failed = false;
  } else if(false == unit->begun) {
    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m error: unit_end() called outside a unit test\n\033[39m");
#else  /* if defined(UNIT_TEST_COLORIZE) */
      printf("unit: error: unit_end() called outside a unit test\n");
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }
  }

  return;
}


void unit_exit(void) {
  int32_t exit_code;

  if(null != unit) {
    if(unit->json_enabled) {
      output_json();
    } else {
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
    }

    exit_code = (0x0 < unit->fail) ? 0x1 : 0x0;
    free(unit);
    exit(exit_code);
  }

  return;
}


void unit_print(const char *msg_) {
  if((null != unit) && (null != msg_) && !unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
    printf("\033[95munit:\033[96m %s\n\033[39m", msg_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
    printf("unit: %s\n", msg_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
  }

  return;
}


void unit_try(int expr_) {
  if((null != unit) && (true == unit->begun)) {
    if(0x0 == expr_) {
      unit->failed = true;
    }
  }

  return;
}


/* Enhanced assertion function implementations */
void unit_assert_equal_impl(const char *file_, int line_, unsigned long actual_, unsigned long expected_, const char *actual_expr_, const char *expected_expr_) {
  if(actual_ != expected_) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, MAX_MESSAGE_LENGTH, "Expected %s (%lu) == %s (%lu)", actual_expr_, actual_, expected_expr_, expected_);

    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
      printf("\033[95munit:\033[91m   %s\n\033[39m", message);
#else
      printf("unit: assert failed at %s:%d\n", file_, line_);
      printf("unit:    %s\n", message);
#endif
    }

    record_assertion(file_, line_, message);
    unit->failed = true;
  }
}


void unit_assert_not_equal_impl(const char *file_, int line_, unsigned long actual_, unsigned long expected_, const char *actual_expr_, const char *expected_expr_) {
  if(actual_ == expected_) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, MAX_MESSAGE_LENGTH, "Expected %s (%lu) != %s (%lu)", actual_expr_, actual_, expected_expr_, expected_);

    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
      printf("\033[95munit:\033[91m   %s\n\033[39m", message);
#else
      printf("unit: assert failed at %s:%d\n", file_, line_);
      printf("unit:    %s\n", message);
#endif
    }

    record_assertion(file_, line_, message);
    unit->failed = true;
  }
}


void unit_assert_null_impl(const char *file_, int line_, const void *ptr_, const char *ptr_expr_) {
  if(ptr_ != null) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, MAX_MESSAGE_LENGTH, "Expected %s to be NULL", ptr_expr_);

    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
      printf("\033[95munit:\033[91m   %s\n\033[39m", message);
#else
      printf("unit: assert failed at %s:%d\n", file_, line_);
      printf("unit:    %s\n", message);
#endif
    }

    record_assertion(file_, line_, message);
    unit->failed = true;
  }
}


void unit_assert_not_null_impl(const char *file_, int line_, const void *ptr_, const char *ptr_expr_) {
  if(ptr_ == null) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, MAX_MESSAGE_LENGTH, "Expected %s to not be NULL", ptr_expr_);

    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
      printf("\033[95munit:\033[91m   %s\n\033[39m", message);
#else
      printf("unit: assert failed at %s:%d\n", file_, line_);
      printf("unit:    %s\n", message);
#endif
    }

    record_assertion(file_, line_, message);
    unit->failed = true;
  }
}


void unit_assert_true_impl(const char *file_, int line_, int condition_, const char *condition_expr_) {
  if(!condition_) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, MAX_MESSAGE_LENGTH, "Expected %s to be true", condition_expr_);

    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
      printf("\033[95munit:\033[91m   %s\n\033[39m", message);
#else
      printf("unit: assert failed at %s:%d\n", file_, line_);
      printf("unit:    %s\n", message);
#endif
    }

    record_assertion(file_, line_, message);
    unit->failed = true;
  }
}


void unit_assert_false_impl(const char *file_, int line_, int condition_, const char *condition_expr_) {
  if(condition_) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, MAX_MESSAGE_LENGTH, "Expected %s to be false", condition_expr_);

    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
      printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
      printf("\033[95munit:\033[91m   %s\n\033[39m", message);
#else
      printf("unit: assert failed at %s:%d\n", file_, line_);
      printf("unit:    %s\n", message);
#endif
    }

    record_assertion(file_, line_, message);
    unit->failed = true;
  }
}


static void record_assertion(const char *file_, int line_, const char *message_) {
  if(unit && unit->test_count < MAX_TESTS && unit->current_test_assertions < MAX_ASSERTIONS_PER_TEST) {
    test_result_t *test = &unit->tests[unit->test_count];
    test_assertion_t *assertion = &test->assertions[unit->current_test_assertions];

    strncpy(assertion->file, file_, 255);
    assertion->file[255] = '\0';
    assertion->line = line_;
    strncpy(assertion->message, message_, MAX_MESSAGE_LENGTH - 1);
    assertion->message[MAX_MESSAGE_LENGTH - 1] = '\0';

    unit->current_test_assertions++;
  }
}


static void output_json(void) {
  FILE *output = stdout;
  time_t rawtime;
  struct tm *timeinfo;
  char timestamp[64];
  int32_t i, j;

  if(unit->json_file) {
    output = fopen(unit->json_file, "w");
    if(!output) {
      fprintf(stderr, "Failed to open JSON output file: %s\n", unit->json_file);
      output = stdout;
    }
  }

  /* Get current timestamp */
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", timeinfo);

  /* Output JSON */
  fprintf(output, "{\n");
  fprintf(output, "  \"timestamp\": \"%s\",\n", timestamp);
  fprintf(output, "  \"summary\": {\n");
  fprintf(output, "    \"total\": %d,\n", unit->pass + unit->fail);
  fprintf(output, "    \"passed\": %d,\n", unit->pass);
  fprintf(output, "    \"failed\": %d\n", unit->fail);
  fprintf(output, "  },\n");
  fprintf(output, "  \"tests\": [\n");

  for(i = 0; i < unit->test_count; i++) {
    test_result_t *test = &unit->tests[i];
    fprintf(output, "    {\n");
    fprintf(output, "      \"name\": \"%s\",\n", test->name);
    fprintf(output, "      \"status\": \"%s\"", test->passed ? "passed" : "failed");

    if(test->assertion_count > 0) {
      fprintf(output, ",\n");
      fprintf(output, "      \"assertions\": [\n");
      for(j = 0; j < test->assertion_count; j++) {
        test_assertion_t *assertion = &test->assertions[j];
        fprintf(output, "        {\n");
        fprintf(output, "          \"file\": \"%s\",\n", assertion->file);
        fprintf(output, "          \"line\": %d,\n", assertion->line);
        fprintf(output, "          \"message\": \"%s\"\n", assertion->message);
        fprintf(output, "        }%s\n", (j < test->assertion_count - 1) ? "," : "");
      }
      fprintf(output, "      ]\n");
    } else {
      fprintf(output, "\n");
    }

    fprintf(output, "    }%s\n", (i < unit->test_count - 1) ? "," : "");
  }

  fprintf(output, "  ]\n");
  fprintf(output, "}\n");

  if(output != stdout) {
    fclose(output);
  }
}