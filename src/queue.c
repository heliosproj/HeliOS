/**
 * @file queue.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for interprocess communication queues in HeliOS
 * @version 0.3.0
 * @date 2022-01-31
 *
 * @copyright
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

extern SysFlags_t sysFlags;

/* The xQueueCreate() system call creates a message queue for inter-task
communication. */
Queue_t *xQueueCreate(Base_t limit_) {

  Queue_t *queue = null;

  /* Check to make sure the limit parameter is greater than or equal to the
  setting CONFIG_QUEUE_MINIMUM_LIMIT. */
  if (limit_ >= CONFIG_QUEUE_MINIMUM_LIMIT) {
    queue = (Queue_t *)xMemAlloc(sizeof(Queue_t));

    /* Check if queue was successfully allocated by xMemAlloc(). */
    if (ISNOTNULLPTR(queue)) {
      queue->length = 0;

      queue->limit = limit_;

      queue->head = null;

      queue->tail = null;

      return queue;
    }
  }

  return null;
}

/* The xQueueDelete() system call will delete a queue created by xQueueCreate(). xQueueDelete()
will delete a queue regardless of how many messages the queue contains at the time xQueueDelete() is called. */
void xQueueDelete(Queue_t *queue_) {
  /* Check if the queue parameter is null */
  if (ISNOTNULLPTR(queue_)) {
    /* If the queue has a head it contains messages, iterate through the queue and drop
    all of the messages. */
    while (ISNOTNULLPTR(queue_->head)) {
      xQueueDropMessage(queue_);
    }

    xMemFree(queue_);
  }

  return;
}

/* The xQueueGetLength() system call returns the length of the queue (the number of messages
the queue currently contains). */
Base_t xQueueGetLength(Queue_t *queue_) {
  Base_t messages = 0;

  Message_t *messageCursor = null;

  /* Check if the queue parameter is not null. */
  if (ISNOTNULLPTR(queue_)) {
    messageCursor = queue_->head;

    /* If the queue has a head, iterate through the queue and count the number of messages. */
    while (ISNOTNULLPTR(messageCursor)) {
      messages++;

      messageCursor = messageCursor->next;
    }

    /* Check to make sure the number of messages counted matches the length attribute of the queue.
    This is to confirm the integrity of the queue before returning its length. */
    if (queue_->length == messages) {
      return messages;
    }
  }

  return 0;
}

/* The xQueueIsEmpty() system call will return a true or false dependent on whether the queue is
empty or contains one or more messages. */
Base_t xQueueIsQueueEmpty(Queue_t *queue_) {
  Base_t messages = 0;

  Message_t *messageCursor = null;

  /* Check if the queue pointer is not null. */
  if (ISNOTNULLPTR(queue_)) {
    messageCursor = queue_->head;

    /* If the queue has a head, iterate through the queue and count the number of messages. */
    while (ISNOTNULLPTR(messageCursor)) {
      messages++;

      messageCursor = messageCursor->next;
    }

    /* Check to make sure the number of messages counted matches the length attribute of the queue
    and if the number of messages equals zero. */
    if (messages == 0 && queue_->length == messages) {
      return true;
    }
  }

  return false;
}

/* The xQueueIsFull() system call will return a true or false dependent on whether the queue is
full or contains zero messages. A queue is considered full if the number of messages in the queue
is equal to the queue's length limit. */
Base_t xQueueIsQueueFull(Queue_t *queue_) {
  Base_t messages = 0;

  Message_t *messageCursor = null;

  /* Check if the queue parameter is not null. */
  if (ISNOTNULLPTR(queue_)) {
    messageCursor = queue_->head;

    /* If the queue has a head, iterate through the queue and count the number of messages. */
    while (ISNOTNULLPTR(messageCursor)) {
      messages++;

      messageCursor = messageCursor->next;
    }

    /* Check to make sure the number of messages counted matches the length attribute of the queue
    and if the number of messages is greater than or equal to the queue length limit. */
    if (messages >= queue_->limit && queue_->length == messages) {
      return true;
    }
  }

  return false;
}

