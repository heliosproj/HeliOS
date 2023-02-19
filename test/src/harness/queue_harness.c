/*UNCRUSTIFY-OFF*/
/**
 * @file queue_harness.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.4.0
 * @date 2022-08-27
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
  unit_try(!ISOK(xQueueCreate(&queue01, 0x4)));
  unit_try(null == queue01);
  unit_try(ISOK(xQueueCreate(&queue01, 0x7)));
  unit_try(null != queue01);
  unit_end();
  unit_begin("xQueueSend()");
  unit_try(ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE1")));
  unit_try(ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE2")));
  unit_try(ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE3")));
  unit_try(ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE4")));
  unit_try(ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE5")));
  unit_try(ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE6")));
  unit_try(ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE7")));
  unit_try(!ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE8")));
  unit_end();
  unit_begin("xQueueGetLength()");
  unit_try(ISOK(xQueueGetLength(queue01, &res)));
  unit_try(0x7u == res);
  unit_end();
  unit_begin("xQueueIsQueueEmpty()");
  unit_try(ISOK(xQueueIsQueueEmpty(queue01, &res)));
  unit_try(false == res);
  unit_end();
  unit_begin("xQueueIsQueueFull()");
  unit_try(ISOK(xQueueIsQueueFull(queue01, &res)));
  unit_try(true == res);
  unit_end();
  unit_begin("xQueueMessagesWaiting()");
  unit_try(ISOK(xQueueMessagesWaiting(queue01, &res)));
  unit_try(true == res);
  unit_end();
  unit_begin("xQueuePeek()");
  queue02 = null;
  unit_try(ISOK(xQueuePeek(queue01, &queue02)));
  unit_try(null != queue02);
  unit_try(0x8u == queue02->messageBytes);
  unit_try(0x0u == strncmp("MESSAGE1", (char *) queue02->messageValue, 0x8));
  unit_try(ISOK(xMemFree(queue02)));
  unit_end();
  unit_begin("xQueueReceive()");
  unit_try(ISOK(xQueueReceive(queue01, &queue02)));
  unit_try(null != queue02);
  unit_try(0x8u == queue02->messageBytes);
  unit_try(0x0u == strncmp("MESSAGE1", (char *) queue02->messageValue, 0x8));
  unit_try(ISOK(xMemFree(queue02)));
  unit_end();
  unit_begin("xQueueDropMessage()");
  unit_try(ISOK(xQueueDropMessage(queue01)));
  unit_try(ISOK(xQueueGetLength(queue01, &res)));
  unit_try(0x5u == res);
  unit_try(ISOK(xQueueDelete(queue01)));
  unit_end();
  unit_begin("xQueueLockQueue()");
  unit_try(ISOK(xQueueCreate(&queue01, 5)));
  unit_try(null != queue01);
  unit_try(ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE1")));
  unit_try(ISOK(xQueueLockQueue(queue01)));
  unit_try(!ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE2")));
  unit_end();
  unit_begin("xQueueUnlockQueue()");
  unit_try(ISOK(xQueueUnLockQueue(queue01)));
  unit_try(ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE3")));
  unit_try(ISOK(xQueueGetLength(queue01, &res)));
  unit_try(0x2u == res);
  unit_end();
  unit_begin("xQueueDelete()");
  unit_try(ISOK(xQueueDelete(queue01)));
  unit_try(!ISOK(xQueueSend(queue01, 0x8, (Byte_t *) "MESSAGE4")));
  unit_end();

  return;
}