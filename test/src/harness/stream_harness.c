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
#define STREAM_BUFFER_SIZE CONFIG_STREAM_BUFFER_BYTES /* Buffer capacity */
#define STREAM_LAST_BYTE_INDEX (CONFIG_STREAM_BUFFER_BYTES - 1) /* Last byte index */
#define STREAM_FILL_COUNT CONFIG_STREAM_BUFFER_BYTES /* Bytes to fill buffer */
#define TEST_BYTE_VALUE 0xAAu /* Test byte value */
#define TEST_PATTERN_START 0x42u /* Starting pattern value */

/* Stress test constants */
#define MAX_STREAMS_STRESS 30
#define STRESS_SEND_COUNT 100
#define STRESS_CYCLE_COUNT 50

/* Helper function prototypes */
static void test_error_handling_and_null_pointers(void);
static void test_stream_creation(void);
static void test_stream_send_operations(void);
static void test_stream_receive_operations(void);
static void test_stream_status_checks(void);
static void test_stream_empty_and_full(void);
static void test_stream_reset_operations(void);
static void test_stream_wrap_around(void);
static void test_multiple_streams(void);
static void test_stream_stress_operations(void);
static void test_stream_partial_operations(void);
static void test_stream_boundary_conditions(void);
static void test_stream_state_persistence(void);
static void test_stream_edge_cases(void);
static void test_stream_delete_and_cleanup(void);


void stream_harness(void) {
  unit_print("=== COMPREHENSIVE STREAM BUFFER TEST SUITE ===");
  test_error_handling_and_null_pointers();
  test_stream_creation();
  test_stream_send_operations();
  test_stream_receive_operations();
  test_stream_status_checks();
  test_stream_empty_and_full();
  test_stream_reset_operations();
  test_stream_wrap_around();
  test_multiple_streams();
  test_stream_stress_operations();
  test_stream_partial_operations();
  test_stream_boundary_conditions();
  test_stream_state_persistence();
  test_stream_edge_cases();
  test_stream_delete_and_cleanup();
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


  /* Test 1.6: xStreamReceive with NULL bytes pointer */
  unit_begin("xStreamReceive with NULL bytes pointer returns error");
  unit_assert_not_ok(xStreamReceive(stream, null, &data));
  unit_end();


  /* Test 1.7: xStreamReceive with NULL data pointer */
  unit_begin("xStreamReceive with NULL data pointer returns error");
  unit_assert_not_ok(xStreamReceive(stream, &bytes, null));
  unit_end();


  /* Test 1.8: NULL pointer in xStreamBytesAvailable */
  unit_begin("xStreamBytesAvailable with NULL stream returns error");
  unit_assert_not_ok(xStreamBytesAvailable(nullStream, &bytes));
  unit_end();


  /* Test 1.9: xStreamBytesAvailable with NULL bytes pointer */
  unit_begin("xStreamBytesAvailable with NULL bytes pointer returns error");
  unit_assert_not_ok(xStreamBytesAvailable(stream, null));
  unit_end();


  /* Test 1.10: NULL pointer in xStreamReset */
  unit_begin("xStreamReset with NULL stream returns error");
  unit_assert_not_ok(xStreamReset(nullStream));
  unit_end();


  /* Test 1.11: NULL pointer in xStreamIsEmpty */
  unit_begin("xStreamIsEmpty with NULL stream returns error");
  unit_assert_not_ok(xStreamIsEmpty(nullStream, &result));
  unit_end();


  /* Test 1.12: xStreamIsEmpty with NULL result pointer */
  unit_begin("xStreamIsEmpty with NULL result pointer returns error");
  unit_assert_not_ok(xStreamIsEmpty(stream, null));
  unit_end();


  /* Test 1.13: NULL pointer in xStreamIsFull */
  unit_begin("xStreamIsFull with NULL stream returns error");
  unit_assert_not_ok(xStreamIsFull(nullStream, &result));
  unit_end();


  /* Test 1.14: xStreamIsFull with NULL result pointer */
  unit_begin("xStreamIsFull with NULL result pointer returns error");
  unit_assert_not_ok(xStreamIsFull(stream, null));
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
  StreamBuffer_t *stream1 = null;
  StreamBuffer_t *stream2 = null;
  StreamBuffer_t *stream3 = null;
  Base_t isEmpty;


  unit_print("--- Section 2: Stream Buffer Creation ---");


  /* Test 2.1: Stream buffer creation succeeds */
  unit_begin("Stream buffer creation succeeds");
  unit_assert_ok(xStreamCreate(&stream1));
  unit_assert_not_null(stream1);
  unit_end();


  /* Test 2.2: Multiple stream creation */
  unit_begin("Multiple stream creation succeeds");
  unit_assert_ok(xStreamCreate(&stream2));
  unit_assert_ok(xStreamCreate(&stream3));
  unit_assert_not_null(stream2);
  unit_assert_not_null(stream3);
  unit_end();


  /* Test 2.3: Newly created streams are empty */
  unit_begin("Newly created streams are empty");
  unit_assert_ok(xStreamIsEmpty(stream1, &isEmpty));
  unit_assert_true(isEmpty);
  unit_assert_ok(xStreamIsEmpty(stream2, &isEmpty));
  unit_assert_true(isEmpty);
  unit_end();


  /* Test 2.4: Newly created streams are not full */
  unit_begin("Newly created streams are not full");
  {
    Base_t isFull;


    unit_assert_ok(xStreamIsFull(stream1, &isFull));
    unit_assert_false(isFull);
  } unit_end();


  /* Cleanup */
  xStreamDelete(stream1);
  xStreamDelete(stream2);
  xStreamDelete(stream3);
}


