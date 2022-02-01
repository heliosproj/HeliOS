/**
 * @file queue.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for interprocess communication queues in HeliOS
 * @version 0.3.0
 * @date 2022-01-31
 * 
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
 * 
 */

#include "queue.h"

Queue_t *xQueueCreate(Base_t limit_) {
  DISABLE_INTERRUPTS();
  Queue_t *queue = null;
  if (limit_ >= CONFIG_QUEUE_MININUM_LIMIT) {
    queue = (Queue_t *)xMemAlloc(sizeof(Queue_t));
    if (queue) {
      queue->length = 0;
      queue->limit = limit_;
      queue->head = null;
      queue->tail = null;
      ENABLE_INTERRUPTS();
      return queue;
    }
  }
  ENABLE_INTERRUPTS();
  return null;
}

void xQueueDelete(Queue_t *queue_) {
  if (queue_) {
    while (queue_->head) {
      xQueueDropMessage(queue_);
    }
    xMemFree(queue_);
  }
  return;
}

Base_t xQueueGetLength(Queue_t *queue_) {
  Base_t messages = 0;
  Message_t *messageCursor = null;
  if (queue_) {
    messageCursor = queue_->head;
    while (messageCursor) {
      messages++;
      messageCursor = messageCursor->next;
    }
    if (queue_->length == messages) {
      return messages;
    }
  }
  return 0;
}

Base_t xQueueIsQueueEmpty(Queue_t *queue_) {
  Base_t messages = 0;
  Message_t *messageCursor = null;
  if (queue_) {
    messageCursor = queue_->head;
    while (messageCursor) {
      messages++;
      messageCursor = messageCursor->next;
    }
    if (messages == 0 && queue_->length == messages) {
      return true;
    }
  }
  return false;
}

Base_t xQueueIsQueueFull(Queue_t *queue_) {
  Base_t messages = 0;
  Message_t *messageCursor = null;
  if (queue_) {
    messageCursor = queue_->head;
    while (messageCursor) {
      messages++;
      messageCursor = messageCursor->next;
    }
    if (messages >= queue_->limit && queue_->length == messages) {
      return true;
    }
  }
  return false;
}

Base_t xQueueMessagesWaiting(Queue_t *queue_) {
  Base_t messages = 0;
  Message_t *messageCursor = null;
  if (queue_) {
    messageCursor = queue_->head;
    while (messageCursor) {
      messages++;
      messageCursor = messageCursor->next;
    }
    if (messages > 0 && queue_->length == messages) {
      return true;
    }
  }
  return false;
}

Base_t xQueueSend(Queue_t *queue_, Base_t messageBytes_, const char *messageValue_) {
  DISABLE_INTERRUPTS();
  Message_t *message = null;
  Base_t messages = 0;
  Message_t *messageCursor = null;
  if (queue_ && messageBytes_ > 0 && messageValue_) {
    messageCursor = queue_->head;
    while (messageCursor) {
      messages++;
      messageCursor = messageCursor->next;
    }
    if (queue_->length < queue_->limit && queue_->length == messages) {
      message = (Message_t *)xMemAlloc(sizeof(Message_t));
      if (message) {
        message->messageBytes = messageBytes_;
        memcpy_(message->messageValue, messageValue_, CONFIG_MESSAGE_VALUE_BYTES);
        message->next = null;
        if (queue_->tail) {
          queue_->tail->next = message;
          queue_->tail = message;
        } else {
          queue_->head = message;
          queue_->tail = message;
        }
        queue_->length++;
        ENABLE_INTERRUPTS();
        return true;
      }
    }
  }
  ENABLE_INTERRUPTS();
  return false;
}

QueueMessage_t *xQueuePeek(Queue_t *queue_) {
  QueueMessage_t *message = null;
  if (queue_) {
    if (queue_->head) {
      message = (QueueMessage_t *)xMemAlloc(sizeof(QueueMessage_t));
      if (message) {
        message->messageBytes = queue_->head->messageBytes;
        memcpy_(message->messageValue, queue_->head->messageValue, CONFIG_MESSAGE_VALUE_BYTES);
        return message;
      }
    }
  }
  return null;
}

void xQueueDropMessage(Queue_t *queue_) {
  DISABLE_INTERRUPTS();
  QueueMessage_t *message = null;
  if (queue_) {
    if (!queue_->head) {
      ENABLE_INTERRUPTS();
      return;
    }
    message = (QueueMessage_t *)queue_->head;
    queue_->head = queue_->head->next;
    if (!queue_->head) {
      queue_->tail = null;
    }
    queue_->length--;
    xMemFree(message);
  }
  ENABLE_INTERRUPTS();
  return;
}

QueueMessage_t *xQueueReceive(Queue_t *queue_) {
  QueueMessage_t *message = null;
  if (queue_) {
    message = xQueuePeek(queue_);
    if (message) {
      xQueueDropMessage(queue_);
      return message;
    }
  }
  return null;
}