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
#define QUEUE_SMALL_CAPACITY 10 /* Small test queue capacity */
#define QUEUE_MEDIUM_CAPACITY 20 /* Medium test queue capacity */
#define QUEUE_LARGE_CAPACITY 50 /* Large test queue capacity */
#define QUEUE_INVALID_CAPACITY 0x4 /* Too small - below minimum */
#define MESSAGE_SIZE 0x8 /* Size of test messages */
#define MAX_QUEUES_STRESS 30 /* Number of queues for stress testing */
#define STRESS_CYCLE_COUNT 20 /* Number of stress test cycles */

/* Test message strings */
#define TEST_MESSAGE_1 "MESSAGE1"
#define TEST_MESSAGE_2 "MESSAGE2"
#define TEST_MESSAGE_3 "MESSAGE3"
#define TEST_MESSAGE_4 "MESSAGE4"
#define TEST_MESSAGE_5 "MESSAGE5"
#define TEST_MESSAGE_6 "MESSAGE6"
#define TEST_MESSAGE_7 "MESSAGE7"
#define TEST_MESSAGE_8 "MESSAGE8"


void queue_harness(void) {
  Queue_t *queue = null;
  Queue_t *queue2 = null;
  Queue_t *queue3 = null;
  Queue_t *queues[MAX_QUEUES_STRESS];
  QueueMessage_t *message = null;
  QueueMessage_t *message2 = null;
  Base_t result;
  Base_t length;
  Base_t isEmpty;
  Base_t isFull;
  Base_t hasMessages;
  Word_t i, j;


  unit_print("=== COMPREHENSIVE QUEUE TEST SUITE ===");


  /* ============================================================================
   * SECTION 1: ERROR HANDLING AND NULL POINTER VALIDATION
   * ============================================================================
   */
  unit_print("--- Section 1: Error Handling and NULL Pointer Tests ---");


  /* Test 1.1: NULL pointer in xQueueCreate */
  unit_begin("xQueueCreate with NULL pointer returns error");
  unit_assert_not_ok(xQueueCreate(null, QUEUE_MIN_CAPACITY));
  unit_end();


  /* Test 1.2: Create a valid queue for error tests */
  unit_begin("Create valid queue for error testing");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_not_null(queue);
  unit_end();


  /* Test 1.3: NULL pointer in xQueueDelete */
  unit_begin("xQueueDelete with NULL pointer returns error");
  unit_assert_not_ok(xQueueDelete(null));
  unit_end();


  /* Test 1.4: NULL pointer in xQueueGetLength */
  unit_begin("xQueueGetLength with NULL queue returns error");
  unit_assert_not_ok(xQueueGetLength(null, &result));
  unit_end();


  /* Test 1.5: NULL pointer in xQueueGetLength result */
  unit_begin("xQueueGetLength with NULL result returns error");
  unit_assert_not_ok(xQueueGetLength(queue, null));
  unit_end();


  /* Test 1.6: NULL pointer in xQueueIsQueueEmpty */
  unit_begin("xQueueIsQueueEmpty with NULL queue returns error");
  unit_assert_not_ok(xQueueIsQueueEmpty(null, &result));
  unit_end();


  /* Test 1.7: NULL pointer in xQueueIsQueueEmpty result */
  unit_begin("xQueueIsQueueEmpty with NULL result returns error");
  unit_assert_not_ok(xQueueIsQueueEmpty(queue, null));
  unit_end();


  /* Test 1.8: NULL pointer in xQueueIsQueueFull */
  unit_begin("xQueueIsQueueFull with NULL queue returns error");
  unit_assert_not_ok(xQueueIsQueueFull(null, &result));
  unit_end();


  /* Test 1.9: NULL pointer in xQueueIsQueueFull result */
  unit_begin("xQueueIsQueueFull with NULL result returns error");
  unit_assert_not_ok(xQueueIsQueueFull(queue, null));
  unit_end();


  /* Test 1.10: NULL pointer in xQueueMessagesWaiting */
  unit_begin("xQueueMessagesWaiting with NULL queue returns error");
  unit_assert_not_ok(xQueueMessagesWaiting(null, &result));
  unit_end();


  /* Test 1.11: NULL pointer in xQueueMessagesWaiting result */
  unit_begin("xQueueMessagesWaiting with NULL result returns error");
  unit_assert_not_ok(xQueueMessagesWaiting(queue, null));
  unit_end();


  /* Test 1.12: NULL pointer in xQueueSend */
  unit_begin("xQueueSend with NULL queue returns error");
  unit_assert_not_ok(xQueueSend(null, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_end();


  /* Test 1.13: NULL pointer in xQueueSend data */
  unit_begin("xQueueSend with NULL data returns error");
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, null));
  unit_end();


  /* Test 1.14: xQueueSend with zero size */
  unit_begin("xQueueSend with zero size returns error");
  unit_assert_not_ok(xQueueSend(queue, 0x0u, (Byte_t *) TEST_MESSAGE_1));
  unit_end();


  /* Test 1.15: NULL pointer in xQueuePeek */
  unit_begin("xQueuePeek with NULL queue returns error");
  unit_assert_not_ok(xQueuePeek(null, &message));
  unit_end();


  /* Test 1.16: NULL pointer in xQueuePeek message */
  unit_begin("xQueuePeek with NULL message pointer returns error");
  unit_assert_not_ok(xQueuePeek(queue, null));
  unit_end();


  /* Test 1.17: NULL pointer in xQueueReceive */
  unit_begin("xQueueReceive with NULL queue returns error");
  unit_assert_not_ok(xQueueReceive(null, &message));
  unit_end();


  /* Test 1.18: NULL pointer in xQueueReceive message */
  unit_begin("xQueueReceive with NULL message pointer returns error");
  unit_assert_not_ok(xQueueReceive(queue, null));
  unit_end();


  /* Test 1.19: NULL pointer in xQueueDropMessage */
  unit_begin("xQueueDropMessage with NULL queue returns error");
  unit_assert_not_ok(xQueueDropMessage(null));
  unit_end();


  /* Test 1.20: NULL pointer in xQueueLockQueue */
  unit_begin("xQueueLockQueue with NULL queue returns error");
  unit_assert_not_ok(xQueueLockQueue(null));
  unit_end();


  /* Test 1.21: NULL pointer in xQueueUnLockQueue */
  unit_begin("xQueueUnLockQueue with NULL queue returns error");
  unit_assert_not_ok(xQueueUnLockQueue(null));
  unit_end();


  /* Cleanup */
  unit_begin("Cleanup error test queue");
  unit_assert_ok(xQueueDelete(queue));
  queue = null;
  unit_end();


  /* ============================================================================
   * SECTION 2: QUEUE CREATION
   * ============================================================================
   */
  unit_print("--- Section 2: Queue Creation ---");


  /* Test 2.1: Queue creation enforces minimum capacity */
  unit_begin("Queue creation enforces minimum capacity of 7 bytes");
  unit_assert_not_ok(xQueueCreate(&queue, QUEUE_INVALID_CAPACITY));
  unit_assert_null(queue);
  unit_end();


  /* Test 2.2: Queue creation with minimum capacity succeeds */
  unit_begin("Queue creation with minimum capacity succeeds");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_not_null(queue);
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* Test 2.3: Queue creation with small capacity succeeds */
  unit_begin("Queue creation with small capacity succeeds");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_SMALL_CAPACITY));
  unit_assert_not_null(queue);
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* Test 2.4: Queue creation with medium capacity succeeds */
  unit_begin("Queue creation with medium capacity succeeds");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MEDIUM_CAPACITY));
  unit_assert_not_null(queue);
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* Test 2.5: Queue creation with large capacity succeeds */
  unit_begin("Queue creation with large capacity succeeds");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_LARGE_CAPACITY));
  unit_assert_not_null(queue);
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* Test 2.6: Multiple queue creation succeeds */
  unit_begin("Multiple queue creation succeeds");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueCreate(&queue2, QUEUE_SMALL_CAPACITY));
  unit_assert_ok(xQueueCreate(&queue3, QUEUE_MEDIUM_CAPACITY));
  xQueueDelete(queue);
  xQueueDelete(queue2);
  xQueueDelete(queue3);
  queue = null;
  queue2 = null;
  queue3 = null;
  unit_end();


  /* Test 2.7: Newly created queues are empty */
  unit_begin("Newly created queues are empty");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueIsQueueEmpty(queue, &isEmpty));
  unit_assert_true(isEmpty);
  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_false(isFull);
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* ============================================================================
   * SECTION 3: QUEUE SEND OPERATIONS
   * ============================================================================
   */
  unit_print("--- Section 3: Queue Send Operations ---");


  /* Test 3.1: Single message send succeeds */
  unit_begin("Single message send succeeds");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_end();


  /* Test 3.2: Multiple message sends succeed */
  unit_begin("Multiple sequential message sends succeed");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_end();


  /* Test 3.3: Queue tracks length correctly */
  unit_begin("Queue tracks message count correctly");
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 4);
  unit_end();


  /* Test 3.4: Fill queue to capacity */
  unit_begin("Queue can be filled to capacity");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_6));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_7));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 7);
  unit_end();


  /* Test 3.5: Full queue rejects new messages */
  unit_begin("Full queue rejects new messages");
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_8));
  unit_end();


  /* Test 3.6: Queue full check returns true */
  unit_begin("Queue full check correctly identifies full queue");
  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_true(isFull);
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  queue = null;


  /* ============================================================================
   * SECTION 4: QUEUE RECEIVE OPERATIONS
   * ============================================================================
   */
  unit_print("--- Section 4: Queue Receive Operations ---");


  /* Test 4.1: Receive from empty queue fails */
  unit_begin("Receive from empty queue returns error");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_not_ok(xQueueReceive(queue, &message));
  unit_end();


  /* Test 4.2: Receive single message succeeds */
  unit_begin("Receive single message succeeds");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_not_null(message);
  unit_assert_equal(message->messageBytes, MESSAGE_SIZE);
  unit_assert_equal(strncmp(TEST_MESSAGE_1, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  message = null;
  unit_end();


  /* Test 4.3: Receive returns messages in FIFO order */
  unit_begin("Receive returns messages in FIFO order");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));

  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_1, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));

  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_2, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));

  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_3, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  message = null;
  unit_end();


  /* Test 4.4: Receiving all messages empties queue */
  unit_begin("Receiving all messages empties queue");
  unit_assert_ok(xQueueIsQueueEmpty(queue, &isEmpty));
  unit_assert_true(isEmpty);
  unit_end();


  /* Test 4.5: Receive from full buffer works correctly */
  unit_begin("Receive from full buffer works correctly");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_6));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_7));

  /* Drain all messages */
  for(i = 0x0u; i < 7; i++) {
    unit_assert_ok(xQueueReceive(queue, &message));
    unit_assert_ok(xMemFree(message));
  }

  unit_assert_ok(xQueueIsQueueEmpty(queue, &isEmpty));
  unit_assert_true(isEmpty);
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  queue = null;


  /* ============================================================================
   * SECTION 5: QUEUE PEEK OPERATIONS
   * ============================================================================
   */
  unit_print("--- Section 5: Queue Peek Operations ---");


  /* Test 5.1: Peek from empty queue fails */
  unit_begin("Peek from empty queue returns error");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_not_ok(xQueuePeek(queue, &message));
  unit_end();


  /* Test 5.2: Peek returns first message without removing it */
  unit_begin("Peek returns first message without removing it");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueuePeek(queue, &message));
  unit_assert_not_null(message);
  unit_assert_equal(message->messageBytes, MESSAGE_SIZE);
  unit_assert_equal(strncmp(TEST_MESSAGE_1, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  message = null;
  unit_end();


  /* Test 5.3: Peek does not change queue length */
  unit_begin("Peek does not change queue length");
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 1);
  unit_end();


  /* Test 5.4: Multiple peeks return same message */
  unit_begin("Multiple peeks return same message");
  unit_assert_ok(xQueuePeek(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_1, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));

  unit_assert_ok(xQueuePeek(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_1, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  message = null;
  unit_end();


  /* Test 5.5: Peek then receive returns same message */
  unit_begin("Peek then receive returns same message");
  unit_assert_ok(xQueuePeek(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_1, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));

  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_1, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  message = null;
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  queue = null;


  /* ============================================================================
   * SECTION 6: QUEUE STATUS CHECKS
   * ============================================================================
   */
  unit_print("--- Section 6: Queue Status Checks ---");


  /* Test 6.1: Empty queue status checks */
  unit_begin("Empty queue status checks work correctly");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueIsQueueEmpty(queue, &isEmpty));
  unit_assert_true(isEmpty);
  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_false(isFull);
  unit_assert_ok(xQueueMessagesWaiting(queue, &hasMessages));
  unit_assert_false(hasMessages);
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 0);
  unit_end();


  /* Test 6.2: Partially filled queue status checks */
  unit_begin("Partially filled queue status checks work correctly");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueIsQueueEmpty(queue, &isEmpty));
  unit_assert_false(isEmpty);
  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_false(isFull);
  unit_assert_ok(xQueueMessagesWaiting(queue, &hasMessages));
  unit_assert_true(hasMessages);
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 2);
  unit_end();


  /* Test 6.3: Full queue status checks */
  unit_begin("Full queue status checks work correctly");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_6));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_7));
  unit_assert_ok(xQueueIsQueueEmpty(queue, &isEmpty));
  unit_assert_false(isEmpty);
  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_true(isFull);
  unit_assert_ok(xQueueMessagesWaiting(queue, &hasMessages));
  unit_assert_true(hasMessages);
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 7);
  unit_end();


  /* Test 6.4: Status after receive */
  unit_begin("Status checks after receive work correctly");
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_ok(xMemFree(message));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 6);
  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_false(isFull);
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  queue = null;


  /* ============================================================================
   * SECTION 7: QUEUE DROP OPERATIONS
   * ============================================================================
   */
  unit_print("--- Section 7: Queue Drop Operations ---");


  /* Test 7.1: Drop from empty queue fails */
  unit_begin("Drop from empty queue returns error");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_not_ok(xQueueDropMessage(queue));
  unit_end();


  /* Test 7.2: Drop single message succeeds */
  unit_begin("Drop single message succeeds");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueDropMessage(queue));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 1);
  unit_end();


  /* Test 7.3: Drop removes first message (FIFO) */
  unit_begin("Drop removes first message in FIFO order");
  /* MESSAGE_2 should still be in queue */
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_2, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  message = null;
  unit_end();


  /* Test 7.4: Multiple drops work correctly */
  unit_begin("Multiple drops work correctly");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueDropMessage(queue));
  unit_assert_ok(xQueueDropMessage(queue));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 1);
  unit_end();


  /* Test 7.5: Drop then send works correctly */
  unit_begin("Drop then send works correctly");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 2);
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  queue = null;


  /* ============================================================================
   * SECTION 8: QUEUE LOCK AND UNLOCK OPERATIONS
   * ============================================================================
   */
  unit_print("--- Section 8: Queue Lock and Unlock Operations ---");


  /* Test 8.1: Lock prevents new messages */
  unit_begin("Lock prevents new messages from being sent");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_SMALL_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueLockQueue(queue));
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_end();


  /* Test 8.2: Lock preserves existing messages */
  unit_begin("Lock preserves existing messages");
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 1);
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_ok(xMemFree(message));
  message = null;
  unit_end();


  /* Test 8.3: Unlock allows messages again */
  unit_begin("Unlock allows messages to be sent again");
  unit_assert_ok(xQueueUnLockQueue(queue));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 2);
  unit_end();


  /* Test 8.4: Multiple lock/unlock cycles work correctly */
  unit_begin("Multiple lock/unlock cycles work correctly");
  unit_assert_ok(xQueueLockQueue(queue));
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueUnLockQueue(queue));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueLockQueue(queue));
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_assert_ok(xQueueUnLockQueue(queue));
  unit_end();


  /* Test 8.5: Unlock on unlocked queue returns error */
  unit_begin("Unlock on already unlocked queue returns error");
  unit_assert_not_ok(xQueueUnLockQueue(queue));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  queue = null;


  /* ============================================================================
   * SECTION 9: MULTIPLE QUEUES
   * ============================================================================
   */
  unit_print("--- Section 9: Multiple Queues ---");


  /* Test 9.1: Create multiple queues independently */
  unit_begin("Create multiple queues independently");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueCreate(&queue2, QUEUE_SMALL_CAPACITY));
  unit_assert_ok(xQueueCreate(&queue3, QUEUE_MEDIUM_CAPACITY));
  unit_end();


  /* Test 9.2: Queues operate independently */
  unit_begin("Queues operate independently");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue2, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueSend(queue2, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueueSend(queue3, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueSend(queue3, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_assert_ok(xQueueSend(queue3, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_6));

  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 1);
  unit_assert_ok(xQueueGetLength(queue2, &length));
  unit_assert_equal(length, 2);
  unit_assert_ok(xQueueGetLength(queue3, &length));
  unit_assert_equal(length, 3);
  unit_end();


  /* Test 9.3: Lock one queue doesn't affect others */
  unit_begin("Lock one queue doesn't affect others");
  unit_assert_ok(xQueueLockQueue(queue2));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_7));
  unit_assert_not_ok(xQueueSend(queue2, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_8));
  unit_assert_ok(xQueueSend(queue3, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_8));
  unit_assert_ok(xQueueUnLockQueue(queue2));
  unit_end();


  /* Test 9.4: Delete one queue doesn't affect others */
  unit_begin("Delete one queue doesn't affect others");
  unit_assert_ok(xQueueDelete(queue2));
  queue2 = null;

  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 2);
  unit_assert_ok(xQueueGetLength(queue3, &length));
  unit_assert_equal(length, 4);
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  xQueueDelete(queue3);
  queue = null;
  queue3 = null;


  /* ============================================================================
   * SECTION 10: QUEUE STRESS OPERATIONS
   * ============================================================================
   */
  unit_print("--- Section 10: Queue Stress Operations ---");


  /* Test 10.1: Create many queues successfully */
  unit_begin("Create many queues successfully");
  for(i = 0x0u; i < MAX_QUEUES_STRESS; i++) {
    unit_assert_ok(xQueueCreate(&queues[i], QUEUE_MIN_CAPACITY));
  }
  unit_end();


  /* Test 10.2: Send messages to all queues */
  unit_begin("Send messages to all queues");
  for(i = 0x0u; i < MAX_QUEUES_STRESS; i++) {
    unit_assert_ok(xQueueSend(queues[i], MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  }
  unit_end();


  /* Test 10.3: Verify all queues have messages */
  unit_begin("Verify all queues have messages");
  for(i = 0x0u; i < MAX_QUEUES_STRESS; i++) {
    unit_assert_ok(xQueueGetLength(queues[i], &length));
    unit_assert_equal(length, 1);
  }
  unit_end();


  /* Test 10.4: Delete all queues */
  unit_begin("Delete all queues");
  for(i = 0x0u; i < MAX_QUEUES_STRESS; i++) {
    unit_assert_ok(xQueueDelete(queues[i]));
  }
  unit_end();


  /* Test 10.5: Rapid send/receive operations */
  unit_begin("Rapid send/receive operations work correctly");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MEDIUM_CAPACITY));
  for(i = 0x0u; i < STRESS_CYCLE_COUNT; i++) {
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
    unit_assert_ok(xQueueReceive(queue, &message));
    unit_assert_ok(xMemFree(message));
    unit_assert_ok(xQueueReceive(queue, &message));
    unit_assert_ok(xMemFree(message));
  }
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* Test 10.6: Rapid create/delete operations */
  unit_begin("Rapid create/delete operations work correctly");
  for(i = 0x0u; i < STRESS_CYCLE_COUNT; i++) {
    unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
    unit_assert_ok(xQueueDelete(queue));
  }
  queue = null;
  unit_end();


  /* ============================================================================
   * SECTION 11: QUEUE BOUNDARY CONDITIONS
   * ============================================================================
   */
  unit_print("--- Section 11: Queue Boundary Conditions ---");


  /* Test 11.1: Fill to exact capacity */
  unit_begin("Fill to exact capacity works correctly");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  for(i = 0x0u; i < 7; i++) {
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  }
  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_true(isFull);
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* Test 11.2: One message away from full */
  unit_begin("One message away from full behaves correctly");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  for(i = 0x0u; i < 6; i++) {
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  }
  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_false(isFull);
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_true(isFull);
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* Test 11.3: Minimum capacity queue operations */
  unit_begin("Minimum capacity queue operations work correctly");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_ok(xMemFree(message));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* Test 11.4: Large capacity queue operations */
  unit_begin("Large capacity queue operations work correctly");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_LARGE_CAPACITY));
  for(i = 0x0u; i < 10; i++) {
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  }
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 10);
  xQueueDelete(queue);
  queue = null;
  unit_end();


  /* ============================================================================
   * SECTION 12: QUEUE STATE PERSISTENCE
   * ============================================================================
   */
  unit_print("--- Section 12: Queue State Persistence ---");


  /* Test 12.1: State persists across operations */
  unit_begin("State persists across operations");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 1);
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 2);
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_ok(xMemFree(message));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 1);
  unit_end();


  /* Test 12.2: Lock state persists */
  unit_begin("Lock state persists across checks");
  unit_assert_ok(xQueueLockQueue(queue));
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueUnLockQueue(queue));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_end();


  /* Test 12.3: Rapid state transitions maintain consistency */
  unit_begin("Rapid state transitions maintain consistency");
  /* Queue currently has 2 messages from previous tests */
  for(i = 0x0u; i < 10; i++) {
    unit_assert_ok(xQueueReceive(queue, &message));
    unit_assert_ok(xMemFree(message));
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  }
  /* After 10 receive/send cycles, queue still has 2 messages */
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 2);
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  queue = null;


  /* ============================================================================
   * SECTION 13: QUEUE EDGE CASES
   * ============================================================================
   */
  unit_print("--- Section 13: Queue Edge Cases ---");


  /* Test 13.1: Send immediately after receive works */
  unit_begin("Send immediately after receive works correctly");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_ok(xMemFree(message));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_end();


  /* Test 13.2: Peek immediately after send works */
  unit_begin("Peek immediately after send works correctly");
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_ok(xMemFree(message));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  unit_assert_ok(xQueuePeek(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_3, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  message = null;
  unit_end();


  /* Test 13.3: Drop immediately after send works */
  unit_begin("Drop immediately after send works correctly");
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  unit_assert_ok(xQueueDropMessage(queue));
  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 1);
  unit_end();


  /* Test 13.4: Interleaved operations on multiple queues */
  unit_begin("Interleaved operations on multiple queues work correctly");
  unit_assert_ok(xQueueCreate(&queue2, QUEUE_SMALL_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_5));
  unit_assert_ok(xQueueSend(queue2, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_6));
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_ok(xMemFree(message));
  unit_assert_ok(xQueueReceive(queue2, &message));
  unit_assert_ok(xMemFree(message));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_7));
  unit_assert_ok(xQueueSend(queue2, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_8));
  xQueueDelete(queue2);
  queue2 = null;
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  queue = null;


  /* ============================================================================
   * SECTION 14: QUEUE DELETE AND CLEANUP
   * ============================================================================
   */
  unit_print("--- Section 14: Queue Delete and Cleanup ---");


  /* Test 14.1: Delete empty queue succeeds */
  unit_begin("Delete empty queue succeeds");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueDelete(queue));
  queue = null;
  unit_end();


  /* Test 14.2: Delete queue with data succeeds */
  unit_begin("Delete queue with data succeeds");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueDelete(queue));
  queue = null;
  unit_end();


  /* Test 14.3: Delete full queue succeeds */
  unit_begin("Delete full queue succeeds");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  for(i = 0x0u; i < 7; i++) {
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  }
  unit_assert_ok(xQueueDelete(queue));
  queue = null;
  unit_end();


  /* Test 14.4: Operations on deleted queue fail gracefully */
  unit_begin("Operations on deleted queue fail gracefully");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueDelete(queue));
  unit_assert_not_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_not_ok(xQueueGetLength(queue, &length));
  queue = null;
  unit_end();


  /* Test 14.5: Create, delete, recreate queue works correctly */
  unit_begin("Create, delete, recreate queue works correctly");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  unit_assert_ok(xQueueDelete(queue));
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));
  unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_equal(strncmp(TEST_MESSAGE_2, (char *) message->messageValue, MESSAGE_SIZE), 0x0u);
  unit_assert_ok(xMemFree(message));
  unit_assert_ok(xQueueDelete(queue));
  queue = null;
  unit_end();


  /* ============================================================================
   * SECTION 15: QUEUE WRAP-AROUND BEHAVIOR
   * ============================================================================
   */
  unit_print("--- Section 15: Queue Wrap-Around Behavior ---");


  /* Test 15.1: Fill, drain, fill again works correctly */
  unit_begin("Fill, drain, fill again works correctly");
  unit_assert_ok(xQueueCreate(&queue, QUEUE_MIN_CAPACITY));

  /* Fill */
  for(i = 0x0u; i < 7; i++) {
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
  }

  /* Drain */
  for(i = 0x0u; i < 7; i++) {
    unit_assert_ok(xQueueReceive(queue, &message));
    unit_assert_ok(xMemFree(message));
  }

  /* Fill again */
  for(i = 0x0u; i < 7; i++) {
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_2));
  }

  unit_assert_ok(xQueueIsQueueFull(queue, &isFull));
  unit_assert_true(isFull);
  unit_end();


  /* Test 15.2: Partial send, receive, send again works */
  unit_begin("Partial send, receive, send again works correctly");
  /* Drain previous messages */
  for(i = 0x0u; i < 7; i++) {
    unit_assert_ok(xQueueReceive(queue, &message));
    unit_assert_ok(xMemFree(message));
  }

  /* Partial send */
  for(i = 0x0u; i < 3; i++) {
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_3));
  }

  /* Receive one */
  unit_assert_ok(xQueueReceive(queue, &message));
  unit_assert_ok(xMemFree(message));

  /* Send more */
  for(i = 0x0u; i < 5; i++) {
    unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_4));
  }

  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 7);
  unit_end();


  /* Test 15.3: Repeated fill/drain cycles work correctly */
  unit_begin("Repeated fill/drain cycles work correctly");
  for(j = 0x0u; j < 5; j++) {
    /* Drain */
    for(i = 0x0u; i < 7; i++) {
      unit_assert_ok(xQueueReceive(queue, &message));
      unit_assert_ok(xMemFree(message));
    }

    /* Fill */
    for(i = 0x0u; i < 7; i++) {
      unit_assert_ok(xQueueSend(queue, MESSAGE_SIZE, (Byte_t *) TEST_MESSAGE_1));
    }
  }

  unit_assert_ok(xQueueGetLength(queue, &length));
  unit_assert_equal(length, 7);
  unit_end();


  /* Cleanup */
  xQueueDelete(queue);
  queue = null;


  unit_print("=== QUEUE TEST SUITE COMPLETE ===");
}
/*UNCRUSTIFY-ON*/