/* The xQueueMessageWaiting() system call returns true or false dependent on whether
there is at least one message waiting. The queue does not have to be full to return true. */
Base_t xQueueMessagesWaiting(Queue_t *queue_) {
  Base_t messages = 0;

  Message_t *messageCursor = null;

  /* Check if the queue parameter is not null. */
  if (ISNOTNULLPTR(queue_)) {
    messageCursor = queue_->head;

    /* If the queue has a head, iterate through the queue and count the number of messages. */
    while (ISNOTNULLPTR(messageCursor)) {
      messages++;

      messageCursor = messageCursor->next;
    }

    /* Check to make sure the number of messages counted matches the length attribute of the queue
    and if the number of messages is greater than zero. */
    if (messages > 0 && queue_->length == messages) {
      return true;
    }
  }

  return false;
}

/* The xQueueSend() system call will send a message to the queue. The size of the message
value is passed in the message bytes parameter. */
Base_t xQueueSend(Queue_t *queue_, Base_t messageBytes_, const char *messageValue_) {

  Message_t *message = null;

  Base_t messages = 0;

  Message_t *messageCursor = null;

  /* Check if the queue parameter is not null, message bytes is between one and CONFIG_MESSAGE_VALUE_BYTES and the message value parameter
  is not null. */
  if (ISNOTNULLPTR(queue_) && messageBytes_ > 0 && messageBytes_ <= CONFIG_MESSAGE_VALUE_BYTES && ISNOTNULLPTR(messageValue_)) {
    messageCursor = queue_->head;

    /* If the queue has a head, iterate through the queue and count the number of messages. */
    while (ISNOTNULLPTR(messageCursor)) {
      messages++;

      messageCursor = messageCursor->next;
    }

    /* Check if the length of the queue is less than the limit and the length of the queue matches the number of messages
    counted. */
    if (queue_->length < queue_->limit && queue_->length == messages) {
      message = (Message_t *)xMemAlloc(sizeof(Message_t));

      /* Check if the message was successfully allocated by xMemAlloc(). */
      if (ISNOTNULLPTR(message)) {
        message->messageBytes = messageBytes_;

        memcpy_(message->messageValue, messageValue_, CONFIG_MESSAGE_VALUE_BYTES);

        message->next = null;

        /* If the queue tail is not null then it already contains messages and append the new message, otherwise
        set the head and tail to the new message. */
        if (ISNOTNULLPTR(queue_->tail)) {
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

/* The xQueuePeek() system call will return the next message in the queue without
dropping the message. */
QueueMessage_t *xQueuePeek(Queue_t *queue_) {
  QueueMessage_t *message = null;

  /* Check if the queue parameter is not null. */
  if (ISNOTNULLPTR(queue_)) {
    /* Check if the head of the queue is not null. */
    if (ISNOTNULLPTR(queue_->head)) {
      message = (QueueMessage_t *)xMemAlloc(sizeof(QueueMessage_t));

      /* Check if a new message was successfully allocatd by xMemAlloc(). */
      if (ISNOTNULLPTR(message)) {
        message->messageBytes = queue_->head->messageBytes;

        memcpy_(message->messageValue, queue_->head->messageValue, CONFIG_MESSAGE_VALUE_BYTES);

        return message;
      }
    }
  }

  return null;
}

/* The xQueueDropMessage() system call will drop the next message from the queue without
returning the message. */
void xQueueDropMessage(Queue_t *queue_) {

  QueueMessage_t *message = null;

  /* Check if the queue parameter is not null. */
  if (ISNOTNULLPTR(queue_)) {
    /* Check if the head of the queue is null, if so enable interrupts and
    return because there is nothing to drop. */
    if (ISNULLPTR(queue_->head)) {

      return;
    }

    message = (QueueMessage_t *)queue_->head;

    queue_->head = queue_->head->next;

    /* Again check if the head of the queue is null, if so set the tail
    of the queue to null. */
    if (ISNULLPTR(queue_->head)) {
      queue_->tail = null;
    }

    queue_->length--;

    xMemFree(message);
  }

  return;
}

/* The xQueueReceive() system call will return the next message in the queue and drop
it from the queue. */
QueueMessage_t *xQueueReceive(Queue_t *queue_) {
  QueueMessage_t *message = null;

  /* Check if the queue parameter is not null. */
  if (ISNOTNULLPTR(queue_)) {
    message = xQueuePeek(queue_);

    /* Check if the message returned from xQueuePeek() is not null. If so, drop the message from the
    queue and return the message. */
    if (ISNOTNULLPTR(message)) {
      xQueueDropMessage(queue_);

      return message;
    }
  }

  return null;
}