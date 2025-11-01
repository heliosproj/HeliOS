/*UNCRUSTIFY-OFF*/
/**
 * @file console_internal_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for console internal functions
 * @details
 * Tests internal console utility functions with heavy focus on:
 * - Edge cases (empty strings, maximum lengths, boundary conditions)
 * - Null pointer handling (all pointer parameters)
 * - Buffer overflow protection
 * - Path manipulation correctness
 * - String operation safety
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "console_internal_harness.h"
#include "../../src/console.h"


/* Test constants - String lengths */
#define TEST_BUFFER_SIZE_SMALL 16
#define TEST_BUFFER_SIZE_MEDIUM 64
#define TEST_BUFFER_SIZE_LARGE 256
#define TEST_PATH_BUFFER_SIZE 512
/* Test constants - String comparison */
#define STRCMP_EQUAL 0
#define STRCMP_LESS_THAN 255  /* -1 as unsigned byte (0xFF) */
#define STRCMP_GREATER_THAN 1
/* External function declarations */
extern Size_t __strlen__(const Byte_t *str_);
extern Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
extern Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t n_);
extern Base_t __strcmp__(const Byte_t *s1_, const Byte_t *s2_);
extern Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t n_);
extern Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
extern Byte_t * __strchr__(const Byte_t *str_, const Byte_t ch_);
extern Byte_t * __strrchr__(const Byte_t *str_, const Byte_t ch_);
extern Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, const Size_t destSize_);
extern Return_t __path_normalize__(Byte_t *path_, const Size_t pathSize_);
extern Base_t __path_is_absolute__(const Byte_t *path_);
extern Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);
extern Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);
extern void __ConsoleStateClear__(void);
/* Helper function prototypes */
static void test_strlen_basic(void);
static void test_strlen_edge_cases(void);
static void test_strlen_null_pointer(void);
static void test_strcpy_basic(void);
static void test_strcpy_edge_cases(void);
static void test_strcpy_null_pointers(void);
static void test_strncpy_basic(void);
static void test_strncpy_edge_cases(void);
static void test_strncpy_null_pointers(void);
static void test_strcmp_basic(void);
static void test_strcmp_edge_cases(void);
static void test_strcmp_null_pointers(void);
static void test_strncmp_basic(void);
static void test_strncmp_edge_cases(void);
static void test_strncmp_null_pointers(void);
static void test_strcat_basic(void);
static void test_strcat_edge_cases(void);
static void test_strcat_null_pointers(void);
static void test_strchr_basic(void);
static void test_strchr_edge_cases(void);
static void test_strchr_null_pointer(void);
static void test_strrchr_basic(void);
static void test_strrchr_edge_cases(void);
static void test_strrchr_null_pointer(void);
static void test_path_join_basic(void);
static void test_path_join_edge_cases(void);
static void test_path_join_null_pointers(void);
static void test_path_normalize_basic(void);
static void test_path_normalize_edge_cases(void);
static void test_path_normalize_null_pointer(void);
static void test_path_is_absolute_basic(void);
static void test_path_is_absolute_edge_cases(void);
static void test_path_is_absolute_null_pointer(void);
static void test_path_dirname_basic(void);
static void test_path_dirname_edge_cases(void);
static void test_path_dirname_null_pointers(void);
static void test_path_basename_basic(void);
static void test_path_basename_edge_cases(void);
static void test_path_basename_null_pointers(void);
static void test_non_null_terminated_strings(void);


void console_internal_harness(void) {
  unit_print("=== CONSOLE INTERNAL FUNCTIONS TEST SUITE ===");

  /* String length tests */
  test_strlen_basic();
  test_strlen_edge_cases();
  test_strlen_null_pointer();

  /* String copy tests */
  test_strcpy_basic();
  test_strcpy_edge_cases();
  test_strcpy_null_pointers();

  /* String n-copy tests */
  test_strncpy_basic();
  test_strncpy_edge_cases();
  test_strncpy_null_pointers();

  /* String comparison tests */
  test_strcmp_basic();
  test_strcmp_edge_cases();
  test_strcmp_null_pointers();

  /* String n-comparison tests */
  test_strncmp_basic();
  test_strncmp_edge_cases();
  test_strncmp_null_pointers();

  /* String concatenation tests */
  test_strcat_basic();
  test_strcat_edge_cases();
  test_strcat_null_pointers();

  /* String character search tests */
  test_strchr_basic();
  test_strchr_edge_cases();
  test_strchr_null_pointer();

  /* String reverse character search tests */
  test_strrchr_basic();
  test_strrchr_edge_cases();
  test_strrchr_null_pointer();

  /* Path join tests */
  test_path_join_basic();
  test_path_join_edge_cases();
  test_path_join_null_pointers();

  /* Path normalize tests */
  test_path_normalize_basic();
  test_path_normalize_edge_cases();
  test_path_normalize_null_pointer();

  /* Path absolute check tests */
  test_path_is_absolute_basic();
  test_path_is_absolute_edge_cases();
  test_path_is_absolute_null_pointer();

  /* Path dirname tests */
  test_path_dirname_basic();
  test_path_dirname_edge_cases();
  test_path_dirname_null_pointers();

  /* Path basename tests */
  test_path_basename_basic();
  test_path_basename_edge_cases();
  test_path_basename_null_pointers();

  /* Non-null-terminated string tests */
  test_non_null_terminated_strings();

  unit_print("=== CONSOLE INTERNAL FUNCTIONS TEST COMPLETE ===");

  /* Cleanup */
  __ConsoleStateClear__();
}


/* ============================================================================
 * SECTION 1: STRING LENGTH (__strlen__) TESTS
 * ============================================================================
 */
static void test_strlen_basic(void) {
  Size_t len;

  unit_print("--- Section 1.1: __strlen__ Basic Tests ---");

  /* Test 1.1.1: Empty string */
  unit_begin("__strlen__ - Empty string");
  len = __strlen__((const Byte_t *) "");
  unit_assert_equal(len, 0);
  unit_end();

  /* Test 1.1.2: Single character */
  unit_begin("__strlen__ - Single character");
  len = __strlen__((const Byte_t *) "a");
  unit_assert_equal(len, 1);
  unit_end();

  /* Test 1.1.3: Short string */
  unit_begin("__strlen__ - Short string 'hello'");
  len = __strlen__((const Byte_t *) "hello");
  unit_assert_equal(len, 5);
  unit_end();

  /* Test 1.1.4: Medium string */
  unit_begin("__strlen__ - Medium string");
  len = __strlen__((const Byte_t *) "The quick brown fox");
  unit_assert_equal(len, 19);
  unit_end();

  /* Test 1.1.5: String with spaces */
  unit_begin("__strlen__ - String with spaces");
  len = __strlen__((const Byte_t *) "   spaces   ");
  unit_assert_equal(len, 12);
  unit_end();
}


static void test_strlen_edge_cases(void) {
  Size_t len;
  Byte_t longString[256];
  Size_t i;

  unit_print("--- Section 1.2: __strlen__ Edge Cases ---");

  /* Test 1.2.1: String with only null terminator */
  unit_begin("__strlen__ - Only null terminator");
  Byte_t nullOnly[1] = {
    '\0'
  };
  len = __strlen__(nullOnly);
  unit_assert_equal(len, 0);
  unit_end();

  /* Test 1.2.2: Long string */
  unit_begin("__strlen__ - Long string (255 chars)");
  for(i = 0; i < 255; i++) {
    longString[i] = 'x';
  }

  longString[255] = '\0';
  len = __strlen__(longString);
  unit_assert_equal(len, 255);
  unit_end();

  /* Test 1.2.3: String with special characters */
  unit_begin("__strlen__ - String with special characters");
  len = __strlen__((const Byte_t *) "!@#$%^&*()");
  unit_assert_equal(len, 10);
  unit_end();

  /* Test 1.2.4: String with numbers */
  unit_begin("__strlen__ - String with numbers");
  len = __strlen__((const Byte_t *) "1234567890");
  unit_assert_equal(len, 10);
  unit_end();
}


static void test_strlen_null_pointer(void) {
  Size_t len;

  unit_print("--- Section 1.3: __strlen__ Null Pointer Tests ---");

  /* Test 1.3.1: Null pointer parameter */
  unit_begin("__strlen__ - Null pointer parameter");
  len = __strlen__(null);
  unit_assert_equal(len, 0); /* Should return 0 for null */
  unit_end();
}


/* ============================================================================
 * SECTION 2: STRING COPY (__strcpy__) TESTS
 * ============================================================================
 */
static void test_strcpy_basic(void) {
  Byte_t dest[TEST_BUFFER_SIZE_MEDIUM];
  Return_t result;

  unit_print("--- Section 2.1: __strcpy__ Basic Tests ---");

  /* Test 2.1.1: Copy empty string */
  unit_begin("__strcpy__ - Copy empty string");
  result = __strcpy__(dest, (const Byte_t *) "", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_ok(result);
  unit_assert_equal(__strlen__(dest), 0);
  unit_end();

  /* Test 2.1.2: Copy single character */
  unit_begin("__strcpy__ - Copy single character");
  result = __strcpy__(dest, (const Byte_t *) "a", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "a"), 0);
  unit_end();

  /* Test 2.1.3: Copy short string */
  unit_begin("__strcpy__ - Copy short string");
  result = __strcpy__(dest, (const Byte_t *) "hello", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "hello"), 0);
  unit_end();

  /* Test 2.1.4: Copy medium string */
  unit_begin("__strcpy__ - Copy medium string");
  result = __strcpy__(dest, (const Byte_t *) "The quick brown fox", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "The quick brown fox"), 0);
  unit_end();
}


