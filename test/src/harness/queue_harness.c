/*UNCRUSTIFY-OFF*/
/**
 * @file queue_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.4.1
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


  unit_begin("xQueueCreate()");
  queue01 = null;
  unit_try(!OK(xQueueCreate(&queue01, 0x4)));
  unit_try(null == queue01);
  unit_try(OK(xQueueCreate(&queue01, 0x7)));
  unit_try(null != queue01);
  unit_end();
  unit_begin("xQueueSend()");
  unit_try(OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE1")));
  unit_try(OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE2")));
  unit_try(OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE3")));
  unit_try(OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE4")));
  unit_try(OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE5")));
  unit_try(OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE6")));
  unit_try(OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE7")));
  unit_try(!OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE8")));
  unit_end();
  unit_begin("xQueueGetLength()");
  unit_try(OK(xQueueGetLength(queue01, &res)));
  unit_try(0x7u == res);
  unit_end();
  unit_begin("xQueueIsQueueEmpty()");
  unit_try(OK(xQueueIsQueueEmpty(queue01, &res)));
  unit_try(false == res);
  unit_end();
  unit_begin("xQueueIsQueueFull()");
  unit_try(OK(xQueueIsQueueFull(queue01, &res)));
  unit_try(true == res);
  unit_end();
  unit_begin("xQueueMessagesWaiting()");
  unit_try(OK(xQueueMessagesWaiting(queue01, &res)));
  unit_try(true == res);
  unit_end();
  unit_begin("xQueuePeek()");
  queue02 = null;
  unit_try(OK(xQueuePeek(queue01, &queue02)));
  unit_try(null != queue02);
  unit_try(0x8u == queue02->messageBytes);
  unit_try(0x0u == strncmp("MESSAGE1", (char *) queue02->messageValue, 0x8));
  unit_try(OK(xMemFree(queue02)));
  unit_end();
  unit_begin("xQueueReceive()");
  unit_try(OK(xQueueReceive(queue01, &queue02)));
  unit_try(null != queue02);
  unit_try(0x8u == queue02->messageBytes);
  unit_try(0x0u == strncmp("MESSAGE1", (char *) queue02->messageValue, 0x8));
  unit_try(OK(xMemFree(queue02)));
  unit_end();
  unit_begin("xQueueDropMessage()");
  unit_try(OK(xQueueDropMessage(queue01)));
  unit_try(OK(xQueueGetLength(queue01, &res)));
  unit_try(0x5u == res);
  unit_try(OK(xQueueDelete(queue01)));
  unit_end();
  unit_begin("xQueueLockQueue()");
  unit_try(OK(xQueueCreate(&queue01, 5)));
  unit_try(null != queue01);
  unit_try(OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE1")));
  unit_try(OK(xQueueLockQueue(queue01)));
  unit_try(!OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE2")));
  unit_end();
  unit_begin("xQueueUnlockQueue()");
  unit_try(OK(xQueueUnLockQueue(queue01)));
  unit_try(OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE3")));
  unit_try(OK(xQueueGetLength(queue01, &res)));
  unit_try(0x2u == res);
  unit_end();
  unit_begin("xQueueDelete()");
  unit_try(OK(xQueueDelete(queue01)));
  unit_try(!OK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE4")));
  unit_end();

  return;
}