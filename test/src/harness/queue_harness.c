/*UNCRUSTIFY-OFF*/
/**
 * @file queue_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for queues
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
#include "queue_harness.h"


/* Test constants */
#define QUEUE_MIN_CAPACITY 0x7 /* Minimum queue capacity in bytes */
#define QUEUE_TEST_CAPACITY 5 /* Test queue capacity for lock tests */
#define QUEUE_INVALID_CAPACITY 0x4 /* Too small - below minimum */
#define MESSAGE_SIZE 0x8 /* Size of test messages */
#define QUEUE_EXPECTED_LENGTH 0x7 /* Expected number of messages */
#define QUEUE_AFTER_DROP 0x5 /* Messages remaining after drop */
#define QUEUE_AFTER_UNLOCK 0x2 /* Messages after unlock test */
/* Test message strings */
#define TEST_MESSAGE_1 "MESSAGE1"
#define TEST_MESSAGE_2 "MESSAGE2"
#define TEST_MESSAGE_3 "MESSAGE3"
#define TEST_MESSAGE_4 "MESSAGE4"
#define TEST_MESSAGE_5 "MESSAGE5"
#define TEST_MESSAGE_6 "MESSAGE6"
#define TEST_MESSAGE_7 "MESSAGE7"
#define TEST_MESSAGE_8 "MESSAGE8"


/* Helper function prototypes */
static void test_error_handling_and_null_pointers(void);
static void test_queue_creation(void);
static void test_queue_send_and_capacity(void);
static void test_queue_status_checks(void);
static void test_queue_peek_and_receive(void);
static void test_queue_drop(void);
static void test_queue_lock_unlock(void);
static void test_queue_delete(void);


void queue_harness(void) {
  unit_print("=== COMPREHENSIVE QUEUE TEST SUITE ===");
  test_error_handling_and_null_pointers();
  test_queue_creation();
  test_queue_send_and_capacity();
  test_queue_status_checks();
  test_queue_peek_and_receive();
  test_queue_drop();
  test_queue_lock_unlock();
  test_queue_delete();
  unit_print("=== QUEUE TEST SUITE COMPLETE ===");
}


/* ============================================================================
 * SECTION 1: ERROR HANDLING AND NULL POINTER VALIDATION
 * ============================================================================
 */
static void test_error_handling_and_null_pointers(void) {
  Queue_t *queue = null;
  Queue_t *nullQueue = null;
  Base_t result;
  QueueMessage_t *message;


  unit_print("--- Section 1: Error Handling and NULL Pointer Tests ---");


  /* Test 1.1: Create a valid queue for error tests */
  unit_begin("Create valid queue for error testing");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_not_null(queue);
  unit_end();


  /* Test 1.2: NULL pointer in xQueueDelete */
  unit_begin("xQueueDelete with NULL pointer returns error");
  unit_assert_not_ok(xQueueDelete(nullQueue));
  unit_end();


  /* Test 1.3: NULL pointer in xQueueGetLength */
  unit_begin("xQueueGetLength with NULL queue returns error");
  unit_assert_not_ok(xQueueGetLength(nullQueue, &result));
  unit_end();


  /* Test 1.4: NULL pointer in xQueueIsQueueEmpty */
  unit_begin("xQueueIsQueueEmpty with NULL queue returns error");
  unit_assert_not_ok(xQueueIsQueueEmpty(nullQueue, &result));
  unit_end();


  /* Test 1.5: NULL pointer in xQueueIsQueueFull */
  unit_begin("xQueueIsQueueFull with NULL queue returns error");
  unit_assert_not_ok(xQueueIsQueueFull(nullQueue, &result));
  unit_end();


  /* Test 1.6: NULL pointer in xQueueMessagesWaiting */
  unit_begin("xQueueMessagesWaiting with NULL queue returns error");
  unit_assert_not_ok(xQueueMessagesWaiting(nullQueue, &result));
  unit_end();


  /* Test 1.7: NULL pointer in xQueueSend */
  unit_begin("xQueueSend with NULL queue returns error");
  unit_assert_not_ok(xQueueSend(nullQueue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_end();


  /* Test 1.8: NULL pointer in xQueuePeek */
  unit_begin("xQueuePeek with NULL queue returns error");
  unit_assert_not_ok(xQueuePeek(nullQueue, &message));
  unit_end();


  /* Test 1.9: NULL pointer in xQueueReceive */
  unit_begin("xQueueReceive with NULL queue returns error");
  unit_assert_not_ok(xQueueReceive(nullQueue, &message));
  unit_end();


  /* Test 1.10: NULL pointer in xQueueDropMessage */
  unit_begin("xQueueDropMessage with NULL queue returns error");
  unit_assert_not_ok(xQueueDropMessage(nullQueue));
  unit_end();


  /* Test 1.11: NULL pointer in xQueueLockQueue */
  unit_begin("xQueueLockQueue with NULL queue returns error");
  unit_assert_not_ok(xQueueLockQueue(nullQueue));
  unit_end();


  /* Test 1.12: NULL pointer in xQueueUnLockQueue */
  unit_begin("xQueueUnLockQueue with NULL queue returns error");
  unit_assert_not_ok(xQueueUnLockQueue(nullQueue));
  unit_end();


  /* Cleanup */
  unit_begin("Cleanup error test queue");
  unit_assert_ok(xQueueDelete(queue));
  unit_end();
}


/* ============================================================================
 * SECTION 2: QUEUE CREATION
 * ============================================================================
 */
static void test_queue_creation(void) {
  Queue_t *queue = null;


  unit_print("--- Section 2: Queue Creation ---");


  /* Test 1.1: Queue creation enforces minimum capacity */
  unit_begin("Queue creation enforces minimum capacity of 7 bytes");


  /* Too small - should fail */
  unit_assert_not_ok(xQueueCreate(&queue, QUEUE_INVALID_CAPACITY));
  unit_assert_null(queue);


  /* Valid size - should succeed */
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_not_null(queue);
  unit_end();


  /* Cleanup for next section */
  xQueueDelete(queue);
}


/* ============================================================================
 * SECTION 3: QUEUE SEND AND CAPACITY
 * ============================================================================
 */
static void test_queue_send_and_capacity(void) {
  Queue_t *queue = null;


  unit_print("--- Section 3: Queue Send and Capacity ---");


  /* Test 2.1: Queue accepts messages until reaching capacity */
  unit_begin("Queue accepts messages until reaching capacity");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_6));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_7));


  /* Should reject additional messages when full */
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_8));
  unit_end();


  /* Cleanup for next section */
  xQueueDelete(queue);
}