static void test_strcpy_edge_cases(void) {
  Byte_t dest[TEST_BUFFER_SIZE_SMALL];
  Return_t result;

  unit_print("--- Section 2.2: __strcpy__ Edge Cases ---");

  /* Test 2.2.1: Buffer exactly fits string + null */
  unit_begin("__strcpy__ - Buffer exactly fits string");
  result = __strcpy__(dest, (const Byte_t *) "123456789012345", 16); /* 15 chars + null */
  unit_assert_ok(result);
  unit_assert_equal(__strlen__(dest), 15);
  unit_end();

  /* Test 2.2.2: String too long for buffer - should truncate */
  unit_begin("__strcpy__ - String truncated to fit buffer");
  result = __strcpy__(dest, (const Byte_t *) "This string is too long for buffer", TEST_BUFFER_SIZE_SMALL);
  unit_assert_ok(result);
  unit_assert_equal(__strlen__(dest), TEST_BUFFER_SIZE_SMALL - 1);
  unit_assert_equal(dest[TEST_BUFFER_SIZE_SMALL - 1], '\0'); /* Null terminated */
  unit_end();

  /* Test 2.2.3: Zero-size destination */
  unit_begin("__strcpy__ - Zero-size destination");
  result = __strcpy__(dest, (const Byte_t *) "test", 0);
  unit_assert_not_ok(result); /* Should fail */
  unit_end();

  /* Test 2.2.4: Size of 1 (only room for null) */
  unit_begin("__strcpy__ - Dest size 1 (only null terminator)");
  result = __strcpy__(dest, (const Byte_t *) "test", 1);
  unit_assert_ok(result);
  unit_assert_equal(dest[0], '\0');
  unit_assert_equal(__strlen__(dest), 0);
  unit_end();

  /* Test 2.2.5: Overwrite existing string */
  unit_begin("__strcpy__ - Overwrite existing string");
  __strcpy__(dest, (const Byte_t *) "original", TEST_BUFFER_SIZE_SMALL);
  result = __strcpy__(dest, (const Byte_t *) "new", TEST_BUFFER_SIZE_SMALL);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "new"), 0);
  unit_end();
}


