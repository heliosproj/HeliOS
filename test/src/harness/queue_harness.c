/*UNCRUSTIFY-OFF*/
/**
 * @file queue_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.5.0
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
#include "queue_harness.h"


void queue_harness(void) {
  Queue_t *queue01;
  QueueMessage_t *queue02;
  Base_t res;


  unit_begin("Queue creation enforces minimum capacity of 7 bytes");
  queue01 = null;
  unit_assert_not_ok(xQueueCreate(&queue01, 0x4));
  unit_assert_null(queue01);
  unit_assert_ok(xQueueCreate(&queue01, 0x7));
  unit_assert_not_null(queue01);
  unit_end();
  unit_begin("Queue accepts messages until reaching capacity");
  unit_assert_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE1"));
  unit_assert_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE2"));
  unit_assert_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE3"));
  unit_assert_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE4"));
  unit_assert_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE5"));
  unit_assert_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE6"));
  unit_assert_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE7"));
  unit_assert_not_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE8"));
  unit_end();
  unit_begin("Queue length reflects number of messages");
  unit_assert_ok(xQueueGetLength(queue01, &res));
  unit_assert_equal(res, 0x7u);
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
  unit_assert_equal(res, 0x5u);
  unit_assert_ok(xQueueDelete(queue01));
  unit_end();
  unit_begin("Queue lock prevents new messages from being sent");
  unit_assert_ok(xQueueCreate(&queue01, 5));
  unit_assert_not_null(queue01);
  unit_assert_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE1"));
  unit_assert_ok(xQueueLockQueue(queue01));
  unit_assert_not_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE2"));
  unit_end();
  unit_begin("Queue unlock allows messages to be sent again");
  unit_assert_ok(xQueueUnLockQueue(queue01));
  unit_assert_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE3"));
  unit_assert_ok(xQueueGetLength(queue01, &res));
  unit_assert_equal(res, 0x2u);
  unit_end();
  unit_begin("Queue delete invalidates queue handle");
  unit_assert_ok(xQueueDelete(queue01));
  unit_assert_not_ok(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE4"));
  unit_end();

  return;
}