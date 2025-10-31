/*UNCRUSTIFY-OFF*/
/**
 * @file queue.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Message queue API header
 * @details
 * Defines queue structures and function prototypes for FIFO message passing between tasks with configurable limits and priority support.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef QUEUE_H_

  #define QUEUE_H_

  #include "config.h"

  #include "defines.h"

  #include "types.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "console.h"

    #include "device.h"

    #include "fat.h"

    #include "fs.h"

  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

  #include "mem.h"

  #include "port.h"

  #include "posix.h"

  #include "streams.h"

  #include "sys.h"

  #include "task.h"

  #include "timer.h"

  #ifdef __cplusplus

    extern "C" {

  #endif /* ifdef __cplusplus */
  /**
   * @brief Creates a new message queue
   * @details Allocates memory for a queue structure and initializes it with the specified message limit.
   *
   * @param[out] queue_ Pointer to store the created queue handle
   * @param[in] limit_ Maximum number of messages allowed in the queue
   *
   * @return ReturnOK if queue was created successfully
   * @return ReturnError if allocation failed or invalid parameters
   *
   * @warning Caller is responsible for deleting the queue with xQueueDelete()
   */
  Return_t xQueueCreate(Queue_t **queue_, const Base_t limit_);

  /**
   * @brief Deletes a message queue
   * @details Frees the queue and all messages it contains.
   *
   * @param[in,out] queue_ Pointer to the queue to delete
   *
   * @return ReturnOK if queue was deleted successfully
   * @return ReturnError if queue is invalid or deallocation failed
   *
   * @warning Using the queue handle after deletion results in undefined behavior
   */
  Return_t xQueueDelete(Queue_t *queue_);

  /**
   * @brief Gets the current number of messages in the queue
   * @details Returns the count of messages currently stored in the queue.
   *
   * @param[in] queue_ Pointer to the queue to query
   * @param[out] res_ Pointer to store the message count
   *
   * @return ReturnOK if count was retrieved successfully
   * @return ReturnError if queue is invalid or invalid parameter
   */
  Return_t xQueueGetLength(const Queue_t *queue_, Base_t *res_);

  /**
   * @brief Checks if the queue is empty
   * @details Returns true if the queue contains no messages.
   *
   * @param[in] queue_ Pointer to the queue to check
   * @param[out] res_ Pointer to store the result (true if empty, false otherwise)
   *
   * @return ReturnOK if check was successful
   * @return ReturnError if queue is invalid or invalid parameter
   */
  Return_t xQueueIsQueueEmpty(const Queue_t *queue_, Base_t *res_);

  /**
   * @brief Checks if the queue is full
   * @details Returns true if the queue has reached its message limit.
   *
   * @param[in] queue_ Pointer to the queue to check
   * @param[out] res_ Pointer to store the result (true if full, false otherwise)
   *
   * @return ReturnOK if check was successful
   * @return ReturnError if queue is invalid or invalid parameter
   */
  Return_t xQueueIsQueueFull(const Queue_t *queue_, Base_t *res_);

  /**
   * @brief Gets the number of messages waiting in the queue
   * @details Returns the count of messages available to be received.
   *
   * @param[in] queue_ Pointer to the queue to query
   * @param[out] res_ Pointer to store the message count
   *
   * @return ReturnOK if count was retrieved successfully
   * @return ReturnError if queue is invalid or invalid parameter
   */
  Return_t xQueueMessagesWaiting(const Queue_t *queue_, Base_t *res_);

  /**
   * @brief Sends a message to the queue
   * @details Adds a message to the end of the queue with optional data payload.
   *
   * @param[in,out] queue_ Pointer to the queue
   * @param[in] bytes_ Number of bytes in the message value
   * @param[in] value_ Pointer to message data (can be NULL if bytes_ is 0)
   *
   * @return ReturnOK if message was sent successfully
   * @return ReturnError if queue is full, invalid, or allocation failed
   */
  Return_t xQueueSend(Queue_t *queue_, const Base_t bytes_, const Byte_t *value_);

  /**
   * @brief Peeks at the first message without removing it
   * @details Allocates and returns a copy of the first message in the queue without dequeuing it.
   *
   * @param[in] queue_ Pointer to the queue to peek
   * @param[out] message_ Pointer to store allocated message structure
   *
   * @return ReturnOK if message was peeked successfully
   * @return ReturnError if queue is empty, invalid, or allocation failed
   *
   * @warning Caller is responsible for freeing the allocated message structure
   */
  Return_t xQueuePeek(const Queue_t *queue_, QueueMessage_t **message_);

  /**
   * @brief Drops the first message from the queue
   * @details Removes and frees the first message in the queue without returning it.
   *
   * @param[in,out] queue_ Pointer to the queue
   *
   * @return ReturnOK if message was dropped successfully
   * @return ReturnError if queue is empty or invalid
   */
  Return_t xQueueDropMessage(Queue_t *queue_);

  /**
   * @brief Receives a message from the queue
   * @details Allocates, returns, and removes the first message from the queue.
   *
   * @param[in,out] queue_ Pointer to the queue
   * @param[out] message_ Pointer to store allocated message structure
   *
   * @return ReturnOK if message was received successfully
   * @return ReturnError if queue is empty, invalid, or allocation failed
   *
   * @warning Caller is responsible for freeing the allocated message structure
   */
  Return_t xQueueReceive(Queue_t *queue_, QueueMessage_t **message_);

  /**
   * @brief Locks the queue to prevent concurrent access
   * @details Sets the lock flag to prevent other tasks from modifying the queue.
   *
   * @param[in,out] queue_ Pointer to the queue to lock
   *
   * @return ReturnOK if queue was locked successfully
   * @return ReturnError if queue is invalid or already locked
   */
  Return_t xQueueLockQueue(Queue_t *queue_);

  /**
   * @brief Unlocks the queue to allow access
   * @details Clears the lock flag to allow other tasks to modify the queue.
   *
   * @param[in,out] queue_ Pointer to the queue to unlock
   *
   * @return ReturnOK if queue was unlocked successfully
   * @return ReturnError if queue is invalid or not locked
   */
  Return_t xQueueUnLockQueue(Queue_t *queue_);

  #ifdef __cplusplus

    }

  #endif /* ifdef __cplusplus */

#endif /* ifndef QUEUE_H_ */