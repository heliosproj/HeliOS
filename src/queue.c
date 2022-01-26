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

Queue_t *xQueueCreate() {
  Queue_t *queue = null;
  queue = (Queue_t *)xMemAlloc(sizeof(Queue_t));
  if (queue) {
    queue->length = 0;
    queue->head = null;
    queue->tail = null;
  }
  return queue;
}

Queue_t *xQueueDestroy(Queue_t *queue_) {
  if (queue_) {
    while (queue_->head) {
      xQueueDrop(queue_);
    }
    xMemFree(queue_);
    queue_ = null;
  }
  return queue_;
}

int16_t xQueueGetLength(Queue_t *queue_) {
  if (queue_)
    return queue_->length;
  return 0;
}

void xQueueGive(Queue_t *queue_, int16_t messageBytes_, char *messageValue_) {
  QueueItem_t *item = null;
  if (queue_ && messageBytes_ > 0 && messageValue_) {
    item = (QueueItem_t *)xMemAlloc(sizeof(QueueItem_t));
    if (item) {
      item->messageBytes = messageBytes_;
      memcpy_(item->messageValue, messageValue_, TNOTIFYVALUE_SIZE);
      if (!queue_->tail) {
        queue_->head = item;
        queue_->tail = item;
        queue_->length++;
        return;
      }
      queue_->tail->next = item;
      queue_->tail = item;
      queue_->length++;
    }
  }
}

QueueItem_t *xQueuePeek(Queue_t *queue_) {
  QueueItem_t *item = null;
  if (queue_->head) {
    item = (QueueItem_t *)xMemAlloc(sizeof(QueueItem_t));
    if (item) {
      memcpy_(item, queue_->head, sizeof(QueueItem_t));
    }
  }
  return item;
}

void xQueueDrop(Queue_t *queue_) {
  QueueItem_t *item = null;
  if (!queue_->head)
    return;
  item = (QueueItem_t *)queue_->head;
  queue_->head = queue_->head->next;
  if (!queue_->head)
    queue_->tail = null;
  queue_->length--;
  xMemFree(item);
}

QueueItem_t *xQueueTake(Queue_t *queue_) {
  QueueItem_t *item = null;
  item = xQueuePeek(queue_);
  if (item) {
    xQueueDrop(queue_);
    return item;
  }
  return null;
}