/* ============================================================================
 * SECTION 3: STREAM SEND OPERATIONS
 * ============================================================================
 */
static void test_stream_send_operations(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i;
  HalfWord_t bytesAvailable;


  unit_print("--- Section 3: Stream Send Operations ---");


  /* Test 3.1: Single byte send succeeds */
  unit_begin("Single byte send succeeds");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_end();


  /* Test 3.2: Multiple byte sends succeed */
  unit_begin("Multiple byte sends succeed");
  for(i = 0x0u; i < 10; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }
  unit_end();


  /* Test 3.3: Stream tracks bytes available correctly */
  unit_begin("Stream tracks bytes available correctly");
  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, 11); /* 1 + 10 bytes sent */
  unit_end();


  /* Test 3.4: Reset and send again */
  unit_begin("Reset and send again works correctly");
  unit_assert_ok(xStreamReset(stream));
  unit_assert_ok(xStreamSend(stream, 0xFF));
  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, 1);
  unit_end();


  /* Test 3.5: Fill buffer completely */
  unit_begin("Fill buffer completely succeeds");
  unit_assert_ok(xStreamReset(stream));

  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, STREAM_BUFFER_SIZE);
  unit_end();


  /* Test 3.6: Full buffer rejects new bytes */
  unit_begin("Full buffer rejects new bytes");
  unit_assert_not_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 4: STREAM RECEIVE OPERATIONS
 * ============================================================================
 */
static void test_stream_receive_operations(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i;
  HalfWord_t bytesReceived;
  Byte_t *receivedData = null;


  unit_print("--- Section 4: Stream Receive Operations ---");


  /* Test 4.1: Receive from empty stream fails */
  unit_begin("Receive from empty stream fails");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_not_ok(xStreamReceive(stream, &bytesReceived, &receivedData));
  unit_end();


  /* Test 4.2: Receive single byte */
  unit_begin("Receive single byte succeeds");
  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &receivedData));
  unit_assert_not_null(receivedData);
  unit_assert_equal(bytesReceived, 1);
  unit_assert_equal(receivedData[0], TEST_BYTE_VALUE);
  unit_end();


  /* Test 4.3: Receive multiple bytes */
  unit_begin("Receive multiple bytes succeeds");
  for(i = 0x0u; i < 10; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &receivedData));
  unit_assert_equal(bytesReceived, 10);
  unit_assert_equal(receivedData[0], 0x0u);
  unit_assert_equal(receivedData[9], 0x9u);
  unit_end();


  /* Test 4.4: Receive clears buffer */
  unit_begin("Receive clears buffer");
  {
    Base_t isEmpty;


    unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
    unit_assert_ok(xStreamReceive(stream, &bytesReceived, &receivedData));
    unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
    unit_assert_true(isEmpty);
  } unit_end();


  /* Test 4.5: Receive full buffer */
  unit_begin("Receive full buffer succeeds");
  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &receivedData));
  unit_assert_equal(bytesReceived, STREAM_BUFFER_SIZE);
  unit_assert_equal(receivedData[STREAM_LAST_BYTE_INDEX], STREAM_LAST_BYTE_INDEX);
  unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 5: STREAM STATUS CHECKS
 * ============================================================================
 */
