/*UNCRUSTIFY-OFF*/
/**
 * @file unit.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "unit.h"
#include <time.h>


/* Global test framework state */
static unit_t *unit = null;
/* Forward declarations */
static void output_json(void);
static void json_write_string(FILE *output, const char *str);
static void record_assertion_structured(const char *file_, int line_, assertion_type_t type_, int32_t passed_, const char *actual_expr_, unsigned long
  actual_val_, const char *expected_expr_, unsigned long expected_val_, const char *condition_expr_, int condition_val_);


/*
 * Test framework initialization functions
 */


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


/*
 * Test control functions
 */


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


/*
 * Utility functions
 */


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


/*
 * Assertion function implementations These functions are called by the
 * assertion macros in unit.h
 */
void unit_assert_equal_impl(const char *file_, int line_, unsigned long actual_, unsigned long expected_, const char *actual_expr_, const char *expected_expr_)
    {
  int32_t passed = (actual_ == expected_);


  if(!passed) {
    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected %s (%lu) == %s (%lu)\n\033[39m", actual_expr_, actual_, expected_expr_, expected_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: assert failed at %s:%d\n", file_, line_);
        printf("unit:    Expected %s (%lu) == %s (%lu)\n", actual_expr_, actual_, expected_expr_, expected_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }

    unit->failed = true;
  }

  record_assertion_structured(file_, line_, ASSERT_EQUAL, passed, actual_expr_, actual_, expected_expr_, expected_, null, 0);
}


void unit_assert_not_equal_impl(const char *file_, int line_, unsigned long actual_, unsigned long expected_, const char *actual_expr_, const char *
  expected_expr_) {
  int32_t passed = (actual_ != expected_);


  if(!passed) {
    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected %s (%lu) != %s (%lu)\n\033[39m", actual_expr_, actual_, expected_expr_, expected_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: assert failed at %s:%d\n", file_, line_);
        printf("unit:    Expected %s (%lu) != %s (%lu)\n", actual_expr_, actual_, expected_expr_, expected_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }

    unit->failed = true;
  }

  record_assertion_structured(file_, line_, ASSERT_NOT_EQUAL, passed, actual_expr_, actual_, expected_expr_, expected_, null, 0);
}


void unit_assert_null_impl(const char *file_, int line_, const void *ptr_, const char *ptr_expr_) {
  int32_t passed = (ptr_ == null);


  if(!passed) {
    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected %s to be NULL\n\033[39m", ptr_expr_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: assert failed at %s:%d\n", file_, line_);
        printf("unit:    Expected %s to be NULL\n", ptr_expr_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }

    unit->failed = true;
  }

  record_assertion_structured(file_, line_, ASSERT_NULL, passed, ptr_expr_, (unsigned long) ptr_, "NULL", 0, null, 0);
}


void unit_assert_not_null_impl(const char *file_, int line_, const void *ptr_, const char *ptr_expr_) {
  int32_t passed = (ptr_ != null);


  if(!passed) {
    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected %s to not be NULL\n\033[39m", ptr_expr_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: assert failed at %s:%d\n", file_, line_);
        printf("unit:    Expected %s to not be NULL\n", ptr_expr_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }

    unit->failed = true;
  }

  record_assertion_structured(file_, line_, ASSERT_NOT_NULL, passed, ptr_expr_, (unsigned long) ptr_, "NOT_NULL", 1, null, 0);
}


void unit_assert_true_impl(const char *file_, int line_, int condition_, const char *condition_expr_) {
  int32_t passed = (condition_ != 0);


  if(!passed) {
    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected %s to be true\n\033[39m", condition_expr_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: assert failed at %s:%d\n", file_, line_);
        printf("unit:    Expected %s to be true\n", condition_expr_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }

    unit->failed = true;
  }

  record_assertion_structured(file_, line_, ASSERT_TRUE, passed, null, 0, null, 0, condition_expr_, condition_);
}


