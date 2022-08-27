/**
 * @file queue.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for message queues
 * @version 0.3.5
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




/* The xQueueCreate() system call creates a message queue for inter-task
communication. */
Queue_t *xQueueCreate(Base_t limit_) {


  Queue_t *ret = NULL;


  /* Assert if the end-user attempted to create a queue with a limit
  that is less than the configured minimum limit. */
  SYSASSERT(CONFIG_QUEUE_MINIMUM_LIMIT <= limit_);


  /* Check if the end-user attempted to create a queue with a limit
  that is less than the configured minimum limit, if they did then
  just head toward the exit. */
  if (CONFIG_QUEUE_MINIMUM_LIMIT <= limit_) {




    ret = (Queue_t *)_KernelAllocateMemory_(sizeof(Queue_t));


    /* Assert if xMemAlloc() didn't return our requested
    heap memory. */
    SYSASSERT(ISNOTNULLPTR(ret));


    /* Check if xMemAlloc() returned our requested
    heap memory. */
    if (ISNOTNULLPTR(ret)) {


      ret->length = zero;

      ret->limit = limit_;

      ret->locked = false;

      ret->head = NULL;

      ret->tail = NULL;
    }
  }

  return ret;
}

/* The xQueueDelete() system call will delete a queue created by xQueueCreate(). xQueueDelete()
will delete a queue regardless of how many messages the queue contains at the time xQueueDelete() is called. */
void xQueueDelete(Queue_t *queue_) {


  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));



  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {


    /* If the queue contains messages, traverse the queue and drop the
    messages as we go. */
    while (ISNOTNULLPTR(queue_->head)) {


      /* Drop the next message in the queue. */
      xQueueDropMessage(queue_);
    }




    _KernelFreeMemory_(queue_);
  }


  return;
}




/* The xQueueGetLength() system call returns the length of the queue (the number of messages
the queue currently contains). */
Base_t xQueueGetLength(Queue_t *queue_) {


  Base_t ret = zero;

  Base_t messages = zero;

  Message_t *cursor = NULL;

  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));


  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {


    cursor = queue_->head;

    /* Traverse the queue and count the number of messages it contains. */
    while (ISNOTNULLPTR(cursor)) {

      messages++;

      cursor = cursor->next;
    }


    /* Assert if the number of messages we counted disagrees with the queue's
    length. This could indicate a problem. */
    SYSASSERT(messages == queue_->length);


    /* Check if the number of messages we counted agrees with the queue's
    length. */
    if (messages == queue_->length) {

      ret = messages;
    }
  }


  return ret;
}




/* The xQueueIsEmpty() system call will return a true or false dependent on whether the queue is
empty or contains one or more messages. */
Base_t xQueueIsQueueEmpty(Queue_t *queue_) {



  Base_t ret = false;

  Base_t messages = zero;

  Message_t *cursor = NULL;


  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));


  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {


    cursor = queue_->head;

    /* Traverse the queue and count the number of messages it contains. */
    while (ISNOTNULLPTR(cursor)) {


      messages++;

      cursor = cursor->next;
    }



    /* Assert if the number of messages we counted disagrees with the queue's
    length. This could indicate a problem. */
    SYSASSERT(messages = queue_->length);


    /* Check if the number of messages we counted agrees with the queue's
    length. Also, for the queue to be empty there must be zero messages. */
    if ((zero == messages) && (messages == queue_->length)) {

      ret = true;
    }
  }


  return ret;
}



