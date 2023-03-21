/*UNCRUSTIFY-OFF*/
/**
 * @file stream.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for stream buffers for inter-task communication
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
#include "stream.h"


Return_t xStreamCreate(StreamBuffer_t **stream_) {
  RET_DEFINE;

  if(NOTNULLPTR(stream_)) {
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) stream_, sizeof(StreamBuffer_t)))) {
      if(NOTNULLPTR(*stream_)) {
        (*stream_)->length = zero;
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


Return_t xStreamDelete(const StreamBuffer_t *stream_) {
  RET_DEFINE;

  if(NOTNULLPTR(stream_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(OK(__KernelFreeMemory__(stream_))) {
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


Return_t xStreamSend(StreamBuffer_t *stream_, const Byte_t byte_) {
  RET_DEFINE;

  if(NOTNULLPTR(stream_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(CONFIG_STREAM_BUFFER_BYTES > stream_->length) {
        stream_->buffer[stream_->length] = byte_;
        stream_->length++;
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


Return_t xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_, Byte_t **data_) {
  RET_DEFINE;

  if(NOTNULLPTR(stream_) && NOTNULLPTR(bytes_) && NOTNULLPTR(data_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(zero < stream_->length) {
        if(OK(__HeapAllocateMemory__((volatile Addr_t **) data_, stream_->length * sizeof(Byte_t)))) {
          if(NOTNULLPTR(*data_)) {
            *bytes_ = stream_->length;

            if(OK(__memcpy__(*data_, stream_->buffer, stream_->length * sizeof(Byte_t)))) {
              if(OK(__memset__(stream_, zero, sizeof(StreamBuffer_t)))) {
                RET_OK;
              } else {
                ASSERT;


                /* Free heap memory because __memset__() failed. */
                __HeapFreeMemory__(*data_);
              }
            } else {
              ASSERT;


              /* Free heap memory because __memcpy__() failed. */
              __HeapFreeMemory__(*data_);
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


Return_t xStreamBytesAvailable(const StreamBuffer_t *stream_, HalfWord_t *bytes_) {
  RET_DEFINE;

  if(NOTNULLPTR(stream_) && NOTNULLPTR(bytes_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(zero < stream_->length) {
        *bytes_ = stream_->length;
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


Return_t xStreamReset(const StreamBuffer_t *stream_) {
  RET_DEFINE;

  if(NOTNULLPTR(stream_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(zero < stream_->length) {
        if(OK(__memset__(stream_, zero, sizeof(StreamBuffer_t)))) {
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


Return_t xStreamIsEmpty(const StreamBuffer_t *stream_, Base_t *res_) {
  RET_DEFINE;

  if(NOTNULLPTR(stream_) && NOTNULLPTR(res_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(zero < stream_->length) {
        *res_ = false;
        RET_OK;
      } else {
        *res_ = true;
        RET_OK;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xStreamIsFull(const StreamBuffer_t *stream_, Base_t *res_) {
  RET_DEFINE;

  if(NOTNULLPTR(stream_) && NOTNULLPTR(res_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(CONFIG_STREAM_BUFFER_BYTES == stream_->length) {
        *res_ = true;
        RET_OK;
      } else {
        *res_ = false;
        RET_OK;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}