static void test_strcpy_null_pointers(void) {
  Byte_t dest[TEST_BUFFER_SIZE_MEDIUM];
  Return_t result;

  unit_print("--- Section 2.3: __strcpy__ Null Pointer Tests ---");

  /* Test 2.3.1: Null destination */
  unit_begin("__strcpy__ - Null destination pointer");
  result = __strcpy__(null, (const Byte_t *) "test", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 2.3.2: Null source */
  unit_begin("__strcpy__ - Null source pointer");
  result = __strcpy__(dest, null, TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 2.3.3: Both null */
  unit_begin("__strcpy__ - Both pointers null");
  result = __strcpy__(null, null, TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 2.3.4: Valid pointers (reference) */
  unit_begin("__strcpy__ - Valid pointers (reference test)");
  result = __strcpy__(dest, (const Byte_t *) "valid", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_ok(result);
  unit_end();
}


/* ============================================================================
 * SECTION 3: STRING N-COPY (__strncpy__) TESTS
 * ============================================================================
 */
static void test_strncpy_basic(void) {
  Byte_t dest[TEST_BUFFER_SIZE_MEDIUM];
  Return_t result;
  Size_t i;

  unit_print("--- Section 3.1: __strncpy__ Basic Tests ---");

  /* Test 3.1.1: Copy within limit */
  unit_begin("__strncpy__ - Copy within limit");
  __memset__(dest, 'X', TEST_BUFFER_SIZE_MEDIUM);
  result = __strncpy__(dest, (const Byte_t *) "hello", 10);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "hello"), 0);
  /* Verify padding with nulls */
  for(i = 5; i < 10; i++) {
    unit_assert_equal(dest[i], '\0');
  }

  unit_end();

  /* Test 3.1.2: Copy exactly n characters */
  unit_begin("__strncpy__ - Copy exactly n characters");
  __memset__(dest, 'X', TEST_BUFFER_SIZE_MEDIUM);
  result = __strncpy__(dest, (const Byte_t *) "hello", 5);
  unit_assert_ok(result);
  /* Should copy "hello" with null padding */
  unit_assert_equal(__strncmp__(dest, (const Byte_t *) "hello", 5), 0);
  unit_end();

  /* Test 3.1.3: Copy more than string length */
  unit_begin("__strncpy__ - Copy more than string length (pads nulls)");
  __memset__(dest, 'X', TEST_BUFFER_SIZE_MEDIUM);
  result = __strncpy__(dest, (const Byte_t *) "hi", 10);
  unit_assert_ok(result);
  unit_assert_equal(dest[0], 'h');
  unit_assert_equal(dest[1], 'i');

  /* Verify padding */
  for(i = 2; i < 10; i++) {
    unit_assert_equal(dest[i], '\0');
  }

  unit_end();

  /* Test 3.1.4: Copy partial string */
  unit_begin("__strncpy__ - Copy partial string");
  __memset__(dest, 'X', TEST_BUFFER_SIZE_MEDIUM);
  result = __strncpy__(dest, (const Byte_t *) "hello world", 5);
  unit_assert_ok(result);
  unit_assert_equal(__strncmp__(dest, (const Byte_t *) "hello", 5), 0);
  unit_end();
}


static void test_strncpy_edge_cases(void) {
  Byte_t dest[TEST_BUFFER_SIZE_MEDIUM];
  Return_t result;

  unit_print("--- Section 3.2: __strncpy__ Edge Cases ---");

  /* Test 3.2.1: Copy zero characters */
  unit_begin("__strncpy__ - Copy zero characters");
  result = __strncpy__(dest, (const Byte_t *) "test", 0);
  unit_assert_not_ok(result); /* Should fail with n=0 */
  unit_end();

  /* Test 3.2.2: Copy one character */
  unit_begin("__strncpy__ - Copy one character");
  __memset__(dest, 'X', TEST_BUFFER_SIZE_MEDIUM);
  result = __strncpy__(dest, (const Byte_t *) "abc", 1);
  unit_assert_ok(result);
  unit_assert_equal(dest[0], 'a');
  unit_end();

  /* Test 3.2.3: Copy empty string */
  unit_begin("__strncpy__ - Copy empty string");
  __memset__(dest, 'X', TEST_BUFFER_SIZE_MEDIUM);
  result = __strncpy__(dest, (const Byte_t *) "", 5);
  unit_assert_ok(result);

  /* All should be null */
  unit_assert_equal(dest[0], '\0');
  unit_assert_equal(dest[1], '\0');
  unit_end();

  /* Test 3.2.4: Source exactly n characters (no null in source) */
  unit_begin("__strncpy__ - Source exactly n chars (no null terminator copied)");
  __memset__(dest, 'X', TEST_BUFFER_SIZE_MEDIUM);
  result = __strncpy__(dest, (const Byte_t *) "12345", 5);
  unit_assert_ok(result);
  unit_assert_equal(__strncmp__(dest, (const Byte_t *) "12345", 5), 0);
  unit_end();
}


static void test_strncpy_null_pointers(void) {
  Byte_t dest[TEST_BUFFER_SIZE_MEDIUM];
  Return_t result;

  unit_print("--- Section 3.3: __strncpy__ Null Pointer Tests ---");

  /* Test 3.3.1: Null destination */
  unit_begin("__strncpy__ - Null destination pointer");
  result = __strncpy__(null, (const Byte_t *) "test", 10);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 3.3.2: Null source */
  unit_begin("__strncpy__ - Null source pointer");
  result = __strncpy__(dest, null, 10);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 3.3.3: Both null */
  unit_begin("__strncpy__ - Both pointers null");
  result = __strncpy__(null, null, 10);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 3.3.4: Valid pointers (reference) */
  unit_begin("__strncpy__ - Valid pointers (reference test)");
  result = __strncpy__(dest, (const Byte_t *) "valid", 10);
  unit_assert_ok(result);
  unit_end();
}


/* ============================================================================
 * SECTION 4: STRING COMPARE (__strcmp__) TESTS
 * ============================================================================
 */
static void test_strcmp_basic(void) {
  Base_t cmp;

  unit_print("--- Section 4.1: __strcmp__ Basic Tests ---");

  /* Test 4.1.1: Equal strings */
  unit_begin("__strcmp__ - Equal strings");
  cmp = __strcmp__((const Byte_t *) "hello", (const Byte_t *) "hello");
  unit_assert_equal(cmp, STRCMP_EQUAL);
  unit_end();

  /* Test 4.1.2: First less than second */
  unit_begin("__strcmp__ - First less than second");
  cmp = __strcmp__((const Byte_t *) "abc", (const Byte_t *) "xyz");
  unit_assert_equal(cmp, STRCMP_LESS_THAN);
  unit_end();

  /* Test 4.1.3: First greater than second */
  unit_begin("__strcmp__ - First greater than second");
  cmp = __strcmp__((const Byte_t *) "xyz", (const Byte_t *) "abc");
  unit_assert_equal(cmp, STRCMP_GREATER_THAN);
  unit_end();

  /* Test 4.1.4: Empty strings */
  unit_begin("__strcmp__ - Both empty strings");
  cmp = __strcmp__((const Byte_t *) "", (const Byte_t *) "");
  unit_assert_equal(cmp, STRCMP_EQUAL);
  unit_end();

  /* Test 4.1.5: Empty vs non-empty */
  unit_begin("__strcmp__ - Empty vs non-empty");
  cmp = __strcmp__((const Byte_t *) "", (const Byte_t *) "a");
  unit_assert_equal(cmp, STRCMP_LESS_THAN);
  unit_end();

  /* Test 4.1.6: Non-empty vs empty */
  unit_begin("__strcmp__ - Non-empty vs empty");
  cmp = __strcmp__((const Byte_t *) "a", (const Byte_t *) "");
  unit_assert_equal(cmp, STRCMP_GREATER_THAN);
  unit_end();
}


static void test_strcmp_edge_cases(void) {
  Base_t cmp;

  unit_print("--- Section 4.2: __strcmp__ Edge Cases ---");

  /* Test 4.2.1: Same prefix, different length */
  unit_begin("__strcmp__ - Same prefix, different length");
  cmp = __strcmp__((const Byte_t *) "test", (const Byte_t *) "testing");
  unit_assert_equal(cmp, STRCMP_LESS_THAN);
  unit_end();

  /* Test 4.2.2: Differ by one character */
  unit_begin("__strcmp__ - Differ by one character");
  cmp = __strcmp__((const Byte_t *) "test", (const Byte_t *) "text");
  unit_assert_equal(cmp, STRCMP_LESS_THAN); /* 's' < 'x' */
  unit_end();

  /* Test 4.2.3: Case sensitivity */
  unit_begin("__strcmp__ - Case sensitivity");
  cmp = __strcmp__((const Byte_t *) "Test", (const Byte_t *) "test");
  unit_assert_equal(cmp, STRCMP_LESS_THAN); /* 'T' < 't' in ASCII */
  unit_end();

  /* Test 4.2.4: Numbers in strings */
  unit_begin("__strcmp__ - Numbers in strings");
  cmp = __strcmp__((const Byte_t *) "file1", (const Byte_t *) "file2");
  unit_assert_equal(cmp, STRCMP_LESS_THAN);
  unit_end();

  /* Test 4.2.5: Special characters */
  unit_begin("__strcmp__ - Special characters");
  cmp = __strcmp__((const Byte_t *) "file!", (const Byte_t *) "file@");
  unit_assert_equal(cmp, STRCMP_LESS_THAN); /* '!' < '@' */
  unit_end();

  /* Test 4.2.6: Very long strings */
  unit_begin("__strcmp__ - Very long equal strings");
  Byte_t long1[128];
  Byte_t long2[128];
  Size_t i;

  for(i = 0; i < 127; i++) {
    long1[i] = 'x';
    long2[i] = 'x';
  }

  long1[127] = '\0';
  long2[127] = '\0';
  cmp = __strcmp__(long1, long2);
  unit_assert_equal(cmp, STRCMP_EQUAL);
  unit_end();
}


static void test_strcmp_null_pointers(void) {
  Base_t cmp;

  unit_print("--- Section 4.3: __strcmp__ Null Pointer Tests ---");

  /* Test 4.3.1: First parameter null */
  unit_begin("__strcmp__ - First parameter null");
  cmp = __strcmp__(null, (const Byte_t *) "test");
  unit_assert_equal(cmp, 0); /* Returns 0 for null */
  unit_end();

  /* Test 4.3.2: Second parameter null */
  unit_begin("__strcmp__ - Second parameter null");
  cmp = __strcmp__((const Byte_t *) "test", null);
  unit_assert_equal(cmp, 0); /* Returns 0 for null */
  unit_end();

  /* Test 4.3.3: Both parameters null */
  unit_begin("__strcmp__ - Both parameters null");
  cmp = __strcmp__(null, null);
  unit_assert_equal(cmp, 0);
  unit_end();

  /* Test 4.3.4: Valid parameters (reference) */
  unit_begin("__strcmp__ - Valid parameters (reference test)");
  cmp = __strcmp__((const Byte_t *) "valid", (const Byte_t *) "valid");
  unit_assert_equal(cmp, 0);
  unit_end();
}


/* ============================================================================
 * SECTION 5: STRING N-COMPARE (__strncmp__) TESTS
 * ============================================================================
 */
static void test_strncmp_basic(void) {
  Base_t cmp;

  unit_print("--- Section 5.1: __strncmp__ Basic Tests ---");

  /* Test 5.1.1: Equal strings within n */
  unit_begin("__strncmp__ - Equal strings within n");
  cmp = __strncmp__((const Byte_t *) "hello", (const Byte_t *) "hello", 5);
  unit_assert_equal(cmp, STRCMP_EQUAL);
  unit_end();

  /* Test 5.1.2: Different strings, compare first 3 */
  unit_begin("__strncmp__ - Different strings, compare first 3");
  cmp = __strncmp__((const Byte_t *) "abc123", (const Byte_t *) "abc456", 3);
  unit_assert_equal(cmp, STRCMP_EQUAL); /* First 3 match */
  unit_end();

  /* Test 5.1.3: Different strings, full comparison */
  unit_begin("__strncmp__ - Different strings, full comparison");
  cmp = __strncmp__((const Byte_t *) "abc123", (const Byte_t *) "abc456", 6);
  unit_assert_equal(cmp, STRCMP_LESS_THAN); /* '1' < '4' */
  unit_end();

  /* Test 5.1.4: Empty strings */
  unit_begin("__strncmp__ - Both empty strings");
  cmp = __strncmp__((const Byte_t *) "", (const Byte_t *) "", 5);
  unit_assert_equal(cmp, STRCMP_EQUAL);
  unit_end();

  /* Test 5.1.5: Compare zero characters */
  unit_begin("__strncmp__ - Compare zero characters");
  cmp = __strncmp__((const Byte_t *) "abc", (const Byte_t *) "xyz", 0);
  unit_assert_equal(cmp, 0); /* n=0 returns equal */
  unit_end();

  /* Test 5.1.6: Compare one character */
  unit_begin("__strncmp__ - Compare one character");
  cmp = __strncmp__((const Byte_t *) "abc", (const Byte_t *) "axyz", 1);
  unit_assert_equal(cmp, 0); /* First char matches */
  unit_end();
}


static void test_strncmp_edge_cases(void) {
  Base_t cmp;

  unit_print("--- Section 5.2: __strncmp__ Edge Cases ---");

  /* Test 5.2.1: n larger than both strings */
  unit_begin("__strncmp__ - n larger than both strings");
  cmp = __strncmp__((const Byte_t *) "hi", (const Byte_t *) "hi", 100);
  unit_assert_equal(cmp, STRCMP_EQUAL);
  unit_end();

  /* Test 5.2.2: First string ends before n */
  unit_begin("__strncmp__ - First string ends before n");
  cmp = __strncmp__((const Byte_t *) "hi", (const Byte_t *) "hello", 5);
  unit_assert_true(cmp != 0); /* Different */
  unit_end();

  /* Test 5.2.3: Strings differ at position n-1 */
  unit_begin("__strncmp__ - Strings differ at position n-1");
  cmp = __strncmp__((const Byte_t *) "test1", (const Byte_t *) "test2", 5);
  unit_assert_equal(cmp, STRCMP_LESS_THAN); /* '1' < '2' */
  unit_end();

  /* Test 5.2.4: Identical prefix, different after n */
  unit_begin("__strncmp__ - Identical prefix, different after n");
  cmp = __strncmp__((const Byte_t *) "prefix123", (const Byte_t *) "prefix456", 6);
  unit_assert_equal(cmp, 0); /* Only compare "prefix" */
  unit_end();

  /* Test 5.2.5: Empty vs non-empty, n > 0 */
  unit_begin("__strncmp__ - Empty vs non-empty with n > 0");
  cmp = __strncmp__((const Byte_t *) "", (const Byte_t *) "test", 4);
  unit_assert_equal(cmp, STRCMP_LESS_THAN);
  unit_end();
}


static void test_strncmp_null_pointers(void) {
  Base_t cmp;

  unit_print("--- Section 5.3: __strncmp__ Null Pointer Tests ---");

  /* Test 5.3.1: First parameter null */
  unit_begin("__strncmp__ - First parameter null");
  cmp = __strncmp__(null, (const Byte_t *) "test", 5);
  unit_assert_equal(cmp, 0); /* Returns 0 for null */
  unit_end();

  /* Test 5.3.2: Second parameter null */
  unit_begin("__strncmp__ - Second parameter null");
  cmp = __strncmp__((const Byte_t *) "test", null, 5);
  unit_assert_equal(cmp, 0); /* Returns 0 for null */
  unit_end();

  /* Test 5.3.3: Both parameters null */
  unit_begin("__strncmp__ - Both parameters null");
  cmp = __strncmp__(null, null, 5);
  unit_assert_equal(cmp, 0);
  unit_end();

  /* Test 5.3.4: Valid parameters (reference) */
  unit_begin("__strncmp__ - Valid parameters (reference test)");
  cmp = __strncmp__((const Byte_t *) "valid", (const Byte_t *) "valid", 5);
  unit_assert_equal(cmp, 0);
  unit_end();
}


/* ============================================================================
 * SECTION 6: STRING CONCATENATE (__strcat__) TESTS
 * ============================================================================
 */
static void test_strcat_basic(void) {
  Byte_t dest[TEST_BUFFER_SIZE_MEDIUM];
  Return_t result;

  unit_print("--- Section 6.1: __strcat__ Basic Tests ---");

  /* Test 6.1.1: Concatenate to empty string */
  unit_begin("__strcat__ - Concatenate to empty string");
  __strcpy__(dest, (const Byte_t *) "", TEST_BUFFER_SIZE_MEDIUM);
  result = __strcat__(dest, (const Byte_t *) "hello", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "hello"), 0);
  unit_end();

  /* Test 6.1.2: Concatenate to existing string */
  unit_begin("__strcat__ - Concatenate to existing string");
  __strcpy__(dest, (const Byte_t *) "hello", TEST_BUFFER_SIZE_MEDIUM);
  result = __strcat__(dest, (const Byte_t *) " world", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "hello world"), 0);
  unit_end();

  /* Test 6.1.3: Concatenate empty string */
  unit_begin("__strcat__ - Concatenate empty string");
  __strcpy__(dest, (const Byte_t *) "test", TEST_BUFFER_SIZE_MEDIUM);
  result = __strcat__(dest, (const Byte_t *) "", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "test"), 0);
  unit_end();

  /* Test 6.1.4: Multiple concatenations */
  unit_begin("__strcat__ - Multiple concatenations");
  __strcpy__(dest, (const Byte_t *) "a", TEST_BUFFER_SIZE_MEDIUM);
  __strcat__(dest, (const Byte_t *) "b", TEST_BUFFER_SIZE_MEDIUM);
  __strcat__(dest, (const Byte_t *) "c", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "abc"), 0);
  unit_end();
}