void unit_assert_false_impl(const char *file_, int line_, int condition_, const char *condition_expr_) {
  int32_t passed = (condition_ == 0);


  if(!passed) {
    if(!unit->json_enabled) {
#if defined(UNIT_TEST_COLORIZE)
        printf("\033[95munit:\033[91m assert failed at %s:%d\n", file_, line_);
        printf("\033[95munit:\033[91m   Expected %s to be false\n\033[39m", condition_expr_);
#else  /* if defined(UNIT_TEST_COLORIZE) */
        printf("unit: assert failed at %s:%d\n", file_, line_);
        printf("unit:    Expected %s to be false\n", condition_expr_);
#endif /* if defined(UNIT_TEST_COLORIZE) */
    }

    unit->failed = true;
  }

  record_assertion_structured(file_, line_, ASSERT_FALSE, passed, null, 0, null, 0, condition_expr_, condition_);
}


/* Record detailed assertion information for JSON output */
static void record_assertion_structured(const char *file_, int line_, assertion_type_t type_, int32_t passed_, const char *actual_expr_, unsigned long
  actual_val_, const char *expected_expr_, unsigned long expected_val_, const char *condition_expr_, int condition_val_) {
  if(unit && (unit->test_count < MAX_TESTS) && (unit->current_test_assertions < MAX_ASSERTIONS_PER_TEST)) {
    test_result_t *test = &unit->tests[unit->test_count];
    test_assertion_t *assertion = &test->assertions[unit->current_test_assertions];


    strncpy(assertion->file, file_, MAX_FILE_PATH_LENGTH - 1);
    assertion->file[MAX_FILE_PATH_LENGTH - 1] = '\0';
    assertion->line = line_;
    assertion->type = type_;
    assertion->passed = passed_;

    /* Store actual/expected values and expressions */
    if(actual_expr_) {
      strncpy(assertion->actual_expr, actual_expr_, MAX_EXPRESSION_LENGTH - 1);
      assertion->actual_expr[MAX_EXPRESSION_LENGTH - 1] = '\0';
    } else {
      assertion->actual_expr[0] = '\0';
    }

    if(expected_expr_) {
      strncpy(assertion->expected_expr, expected_expr_, MAX_EXPRESSION_LENGTH - 1);
      assertion->expected_expr[MAX_EXPRESSION_LENGTH - 1] = '\0';
    } else {
      assertion->expected_expr[0] = '\0';
    }

    assertion->actual_value = actual_val_;
    assertion->expected_value = expected_val_;

    /* Store condition expression for boolean assertions */
    if(condition_expr_) {
      strncpy(assertion->condition_expr, condition_expr_, MAX_EXPRESSION_LENGTH - 1);
      assertion->condition_expr[MAX_EXPRESSION_LENGTH - 1] = '\0';
    } else {
      assertion->condition_expr[0] = '\0';
    }

    assertion->condition_value = condition_val_;
    unit->current_test_assertions++;
  }
}


/*
 * JSON output helper functions
 */


/* Write a properly JSON-escaped string to the output */
static void json_write_string(FILE *output, const char *str) {
  const char *p;


  if(!str) {
    fprintf(output, "null");

    return;
  }

  fputc('"', output);

  for(p = str; *p; p++) {
    switch(*p) {
    case '"': fprintf(output, "\\\"");
      break;
    case '\\': fprintf(output, "\\\\");
      break;
    case '\b': fprintf(output, "\\b");
      break;
    case '\f': fprintf(output, "\\f");
      break;
    case '\n': fprintf(output, "\\n");
      break;
    case '\r': fprintf(output, "\\r");
      break;
    case '\t': fprintf(output, "\\t");
      break;
    default:

      if((*p >= 32) && (*p <= 126)) {
        fputc(*p, output);
      } else {
        fprintf(output, "\\u%04x", (unsigned char) *p);
      }
    }
  }

  fputc('"', output);
}