/* The xQueueIsFull() system call will return a true or false dependent on whether the queue is
full or contains zero messages. A queue is considered full if the number of messages in the queue
is equal to the queue's length limit. */
Base_t xQueueIsQueueFull(Queue_t *queue_) {


  Base_t ret = false;

  Base_t messages = zero;

  Message_t *cursor = NULL;


  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));


  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {

    cursor = queue_->head;



    /* Traverse the queue and count the number of messages it contains. */
    while (ISNOTNULLPTR(cursor)) {

      messages++;

      cursor = cursor->next;
    }


    /* Assert if the number of messages we counted disagrees with the queue's
    length. This could indicate a problem. */
    SYSASSERT(messages == queue_->length);

    /* Check if the number of messages we counted agrees with the queue's
    length. Also, for the queue to be full the number of messages must be
    equal to the queue limit. */
    if ((messages >= queue_->limit) && (messages == queue_->length)) {


      ret = true;
    }
  }

  return ret;
}




/* The xQueueMessageWaiting() system call returns true or false dependent on whether
there is at least one message waiting. The queue does not have to be full to return true. */
Base_t xQueueMessagesWaiting(Queue_t *queue_) {



  Base_t ret = false;

  Base_t messages = zero;

  Message_t *cursor = NULL;


  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));



  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {

    cursor = queue_->head;

    /* Traverse the queue and count the number of messages it contains. */
    while (ISNOTNULLPTR(cursor)) {

      messages++;

      cursor = cursor->next;
    }


    /* Assert if the number of messages we counted disagrees with the queue's
    length. This could indicate a problem. */
    SYSASSERT(messages == queue_->length);


    /* Check if the number of messages we counted agrees with the queue's
    length. Also, for there to be waiting messages look to make sure the
    count is greater than zero. */
    if ((zero < messages) && (messages == queue_->length)) {


      ret = true;
    }
  }


  return ret;
}




/* The xQueueSend() system call will send a message to the queue. The size of the message
value is passed in the message bytes parameter. */
Base_t xQueueSend(Queue_t *queue_, Base_t messageBytes_, const char *messageValue_) {



  Base_t ret = RETURN_FAILURE;

  Message_t *message = NULL;

  Base_t messages = zero;

  Message_t *cursor = NULL;


  /* Assert if the end-user passed zero message bytes. A message
  must have at least one byte in it. */
  SYSASSERT(zero < messageBytes_);


  /* Assert if the end-user passed a number of message bytes that
  exceeds the size of the message value. */
  SYSASSERT(CONFIG_MESSAGE_VALUE_BYTES >= messageBytes_);


  /* Assert if the end-user passed a null pointer for the message
  value. */
  SYSASSERT(ISNOTNULLPTR(messageValue_));

  /* Assert if the user passed an invaid queue. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));


  /* Check if the message bytes is within parameters and the message value is not null. */
  if ((RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) && (zero < messageBytes_) && (CONFIG_MESSAGE_VALUE_BYTES >= messageBytes_) && (ISNOTNULLPTR(messageValue_))) {


    /* Assert if the queue is locked (locked queues can not SEND messages). */
    SYSASSERT(false == queue_->locked);


    /* check to make sure queue is NOT locked (locked queues can not SEND messages). */
    if (false == queue_->locked) {

      /* Assert if the heap fails its health check or if the queue pointer the end-user
      passed is invalid. */
      SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));

      /* Check if the heap is health and the queue pointer the end-user passed is valid.
      If so, continue. Otherwise, head toward the exit. */
      if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {

        cursor = queue_->head;

        /* If the queue has a head, iterate through the queue and count the number of messages. */
        while (ISNOTNULLPTR(cursor)) {

          messages++;

          cursor = cursor->next;
        }


        /* Assert if the queue is full. */
        SYSASSERT(queue_->limit > queue_->length);


        /* Assert if the messages counted disagrees with the queue length. If
        so there is a problem. */
        SYSASSERT(messages == queue_->length);


        /* Check if the queue is not full and that the messages counted agrees with the
        queue length. */
        if ((queue_->limit > queue_->length) && (messages == queue_->length)) {




          message = (Message_t *)_KernelAllocateMemory_(sizeof(Message_t));


          /* Assert if xMemAlloc() did not allocate our requested memory. */
          SYSASSERT(ISNOTNULLPTR(message));

          /* Check if the message was successfully allocated by xMemAlloc(). */
          if (ISNOTNULLPTR(message)) {

            message->messageBytes = messageBytes_;

            _memcpy_(message->messageValue, messageValue_, CONFIG_MESSAGE_VALUE_BYTES);

            message->next = NULL;

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

            ret = RETURN_SUCCESS;
          }
        }
      }
    }
  }

  return ret;
}