/* ============================================================================
 * SECTION 4: QUEUE STATUS CHECKS
 * ============================================================================
 */
static void test_queue_status_checks(void) {
  Queue_t *queue = null;
  Base_t result;


  unit_print("--- Section 4: Queue Status Checks ---");


  /* Test 3.1: Queue length */
  unit_begin("Queue length reflects number of messages");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_6));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_7));
  unit_assert_ok(xQueueGetLength(queue, &result));
  unit_assert_equal(result, QUEUE_EXPECTED_LENGTH);
  unit_end();


  /* Test 3.2: Queue empty check */
  unit_begin("Queue empty check correctly identifies non-empty queue");
  unit_assert_ok(xQueueIsQueueEmpty(queue, &result));
  unit_assert_false(result);
  unit_end();


  /* Test 3.3: Queue full check */
  unit_begin("Queue full check correctly identifies full queue");
  unit_assert_ok(xQueueIsQueueFull(queue, &result));
  unit_assert_true(result);
  unit_end();


  /* Test 3.4: Messages waiting check */
  unit_begin("Queue messages waiting check returns true when messages present");
  unit_assert_ok(xQueueMessagesWaiting(queue, &result));
  unit_assert_true(result);
  unit_end();


  /* Cleanup for next section */
  xQueueDelete(queue);
}


/* ============================================================================
 * SECTION 5: QUEUE PEEK AND RECEIVE
 * ============================================================================
 */
static void test_queue_peek_and_receive(void) {
  Queue_t *queue = null;
  QueueMessage_t *message = null;


  unit_print("--- Section 5: Queue Peek and Receive ---");


  /* Test 4.1: Queue peek */
  unit_begin("Queue peek returns first message without removing it");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueuePeek(queue, &message));
  unit_assert_not_null(message);
  unit_assert_equal(message->messageBytes, MESSAGE_SIZE);
  unit_assert_equal(strncmp(TEST_MESSAGE_1, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  unit_end();


  /* Test 4.2: Queue receive */
  unit_begin("Queue receive returns and removes first message");
  message = null;
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_not_null(message);
  unit_assert_equal(message->messageBytes, MESSAGE_SIZE);
  unit_assert_equal(strncmp(TEST_MESSAGE_1, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  unit_end();


  /* Cleanup for next section */
  xQueueDelete(queue);
}


/* ============================================================================
 * SECTION 6: QUEUE DROP OPERATIONS
 * ============================================================================
 */
static void test_queue_drop(void) {
  Queue_t *queue = null;
  Base_t length;
  QueueMessage_t *message = null;


  unit_print("--- Section 6: Queue Drop Operations ---");


  /* Test 5.1: Drop message */
  unit_begin("Queue drop message removes first message");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_6));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_7));
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_ok(xMemFree(message));
  unit_assert_ok(xQueueDropMessage(queue));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, QUEUE_AFTER_DROP);
  unit_end();


  /* Cleanup for next section */
  xQueueDelete(queue);
}


/* ============================================================================
 * SECTION 7: QUEUE LOCK AND UNLOCK
 * ============================================================================
 */
static void test_queue_lock_unlock(void) {
  Queue_t *queue = null;
  Base_t length;


  unit_print("--- Section 7: Queue Lock and Unlock ---");


  /* Test 6.1: Queue lock */
  unit_begin("Queue lock prevents new messages from being sent");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_TEST_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueLockQueue(queue));
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_end();


  /* Test 6.2: Queue unlock */
  unit_begin("Queue unlock allows messages to be sent again");
  unit_assert_ok(xQueueUnLockQueue(queue));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, QUEUE_AFTER_UNLOCK);
  unit_end();


  /* Cleanup for next section */
  xQueueDelete(queue);
}


/* ============================================================================
 * SECTION 8: QUEUE DELETE AND CLEANUP
 * ============================================================================
 */
static void test_queue_delete(void) {
  Queue_t *queue = null;


  unit_print("--- Section 8: Queue Delete and Cleanup ---");


  /* Test 7.1: Queue delete invalidates handle */
  unit_begin("Queue delete invalidates queue handle");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_TEST_CAPACITY));


  /* Delete should succeed */
  unit_assert_ok(xQueueDelete(queue));


  /* Operations on deleted queue should fail */
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_end();
}