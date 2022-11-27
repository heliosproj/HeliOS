/**
 * @file queue_harness.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.3.5
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

#include "queue_harness.h"


void queue_harness(void) {

  Queue_t *queue01;

  QueueMessage_t *queue02;

  unit_begin("xQueueCreate()");

  queue01 = NULL;

  queue01 = xQueueCreate(0x4);

  unit_try(NULL == queue01);

  queue01 = xQueueCreate(0x7);

  unit_try(NULL != queue01);

  unit_end();



  unit_begin("xQueueSend()");

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE1"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE2"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE3"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE4"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE5"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE6"));

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE7"));

  unit_try(RETURN_FAILURE == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE8"));

  unit_end();



  unit_begin("xQueueGetLength()");

  unit_try(0x7u == xQueueGetLength(queue01));

  unit_end();



  unit_begin("xQueueIsQueueEmpty()");

  unit_try(false == xQueueIsQueueEmpty(queue01));

  unit_end();



  unit_begin("xQueueIsQueueFull()");

  unit_try(true == xQueueIsQueueFull(queue01));

  unit_end();



  unit_begin("xQueueMessagesWaiting()");

  unit_try(true == xQueueMessagesWaiting(queue01));

  unit_end();



  unit_begin("xQueuePeek()");

  queue02 = NULL;

  queue02 = xQueuePeek(queue01);

  unit_try(NULL != queue02);

  unit_try(0x8u == queue02->messageBytes);

  unit_try(0x0u == strncmp("MESSAGE1", (char *)queue02->messageValue, 0x8));

  xMemFree(queue02);

  unit_end();



  unit_begin("xQueueReceive()");

  queue02 = xQueueReceive(queue01);

  unit_try(NULL != queue02);

  unit_try(0x8u == queue02->messageBytes);

  unit_try(0x0u == strncmp("MESSAGE1", (char *)queue02->messageValue, 0x8));

  xMemFree(queue02);

  unit_end();



  unit_begin("xQueueDropMessage()");

  xQueueDropMessage(queue01);

  unit_try(0x6u != xQueueGetLength(queue01));

  xQueueDelete(queue01);

  unit_end();



  unit_begin("xQueueLockQueue()");

  queue01 = xQueueCreate(5);

  unit_try(NULL != queue01);

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE1"));

  xQueueLockQueue(queue01);

  unit_try(RETURN_FAILURE == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE2"));

  unit_end();



  unit_begin("xQueueUnlockQueue()");

  xQueueUnLockQueue(queue01);

  unit_try(RETURN_SUCCESS == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE3"));

  unit_try(0x2u == xQueueGetLength(queue01));

  unit_end();



  unit_begin("xQueueDelete()");

  xQueueDelete(queue01);

  unit_try(RETURN_FAILURE == xQueueSend(queue01, 0x8, (Char_t *)"MESSAGE4"));

  unit_end();


  return;
}
