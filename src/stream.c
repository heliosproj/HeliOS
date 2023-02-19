/*UNCRUSTIFY-OFF*/
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
/*UNCRUSTIFY-ON*/
#include "stream.h"


Return_t xStreamCreate(StreamBuffer_t **stream_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(stream_) && ISOK(__KernelAllocateMemory__((volatile Addr_t **) stream_, sizeof(StreamBuffer_t)))) {
    if(ISNOTNULLPTR(*stream_)) {
      (*stream_)->length = zero;
      RET_SUCCESS;
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

  if(ISNOTNULLPTR(stream_) && ISOK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    if(ISOK(__KernelFreeMemory__(stream_))) {
      RET_SUCCESS;
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

  if(ISNOTNULLPTR(stream_) && ISOK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    if(CONFIG_STREAM_BUFFER_BYTES > stream_->length) {
      stream_->buffer[stream_->length] = byte_;
      stream_->length++;
      RET_SUCCESS;
    }
  }

  RET_RETURN;
}


Return_t xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_, Byte_t **data_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(stream_) && ISNOTNULLPTR(bytes_) && ISNOTNULLPTR(data_) && ISOK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    if(zero < stream_->length) {
      if(ISOK(__HeapAllocateMemory__((volatile Addr_t **) data_, stream_->length * sizeof(Byte_t)))) {
        if(ISNOTNULLPTR(*data_)) {
          *bytes_ = stream_->length;

          if(ISOK(__memcpy__(*data_, stream_->buffer, stream_->length * sizeof(Byte_t)))) {
            if(ISOK(__memset__(stream_, zero, sizeof(StreamBuffer_t)))) {
              RET_SUCCESS;
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


Return_t xStreamBytesAvailable(const StreamBuffer_t *stream_, HalfWord_t *bytes_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(stream_) && ISNOTNULLPTR(bytes_) && ISOK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    if(zero < stream_->length) {
      *bytes_ = stream_->length;
      RET_SUCCESS;
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

  if(ISNOTNULLPTR(stream_) && ISOK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    if(zero < stream_->length) {
      if(ISOK(__memset__(stream_, zero, sizeof(StreamBuffer_t)))) {
        RET_SUCCESS;
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

  if(ISNOTNULLPTR(stream_) && ISNOTNULLPTR(res_) && ISOK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    if(zero < stream_->length) {
      *res_ = false;
      RET_SUCCESS;
    } else {
      *res_ = true;
      RET_SUCCESS;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xStreamIsFull(const StreamBuffer_t *stream_, Base_t *res_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(stream_) && ISNOTNULLPTR(res_) && ISOK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
    if(CONFIG_STREAM_BUFFER_BYTES == stream_->length) {
      *res_ = true;
      RET_SUCCESS;
    } else {
      *res_ = false;
      RET_SUCCESS;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}