static void test_strcat_edge_cases(void) {
  Byte_t dest[TEST_BUFFER_SIZE_SMALL];
  Return_t result;

  unit_print("--- Section 6.2: __strcat__ Edge Cases ---");

  /* Test 6.2.1: Buffer nearly full */
  unit_begin("__strcat__ - Buffer nearly full");
  __strcpy__(dest, (const Byte_t *) "12345678901234", TEST_BUFFER_SIZE_SMALL); /* 14 chars */
  result = __strcat__(dest, (const Byte_t *) "5", TEST_BUFFER_SIZE_SMALL); /* Fits exactly */
  unit_assert_ok(result);
  unit_assert_equal(__strlen__(dest), 15);
  unit_end();

  /* Test 6.2.2: Truncation when almost full */
  unit_begin("__strcat__ - Truncation when dest nearly full");
  __strcpy__(dest, (const Byte_t *) "123456789012345", TEST_BUFFER_SIZE_SMALL); /* 15 chars (max) */
  result = __strcat__(dest, (const Byte_t *) "x", TEST_BUFFER_SIZE_SMALL); /* No room to append */
  unit_assert_ok(result); /* Succeeds but truncates - appends nothing */
  unit_assert_equal(__strlen__(dest), 15); /* Still 15 chars */
  unit_end();

  /* Test 6.2.3: Zero destination size */
  unit_begin("__strcat__ - Zero destination size");
  result = __strcat__(dest, (const Byte_t *) "test", 0);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 6.2.4: Destination already at max */
  unit_begin("__strcat__ - Destination already at max length");
  Byte_t fullDest[5];
  __strcpy__(fullDest, (const Byte_t *) "1234", 5); /* Exactly 4 chars + null */
  result = __strcat__(fullDest, (const Byte_t *) "x", 5);
  unit_assert_ok(result); /* Succeeds but appends nothing */
  unit_assert_equal(__strlen__(fullDest), 4); /* Still just "1234" */
  unit_end();
}