/* Generate the complete JSON test report */
static void output_json(void) {
  FILE *output = stdout;
  time_t rawtime;
  struct tm *timeinfo;
  char timestamp[64];
  int32_t i, j;
  double pass_rate;
  int32_t total;


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


  /* Calculate pass rate */
  total = unit->pass + unit->fail;
  pass_rate = (total > 0) ? (double) unit->pass / total : 0.0;


  /* Output structured JSON */
  fprintf(output, "{\n");
  fprintf(output, "  \"timestamp\": \"%s\",\n", timestamp);
  fprintf(output, "  \"summary\": {\n");
  fprintf(output, "    \"total\": %d,\n", total);
  fprintf(output, "    \"passed\": %d,\n", unit->pass);
  fprintf(output, "    \"failed\": %d,\n", unit->fail);
  fprintf(output, "    \"passRate\": %.4f\n", pass_rate);
  fprintf(output, "  },\n");
  fprintf(output, "  \"tests\": [\n");

  for(i = 0; i < unit->test_count; i++) {
    test_result_t *test = &unit->tests[i];


    fprintf(output, "    {\n");
    fprintf(output, "      \"name\": ");
    json_write_string(output, test->name);
    fprintf(output, ",\n");
    fprintf(output, "      \"passed\": %s", test->passed ? "true" : "false");

    if(test->assertion_count > 0) {
      fprintf(output, ",\n");
      fprintf(output, "      \"assertions\": [\n");

      for(j = 0; j < test->assertion_count; j++) {
        test_assertion_t *assertion = &test->assertions[j];
        const char *type_str = "";


        fprintf(output, "        {\n");

        /* Assertion type as string */
        switch(assertion->type) {
        case ASSERT_EQUAL: type_str = "EQUAL";
          break;
        case ASSERT_NOT_EQUAL: type_str = "NOT_EQUAL";
          break;
        case ASSERT_NULL: type_str = "NULL";
          break;
        case ASSERT_NOT_NULL: type_str = "NOT_NULL";
          break;
        case ASSERT_TRUE: type_str = "TRUE";
          break;
        case ASSERT_FALSE: type_str = "FALSE";
          break;
        }

        fprintf(output, "          \"type\": \"%s\",\n", type_str);
        fprintf(output, "          \"passed\": %s,\n", assertion->passed ? "true" : "false");
        fprintf(output, "          \"location\": {\n");
        fprintf(output, "            \"file\": ");
        json_write_string(output, assertion->file);
        fprintf(output, ",\n");
        fprintf(output, "            \"line\": %d\n", assertion->line);
        fprintf(output, "          }");

        /* Output type-specific data */
        if((assertion->type == ASSERT_EQUAL) || (assertion->type == ASSERT_NOT_EQUAL) || (assertion->type == ASSERT_NULL) || (assertion->type == ASSERT_NOT_NULL
          )) {
          fprintf(output, ",\n");
          fprintf(output, "          \"expected\": {\n");
          fprintf(output, "            \"expression\": ");
          json_write_string(output, assertion->expected_expr);
          fprintf(output, ",\n");
          fprintf(output, "            \"value\": %lu,\n", assertion->expected_value);
          fprintf(output, "            \"type\": \"integer\"\n");
          fprintf(output, "          },\n");
          fprintf(output, "          \"actual\": {\n");
          fprintf(output, "            \"expression\": ");
          json_write_string(output, assertion->actual_expr);
          fprintf(output, ",\n");
          fprintf(output, "            \"value\": %lu,\n", assertion->actual_value);
          fprintf(output, "            \"type\": \"integer\"\n");
          fprintf(output, "          }\n");
        } else if((assertion->type == ASSERT_TRUE) || (assertion->type == ASSERT_FALSE)) {
          fprintf(output, ",\n");
          fprintf(output, "          \"condition\": {\n");
          fprintf(output, "            \"expression\": ");
          json_write_string(output, assertion->condition_expr);
          fprintf(output, ",\n");
          fprintf(output, "            \"value\": %s\n", assertion->condition_value ? "true" : "false");
          fprintf(output, "          }\n");
        } else {
          fprintf(output, "\n");
        }

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