static void test_stream_status_checks(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i;
  HalfWord_t bytesAvailable;


  unit_print("--- Section 5: Stream Status Checks ---");


  /* Test 5.1: Bytes available on empty stream fails */
  unit_begin("Bytes available on empty stream returns error");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_not_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_end();


  /* Test 5.2: Bytes available increases with sends */
  unit_begin("Bytes available increases with sends");
  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, 1);

  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, 2);
  unit_end();


  /* Test 5.3: Bytes available on full buffer */
  unit_begin("Bytes available on full buffer returns capacity");
  unit_assert_ok(xStreamReset(stream));

  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, STREAM_BUFFER_SIZE);
  unit_end();


  /* Test 5.4: Bytes available after receive that empties stream */
  unit_begin("Bytes available fails after stream is emptied");
  {
    HalfWord_t bytesReceived;
    Byte_t *data;


    unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
    unit_assert_not_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  } unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 6: STREAM EMPTY AND FULL CHECKS
 * ============================================================================
 */
static void test_stream_empty_and_full(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i;
  Base_t isEmpty, isFull;


  unit_print("--- Section 6: Stream Empty and Full Checks ---");


  /* Test 6.1: Empty stream check */
  unit_begin("Empty stream check returns true");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
  unit_assert_true(isEmpty);
  unit_assert_ok(xStreamIsFull(stream, &isFull));
  unit_assert_false(isFull);
  unit_end();


  /* Test 6.2: Partially filled stream */
  unit_begin("Partially filled stream is neither empty nor full");
  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
  unit_assert_false(isEmpty);
  unit_assert_ok(xStreamIsFull(stream, &isFull));
  unit_assert_false(isFull);
  unit_end();


  /* Test 6.3: Full stream check */
  unit_begin("Full stream check returns true");
  for(i = 0x0u; i < STREAM_FILL_COUNT - 1; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamIsFull(stream, &isFull));
  unit_assert_true(isFull);
  unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
  unit_assert_false(isEmpty);
  unit_end();


  /* Test 6.4: Empty check after receive */
  unit_begin("Empty check after receive returns true");
  {
    HalfWord_t bytesReceived;
    Byte_t *data;


    unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
    unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
    unit_assert_true(isEmpty);
  } unit_end();


  /* Test 6.5: State transitions */
  unit_begin("State transitions work correctly");
  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
  unit_assert_false(isEmpty);

  unit_assert_ok(xStreamReset(stream));
  unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
  unit_assert_true(isEmpty);
  unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 7: STREAM RESET OPERATIONS
 * ============================================================================
 */
static void test_stream_reset_operations(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i;
  Base_t isEmpty, isFull;
  HalfWord_t bytesAvailable;


  unit_print("--- Section 7: Stream Reset Operations ---");


  /* Test 7.1: Reset empty stream fails */
  unit_begin("Reset empty stream returns error");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_not_ok(xStreamReset(stream));
  unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
  unit_assert_true(isEmpty);
  unit_end();


  /* Test 7.2: Reset partially filled stream */
  unit_begin("Reset partially filled stream clears contents");
  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamReset(stream));
  unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
  unit_assert_true(isEmpty);
  unit_end();


  /* Test 7.3: Reset full stream */
  unit_begin("Reset full stream clears contents");
  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamIsFull(stream, &isFull));
  unit_assert_true(isFull);

  unit_assert_ok(xStreamReset(stream));
  unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
  unit_assert_true(isEmpty);
  unit_assert_ok(xStreamIsFull(stream, &isFull));
  unit_assert_false(isFull);
  unit_end();


  /* Test 7.4: Reset allows new data */
  unit_begin("Reset allows new data to be sent");
  unit_assert_ok(xStreamSend(stream, 0xAA));
  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, 1);
  unit_end();


  /* Test 7.5: Multiple consecutive resets - only first succeeds */
  unit_begin("Reset only works on non-empty streams");
  /* Stream has data from previous test */
  unit_assert_ok(xStreamReset(stream));  /* First reset succeeds (stream has data) */
  unit_assert_not_ok(xStreamReset(stream));  /* Second reset fails (stream now empty) */
  unit_assert_not_ok(xStreamReset(stream));  /* Third reset fails (stream still empty) */
  unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
  unit_assert_true(isEmpty);
  unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 8: STREAM WRAP-AROUND BEHAVIOR
 * ============================================================================
 */
