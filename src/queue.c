/*UNCRUSTIFY-OFF*/
/**
 * @file queue.c
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
    while(NOTNULLPTR(queue_->head)) {
      if(OK(xQueueDropMessage(queue_))) {
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


  if(NOTNULLPTR(queue_) && (zero < bytes_) && (CONFIG_MESSAGE_VALUE_BYTES >= bytes_) && (NOTNULLPTR(value_)) && (OK(__MemoryRegionCheckKernel__(queue_,
    MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
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

              /* If the queue tail is not null then it already contains messages
               * and append the new message, otherwise set the head and tail to
               * the new message. */
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

        queue_->length--;

        if(OK(__KernelFreeMemory__(message))) {
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