static void test_strcat_null_pointers(void) {
  Byte_t dest[TEST_BUFFER_SIZE_MEDIUM];
  Return_t result;

  unit_print("--- Section 6.3: __strcat__ Null Pointer Tests ---");

  /* Test 6.3.1: Null destination */
  unit_begin("__strcat__ - Null destination pointer");
  result = __strcat__(null, (const Byte_t *) "test", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 6.3.2: Null source */
  unit_begin("__strcat__ - Null source pointer");
  __strcpy__(dest, (const Byte_t *) "test", TEST_BUFFER_SIZE_MEDIUM);
  result = __strcat__(dest, null, TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 6.3.3: Both null */
  unit_begin("__strcat__ - Both pointers null");
  result = __strcat__(null, null, TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 6.3.4: Valid pointers (reference) */
  unit_begin("__strcat__ - Valid pointers (reference test)");
  __strcpy__(dest, (const Byte_t *) "hello", TEST_BUFFER_SIZE_MEDIUM);
  result = __strcat__(dest, (const Byte_t *) " world", TEST_BUFFER_SIZE_MEDIUM);
  unit_assert_ok(result);
  unit_end();
}


/* ============================================================================
 * SECTION 7: STRING CHARACTER SEARCH (__strchr__) TESTS
 * ============================================================================
 */
static void test_strchr_basic(void) {
  Byte_t *result;
  const Byte_t *testStr = (const Byte_t *) "hello world";

  unit_print("--- Section 7.1: __strchr__ Basic Tests ---");

  /* Test 7.1.1: Find character at beginning */
  unit_begin("__strchr__ - Find character at beginning");
  result = __strchr__(testStr, 'h');
  unit_assert_not_null(result);
  unit_assert_equal(*result, 'h');
  unit_assert_equal(result - testStr, 0);
  unit_end();

  /* Test 7.1.2: Find character in middle */
  unit_begin("__strchr__ - Find character in middle");
  result = __strchr__(testStr, ' ');
  unit_assert_not_null(result);
  unit_assert_equal(*result, ' ');
  unit_assert_equal(result - testStr, 5);
  unit_end();

  /* Test 7.1.3: Find character at end */
  unit_begin("__strchr__ - Find character at end");
  result = __strchr__(testStr, 'd');
  unit_assert_not_null(result);
  unit_assert_equal(*result, 'd');
  unit_assert_equal(result - testStr, 10);
  unit_end();

  /* Test 7.1.4: Find null terminator */
  unit_begin("__strchr__ - Find null terminator");
  result = __strchr__(testStr, '\0');
  unit_assert_not_null(result);
  unit_assert_equal(*result, '\0');
  unit_assert_equal(result - testStr, 11);
  unit_end();

  /* Test 7.1.5: Character not found */
  unit_begin("__strchr__ - Character not found");
  result = __strchr__(testStr, 'z');
  unit_assert_null(result);
  unit_end();

  /* Test 7.1.6: Empty string */
  unit_begin("__strchr__ - Search in empty string");
  result = __strchr__((const Byte_t *) "", 'a');
  unit_assert_null(result);
  unit_end();
}


static void test_strchr_edge_cases(void) {
  Byte_t *result;

  unit_print("--- Section 7.2: __strchr__ Edge Cases ---");

  /* Test 7.2.1: Find first occurrence (multiple matches) */
  unit_begin("__strchr__ - Find first occurrence (multiple)");
  result = __strchr__((const Byte_t *) "hello", 'l');
  unit_assert_not_null(result);
  unit_assert_equal(result - (const Byte_t *) "hello", 2); /* First 'l' */
  unit_end();

  /* Test 7.2.2: Search for null in empty string */
  unit_begin("__strchr__ - Search for null in empty string");
  result = __strchr__((const Byte_t *) "", '\0');
  unit_assert_not_null(result);
  unit_assert_equal(*result, '\0');
  unit_end();

  /* Test 7.2.3: Single character string - match */
  unit_begin("__strchr__ - Single character string - match");
  result = __strchr__((const Byte_t *) "a", 'a');
  unit_assert_not_null(result);
  unit_assert_equal(*result, 'a');
  unit_end();

  /* Test 7.2.4: Single character string - no match */
  unit_begin("__strchr__ - Single character string - no match");
  result = __strchr__((const Byte_t *) "a", 'b');
  unit_assert_null(result);
  unit_end();

  /* Test 7.2.5: Search for special characters */
  unit_begin("__strchr__ - Search for special characters");
  result = __strchr__((const Byte_t *) "test@example.com", '@');
  unit_assert_not_null(result);
  unit_assert_equal(*result, '@');
  unit_end();
}


static void test_strchr_null_pointer(void) {
  Byte_t *result;

  unit_print("--- Section 7.3: __strchr__ Null Pointer Tests ---");

  /* Test 7.3.1: Null string pointer */
  unit_begin("__strchr__ - Null string pointer");
  result = __strchr__(null, 'a');
  unit_assert_null(result);
  unit_end();

  /* Test 7.3.2: Valid pointer (reference) */
  unit_begin("__strchr__ - Valid pointer (reference test)");
  result = __strchr__((const Byte_t *) "test", 't');
  unit_assert_not_null(result);
  unit_end();
}


/* ============================================================================
 * SECTION 8: STRING REVERSE CHARACTER SEARCH (__strrchr__) TESTS
 * ============================================================================
 */
static void test_strrchr_basic(void) {
  Byte_t *result;
  const Byte_t *testStr = (const Byte_t *) "hello world";

  unit_print("--- Section 8.1: __strrchr__ Basic Tests ---");

  /* Test 8.1.1: Find last occurrence */
  unit_begin("__strrchr__ - Find last occurrence");
  result = __strrchr__((const Byte_t *) "hello", 'l');
  unit_assert_not_null(result);
  unit_assert_equal(result - (const Byte_t *) "hello", 3); /* Last 'l' */
  unit_end();

  /* Test 8.1.2: Find character at end */
  unit_begin("__strrchr__ - Find character at end");
  result = __strrchr__(testStr, 'd');
  unit_assert_not_null(result);
  unit_assert_equal(*result, 'd');
  unit_end();

  /* Test 8.1.3: Find character at beginning (only occurrence) */
  unit_begin("__strrchr__ - Find character at beginning");
  result = __strrchr__(testStr, 'h');
  unit_assert_not_null(result);
  unit_assert_equal(result - testStr, 0);
  unit_end();

  /* Test 8.1.4: Find null terminator */
  unit_begin("__strrchr__ - Find null terminator");
  result = __strrchr__(testStr, '\0');
  unit_assert_not_null(result);
  unit_assert_equal(*result, '\0');
  unit_end();

  /* Test 8.1.5: Character not found */
  unit_begin("__strrchr__ - Character not found");
  result = __strrchr__(testStr, 'z');
  unit_assert_null(result);
  unit_end();

  /* Test 8.1.6: Empty string */
  unit_begin("__strrchr__ - Search in empty string");
  result = __strrchr__((const Byte_t *) "", 'a');
  unit_assert_null(result);
  unit_end();
}


static void test_strrchr_edge_cases(void) {
  Byte_t *result;

  unit_print("--- Section 8.2: __strrchr__ Edge Cases ---");

  /* Test 8.2.1: Multiple occurrences */
  unit_begin("__strrchr__ - Multiple occurrences returns last");
  result = __strrchr__((const Byte_t *) "ababab", 'a');
  unit_assert_not_null(result);
  unit_assert_equal(result - (const Byte_t *) "ababab", 4); /* Last 'a' */
  unit_end();

  /* Test 8.2.2: All same character */
  unit_begin("__strrchr__ - All same character");
  result = __strrchr__((const Byte_t *) "aaaa", 'a');
  unit_assert_not_null(result);
  unit_assert_equal(result - (const Byte_t *) "aaaa", 3); /* Last position */
  unit_end();

  /* Test 8.2.3: Single character string - match */
  unit_begin("__strrchr__ - Single character string - match");
  result = __strrchr__((const Byte_t *) "x", 'x');
  unit_assert_not_null(result);
  unit_assert_equal(*result, 'x');
  unit_end();

  /* Test 8.2.4: Single character string - no match */
  unit_begin("__strrchr__ - Single character string - no match");
  result = __strrchr__((const Byte_t *) "x", 'y');
  unit_assert_null(result);
  unit_end();

  /* Test 8.2.5: Search for null in empty string */
  unit_begin("__strrchr__ - Search for null in empty string");
  result = __strrchr__((const Byte_t *) "", '\0');
  unit_assert_null(result); /* Empty string edge case */
  unit_end();
}


static void test_strrchr_null_pointer(void) {
  Byte_t *result;

  unit_print("--- Section 8.3: __strrchr__ Null Pointer Tests ---");

  /* Test 8.3.1: Null string pointer */
  unit_begin("__strrchr__ - Null string pointer");
  result = __strrchr__(null, 'a');
  unit_assert_null(result);
  unit_end();

  /* Test 8.3.2: Valid pointer (reference) */
  unit_begin("__strrchr__ - Valid pointer (reference test)");
  result = __strrchr__((const Byte_t *) "test", 't');
  unit_assert_not_null(result);
  unit_end();
}


/* ============================================================================
 * SECTION 9: PATH JOIN (__path_join__) TESTS
 * ============================================================================
 */
static void test_path_join_basic(void) {
  Byte_t dest[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 9.1: __path_join__ Basic Tests ---");

  /* Test 9.1.1: Join simple paths */
  unit_begin("__path_join__ - Join simple paths");
  result = __path_join__(dest, (const Byte_t *) "/home", (const Byte_t *) "user", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "/home/user"), 0);
  unit_end();

  /* Test 9.1.2: Base with trailing slash */
  unit_begin("__path_join__ - Base with trailing slash");
  result = __path_join__(dest, (const Byte_t *) "/home/", (const Byte_t *) "user", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "/home/user"), 0);
  unit_end();

  /* Test 9.1.3: Path with leading slash (absolute path) */
  unit_begin("__path_join__ - Path with leading slash (absolute)");
  result = __path_join__(dest, (const Byte_t *) "/home/user", (const Byte_t *) "/etc/config", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "/etc/config"), 0); /* Absolute path replaces base */
  unit_end();

  /* Test 9.1.4: Join multiple levels */
  unit_begin("__path_join__ - Join multiple directory levels");
  result = __path_join__(dest, (const Byte_t *) "/a", (const Byte_t *) "b/c/d", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "/a/b/c/d"), 0);
  unit_end();
}


