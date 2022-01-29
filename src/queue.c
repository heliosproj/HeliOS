/*
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
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
 */

#include "queue.h"

Queue_t *xQueueCreate(int16_t limit_) {
  Queue_t *queue = null;
  if (limit_ >= QUEUE_MINIMUM_LIMIT) {
    queue = (Queue_t *)xMemAlloc(sizeof(Queue_t));
    if (queue) {
      queue->length = 0;
      queue->limit = limit_;
      queue->head = null;
      queue->tail = null;
      return queue;
    }
  }
  return null;
}

void xQueueDelete(Queue_t *queue_) {
  if (queue_) {
    while (queue_->head) {
      xQueueDropMessage(queue_);
    }
    xMemFree(queue_);
    queue_ = null;
  }
  return;
}

int16_t xQueueGetLength(Queue_t *queue_) {
  if (queue_)
    return queue_->length;
  return 0;
}

int16_t xQueueIsQueueEmpty(Queue_t *queue_) {
  if (queue_)
    if (queue_->length)
      return false;
  return true;
}

int16_t xQueueIsQueueFull(Queue_t *queue_) {
  if (queue_)
    if (queue_->length < queue_->limit)
      return false;
  return true;
}

int16_t xQueueMessagesWaiting(Queue_t *queue_) {
  if (queue_)
    if (queue_->length == 0)
      return false;
  return true;
}

int16_t xQueueSend(Queue_t *queue_, int16_t messageBytes_, char *messageValue_) {
  QueueMessage_t *message = null;
  if (queue_ && messageBytes_ > 0 && messageValue_) {
    if (queue_->length < queue_->limit) {
      message = (QueueMessage_t *)xMemAlloc(sizeof(QueueMessage_t));
      if (message) {
        message->messageBytes = messageBytes_;
        memcpy_(message->messageValue, messageValue_, TNOTIFYVALUE_SIZE);
        message->next = null;
        if (queue_->tail) {
          queue_->tail->next = message;
          queue_->tail = message;
        } else {
          queue_->head = message;
          queue_->tail = message;
        }
        queue_->length++;
        return true;
      }
    }
  }
  return false;
}

QueueMessage_t *xQueuePeek(Queue_t *queue_) {
  QueueMessage_t *message = null;
  if (queue_->head) {
    message = (QueueMessage_t *)xMemAlloc(sizeof(QueueMessage_t));
    if (message) {
      memcpy_(message, queue_->head, sizeof(QueueMessage_t));
    }
  }
  return message;
}

void xQueueDropMessage(Queue_t *queue_) {
  QueueMessage_t *message = null;
  if (!queue_->head)
    return;
  message = (QueueMessage_t *)queue_->head;
  queue_->head = queue_->head->next;
  if (!queue_->head)
    queue_->tail = null;
  queue_->length--;
  xMemFree(message);
}

QueueMessage_t *xQueueReceive(Queue_t *queue_) {
  QueueMessage_t *message = null;
  message = xQueuePeek(queue_);
  if (message) {
    xQueueDrop(queue_);
    return message;
  }
  return null;
}