static void test_stream_wrap_around(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i;
  HalfWord_t bytesReceived;
  Byte_t *data;


  unit_print("--- Section 8: Stream Wrap-Around Behavior ---");


  /* Test 8.1: Fill, receive, fill again */
  unit_begin("Fill, receive, fill again works correctly");
  unit_assert_ok(xStreamCreate(&stream));

  /* First fill */
  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
  unit_assert_equal(bytesReceived, STREAM_BUFFER_SIZE);

  /* Second fill */
  for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
    unit_assert_ok(xStreamSend(stream, i + 0x10));
  }

  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
  unit_assert_equal(data[0], 0x10);
  unit_end();


  /* Test 8.2: Partial send, receive, send again */
  unit_begin("Partial send, receive, send again works correctly");
  for(i = 0x0u; i < 10; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));

  for(i = 0x0u; i < 10; i++) {
    unit_assert_ok(xStreamSend(stream, i + 0x20));
  }

  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
  unit_assert_equal(bytesReceived, 10);
  unit_end();


  /* Test 8.3: Repeated fill/receive cycles */
  unit_begin("Repeated fill/receive cycles work correctly");
  {
    HalfWord_t cycle;


    for(cycle = 0x0u; cycle < 5; cycle++) {
      for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
        unit_assert_ok(xStreamSend(stream, i));
      }

      unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
      unit_assert_equal(bytesReceived, STREAM_BUFFER_SIZE);
    }
  } unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 9: MULTIPLE STREAMS
 * ============================================================================
 */
static void test_multiple_streams(void) {
  StreamBuffer_t *stream1 = null;
  StreamBuffer_t *stream2 = null;
  StreamBuffer_t *stream3 = null;
  HalfWord_t bytes1, bytes2, bytes3;


  unit_print("--- Section 9: Multiple Streams ---");


  /* Test 9.1: Create multiple streams */
  unit_begin("Create multiple streams independently");
  unit_assert_ok(xStreamCreate(&stream1));
  unit_assert_ok(xStreamCreate(&stream2));
  unit_assert_ok(xStreamCreate(&stream3));
  unit_assert_not_null(stream1);
  unit_assert_not_null(stream2);
  unit_assert_not_null(stream3);
  unit_end();


  /* Test 9.2: Independent stream operations */
  unit_begin("Streams operate independently");
  unit_assert_ok(xStreamSend(stream1, 0x11));
  unit_assert_ok(xStreamSend(stream2, 0x22));
  unit_assert_ok(xStreamSend(stream2, 0x22));
  unit_assert_ok(xStreamSend(stream3, 0x33));
  unit_assert_ok(xStreamSend(stream3, 0x33));
  unit_assert_ok(xStreamSend(stream3, 0x33));

  unit_assert_ok(xStreamBytesAvailable(stream1, &bytes1));
  unit_assert_ok(xStreamBytesAvailable(stream2, &bytes2));
  unit_assert_ok(xStreamBytesAvailable(stream3, &bytes3));

  unit_assert_equal(bytes1, 1);
  unit_assert_equal(bytes2, 2);
  unit_assert_equal(bytes3, 3);
  unit_end();


  /* Test 9.3: Reset one stream doesn't affect others */
  unit_begin("Reset one stream doesn't affect others");
  unit_assert_ok(xStreamReset(stream2));

  unit_assert_ok(xStreamBytesAvailable(stream1, &bytes1));
  unit_assert_not_ok(xStreamBytesAvailable(stream2, &bytes2));  /* stream2 is now empty */
  unit_assert_ok(xStreamBytesAvailable(stream3, &bytes3));

  unit_assert_equal(bytes1, 1);
  unit_assert_equal(bytes3, 3);
  unit_end();


  /* Test 9.4: Delete one stream doesn't affect others */
  unit_begin("Delete one stream doesn't affect others");
  unit_assert_ok(xStreamDelete(stream2));

  unit_assert_ok(xStreamBytesAvailable(stream1, &bytes1));
  unit_assert_ok(xStreamBytesAvailable(stream3, &bytes3));

  unit_assert_equal(bytes1, 1);
  unit_assert_equal(bytes3, 3);
  unit_end();


  /* Cleanup */
  xStreamDelete(stream1);
  xStreamDelete(stream3);
}