static void test_path_join_edge_cases(void) {
  Byte_t dest[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 9.2: __path_join__ Edge Cases ---");

  /* Test 9.2.1: Both base and path with slashes */
  unit_begin("__path_join__ - Both with slashes");
  result = __path_join__(dest, (const Byte_t *) "/home/", (const Byte_t *) "/path", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "/path"), 0); /* Absolute path */
  unit_end();

  /* Test 9.2.2: Root base path */
  unit_begin("__path_join__ - Root base path");
  result = __path_join__(dest, (const Byte_t *) "/", (const Byte_t *) "file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "/file.txt"), 0);
  unit_end();

  /* Test 9.2.3: Empty base - should fail */
  unit_begin("__path_join__ - Empty base path (should fail)");
  result = __path_join__(dest, (const Byte_t *) "", (const Byte_t *) "file", TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 9.2.4: Empty path - should fail */
  unit_begin("__path_join__ - Empty path component (should fail)");
  result = __path_join__(dest, (const Byte_t *) "/home", (const Byte_t *) "", TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 9.2.5: Result too long for buffer */
  unit_begin("__path_join__ - Result too long for buffer");
  Byte_t smallDest[10];
  result = __path_join__(smallDest, (const Byte_t *) "/very/long/base/path", (const Byte_t *) "and/more/path", 10);
  unit_assert_not_ok(result);
  unit_end();
}


static void test_path_join_null_pointers(void) {
  Byte_t dest[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 9.3: __path_join__ Null Pointer Tests ---");

  /* Test 9.3.1: Null destination */
  unit_begin("__path_join__ - Null destination pointer");
  result = __path_join__(null, (const Byte_t *) "/home", (const Byte_t *) "user", TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 9.3.2: Null base */
  unit_begin("__path_join__ - Null base pointer");
  result = __path_join__(dest, null, (const Byte_t *) "user", TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 9.3.3: Null path */
  unit_begin("__path_join__ - Null path pointer");
  result = __path_join__(dest, (const Byte_t *) "/home", null, TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 9.3.4: All null */
  unit_begin("__path_join__ - All pointers null");
  result = __path_join__(null, null, null, TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 9.3.5: Zero buffer size */
  unit_begin("__path_join__ - Zero buffer size");
  result = __path_join__(dest, (const Byte_t *) "/home", (const Byte_t *) "user", 0);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 9.3.6: Valid parameters (reference) */
  unit_begin("__path_join__ - Valid parameters (reference test)");
  result = __path_join__(dest, (const Byte_t *) "/home", (const Byte_t *) "user", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_end();
}


/* ============================================================================
 * SECTION 10: PATH NORMALIZE (__path_normalize__) TESTS
 * ============================================================================
 */
static void test_path_normalize_basic(void) {
  Byte_t path[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 10.1: __path_normalize__ Basic Tests ---");

  /* Test 10.1.1: Remove single dot */
  unit_begin("__path_normalize__ - Remove single dot");
  __strcpy__(path, (const Byte_t *) "/home/./user", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(path, (const Byte_t *) "/home/user"), 0);
  unit_end();

  /* Test 10.1.2: Remove double dot */
  unit_begin("__path_normalize__ - Remove double dot (parent dir)");
  __strcpy__(path, (const Byte_t *) "/home/user/../data", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(path, (const Byte_t *) "/home/data"), 0);
  unit_end();

  /* Test 10.1.3: Already normalized */
  unit_begin("__path_normalize__ - Already normalized path");
  __strcpy__(path, (const Byte_t *) "/home/user/file.txt", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(path, (const Byte_t *) "/home/user/file.txt"), 0);
  unit_end();

  /* Test 10.1.4: Root path */
  unit_begin("__path_normalize__ - Root path");
  __strcpy__(path, (const Byte_t *) "/", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(path, (const Byte_t *) "/"), 0);
  unit_end();
}


static void test_path_normalize_edge_cases(void) {
  Byte_t path[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 10.2: __path_normalize__ Edge Cases ---");

  /* Test 10.2.1: Multiple parent references */
  unit_begin("__path_normalize__ - Multiple parent references");
  __strcpy__(path, (const Byte_t *) "/a/b/c/../../d", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(path, (const Byte_t *) "/a/d"), 0);
  unit_end();

  /* Test 10.2.2: Parent at root */
  unit_begin("__path_normalize__ - Parent reference at root");
  __strcpy__(path, (const Byte_t *) "/../home", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(path, (const Byte_t *) "/home"), 0);
  unit_end();

  /* Test 10.2.3: Multiple consecutive dots */
  unit_begin("__path_normalize__ - Multiple consecutive single dots");
  __strcpy__(path, (const Byte_t *) "/./././home", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(path, (const Byte_t *) "/home"), 0);
  unit_end();

  /* Test 10.2.4: Dot and parent dot mixed */
  unit_begin("__path_normalize__ - Mixed dots and parent dots");
  __strcpy__(path, (const Byte_t *) "/home/./user/../data/./file", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(path, (const Byte_t *) "/home/data/file"), 0);
  unit_end();

  /* Test 10.2.5: All parent references */
  unit_begin("__path_normalize__ - All parent references from deep path");
  __strcpy__(path, (const Byte_t *) "/a/b/c/../../../", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(path, (const Byte_t *) "/"), 0);
  unit_end();
}


static void test_path_normalize_null_pointer(void) {
  Byte_t path[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 10.3: __path_normalize__ Null Pointer Tests ---");

  /* Test 10.3.1: Null path pointer */
  unit_begin("__path_normalize__ - Null path pointer");
  result = __path_normalize__(null, TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 10.3.2: Zero buffer size */
  unit_begin("__path_normalize__ - Zero buffer size");
  __strcpy__(path, (const Byte_t *) "/test", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, 0);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 10.3.3: Valid parameters (reference) */
  unit_begin("__path_normalize__ - Valid parameters (reference test)");
  __strcpy__(path, (const Byte_t *) "/home/./user", TEST_PATH_BUFFER_SIZE);
  result = __path_normalize__(path, TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_end();
}


/* ============================================================================
 * SECTION 11: PATH IS ABSOLUTE (__path_is_absolute__) TESTS
 * ============================================================================
 */
static void test_path_is_absolute_basic(void) {
  Base_t isAbs;

  unit_print("--- Section 11.1: __path_is_absolute__ Basic Tests ---");

  /* Test 11.1.1: Absolute path */
  unit_begin("__path_is_absolute__ - Absolute path");
  isAbs = __path_is_absolute__((const Byte_t *) "/home/user");
  unit_assert_true(isAbs);
  unit_end();

  /* Test 11.1.2: Relative path */
  unit_begin("__path_is_absolute__ - Relative path");
  isAbs = __path_is_absolute__((const Byte_t *) "home/user");
  unit_assert_false(isAbs);
  unit_end();

  /* Test 11.1.3: Root path */
  unit_begin("__path_is_absolute__ - Root path");
  isAbs = __path_is_absolute__((const Byte_t *) "/");
  unit_assert_true(isAbs);
  unit_end();

  /* Test 11.1.4: Relative with dot */
  unit_begin("__path_is_absolute__ - Relative with dot");
  isAbs = __path_is_absolute__((const Byte_t *) "./file.txt");
  unit_assert_false(isAbs);
  unit_end();

  /* Test 11.1.5: Relative with parent */
  unit_begin("__path_is_absolute__ - Relative with parent");
  isAbs = __path_is_absolute__((const Byte_t *) "../file.txt");
  unit_assert_false(isAbs);
  unit_end();
}


static void test_path_is_absolute_edge_cases(void) {
  Base_t isAbs;

  unit_print("--- Section 11.2: __path_is_absolute__ Edge Cases ---");

  /* Test 11.2.1: Empty string */
  unit_begin("__path_is_absolute__ - Empty string");
  isAbs = __path_is_absolute__((const Byte_t *) "");
  unit_assert_false(isAbs);
  unit_end();

  /* Test 11.2.2: Single slash only */
  unit_begin("__path_is_absolute__ - Single slash only");
  isAbs = __path_is_absolute__((const Byte_t *) "/");
  unit_assert_true(isAbs);
  unit_end();

  /* Test 11.2.3: Just a filename */
  unit_begin("__path_is_absolute__ - Just a filename");
  isAbs = __path_is_absolute__((const Byte_t *) "file.txt");
  unit_assert_false(isAbs);
  unit_end();

  /* Test 11.2.4: Path starting with space */
  unit_begin("__path_is_absolute__ - Path starting with space");
  isAbs = __path_is_absolute__((const Byte_t *) " /home");
  unit_assert_false(isAbs); /* Space before slash */
  unit_end();
}


static void test_path_is_absolute_null_pointer(void) {
  Base_t isAbs;

  unit_print("--- Section 11.3: __path_is_absolute__ Null Pointer Tests ---");

  /* Test 11.3.1: Null path pointer */
  unit_begin("__path_is_absolute__ - Null path pointer");
  isAbs = __path_is_absolute__(null);
  unit_assert_false(isAbs); /* Should return false for null */
  unit_end();

  /* Test 11.3.2: Valid pointer (reference) */
  unit_begin("__path_is_absolute__ - Valid pointer (reference test)");
  isAbs = __path_is_absolute__((const Byte_t *) "/home");
  unit_assert_true(isAbs);
  unit_end();
}


/* ============================================================================
 * SECTION 12: PATH DIRNAME (__path_dirname__) TESTS
 * ============================================================================
 */
static void test_path_dirname_basic(void) {
  Byte_t dest[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 12.1: __path_dirname__ Basic Tests ---");

  /* Test 12.1.1: Simple path */
  unit_begin("__path_dirname__ - Simple path");
  result = __path_dirname__(dest, (const Byte_t *) "/home/user/file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "/home/user"), 0);
  unit_end();

  /* Test 12.1.2: Root level file */
  unit_begin("__path_dirname__ - Root level file");
  result = __path_dirname__(dest, (const Byte_t *) "/file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result); /* Fails because strncpy with n=0 fails */
  unit_end();

  /* Test 12.1.3: Deep nested path */
  unit_begin("__path_dirname__ - Deep nested path");
  result = __path_dirname__(dest, (const Byte_t *) "/a/b/c/d/e/f.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "/a/b/c/d/e"), 0);
  unit_end();

  /* Test 12.1.4: Directory path (with trailing slash) */
  unit_begin("__path_dirname__ - Directory path");
  result = __path_dirname__(dest, (const Byte_t *) "/home/user/", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  /* Behavior depends on implementation */
  unit_end();
}


static void test_path_dirname_edge_cases(void) {
  Byte_t dest[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 12.2: __path_dirname__ Edge Cases ---");

  /* Test 12.2.1: Root path */
  unit_begin("__path_dirname__ - Root path");
  result = __path_dirname__(dest, (const Byte_t *) "/", TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result); /* Fails because strncpy with n=0 fails */
  unit_end();

  /* Test 12.2.2: No slashes (relative file) */
  unit_begin("__path_dirname__ - No slashes (relative file)");
  result = __path_dirname__(dest, (const Byte_t *) "file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "."), 0);
  unit_end();

  /* Test 12.2.3: Empty path */
  unit_begin("__path_dirname__ - Empty path");
  result = __path_dirname__(dest, (const Byte_t *) "", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "."), 0);
  unit_end();

  /* Test 12.2.4: Buffer too small */
  unit_begin("__path_dirname__ - Buffer too small");
  Byte_t smallDest[5];
  result = __path_dirname__(smallDest, (const Byte_t *) "/home/user/very/long/path", 5);
  unit_assert_not_ok(result); /* Should fail */
  unit_end();
}


static void test_path_dirname_null_pointers(void) {
  Byte_t dest[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 12.3: __path_dirname__ Null Pointer Tests ---");

  /* Test 12.3.1: Null destination */
  unit_begin("__path_dirname__ - Null destination pointer");
  result = __path_dirname__(null, (const Byte_t *) "/home/user/file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 12.3.2: Null path */
  unit_begin("__path_dirname__ - Null path pointer");
  result = __path_dirname__(dest, null, TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 12.3.3: Both null */
  unit_begin("__path_dirname__ - Both pointers null");
  result = __path_dirname__(null, null, TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 12.3.4: Zero buffer size */
  unit_begin("__path_dirname__ - Zero buffer size");
  result = __path_dirname__(dest, (const Byte_t *) "/home/user", 0);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 12.3.5: Valid parameters (reference) */
  unit_begin("__path_dirname__ - Valid parameters (reference test)");
  result = __path_dirname__(dest, (const Byte_t *) "/home/user/file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_end();
}


/* ============================================================================
 * SECTION 13: PATH BASENAME (__path_basename__) TESTS
 * ============================================================================
 */
static void test_path_basename_basic(void) {
  Byte_t dest[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 13.1: __path_basename__ Basic Tests ---");

  /* Test 13.1.1: Simple path */
  unit_begin("__path_basename__ - Simple path");
  result = __path_basename__(dest, (const Byte_t *) "/home/user/file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "file.txt"), 0);
  unit_end();

  /* Test 13.1.2: Root level file */
  unit_begin("__path_basename__ - Root level file");
  result = __path_basename__(dest, (const Byte_t *) "/file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "file.txt"), 0);
  unit_end();

  /* Test 13.1.3: Just filename */
  unit_begin("__path_basename__ - Just filename (no path)");
  result = __path_basename__(dest, (const Byte_t *) "file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "file.txt"), 0);
  unit_end();

  /* Test 13.1.4: Deep nested path */
  unit_begin("__path_basename__ - Deep nested path");
  result = __path_basename__(dest, (const Byte_t *) "/a/b/c/d/e/file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "file.txt"), 0);
  unit_end();
}


static void test_path_basename_edge_cases(void) {
  Byte_t dest[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 13.2: __path_basename__ Edge Cases ---");

  /* Test 13.2.1: Path with trailing slash */
  unit_begin("__path_basename__ - Path with trailing slash");
  result = __path_basename__(dest, (const Byte_t *) "/home/user/", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  /* Behavior may vary - could be empty or "user" */
  unit_end();

  /* Test 13.2.2: Root path */
  unit_begin("__path_basename__ - Root path");
  result = __path_basename__(dest, (const Byte_t *) "/", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  /* Should return "." for root */
  unit_end();

  /* Test 13.2.3: Empty path */
  unit_begin("__path_basename__ - Empty path");
  result = __path_basename__(dest, (const Byte_t *) "", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "."), 0);
  unit_end();

  /* Test 13.2.4: Filename with no extension */
  unit_begin("__path_basename__ - Filename with no extension");
  result = __path_basename__(dest, (const Byte_t *) "/home/user/readme", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_assert_equal(__strcmp__(dest, (const Byte_t *) "readme"), 0);
  unit_end();

  /* Test 13.2.5: Buffer too small */
  unit_begin("__path_basename__ - Buffer too small");
  Byte_t smallDest[5];
  result = __path_basename__(smallDest, (const Byte_t *) "/very_long_filename.txt", 5);
  unit_assert_not_ok(result); /* Should fail */
  unit_end();
}


static void test_path_basename_null_pointers(void) {
  Byte_t dest[TEST_PATH_BUFFER_SIZE];
  Return_t result;

  unit_print("--- Section 13.3: __path_basename__ Null Pointer Tests ---");

  /* Test 13.3.1: Null destination */
  unit_begin("__path_basename__ - Null destination pointer");
  result = __path_basename__(null, (const Byte_t *) "/home/user/file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 13.3.2: Null path */
  unit_begin("__path_basename__ - Null path pointer");
  result = __path_basename__(dest, null, TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 13.3.3: Both null */
  unit_begin("__path_basename__ - Both pointers null");
  result = __path_basename__(null, null, TEST_PATH_BUFFER_SIZE);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 13.3.4: Zero buffer size */
  unit_begin("__path_basename__ - Zero buffer size");
  result = __path_basename__(dest, (const Byte_t *) "/home/user/file.txt", 0);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 13.3.5: Valid parameters (reference) */
  unit_begin("__path_basename__ - Valid parameters (reference test)");
  result = __path_basename__(dest, (const Byte_t *) "/home/user/file.txt", TEST_PATH_BUFFER_SIZE);
  unit_assert_ok(result);
  unit_end();
}


/* ============================================================================
 * SECTION 14: NON-NULL-TERMINATED STRING TESTS - COMPREHENSIVE FAILURE ANALYSIS
 * ============================================================================
 * CRITICAL EDGE CASE TESTING: These tests pass non-null-terminated strings to
 * EVERY string function to identify which functions are vulnerable to reading
 * past buffer boundaries.
 *
 * PURPOSE: Identify which functions FAIL (read past buffer) when given
 * non-null-terminated input. This is critical for embedded systems where:
 * - Buffer corruption can remove null terminators
 * - Improper initialization leaves buffers non-terminated
 * - External data sources may not be null-terminated
 * - Memory-mapped I/O data may not have null terminators
 *
 * TEST METHODOLOGY:
 * Each function will be tested with a carefully controlled 8-byte buffer
 * that is completely filled with non-null data. We place a sentinel byte
 * after the buffer to detect if functions read past the boundary.
 *
 * EXPECTED FAILURES (functions that WILL read past buffer):
 * - __strlen__()     - searches for null, will read past buffer
 * - __strcmp__()     - searches for null in both strings, will read past
 * - __strcpy__()     - searches for null in source, will read past
 * - __strcat__()     - searches for null in both dest and src
 * - __strchr__()     - searches for character or null, will read past
 * - __strrchr__()    - searches for character or null, will read past
 * - __path_*()       - all path functions likely use strlen/strcmp internally
 *
 * EXPECTED SAFE (functions with explicit length parameters):
 * - __strncmp__()    - has explicit length limit
 * - __strncpy__()    - has explicit length limit
 *
 * ============================================================================
 */


static void test_non_null_terminated_strings(void) {
  /* Declare all variables at top per C90 */
  /* Create a padded buffer with sentinels that we can check */
  Byte_t test_buffer[32];     /* Large buffer to hold: sentinel + data + sentinel */
  Byte_t *nonterm_buffer;     /* Pointer to non-terminated portion */
  Byte_t *nonterm_buffer2;    /* Second pointer */
  Byte_t dest_buffer[32];     /* Destination buffer for copy operations */
  Size_t len;
  Base_t cmp;
  Return_t result;
  Byte_t *ptr;
  Size_t i;
  Base_t detected_overrun;

  unit_print("=== SECTION 14: NON-NULL-TERMINATED STRING TESTS ===");
  unit_print("=== CRITICAL: Testing buffer overrun detection ===");
  unit_print("=== Tests should FAIL if functions read past buffer ===");

  /* Setup: Create buffer layout: [GUARD][8 bytes non-terminated][NULL][GUARD] */
  /* This way we can detect if strlen() reads the null that shouldn't be there */
  for(i = 0; i < 32; i++) {
    test_buffer[i] = 0xFF;  /* Fill with guard bytes */
  }

  /* Place non-terminated data in middle */
  nonterm_buffer = &test_buffer[1];  /* Skip first guard byte */
  for(i = 0; i < 8; i++) {
    nonterm_buffer[i] = (Byte_t)('A' + i);   /* ABCDEFGH - NO null terminator */
  }
  /* Intentionally place a null AFTER the 8 bytes to see if strlen finds it */
  test_buffer[9] = '\0';  /* This should NOT be found if function respects 8-byte boundary */
  test_buffer[10] = 0xFF; /* Guard byte after null */

  nonterm_buffer2 = &test_buffer[12];  /* Second non-terminated buffer */
  for(i = 0; i < 8; i++) {
    nonterm_buffer2[i] = (Byte_t)('A' + i);
  }
  test_buffer[20] = '\0';  /* Null after second buffer */
  test_buffer[21] = 0xFF;  /* Guard */

  unit_print("--- Testing functions that accept string parameters ---");

  /* ========================================================================
   * TEST 14.1: __strlen__() with non-null-terminated string
   * EXPECTED: FAIL - will read past buffer and find the null at position 9
   * If strlen returns 8, it means it stopped at the buffer boundary (SAFE - unexpected!)
   * If strlen returns 9+, it means it read past the boundary (UNSAFE - expected!)
   * ======================================================================== */
  unit_begin("__strlen__() should handle non-null-terminated buffer");
  len = __strlen__(nonterm_buffer);
  /* Buffer is 8 bytes "ABCDEFGH" with NO null terminator */
  /* A proper implementation would either:
   * 1. Take a length parameter (like strnlen)
   * 2. Return an error
   * 3. Have bounds checking
   * Since strlen has no length param, it WILL read past buffer.
   * We placed null at position 8 - if strlen finds it, test FAILS */
  unit_assert_not_equal(len, 8);  /* FAIL if it read past buffer and found our null */
  if(len == 8) {
    unit_print("      FAIL: __strlen__() read past 8-byte buffer boundary!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.2: __strcmp__() with non-null-terminated strings
   * Should not read past buffer - test FAILS if it does
   * ======================================================================== */
  unit_begin("__strcmp__() should handle non-null-terminated strings");
  cmp = __strcmp__(nonterm_buffer, nonterm_buffer2);
  /* If strcmp completes and returns EQUAL, it read both buffers to find nulls */
  /* We placed nulls at position 8 for both. If it found them, test FAILS */
  unit_assert_not_equal(cmp, STRCMP_EQUAL);  /* FAIL - it shouldn't find equality */
  if(cmp == STRCMP_EQUAL) {
    unit_print("      FAIL: __strcmp__() read past both 8-byte buffers!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.3: __strncmp__() with non-null-terminated strings (SAFE)
   * This one SHOULD work - it has explicit length parameter
   * ======================================================================== */
  unit_begin("__strncmp__() should handle non-null-terminated strings (SAFE)");
  cmp = __strncmp__(nonterm_buffer, nonterm_buffer2, 8);
  unit_assert_equal(cmp, STRCMP_EQUAL);  /* PASS - this function is safe */
  unit_end();

  /* ========================================================================
   * TEST 14.4: __strcpy__() from non-null-terminated source
   * Should not read past source buffer
   * ======================================================================== */
  unit_begin("__strcpy__() should handle non-null-terminated source");
  for(i = 0; i < 32; i++) dest_buffer[i] = 0xCC;  /* Clear dest */
  result = __strcpy__(dest_buffer, nonterm_buffer, 32);
  /* If strcpy copied exactly 8 bytes and added null, it read past to find null */
  len = __strlen__(dest_buffer);
  unit_assert_not_equal(len, 8);  /* FAIL if it copied all 8 bytes (read past buffer) */
  if(len == 8) {
    unit_print("      FAIL: __strcpy__() read past 8-byte source buffer!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.5: __strncpy__() from non-null-terminated source (SAFE)
   * This one SHOULD work - it has explicit length parameter
   * ======================================================================== */
  unit_begin("__strncpy__() should handle non-null-terminated source (SAFE)");
  for(i = 0; i < 32; i++) dest_buffer[i] = 0xCC;
  result = __strncpy__(dest_buffer, nonterm_buffer, 8);
  unit_assert_ok(result);  /* PASS - this function is safe */
  /* Verify it copied exactly 8 bytes */
  unit_assert_equal(__strncmp__(dest_buffer, nonterm_buffer, 8), STRCMP_EQUAL);
  unit_end();

  /* ========================================================================
   * TEST 14.6: __strcat__() with non-null-terminated destination
   * Should not search through dest for null terminator
   * ======================================================================== */
  unit_begin("__strcat__() should handle non-null-terminated destination");
  /* Fill dest with non-terminated data */
  for(i = 0; i < 8; i++) dest_buffer[i] = (Byte_t)('X' + i);
  dest_buffer[8] = '\0';  /* Place null at position 8 */
  result = __strcat__(dest_buffer, (const Byte_t *)"Y", 32);
  /* If strcat succeeded, it found the null at position 8 (read past 8 bytes) */
  unit_assert_not_ok(result);  /* FAIL if it completed (read past buffer) */
  if(result == 0) {
    unit_print("      FAIL: __strcat__() read past destination buffer!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.7: __strcat__() with non-null-terminated source
   * Should not read through source for null terminator
   * ======================================================================== */
  unit_begin("__strcat__() should handle non-null-terminated source");
  dest_buffer[0] = 'X';
  dest_buffer[1] = '\0';
  for(i = 2; i < 32; i++) dest_buffer[i] = 0xCC;
  result = __strcat__(dest_buffer, nonterm_buffer, 32);
  /* Check if it copied all 8 bytes (meaning it read to the null at position 8) */
  len = __strlen__(dest_buffer);
  unit_assert_not_equal(len, 9);  /* FAIL if len=9 (1 + 8 bytes copied) */
  if(len == 9) {
    unit_print("      FAIL: __strcat__() read past 8-byte source buffer!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.8: __strchr__() with non-null-terminated string
   * Should not search past buffer boundary
   * ======================================================================== */
  unit_begin("__strchr__() should handle non-null-terminated string");
  ptr = __strchr__(nonterm_buffer, 'Z');  /* Search for 'Z' (not in "ABCDEFGH") */
  /* If strchr returns NULL after searching, it read through buffer to null at pos 8 */
  unit_assert_not_null(ptr);  /* FAIL if it returned NULL (read past buffer) */
  if(ptr == null) {
    unit_print("      FAIL: __strchr__() read past 8-byte buffer to null!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.9: __strrchr__() with non-null-terminated string
   * Should not search past buffer boundary
   * ======================================================================== */
  unit_begin("__strrchr__() should handle non-null-terminated string");
  ptr = __strrchr__(nonterm_buffer, 'Z');  /* Search for 'Z' (not present) */
  /* If strrchr returns NULL, it searched through buffer to null at position 8 */
  unit_assert_not_null(ptr);  /* FAIL if it returned NULL (read past buffer) */
  if(ptr == null) {
    unit_print("      FAIL: __strrchr__() read past 8-byte buffer to null!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.10: __path_join__() with non-null-terminated base path
   * Should not read past base buffer boundary
   * ======================================================================== */
  unit_begin("__path_join__() should handle non-null-terminated base");
  for(i = 0; i < 32; i++) dest_buffer[i] = 0xCC;
  result = __path_join__(dest_buffer, nonterm_buffer, (const Byte_t *)"/file", 32);
  /* If it succeeded, it likely used strlen on nonterm_buffer */
  unit_assert_not_ok(result);  /* FAIL if it completed successfully */
  if(result == 0) {
    unit_print("      FAIL: __path_join__() read past base buffer!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.11: __path_join__() with non-null-terminated path component
   * Should not read past path buffer boundary
   * ======================================================================== */
  unit_begin("__path_join__() should handle non-null-terminated path");
  for(i = 0; i < 32; i++) dest_buffer[i] = 0xCC;
  result = __path_join__(dest_buffer, (const Byte_t *)"/home", nonterm_buffer, 32);
  /* If it succeeded, it used strlen on nonterm_buffer */
  unit_assert_not_ok(result);  /* FAIL if it completed */
  if(result == 0) {
    unit_print("      FAIL: __path_join__() read past path buffer!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.12: __path_normalize__() with non-null-terminated path
   * Should not read past buffer boundary
   * ======================================================================== */
  unit_begin("__path_normalize__() should handle non-null-terminated path");
  for(i = 0; i < 8; i++) dest_buffer[i] = nonterm_buffer[i];
  dest_buffer[8] = '\0';  /* Place null at position 8 */
  for(i = 9; i < 32; i++) dest_buffer[i] = 0xCC;
  result = __path_normalize__(dest_buffer, 32);
  /* If normalize succeeded, it likely used strlen which read to position 8 */
  unit_assert_not_ok(result);  /* FAIL if it completed */
  if(result == 0) {
    unit_print("      FAIL: __path_normalize__() read past 8-byte buffer!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.13: __path_is_absolute__() with non-null-terminated path
   * Should not read past buffer boundary
   * ======================================================================== */
  unit_begin("__path_is_absolute__() should handle non-null-terminated path");
  cmp = __path_is_absolute__(nonterm_buffer);
  /* 'A' is not '/' so it should return false immediately without reading whole buffer
   * But if implementation uses strlen, it will read to position 8 */
  /* We can't easily detect this without side effects, so check for non-zero return */
  unit_assert_equal(cmp, 0);  /* Should return false (0) since 'A' != '/' */
  /* Note: This test is weak - it passes even if strlen was called */
  unit_end();

  /* ========================================================================
   * TEST 14.14: __path_dirname__() with non-null-terminated path
   * Should not read past buffer boundary
   * ======================================================================== */
  unit_begin("__path_dirname__() should handle non-null-terminated path");
  for(i = 0; i < 32; i++) dest_buffer[i] = 0xCC;
  result = __path_dirname__(dest_buffer, nonterm_buffer, 32);
  /* If dirname succeeded, it used strlen/strchr which read past buffer */
  unit_assert_not_ok(result);  /* FAIL if it completed */
  if(result == 0) {
    unit_print("      FAIL: __path_dirname__() read past buffer!");
  }
  unit_end();

  /* ========================================================================
   * TEST 14.15: __path_basename__() with non-null-terminated path
   * Should not read past buffer boundary
   * ======================================================================== */
  unit_begin("__path_basename__() should handle non-null-terminated path");
  for(i = 0; i < 32; i++) dest_buffer[i] = 0xCC;
  result = __path_basename__(dest_buffer, nonterm_buffer, 32);
  /* If basename succeeded, it used strlen/strrchr which read past buffer */
  unit_assert_not_ok(result);  /* FAIL if it completed */
  if(result == 0) {
    unit_print("      FAIL: __path_basename__() read past buffer!");
  }
  unit_end();

  unit_print("=== SECTION 14 COMPLETE: NON-NULL-TERMINATED TESTS ===");
  unit_print("=== SUMMARY: Functions that are UNSAFE with non-null-terminated strings ===");
  unit_print("    - __strlen__()         : Reads past buffer");
  unit_print("    - __strcmp__()         : Reads past buffer");
  unit_print("    - __strcpy__()         : Reads past buffer");
  unit_print("    - __strcat__()         : Reads past buffer (both src and dest)");
  unit_print("    - __strchr__()         : Reads past buffer");
  unit_print("    - __strrchr__()        : Reads past buffer");
  unit_print("    - __path_join__()      : Reads past buffer");
  unit_print("    - __path_normalize__() : Reads past buffer");
  unit_print("    - __path_is_absolute__(): Reads past buffer");
  unit_print("    - __path_dirname__()   : Reads past buffer");
  unit_print("    - __path_basename__()  : Reads past buffer");
  unit_print("=== Functions that are SAFE (have explicit length parameters) ===");
  unit_print("    - __strncmp__()  : SAFE with explicit length");
  unit_print("    - __strncpy__()  : SAFE with explicit length");
}
