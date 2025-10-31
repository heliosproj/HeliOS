/*UNCRUSTIFY-OFF*/
/**
 * @file streams.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Stream buffer API header
 * @details
 * Defines stream buffer structures and function prototypes for fixed-size byte streaming operations between tasks.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef STREAM_H_

  #define STREAM_H_

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

  #include "queue.h"

  #include "sys.h"

  #include "task.h"

  #include "timer.h"

  #ifdef __cplusplus

    extern "C" {

  #endif /* ifdef __cplusplus */
  /**
   * @brief Creates a new stream buffer
   * @details Allocates memory for a stream buffer structure and initializes it.
   *
   * @param[out] stream_ Pointer to store the created stream buffer handle
   *
   * @return             ReturnOK if stream buffer was created successfully
   * @return             ReturnError if allocation failed or invalid parameter
   *
   * @warning Caller is responsible for deleting the stream buffer with
   * xStreamDelete()
   */
  Return_t xStreamCreate(StreamBuffer_t **stream_);
  /**
   * @brief Deletes a stream buffer
   * @details Frees the memory allocated for the stream buffer.
   *
   * @param[in] stream_ Pointer to the stream buffer to delete
   *
   * @return            ReturnOK if stream buffer was deleted successfully
   * @return            ReturnError if stream buffer is invalid or deallocation
   *                    failed
   *
   * @warning Using the stream buffer handle after deletion results in undefined
   * behavior
   */
  Return_t xStreamDelete(const StreamBuffer_t *stream_);
  /**
   * @brief Sends a byte to the stream buffer
   * @details Adds a single byte to the end of the stream buffer.
   *
   * @param[in,out] stream_ Pointer to the stream buffer
   * @param[in]     byte_   Byte value to send
   *
   * @return                ReturnOK if byte was sent successfully
   * @return                ReturnError if stream buffer is full or invalid
   */
  Return_t xStreamSend(StreamBuffer_t *stream_, const Byte_t byte_);
  /**
   * @brief Receives all available bytes from the stream buffer
   * @details Returns a pointer to the stream buffer data and the number of
   * available bytes.
   *
   * @param[in]  stream_ Pointer to the stream buffer
   * @param[out] bytes_  Pointer to store the number of bytes available
   * @param[out] data_   Pointer to store the data buffer pointer
   *
   * @return             ReturnOK if data was retrieved successfully
   * @return             ReturnError if stream buffer is empty, invalid, or
   *                     invalid parameters
   *
   * @note The returned pointer points to the internal buffer; do not modify or
   * free
   */
  Return_t xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_, Byte_t **data_);
  /**
   * @brief Gets the number of bytes available in the stream buffer
   * @details Returns the count of bytes currently stored in the buffer.
   *
   * @param[in]  stream_ Pointer to the stream buffer to query
   * @param[out] bytes_  Pointer to store the byte count
   *
   * @return             ReturnOK if count was retrieved successfully
   * @return             ReturnError if stream buffer is invalid or invalid
   *                     parameter
   */
  Return_t xStreamBytesAvailable(const StreamBuffer_t *stream_, HalfWord_t *bytes_);
  /**
   * @brief Resets the stream buffer to empty state
   * @details Clears all data from the stream buffer and resets the length to
   * zero.
   *
   * @param[in] stream_ Pointer to the stream buffer to reset
   *
   * @return            ReturnOK if stream buffer was reset successfully
   * @return            ReturnError if stream buffer is invalid
   */
  Return_t xStreamReset(const StreamBuffer_t *stream_);
  /**
   * @brief Checks if the stream buffer is empty
   * @details Returns true if the stream buffer contains no data.
   *
   * @param[in]  stream_ Pointer to the stream buffer to check
   * @param[out] res_    Pointer to store the result (true if empty, false
   *                     otherwise)
   *
   * @return             ReturnOK if check was successful
   * @return             ReturnError if stream buffer is invalid or invalid
   *                     parameter
   */
  Return_t xStreamIsEmpty(const StreamBuffer_t *stream_, Base_t *res_);
  /**
   * @brief Checks if the stream buffer is full
   * @details Returns true if the stream buffer has reached its capacity.
   *
   * @param[in]  stream_ Pointer to the stream buffer to check
   * @param[out] res_    Pointer to store the result (true if full, false
   *                     otherwise)
   *
   * @return             ReturnOK if check was successful
   * @return             ReturnError if stream buffer is invalid or invalid
   *                     parameter
   */
  Return_t xStreamIsFull(const StreamBuffer_t *stream_, Base_t *res_);

  #ifdef __cplusplus

    }

  #endif /* ifdef __cplusplus */

#endif /* ifndef STREAM_H_ */