/**
 * @file stream.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for stream buffers
 * @version 0.4.0
 * @date 2022-08-30
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
#include "stream.h"


/* The xStreamCreate() system call will create a new stream. Streams
   are fast for sending streams of bytes between tasks. */
StreamBuffer_t *xStreamCreate() {
  StreamBuffer_t *ret = NULL;


  if(ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &ret, sizeof(StreamBuffer_t)))) {
    /* Assert if xMemAlloc() didn't return our requested
       kernel memory. */
    SYSASSERT(ISNOTNULLPTR(ret));


    /* Check if xMemAlloc() returned our requested
       kernel memory. */
    if(ISNOTNULLPTR(ret)) {
      ret->length = zero;
    }
  }

  return(ret);
}


/* The xStreamDelete() system call will delete a stream. Streams
   are fast for sending streams of bytes between tasks. */
void xStreamDelete(const StreamBuffer_t *stream_) {
  /* Assert if the heap fails its health check or if the stream pointer the end-user
     passed is invalid. */
  SYSASSERT(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR)));


  /* Check if the heap is health and the stream pointer the end-user passed is valid.
     If so, continue. Otherwise, head toward the exit. */
  if(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    __KernelFreeMemory__(stream_);
  }

  return;
}


/* The xStreamSend() system call will send one byte to
   the designated stream. Sending to a stream must be done
   one byte at a time. However, receiving from a stream
   will return all available bytes in a stream. */
Base_t xStreamSend(StreamBuffer_t *stream_, const Byte_t byte_) {
  Base_t ret = RETURN_FAILURE;


  /* Assert if the heap fails its health check or if the stream pointer the end-user
     passed is invalid. */
  SYSASSERT(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR)));


  /* Check if the heap is health and the stream pointer the end-user passed is valid.
     If so, continue. Otherwise, head toward the exit. */
  if(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    /* Check to make sure our stream buffer is less than
       CONFIG_STREAM_BUFFER_BYTES in length before we attempt
       to write a byte to it. */
    if(CONFIG_STREAM_BUFFER_BYTES > stream_->length) {
      stream_->length++;


      /* Offset the stream buffer length by -1 because array
         is base zero. */
      stream_->buffer[stream_->length - 1] = byte_;
      ret = RETURN_SUCCESS;
    }
  }

  return(ret);
}


/* The xStreamReceive() system call will receive all waiting bytes
   in the stream. */
Byte_t *xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_) {
  Byte_t *ret = NULL;


  /* Assert if the bytes paramater is null. It can't be
     null because we need to return the number of bytes
     received. */
  SYSASSERT(ISNOTNULLPTR(bytes_));


  /* Check if the bytes parameter is null. It can't be null
     because we need to return the number of bytes received. */
  if(ISNOTNULLPTR(bytes_)) {
    /* Assert if the heap fails its health check or if the stream pointer the end-user
       passed is invalid. */
    SYSASSERT(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR)));


    /* Check if the heap is health and the stream pointer the end-user passed is valid.
       If so, continue. Otherwise, head toward the exit. */
    if(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      /* If there is nothing to receive then just head toward the exit. */
      if(zero < stream_->length) {
        if(ISSUCCESSFUL(__HeapAllocateMemory__((volatile Addr_t **) &ret, stream_->length * sizeof(Byte_t)))) {
          /* Assert if we didn't get the memory we requested. */
          SYSASSERT(ISNOTNULLPTR(ret));


          /* Check to make sure we got the memory we requested. */
          if(ISNOTNULLPTR(ret)) {
            *bytes_ = stream_->length;
            __memcpy__(ret, stream_->buffer, stream_->length * sizeof(Byte_t));
            __memset__(stream_, zero, sizeof(StreamBuffer_t));
          }
        }
      }
    }
  }

  return(ret);
}


/* The xStreamBytesAvailable() system call will return the number
   of waiting bytes in the stream. */
HalfWord_t xStreamBytesAvailable(const StreamBuffer_t *stream_) {
  HalfWord_t ret = zero;


  /* Assert if the heap fails its health check or if the stream pointer the end-user
     passed is invalid. */
  SYSASSERT(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR)));


  /* Check if the heap is health and the stream pointer the end-user passed is valid.
     If so, continue. Otherwise, head toward the exit. */
  if(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    if(zero < stream_->length) {
      ret = stream_->length;
    }
  }

  return(ret);
}


/* The xStreamReset() system call will reset the stream and clear
   its buffer of all waiting bytes. */
void xStreamReset(const StreamBuffer_t *stream_) {
  /* Assert if the heap fails its health check or if the stream pointer the end-user
     passed is invalid. */
  SYSASSERT(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR)));


  /* Check if the heap is health and the stream pointer the end-user passed is valid.
     If so, continue. Otherwise, head toward the exit. */
  if(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    /* If there is nothing to receive then just head toward the exit. */
    if(zero < stream_->length) {
      __memset__(stream_, zero, sizeof(StreamBuffer_t));
    }
  }

  return;
}


/* The xStreamIsEmpty() system call will return true if the stream
   buffer is empty (i.e., zero in size). */
Base_t xStreamIsEmpty(const StreamBuffer_t *stream_) {
  Base_t ret = true;


  /* Assert if the heap fails its health check or if the stream pointer the end-user
     passed is invalid. */
  SYSASSERT(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR)));


  /* Check if the heap is health and the stream pointer the end-user passed is valid.
     If so, continue. Otherwise, head toward the exit. */
  if(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    /* If there is nothing to receive then just head toward the exit. */
    if(zero < stream_->length) {
      ret = false;
    }
  }

  return(ret);
}


/* The xStreamIsFull() system call will return true if the stream
   buffer is full (i.e., size is equal to CONFIG_STREAM_BUFFER_BYTES) */
Base_t xStreamIsFull(const StreamBuffer_t *stream_) {
  Base_t ret = false;


  /* Assert if the heap fails its health check or if the stream pointer the end-user
     passed is invalid. */
  SYSASSERT(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR)));


  /* Check if the heap is health and the stream pointer the end-user passed is valid.
     If so, continue. Otherwise, head toward the exit. */
  if(ISSUCCESSFUL(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    /* If there is nothing to receive then just head toward the exit. */
    if(CONFIG_STREAM_BUFFER_BYTES == stream_->length) {
      ret = true;
    }
  }

  return(ret);
}