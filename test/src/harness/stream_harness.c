/*UNCRUSTIFY-OFF*/
/**
 * @file stream_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for stream buffers
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "stream_harness.h"


/* Test constants */
#define STREAM_BUFFER_SIZE 0x20u /* 32 bytes - stream buffer capacity */
#define STREAM_LAST_BYTE_INDEX 0x1Fu /* Index of last byte in buffer (31) */
#define STREAM_FILL_COUNT CONFIG_STREAM_BUFFER_BYTES /* Number of bytes to fill
                                                      * buffer */
#define TEST_BYTE_VALUE 0x0u /* Test byte value */
/* Helper function prototypes */
static void test_error_handling_and_null_pointers(void);
static void test_stream_creation(void);
static void test_stream_send_and_fill(void);
static void test_stream_status_checks(void);
static void test_stream_receive(void);
static void test_stream_reset(void);
static void test_stream_delete(void);


void stream_harness(void) {
  unit_print("=== COMPREHENSIVE STREAM BUFFER TEST SUITE ===");
  test_error_handling_and_null_pointers();
  test_stream_creation();
  test_stream_send_and_fill();
  test_stream_status_checks();
  test_stream_receive();
  test_stream_reset();
  test_stream_delete();
  unit_print("=== STREAM BUFFER TEST SUITE COMPLETE ===");
}


/* ============================================================================
 * SECTION 1: ERROR HANDLING AND NULL POINTER VALIDATION
 * ============================================================================
 */
static void test_error_handling_and_null_pointers(void) {
  StreamBuffer_t *stream = null;
  StreamBuffer_t *nullStream = null;
  Base_t result;
  HalfWord_t bytes;
  Byte_t *data;


  unit_print("--- Section 1: Error Handling and NULL Pointer Tests ---");


  /* Test 1.1: NULL pointer in xStreamCreate */
  unit_begin("xStreamCreate with NULL pointer returns error");
  unit_assert_not_ok(xStreamCreate(null));
  unit_end();


  /* Test 1.2: Create a valid stream for error tests */
  unit_begin("Create valid stream for error testing");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_not_null(stream);
  unit_end();


  /* Test 1.3: NULL pointer in xStreamDelete */
  unit_begin("xStreamDelete with NULL pointer returns error");
  unit_assert_not_ok(xStreamDelete(nullStream));
  unit_end();


  /* Test 1.4: NULL pointer in xStreamSend */
  unit_begin("xStreamSend with NULL stream returns error");
  unit_assert_not_ok(xStreamSend(nullStream, TEST_BYTE_VALUE));
  unit_end();


  /* Test 1.5: NULL pointer in xStreamReceive */
  unit_begin("xStreamReceive with NULL stream returns error");
  unit_assert_not_ok(xStreamReceive(nullStream, &bytes, &data));
  unit_end();


  /* Test 1.6: NULL pointer in xStreamBytesAvailable */
  unit_begin("xStreamBytesAvailable with NULL stream returns error");
  unit_assert_not_ok(xStreamBytesAvailable(nullStream, &bytes));
  unit_end();


  /* Test 1.7: NULL pointer in xStreamReset */
  unit_begin("xStreamReset with NULL stream returns error");
  unit_assert_not_ok(xStreamReset(nullStream));
  unit_end();


  /* Test 1.8: NULL pointer in xStreamIsEmpty */
  unit_begin("xStreamIsEmpty with NULL stream returns error");
  unit_assert_not_ok(xStreamIsEmpty(nullStream, &result));
  unit_end();


  /* Test 1.9: NULL pointer in xStreamIsFull */
  unit_begin("xStreamIsFull with NULL stream returns error");
  unit_assert_not_ok(xStreamIsFull(nullStream, &result));
  unit_end();


  /* Cleanup */
  unit_begin("Cleanup error test stream");
  unit_assert_ok(xStreamDelete(stream));
  unit_end();
}


/* ============================================================================
 * SECTION 2: STREAM BUFFER CREATION
 * ============================================================================
 */
