/*UNCRUSTIFY-OFF*/
/**
 * @file queue.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Message queue implementation
 * @details
 * Implements FIFO message queues for inter-task communication with configurable limits, message prioritization, and queue management operations.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/


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
/**
 * @brief Creates a new message queue
 * @details Allocates memory for a queue structure and initializes it with the specified message limit.
 *
 * @param[out] queue_ Pointer to store the created queue handle
 * @param[in]  limit_ Maximum number of messages allowed in the queue
 *
 * @return            ReturnOK if queue was created successfully
 * @return            ReturnError if allocation failed or invalid parameters
 *
 * @warning Caller is responsible for deleting the queue with xQueueDelete()
 */
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


/**
 * @brief Deletes a message queue
 * @details Frees the queue and all messages it contains.
 *
 * @param[in,out] queue_ Pointer to the queue to delete
 *
 * @return               ReturnOK if queue was deleted successfully
 * @return               ReturnError if queue is invalid or deallocation failed
 *
 * @warning Using the queue handle after deletion results in undefined behavior
 */
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


/**
 * @brief Gets the current number of messages in the queue
 * @details Returns the count of messages currently stored in the queue.
 *
 * @param[in]  queue_ Pointer to the queue to query
 * @param[out] res_   Pointer to store the message count
 *
 * @return            ReturnOK if count was retrieved successfully
 * @return            ReturnError if queue is invalid or invalid parameter
 */
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


/**
 * @brief Checks if the queue is empty
 * @details Returns true if the queue contains no messages.
 *
 * @param[in]  queue_ Pointer to the queue to check
 * @param[out] res_   Pointer to store the result (true if empty, false otherwise)
 *
 * @return            ReturnOK if check was successful
 * @return            ReturnError if queue is invalid or invalid parameter
 */
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


/**
 * @brief Checks if the queue is full
 * @details Returns true if the queue has reached its message limit.
 *
 * @param[in]  queue_ Pointer to the queue to check
 * @param[out] res_   Pointer to store the result (true if full, false otherwise)
 *
 * @return            ReturnOK if check was successful
 * @return            ReturnError if queue is invalid or invalid parameter
 */
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


/**
 * @brief Gets the number of messages waiting in the queue
 * @details Returns the count of messages available to be received.
 *
 * @param[in]  queue_ Pointer to the queue to query
 * @param[out] res_   Pointer to store the message count
 *
 * @return            ReturnOK if count was retrieved successfully
 * @return            ReturnError if queue is invalid or invalid parameter
 */
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


/**
 * @brief Sends a message to the queue
 * @details Adds a message to the end of the queue with optional data payload.
 *
 * @param[in,out] queue_ Pointer to the queue
 * @param[in]     bytes_ Number of bytes in the message value
 * @param[in]     value_ Pointer to message data (can be NULL if bytes_ is 0)
 *
 * @return               ReturnOK if message was sent successfully
 * @return               ReturnError if queue is full, invalid, or allocation failed
 */
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


/**
 * @brief Peeks at the first message without removing it
 * @details Allocates and returns a copy of the first message in the queue without dequeuing it.
 *
 * @param[in]  queue_   Pointer to the queue to peek
 * @param[out] message_ Pointer to store allocated message structure
 *
 * @return              ReturnOK if message was peeked successfully
 * @return              ReturnError if queue is empty, invalid, or allocation failed
 *
 * @warning Caller is responsible for freeing the allocated message structure
 */
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


/**
 * @brief Peeks at the next message without removing it
 * @details Internal helper that retrieves the next message from the queue without dequeuing it.
 *
 * @param[in] queue_   Pointer to queue
 * @param[in] message_ Pointer to store message
 *
 * @return             ReturnOK if message was retrieved successfully
 * @return             ReturnError if queue is empty or invalid parameter
 */
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


/**
 * @brief Drops the first message from the queue
 * @details Removes and frees the first message in the queue without returning it.
 *
 * @param[in,out] queue_ Pointer to the queue
 *
 * @return               ReturnOK if message was dropped successfully
 * @return               ReturnError if queue is empty or invalid
 */
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


/**
 * @brief Drops a message from the queue
 * @details Internal helper that removes the first message from the queue.
 *
 * @param[in,out] queue_ Pointer to queue
 *
 * @return               ReturnOK if message was dropped successfully
 * @return               ReturnError if operation failed
 */
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


/**
 * @brief Receives a message from the queue
 * @details Allocates, returns, and removes the first message from the queue.
 *
 * @param[in,out] queue_   Pointer to the queue
 * @param[out]    message_ Pointer to store allocated message structure
 *
 * @return                 ReturnOK if message was received successfully
 * @return                 ReturnError if queue is empty, invalid, or allocation failed
 *
 * @warning Caller is responsible for freeing the allocated message structure
 */
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


/**
 * @brief Locks the queue to prevent concurrent access
 * @details Sets the lock flag to prevent other tasks from modifying the queue.
 *
 * @param[in,out] queue_ Pointer to the queue to lock
 *
 * @return               ReturnOK if queue was locked successfully
 * @return               ReturnError if queue is invalid or already locked
 */
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


/**
 * @brief Unlocks the queue to allow access
 * @details Clears the lock flag to allow other tasks to modify the queue.
 *
 * @param[in,out] queue_ Pointer to the queue to unlock
 *
 * @return               ReturnOK if queue was unlocked successfully
 * @return               ReturnError if queue is invalid or not locked
 */
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