/* The xQueuePeek() system call will return the next message in the queue without
dropping the message. */
QueueMessage_t *xQueuePeek(Queue_t *queue_) {

  return _QueuePeek_(queue_);
}


QueueMessage_t *_QueuePeek_(Queue_t *queue_) {
  QueueMessage_t *ret = NULL;


  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));


  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {

    /* If the head is not null, then there is a message waiting for us to
    peek at. */
    if (ISNOTNULLPTR(queue_->head)) {


      ret = (QueueMessage_t *)_HeapAllocateMemory_(sizeof(QueueMessage_t));


      /* Assert if xMemAlloc() didn't do its job. */
      SYSASSERT(ISNOTNULLPTR(ret));

      /* If xMemAlloc() allocated the heap memory then copy the message into the
      queue message we will return. Otherwise, head toward the exit. */
      if (ISNOTNULLPTR(ret)) {


        ret->messageBytes = queue_->head->messageBytes;

        _memcpy_(ret->messageValue, queue_->head->messageValue, CONFIG_MESSAGE_VALUE_BYTES);
      }
    }
  }


  return ret;
}

/* The xQueueDropMessage() system call will drop the next message from the queue without
returning the message. */
void xQueueDropMessage(Queue_t *queue_) {

  _QueueDropmessage_(queue_);

  return;
}

void _QueueDropmessage_(Queue_t *queue_) {


  Message_t *message = NULL;



  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));



  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {



    /* Check if there is a message in the queue, if there isn't then
    we have nothing to drop so head toward the exit. */
    if (ISNOTNULLPTR(queue_->head)) {


      message = queue_->head;

      queue_->head = queue_->head->next;


      /* If the head is now null, then let's set the tail to null
      too because the queue is now empty. */
      if (ISNULLPTR(queue_->head)) {

        queue_->tail = NULL;
      }

      queue_->length--;


      _KernelFreeMemory_(message);
    }
  }


  return;
}

/* The xQueueReceive() system call will return the next message in the queue and drop
it from the queue. */
QueueMessage_t *xQueueReceive(Queue_t *queue_) {


  QueueMessage_t *ret = NULL;



  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));



  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {


    /* Re-use some code and peek to see if there is a message
    waiting in the queue.

    NOTE: We don't need to allocate any heap memory since xQueuePeek()
    has already done that for us. */
    ret = _QueuePeek_(queue_);


    /* See if xQueuePeek() returned a message, if so we need to drop it
    before we return the message. */
    if (ISNOTNULLPTR(ret)) {


      /* Re-use some code and just call xQueueDropMessage() to drop
      the message we just received. */
      _QueueDropmessage_(queue_);
    }
  }

  return ret;
}


/* The xQueueLockQueue() system call will LOCK the queue and prevent xQueueSend() from
sending a message to the queue. */
void xQueueLockQueue(Queue_t *queue_) {


  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));



  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {

    SYSASSERT(false == queue_->locked);

    queue_->locked = true;
  }


  return;
}


/* The xQueueUnLockQueue() system call will UNLOCK the queue and allow xQueueSend() to
sendi a message to the queue. */
void xQueueUnLockQueue(Queue_t *queue_) {


  /* Assert if the heap fails its health check or if the queue pointer the end-user
  passed is invalid. */
  SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR));



  /* Check if the heap is health and the queue pointer the end-user passed is valid.
  If so, continue. Otherwise, head toward the exit. */
  if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {

    SYSASSERT(true == queue_->locked);

    queue_->locked = false;
  }


  return;
}