static void test_stream_creation(void) {
  StreamBuffer_t *stream = null;


  unit_print("--- Section 2: Stream Buffer Creation ---");


  /* Test 1.1: Stream buffer creation */
  unit_begin("Stream buffer creation succeeds");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_not_null(stream);
  unit_end();


  /* Cleanup for next section */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 3: STREAM SEND AND BUFFER FILL
 * ============================================================================
 */
static void test_stream_send_and_fill(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i = 0x0u;


  unit_print("--- Section 3: Stream Send and Buffer Fill ---");


  /* Test 2.1: Stream accepts bytes until buffer is full */
  unit_begin("Stream accepts bytes until buffer is full");
  unit_assert_ok(xStreamCreate(&stream));

  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }


  /* Should reject additional bytes when full */
  unit_assert_not_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_end();


  /* Cleanup for next section */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 4: STREAM STATUS CHECKS
 * ============================================================================
 */
static void test_stream_status_checks(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i = 0x0u;
  HalfWord_t bytesAvailable = 0x0u;
  Base_t result;


  unit_print("--- Section 4: Stream Status Checks ---");


  /* Test 3.1: Stream full check */
  unit_begin("Stream full check correctly identifies full buffer");
  unit_assert_ok(xStreamCreate(&stream));

  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamIsFull(stream, &result));
  unit_assert_true(result);
  unit_end();


  /* Test 3.2: Stream empty check on non-empty buffer */
  unit_begin("Stream empty check correctly identifies non-empty buffer");
  unit_assert_ok(xStreamIsEmpty(stream, &result));
  unit_assert_false(result);
  unit_end();


  /* Test 3.3: Stream bytes available */
  unit_begin("Stream bytes available returns buffer capacity");
  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, STREAM_BUFFER_SIZE);
  unit_end();


  /* Cleanup for next section */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 5: STREAM RECEIVE OPERATIONS
 * ============================================================================
 */
static void test_stream_receive(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i = 0x0u;
  HalfWord_t bytesReceived = 0x0u;
  Byte_t *receivedData = null;


  unit_print("--- Section 4: Stream Receive Operations ---");


  /* Test 4.1: Stream receive returns all buffered bytes */
  unit_begin("Stream receive returns all buffered bytes");
  unit_assert_ok(xStreamCreate(&stream));

  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &receivedData));
  unit_assert_not_null(receivedData);
  unit_assert_equal(bytesReceived, STREAM_BUFFER_SIZE);
  unit_assert_equal(receivedData[STREAM_LAST_BYTE_INDEX], STREAM_LAST_BYTE_INDEX);
  unit_end();


  /* Cleanup for next section */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 6: STREAM RESET OPERATIONS
 * ============================================================================
 */
static void test_stream_reset(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i = 0x0u;
  Base_t result;


  unit_print("--- Section 5: Stream Reset Operations ---");


  /* Test 5.1: Stream reset clears buffer contents */
  unit_begin("Stream reset clears buffer contents");
  unit_assert_ok(xStreamCreate(&stream));


  /* Fill the buffer */
  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }


  /* Verify full */
  unit_assert_not_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamIsFull(stream, &result));
  unit_assert_true(result);


  /* Reset and verify empty */
  unit_assert_ok(xStreamReset(stream));
  unit_assert_ok(xStreamIsEmpty(stream, &result));
  unit_assert_true(result);
  unit_end();


  /* Cleanup for next section */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 7: STREAM DELETE AND CLEANUP
 * ============================================================================
 */
static void test_stream_delete(void) {
  StreamBuffer_t *stream = null;


  unit_print("--- Section 7: Stream Delete and Cleanup ---");


  /* Test 6.1: Stream delete invalidates stream handle */
  unit_begin("Stream delete invalidates stream handle");
  unit_assert_ok(xStreamCreate(&stream));


  /* Delete should succeed */
  unit_assert_ok(xStreamDelete(stream));


  /* Operations on deleted stream should fail */
  unit_assert_not_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_end();
}