/* ============================================================================
 * SECTION 10: STREAM STRESS OPERATIONS
 * ============================================================================
 */
static void test_stream_stress_operations(void) {
  StreamBuffer_t *streams[MAX_STREAMS_STRESS];
  HalfWord_t i, j;
  HalfWord_t created = 0x0u;


  unit_print("--- Section 10: Stream Stress Operations ---");


  /* Test 10.1: Create many streams */
  unit_begin("Create many streams successfully");
  for(i = 0x0u; i < MAX_STREAMS_STRESS; i++) {
    streams[i] = null;

    if(OK(xStreamCreate(&streams[i]))) {
      created++;
    } else {
      break;
    }
  }

  unit_assert_true(created > 0x0u);
  unit_end();


  /* Test 10.2: Send data to all streams */
  unit_begin("Send data to all streams");
  {
    HalfWord_t sent = 0x0u;


    for(i = 0x0u; i < created; i++) {
      if(OK(xStreamSend(streams[i], (Byte_t) i))) {
        sent++;
      }
    }

    unit_assert_equal(sent, created);
  } unit_end();


  /* Test 10.3: Verify all streams have data */
  unit_begin("Verify all streams have data");
  {
    HalfWord_t count = 0x0u;
    HalfWord_t bytes;


    for(i = 0x0u; i < created; i++) {
      if(OK(xStreamBytesAvailable(streams[i], &bytes)) && (bytes > 0)) {
        count++;
      }
    }

    unit_assert_equal(count, created);
  } unit_end();


  /* Test 10.4: Delete all streams */
  unit_begin("Delete all streams");
  {
    HalfWord_t deleted = 0x0u;


    for(i = 0x0u; i < created; i++) {
      if(OK(xStreamDelete(streams[i]))) {
        deleted++;
      }
    }

    unit_assert_equal(deleted, created);
  } unit_end();


  /* Test 10.5: Rapid send operations */
  unit_begin("Rapid send operations work correctly");
  {
    StreamBuffer_t *stream = null;


    unit_assert_ok(xStreamCreate(&stream));

    for(i = 0x0u; i < STRESS_CYCLE_COUNT; i++) {
      /* Reset only works on non-empty streams, so skip on first iteration */
      if(i > 0x0u) {
        unit_assert_ok(xStreamReset(stream));
      }

      for(j = 0x0u; j < STREAM_FILL_COUNT; j++) {
        unit_assert_ok(xStreamSend(stream, (Byte_t) j));
      }
    }

    xStreamDelete(stream);
  } unit_end();


  /* Test 10.6: Rapid create/delete operations */
  unit_begin("Rapid create/delete operations work correctly");
  {
    StreamBuffer_t *temp = null;


    for(i = 0x0u; i < 20; i++) {
      unit_assert_ok(xStreamCreate(&temp));
      unit_assert_ok(xStreamSend(temp, TEST_BYTE_VALUE));
      unit_assert_ok(xStreamDelete(temp));
    }
  } unit_end();
}


/* ============================================================================
 * SECTION 11: STREAM PARTIAL OPERATIONS
 * ============================================================================
 */
