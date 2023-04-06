/*UNCRUSTIFY-OFF*/
/**
 * @file stream.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for stream buffers for inter-task communication
 * @version 0.5.0
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

#define __StreamLengthNonZero__() (nil < stream_->length)


#define __StreamLengthAtLimit__() (CONFIG_STREAM_BUFFER_BYTES == stream_->length)


Return_t xStreamCreate(StreamBuffer_t **stream_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stream_)) {
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) stream_, sizeof(StreamBuffer_t)))) {
      if(__PointerIsNotNull__(*stream_)) {
        (*stream_)->length = nil;
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


Return_t xStreamDelete(const StreamBuffer_t *stream_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stream_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(OK(__KernelFreeMemory__(stream_))) {
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


Return_t xStreamSend(StreamBuffer_t *stream_, const Byte_t byte_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stream_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(CONFIG_STREAM_BUFFER_BYTES > stream_->length) {
        stream_->buffer[stream_->length] = byte_;
        stream_->length++;
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


Return_t xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_, Byte_t **data_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stream_) && __PointerIsNotNull__(bytes_) && __PointerIsNotNull__(data_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(__StreamLengthNonZero__()) {
        if(OK(__HeapAllocateMemory__((volatile Addr_t **) data_, stream_->length * sizeof(Byte_t)))) {
          if(__PointerIsNotNull__(*data_)) {
            *bytes_ = stream_->length;

            if(OK(__memcpy__(*data_, stream_->buffer, stream_->length * sizeof(Byte_t)))) {
              if(OK(__memset__(stream_, nil, sizeof(StreamBuffer_t)))) {
                __ReturnOk__();
              } else {
                __AssertOnElse__();


                /* Free heap memory because __memset__() failed. */
                __HeapFreeMemory__(*data_);
              }
            } else {
              __AssertOnElse__();


              /* Free heap memory because __memcpy__() failed. */
              __HeapFreeMemory__(*data_);
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


Return_t xStreamBytesAvailable(const StreamBuffer_t *stream_, HalfWord_t *bytes_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stream_) && __PointerIsNotNull__(bytes_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(__StreamLengthNonZero__()) {
        *bytes_ = stream_->length;
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


Return_t xStreamReset(const StreamBuffer_t *stream_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stream_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(__StreamLengthNonZero__()) {
        if(OK(__memset__(stream_, nil, sizeof(StreamBuffer_t)))) {
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


Return_t xStreamIsEmpty(const StreamBuffer_t *stream_, Base_t *res_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stream_) && __PointerIsNotNull__(res_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(__StreamLengthNonZero__()) {
        *res_ = false;
        __ReturnOk__();
      } else {
        *res_ = true;
        __ReturnOk__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xStreamIsFull(const StreamBuffer_t *stream_, Base_t *res_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stream_) && __PointerIsNotNull__(res_)) {
    if(OK(__MemoryRegionCheckKernel__(stream_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      if(__StreamLengthAtLimit__()) {
        *res_ = true;
        __ReturnOk__();
      } else {
        *res_ = false;
        __ReturnOk__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}