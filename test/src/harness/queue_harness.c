/*UNCRUSTIFY-OFF*/
/**
 * @file queue_harness.c
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
#include "queue_harness.h"


/* Test constants */
#define QUEUE_MIN_CAPACITY 0x7 /* Minimum queue capacity in bytes */
#define QUEUE_TEST_CAPACITY 5 /* Test queue capacity for lock tests */
#define MESSAGE_SIZE 0x8 /* Size of test messages */
#define QUEUE_EXPECTED_LENGTH 0x7 /* Expected number of messages */
#define QUEUE_AFTER_DROP 0x5 /* Messages remaining after drop */
#define QUEUE_AFTER_UNLOCK 0x2 /* Messages after unlock test */


void queue_harness(void) {
  Queue_t *queue01;
  QueueMessage_t *queue02;
  Base_t res;


  unit_begin("Queue creation enforces minimum capacity of 7 bytes");
  queue01 = null;
  unit_assert_not_ok(xQueueCreate(&queue01, 0x4) /* Too small - below minimum */);
  unit_assert_null(queue01);
  unit_assert_ok(xQueueCreate(&queue01, QUEUE_MIN_CAPACITY));
  unit_assert_not_null(queue01);
  unit_end();
  unit_begin("Queue accepts messages until reaching capacity");
  unit_assert_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE1"));
  unit_assert_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE2"));
  unit_assert_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE3"));
  unit_assert_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE4"));
  unit_assert_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE5"));
  unit_assert_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE6"));
  unit_assert_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE7"));
  unit_assert_not_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE8"));
  unit_end();
  unit_begin("Queue length reflects number of messages");
  unit_assert_ok(xQueueGetLength(queue01, &res));
  unit_assert_equal(res, QUEUE_EXPECTED_LENGTH);
  unit_end();
  unit_begin("Queue empty check correctly identifies non-empty queue");
  unit_assert_ok(xQueueIsQueueEmpty(queue01, &res));
  unit_assert_false(res);
  unit_end();
  unit_begin("Queue full check correctly identifies full queue");
  unit_assert_ok(xQueueIsQueueFull(queue01, &res));
  unit_assert_true(res);
  unit_end();
  unit_begin("Queue messages waiting check returns true when messages present");
  unit_assert_ok(xQueueMessagesWaiting(queue01, &res));
  unit_assert_true(res);
  unit_end();
  unit_begin("Queue peek returns first message without removing it");
  queue02 = null;
  unit_assert_ok(xQueuePeek(queue01, &queue02));
  unit_assert_not_null(queue02);
  unit_assert_equal(queue02->messageBytes, 0x8u);
  unit_assert_equal(strncmp("MESSAGE1", (char *) queue02->messageValue, 0x8), 0x0u);
  unit_assert_ok(xMemFree(queue02));
  unit_end();
  unit_begin("Queue receive returns and removes first message");
  unit_assert_ok(xQueueReceive(queue01, &queue02));
  unit_assert_not_null(queue02);
  unit_assert_equal(queue02->messageBytes, 0x8u);
  unit_assert_equal(strncmp("MESSAGE1", (char *) queue02->messageValue, 0x8), 0x0u);
  unit_assert_ok(xMemFree(queue02));
  unit_end();
  unit_begin("Queue drop message removes first message");
  unit_assert_ok(xQueueDropMessage(queue01));
  unit_assert_ok(xQueueGetLength(queue01, &res));
  unit_assert_equal(res, QUEUE_AFTER_DROP);
  unit_assert_ok(xQueueDelete(queue01));
  unit_end();
  unit_begin("Queue lock prevents new messages from being sent");
  unit_assert_ok(xQueueCreate(&queue01, QUEUE_TEST_CAPACITY));
  unit_assert_not_null(queue01);
  unit_assert_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE1"));
  unit_assert_ok(xQueueLockQueue(queue01));
  unit_assert_not_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE2"));
  unit_end();
  unit_begin("Queue unlock allows messages to be sent again");
  unit_assert_ok(xQueueUnLockQueue(queue01));
  unit_assert_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE3"));
  unit_assert_ok(xQueueGetLength(queue01, &res));
  unit_assert_equal(res, QUEUE_AFTER_UNLOCK);
  unit_end();
  unit_begin("Queue delete invalidates queue handle");
  unit_assert_ok(xQueueDelete(queue01));
  unit_assert_not_ok(xQueueSend(queue01, MESSAGE_SIZE, (Byte_t *) "MESSAGE4"));
  unit_end();

  return;
}