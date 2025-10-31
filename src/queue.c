#include "queue.h"
static Return_t __QueueDropmessage__(Queue_t *queue_);
static Return_t __QueuePeek__(const Queue_t *queue_, QueueMessage_t **message_);
#define __GetQueueLength__() \
        cursor = queue_->head; \
        while(__PointerIsNotNull__(cursor)) { \
          messages++; \
          cursor = cursor->next; \
        }
#define __QueueLengthCorrect__() (queue_->length == messages)
#define __QueueLengthZero__() (messages == 0x0u)
#define __QueueLengthNonZero__() (0x0u < messages)
#define __QueueLengthAtLimit__() (queue_->limit <= messages)
#define __QueueLengthNotAtLimit__() (queue_->limit > messages)
Return_t xQueueCreate(Queue_t **queue_, const Base_t limit_) {
  FUNCTION_ENTER;
  if(__PointerIsNotNull__(queue_) && (CONFIG_QUEUE_MINIMUM_LIMIT <= limit_)) {
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) queue_, sizeof(Queue_t)))) {
      if(__PointerIsNotNull__(*queue_)) {
        (*queue_)->valid = VALID;
        (*queue_)->length = 0x0u;
        (*queue_)->limit = limit_;
        (*queue_)->locked = false;
        (*queue_)->head = null;
        (*queue_)->tail = null;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueDelete(Queue_t *queue_) {
  FUNCTION_ENTER;
  if(__ObjectIsValid__(queue_)) {
    while(__PointerIsNotNull__(queue_->head)) {
      if(OK(__QueueDropmessage__(queue_))) {
      } else {
        __AssertOnElse__();
        break;
      }
    }
    queue_->valid = INVALID;
    if(OK(__KernelFreeMemory__(queue_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueGetLength(const Queue_t *queue_, Base_t *res_) {
  FUNCTION_ENTER;
  Base_t messages = 0x0u;
  Message_t *cursor = null;
  if(__ObjectIsValid__(queue_) && __PointerIsNotNull__(res_)) {
    __GetQueueLength__();
    if(__QueueLengthCorrect__()) {
      *res_ = messages;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueIsQueueEmpty(const Queue_t *queue_, Base_t *res_) {
  FUNCTION_ENTER;
  Base_t messages = 0x0u;
  Message_t *cursor = null;
  if(__ObjectIsValid__(queue_) && __PointerIsNotNull__(res_)) {
    __GetQueueLength__();
    if(__QueueLengthZero__() && __QueueLengthCorrect__()) {
      *res_ = true;
      __ReturnOk__();
    } else if(__QueueLengthNonZero__() && __QueueLengthCorrect__()) {
      *res_ = false;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueIsQueueFull(const Queue_t *queue_, Base_t *res_) {
  FUNCTION_ENTER;
  Base_t messages = 0x0u;
  Message_t *cursor = null;
  if(__ObjectIsValid__(queue_) && __PointerIsNotNull__(res_)) {
    __GetQueueLength__();
    if(__QueueLengthAtLimit__() && __QueueLengthCorrect__()) {
      *res_ = true;
      __ReturnOk__();
    } else if(__QueueLengthNotAtLimit__() && __QueueLengthCorrect__()) {
      *res_ = false;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueMessagesWaiting(const Queue_t *queue_, Base_t *res_) {
  FUNCTION_ENTER;
  Base_t messages = 0x0u;
  Message_t *cursor = null;
  if(__ObjectIsValid__(queue_) && __PointerIsNotNull__(res_)) {
    __GetQueueLength__();
    if(__QueueLengthNonZero__() && __QueueLengthCorrect__()) {
      *res_ = true;
      __ReturnOk__();
    } else if(__QueueLengthZero__() && __QueueLengthCorrect__()) {
      *res_ = false;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueSend(Queue_t *queue_, const Base_t bytes_, const Byte_t *value_) {
  FUNCTION_ENTER;
  Message_t *message = null;
  Base_t messages = 0x0u;
  Message_t *cursor = null;
  if(__ObjectIsValid__(queue_) && (0x0u < bytes_) && (CONFIG_MESSAGE_VALUE_BYTES >= bytes_) && __PointerIsNotNull__(value_)) {
    if(false == queue_->locked) {
      __GetQueueLength__();
      if((queue_->limit > queue_->length) && __QueueLengthCorrect__()) {
        if(OK(__KernelAllocateMemory__((volatile Addr_t **) &message, sizeof(Message_t)))) {
          if(__PointerIsNotNull__(message)) {
            message->valid = VALID;
            if(OK(__memcpy__(message->messageValue, value_, CONFIG_MESSAGE_VALUE_BYTES))) {
              message->messageBytes = bytes_;
              message->next = null;
              if(__PointerIsNotNull__(queue_->tail)) {
                queue_->tail->next = message;
                queue_->tail = message;
              } else {
                queue_->head = message;
                queue_->tail = message;
              }
              queue_->length++;
              __ReturnOk__();
            } else {
              __AssertOnElse__();
              __KernelFreeMemory__(message);
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueuePeek(const Queue_t *queue_, QueueMessage_t **message_) {
  FUNCTION_ENTER;
  if(__ObjectIsValid__(queue_) && __PointerIsNotNull__(message_)) {
    if(OK(__QueuePeek__(queue_, message_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
static Return_t __QueuePeek__(const Queue_t *queue_, QueueMessage_t **message_) {
  FUNCTION_ENTER;
  if(__ObjectIsValid__(queue_) && __PointerIsNotNull__(message_)) {
    if(__PointerIsNotNull__(queue_->head)) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) message_, sizeof(QueueMessage_t)))) {
        if(__PointerIsNotNull__(*message_)) {
          (*message_)->valid = VALID;
          (*message_)->messageBytes = queue_->head->messageBytes;
          if(OK(__memcpy__((*message_)->messageValue, queue_->head->messageValue, CONFIG_MESSAGE_VALUE_BYTES))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();
            (*message_)->valid = INVALID;
            __HeapFreeMemory__(*message_);
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueDropMessage(Queue_t *queue_) {
  FUNCTION_ENTER;
  if(__ObjectIsValid__(queue_)) {
    if(OK(__QueueDropmessage__(queue_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
static Return_t __QueueDropmessage__(Queue_t *queue_) {
  FUNCTION_ENTER;
  Message_t *message = null;
  if(__ObjectIsValid__(queue_)) {
    if(__PointerIsNotNull__(queue_->head)) {
      message = queue_->head;
      queue_->head = queue_->head->next;
      if(__PointerIsNull__(queue_->head)) {
        queue_->tail = null;
      }
      message->valid = INVALID;
      if(OK(__KernelFreeMemory__(message))) {
        queue_->length--;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueReceive(Queue_t *queue_, QueueMessage_t **message_) {
  FUNCTION_ENTER;
  if(__ObjectIsValid__(queue_) && __PointerIsNotNull__(message_)) {
    if(OK(__QueuePeek__(queue_, message_))) {
      if(__PointerIsNotNull__(*message_)) {
        if(OK(__QueueDropmessage__(queue_))) {
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueLockQueue(Queue_t *queue_) {
  FUNCTION_ENTER;
  if(__ObjectIsValid__(queue_)) {
    if(false == queue_->locked) {
      queue_->locked = true;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xQueueUnLockQueue(Queue_t *queue_) {
  FUNCTION_ENTER;
  if(__ObjectIsValid__(queue_)) {
    if(true == queue_->locked) {
      queue_->locked = false;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}