static void test_stream_partial_operations(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i;
  HalfWord_t bytesReceived;
  Byte_t *data;


  unit_print("--- Section 11: Stream Partial Operations ---");


  /* Test 11.1: Send and receive partial data */
  unit_begin("Send and receive partial data works correctly");
  unit_assert_ok(xStreamCreate(&stream));

  for(i = 0x0u; i < 5; i++) {
    unit_assert_ok(xStreamSend(stream, i));
  }

  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
  unit_assert_equal(bytesReceived, 5);
  unit_end();


  /* Test 11.2: Multiple partial sends and receives */
  unit_begin("Multiple partial sends and receives work correctly");
  {
    HalfWord_t cycle;


    for(cycle = 0x0u; cycle < 10; cycle++) {
      for(i = 0x0u; i < 3; i++) {
        unit_assert_ok(xStreamSend(stream, (Byte_t) (cycle + i)));
      }

      unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
      unit_assert_equal(bytesReceived, 3);
    }
  } unit_end();


  /* Test 11.3: Varying send sizes */
  unit_begin("Varying send sizes work correctly");
  for(i = 1; i <= 10; i++) {
    HalfWord_t j;


    for(j = 0x0u; j < i; j++) {
      unit_assert_ok(xStreamSend(stream, (Byte_t) j));
    }

    unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
    unit_assert_equal(bytesReceived, i);
  }

  unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 12: STREAM BOUNDARY CONDITIONS
 * ============================================================================
 */
static void test_stream_boundary_conditions(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i;
  HalfWord_t bytesAvailable;


  unit_print("--- Section 12: Stream Boundary Conditions ---");


  /* Test 12.1: Fill to exact capacity */
  unit_begin("Fill to exact capacity works correctly");
  unit_assert_ok(xStreamCreate(&stream));

  for(i = 0x0u; i < STREAM_BUFFER_SIZE; i++) {
    unit_assert_ok(xStreamSend(stream, (Byte_t) i));
  }

  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, STREAM_BUFFER_SIZE);
  unit_end();


  /* Test 12.2: One byte away from full */
  unit_begin("One byte away from full behaves correctly");
  unit_assert_ok(xStreamReset(stream));

  for(i = 0x0u; i < STREAM_BUFFER_SIZE - 1; i++) {
    unit_assert_ok(xStreamSend(stream, (Byte_t) i));
  }

  unit_assert_ok(xStreamSend(stream, 0xFF));
  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, STREAM_BUFFER_SIZE);
  unit_end();


  /* Test 12.3: Send maximum value byte */
  unit_begin("Send maximum value byte works correctly");
  unit_assert_ok(xStreamReset(stream));
  unit_assert_ok(xStreamSend(stream, 0xFF));

  {
    HalfWord_t bytesReceived;
    Byte_t *data;


    unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
    unit_assert_equal(data[0], 0xFF);
  }

  unit_end();


  /* Test 12.4: Send zero value byte */
  unit_begin("Send zero value byte works correctly");
  unit_assert_ok(xStreamSend(stream, 0x00));

  {
    HalfWord_t bytesReceived;
    Byte_t *data;


    unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
    unit_assert_equal(data[0], 0x00);
  }

  unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 13: STREAM STATE PERSISTENCE
 * ============================================================================
 */
static void test_stream_state_persistence(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t i;
  HalfWord_t bytesAvailable;
  Base_t isEmpty;


  unit_print("--- Section 13: Stream State Persistence ---");


  /* Test 13.1: State persists across operations */
  unit_begin("State persists across operations");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, 1);

  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
  unit_assert_equal(bytesAvailable, 2);
  unit_end();


  /* Test 13.2: State consistency after multiple operations */
  unit_begin("State consistency after multiple operations");
  unit_assert_ok(xStreamReset(stream));

  for(i = 0x0u; i < 10; i++) {
    unit_assert_ok(xStreamSend(stream, (Byte_t) i));
    unit_assert_ok(xStreamBytesAvailable(stream, &bytesAvailable));
    unit_assert_equal(bytesAvailable, i + 1);
  }

  unit_end();


  /* Test 13.3: Rapid state transitions */
  unit_begin("Rapid state transitions maintain consistency");
  {
    HalfWord_t cycle;
    HalfWord_t bytesReceived;
    Byte_t *data;


    for(cycle = 0x0u; cycle < 10; cycle++) {
      unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
      unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
      unit_assert_false(isEmpty);

      unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
      unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
      unit_assert_true(isEmpty);
    }
  } unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 14: STREAM EDGE CASES
 * ============================================================================
 */
