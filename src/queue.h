/*UNCRUSTIFY-OFF*/
/**
 * @file queue.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for message queues
 * @version 0.4.0
 * @date 2022-01-31
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
#ifndef QUEUE_H_
#define QUEUE_H_

#include "config.h"
#include "defines.h"
#include "types.h"
#include "port.h"
#include "device.h"
#include "mem.h"
#include "stream.h"
#include "sys.h"
#include "task.h"
#include "timer.h"

#ifdef __cplusplus
  extern "C" {
#endif
Queue_t *xQueueCreate(const Base_t limit_);
void xQueueDelete(Queue_t *queue_);
Base_t xQueueGetLength(const Queue_t *queue_);
Base_t xQueueIsQueueEmpty(const Queue_t *queue_);
Base_t xQueueIsQueueFull(const Queue_t *queue_);
Base_t xQueueMessagesWaiting(const Queue_t *queue_);
Base_t xQueueSend(Queue_t *queue_, const Base_t messageBytes_, const Char_t *messageValue_);
QueueMessage_t *xQueuePeek(const Queue_t *queue_);
void xQueueDropMessage(Queue_t *queue_);
QueueMessage_t *xQueueReceive(Queue_t *queue_);
void xQueueLockQueue(Queue_t *queue_);
void xQueueUnLockQueue(Queue_t *queue_);

#ifdef __cplusplus
  }
#endif
#endif