/*UNCRUSTIFY-OFF*/
/**
 * @file queue.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for message queue inter-task communication
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
#include "queue.h"
static Return_t __QueueDropmessage__(Queue_t *queue_);
static Return_t __QueuePeek__(const Queue_t *queue_, QueueMessage_t **message_);


Return_t xQueueCreate(Queue_t **queue_, Base_t limit_) {
  RET_DEFINE;

  if(NOTNULLPTR(queue_) && (CONFIG_QUEUE_MINIMUM_LIMIT <= limit_)) {
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) queue_, sizeof(Queue_t)))) {
      if(NOTNULLPTR(*queue_)) {
        (*queue_)->length = zero;
        (*queue_)->limit = limit_;
        (*queue_)->locked = false;
        (*queue_)->head = null;
        (*queue_)->tail = null;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueDelete(Queue_t *queue_) {
  RET_DEFINE;

  if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    /* Loop through the queue while it contains messages and drop each message
     * until there are no more messages. */
    while(NOTNULLPTR(queue_->head)) {
      if(OK(__QueueDropmessage__(queue_))) {
        /* Do nothing - literally. */
      } else {
        ASSERT;
        break;
      }
    }

    if(OK(__KernelFreeMemory__(queue_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueGetLength(const Queue_t *queue_, Base_t *res_) {
  RET_DEFINE;


  Base_t messages = zero;
  Message_t *cursor = null;


  if(NOTNULLPTR(queue_) && NOTNULLPTR(res_)) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      cursor = queue_->head;

      while(NOTNULLPTR(cursor)) {
        messages++;
        cursor = cursor->next;
      }


      /* Confirm the length of the queue matches the number of the messages we
       * counted while traversing the queue. If they match, then set res_ to the
       * number of messages. */
      if(messages == queue_->length) {
        *res_ = messages;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueIsQueueEmpty(const Queue_t *queue_, Base_t *res_) {
  RET_DEFINE;


  Base_t messages = zero;
  Message_t *cursor = null;


  if(NOTNULLPTR(queue_) && NOTNULLPTR(res_)) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      cursor = queue_->head;

      while(NOTNULLPTR(cursor)) {
        messages++;
        cursor = cursor->next;
      }


      /* Confirm the length of the queue matches the number of the messages we
       * counted while traversing the queue.
       *
       * If the number of messages is zero, then set res_ to true because the
       * queue is empty. Otherwise set res_ to false because the queue is *NOT*
       * empty. */
      if((zero == messages) && (messages == queue_->length)) {
        *res_ = true;
        RET_OK;
      } else if((zero != messages) && (messages == queue_->length)) {
        *res_ = false;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueIsQueueFull(const Queue_t *queue_, Base_t *res_) {
  RET_DEFINE;


  Base_t messages = zero;
  Message_t *cursor = null;


  if(NOTNULLPTR(queue_) && NOTNULLPTR(res_)) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      cursor = queue_->head;

      while(NOTNULLPTR(cursor)) {
        messages++;
        cursor = cursor->next;
      }


      /* Confirm the length of the queue matches the number of the messages we
       * counted while traversing the queue.
       *
       * If the number of messages greater than or equal to the queue length
       * limit, then set res_ to true because the queue is full. If the number
       * of messages is less than the queue length limit, then set res_ to false
       * because the queue is *NOT* full. */
      if((messages >= queue_->limit) && (messages == queue_->length)) {
        *res_ = true;
        RET_OK;
      } else if((messages < queue_->limit) && (messages == queue_->length)) {
        *res_ = false;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueMessagesWaiting(const Queue_t *queue_, Base_t *res_) {
  RET_DEFINE;


  Base_t messages = zero;
  Message_t *cursor = null;


  if(NOTNULLPTR(queue_) && NOTNULLPTR(res_)) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      cursor = queue_->head;

      while(NOTNULLPTR(cursor)) {
        messages++;
        cursor = cursor->next;
      }


      /* Confirm the length of the queue matches the number of the messages we
       * counted while traversing the queue.
       *
       * If the number of messages greater than zero, then set res_ to true
       * because there is at least one message waiting - possibly more.
       * Otherwise set res_ to false because there are no messages waiting in
       * the queue. */
      if((zero < messages) && (messages == queue_->length)) {
        *res_ = true;
        RET_OK;
      } else if((zero == messages) && (messages == queue_->length)) {
        *res_ = false;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueSend(Queue_t *queue_, const Base_t bytes_, const Byte_t *value_) {
  RET_DEFINE;


  Message_t *message = null;
  Base_t messages = zero;
  Message_t *cursor = null;


  if(NOTNULLPTR(queue_) && (zero < bytes_) && (CONFIG_MESSAGE_VALUE_BYTES >= bytes_) && (NOTNULLPTR(value_))) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(false == queue_->locked) {
        cursor = queue_->head;

        while(NOTNULLPTR(cursor)) {
          messages++;
          cursor = cursor->next;
        }

        if((queue_->limit > queue_->length) && (messages == queue_->length)) {
          if(OK(__KernelAllocateMemory__((volatile Addr_t **) &message, sizeof(Message_t)))) {
            if(NOTNULLPTR(message)) {
              if(OK(__memcpy__(message->messageValue, value_, CONFIG_MESSAGE_VALUE_BYTES))) {
                message->messageBytes = bytes_;
                message->next = null;


                /* If the queue tail is not null then it already contains
                 * messages and append the new message, otherwise set the head
                 * and tail to the new message. */
                if(NOTNULLPTR(queue_->tail)) {
                  queue_->tail->next = message;
                  queue_->tail = message;
                } else {
                  queue_->head = message;
                  queue_->tail = message;
                }

                queue_->length++;
                RET_OK;
              } else {
                ASSERT;


                /* Free the kernel memory because __memcpy__() failed. */
                __KernelFreeMemory__(message);
              }
            } else {
              ASSERT;
            }
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueuePeek(const Queue_t *queue_, QueueMessage_t **message_) {
  RET_DEFINE;

  if(NOTNULLPTR(queue_) && NOTNULLPTR(message_)) {
    if(OK(__QueuePeek__(queue_, message_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __QueuePeek__(const Queue_t *queue_, QueueMessage_t **message_) {
  RET_DEFINE;

  if(NOTNULLPTR(queue_) && NOTNULLPTR(message_)) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(NOTNULLPTR(queue_->head)) {
        if(OK(__HeapAllocateMemory__((volatile Addr_t **) message_, sizeof(QueueMessage_t)))) {
          if(NOTNULLPTR(*message_)) {
            (*message_)->messageBytes = queue_->head->messageBytes;

            if(OK(__memcpy__((*message_)->messageValue, queue_->head->messageValue, CONFIG_MESSAGE_VALUE_BYTES))) {
              RET_OK;
            } else {
              ASSERT;


              /* Free the heap memory because __memcpy__() failed. */
              __HeapFreeMemory__(*message_);
            }
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueDropMessage(Queue_t *queue_) {
  RET_DEFINE;

  if(NOTNULLPTR(queue_)) {
    if(OK(__QueueDropmessage__(queue_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __QueueDropmessage__(Queue_t *queue_) {
  RET_DEFINE;


  Message_t *message = null;


  if(NOTNULLPTR(queue_)) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(NOTNULLPTR(queue_->head)) {
        message = queue_->head;
        queue_->head = queue_->head->next;

        if(NULLPTR(queue_->head)) {
          queue_->tail = null;
        }

        if(OK(__KernelFreeMemory__(message))) {
          queue_->length--;
          RET_OK;
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueReceive(Queue_t *queue_, QueueMessage_t **message_) {
  RET_DEFINE;

  if(NOTNULLPTR(queue_) && NOTNULLPTR(message_)) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(OK(__QueuePeek__(queue_, message_))) {
        if(NOTNULLPTR(*message_)) {
          if(OK(__QueueDropmessage__(queue_))) {
            RET_OK;
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueLockQueue(Queue_t *queue_) {
  RET_DEFINE;

  if(NOTNULLPTR(queue_)) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(false == queue_->locked) {
        queue_->locked = true;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xQueueUnLockQueue(Queue_t *queue_) {
  RET_DEFINE;

  if(NOTNULLPTR(queue_)) {
    if(OK(__MemoryRegionCheckKernel__(queue_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(true == queue_->locked) {
        queue_->locked = false;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}