static void test_stream_edge_cases(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t bytesReceived;
  Byte_t *data;


  unit_print("--- Section 14: Stream Edge Cases ---");


  /* Test 14.1: Reset immediately after creation fails */
  unit_begin("Reset on empty stream returns error");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_not_ok(xStreamReset(stream));

  {
    Base_t isEmpty;


    unit_assert_ok(xStreamIsEmpty(stream, &isEmpty));
    unit_assert_true(isEmpty);
  }

  unit_end();


  /* Test 14.2: Reset only works on non-empty streams */
  unit_begin("Reset requires stream to have data");
  /* Add data so reset can work */
  unit_assert_ok(xStreamSend(stream, 0xAA));
  unit_assert_ok(xStreamReset(stream));  /* Works - stream had data */
  unit_assert_not_ok(xStreamReset(stream));  /* Fails - stream now empty */
  unit_assert_not_ok(xStreamReset(stream));  /* Fails - stream still empty */
  unit_end();


  /* Test 14.3: Send immediately after receive */
  unit_begin("Send immediately after receive works correctly");
  unit_assert_ok(xStreamSend(stream, 0xAA));
  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
  unit_assert_ok(xStreamSend(stream, 0xBB));
  unit_assert_ok(xStreamReceive(stream, &bytesReceived, &data));
  unit_assert_equal(data[0], 0xBB);
  unit_end();


  /* Test 14.4: Interleaved operations on multiple streams */
  unit_begin("Interleaved operations on multiple streams work correctly");
  {
    StreamBuffer_t *stream1 = null;
    StreamBuffer_t *stream2 = null;


    unit_assert_ok(xStreamCreate(&stream1));
    unit_assert_ok(xStreamCreate(&stream2));

    unit_assert_ok(xStreamSend(stream1, 0x11));
    unit_assert_ok(xStreamSend(stream2, 0x22));
    unit_assert_ok(xStreamSend(stream1, 0x33));

    unit_assert_ok(xStreamReceive(stream2, &bytesReceived, &data));
    unit_assert_equal(data[0], 0x22);

    unit_assert_ok(xStreamReceive(stream1, &bytesReceived, &data));
    unit_assert_equal(bytesReceived, 2);

    xStreamDelete(stream1);
    xStreamDelete(stream2);
  } unit_end();


  /* Cleanup */
  xStreamDelete(stream);
}


/* ============================================================================
 * SECTION 15: STREAM DELETE AND CLEANUP
 * ============================================================================
 */
static void test_stream_delete_and_cleanup(void) {
  StreamBuffer_t *stream = null;
  HalfWord_t bytes;


  unit_print("--- Section 15: Stream Delete and Cleanup ---");


  /* Test 15.1: Delete empty stream */
  unit_begin("Delete empty stream succeeds");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_ok(xStreamDelete(stream));
  unit_end();


  /* Test 15.2: Delete stream with data */
  unit_begin("Delete stream with data succeeds");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_ok(xStreamDelete(stream));
  unit_end();


  /* Test 15.3: Delete full stream */
  unit_begin("Delete full stream succeeds");
  {
    HalfWord_t i;


    unit_assert_ok(xStreamCreate(&stream));

    for(i = 0x0u; i < STREAM_FILL_COUNT; i++) {
      unit_assert_ok(xStreamSend(stream, (Byte_t) i));
    }

    unit_assert_ok(xStreamDelete(stream));
  } unit_end();


  /* Test 15.4: Operations on deleted stream fail */
  unit_begin("Operations on deleted stream fail gracefully");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_ok(xStreamDelete(stream));

  /* These should fail */
  unit_assert_not_ok(xStreamSend(stream, TEST_BYTE_VALUE));
  unit_assert_not_ok(xStreamBytesAvailable(stream, &bytes));
  unit_end();


  /* Test 15.5: Create, delete, recreate stream */
  unit_begin("Create, delete, recreate stream works correctly");
  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_ok(xStreamSend(stream, 0xAA));
  unit_assert_ok(xStreamDelete(stream));

  unit_assert_ok(xStreamCreate(&stream));
  unit_assert_ok(xStreamSend(stream, 0xBB));
  unit_assert_ok(xStreamBytesAvailable(stream, &bytes));
  unit_assert_equal(bytes, 1);
  unit_assert_ok(xStreamDelete(stream));
  unit_end();
}
