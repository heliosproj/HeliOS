/*UNCRUSTIFY-OFF*/
/**
 * @file streams.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Stream buffer implementation
 * @details
 * Implements fixed-size byte stream buffers for efficient data streaming operations including send, receive, peek, and buffer management.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#include "streams.h"

#define __StreamLengthNonZero__() (0x0u < stream_->length)

#define __StreamLengthAtLimit__() (stream_->length == CONFIG_STREAM_BUFFER_BYTES)
Return_t xStreamCreate(StreamBuffer_t **stream_) {

  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stream_)) {

    if(OK(__KernelAllocateMemory__((volatile Addr_t **) stream_, sizeof(StreamBuffer_t)))) {

      if(__PointerIsNotNull__(*stream_)) {

        (*stream_)->valid = VALID;

        (*stream_)->length = 0x0u;

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

  if(__ObjectIsValid__(stream_)) {

    ((StreamBuffer_t *) stream_)->valid = INVALID;

    if(OK(__KernelFreeMemory__(stream_))) {

      __ReturnOk__();

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

  if(__ObjectIsValid__(stream_)) {

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

  FUNCTION_EXIT;

}


Return_t xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_, Byte_t **data_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(stream_) && __PointerIsNotNull__(bytes_) && __PointerIsNotNull__(data_)) {

    if(__StreamLengthNonZero__()) {

      if(OK(__HeapAllocateMemory__((volatile Addr_t **) data_, stream_->length * sizeof(Byte_t)))) {

        if(__PointerIsNotNull__(*data_)) {

          *bytes_ = stream_->length;

          if(OK(__memcpy__(*data_, stream_->buffer, stream_->length * sizeof(Byte_t)))) {

            if(OK(__memset__((Addr_t *) stream_->buffer, 0x0u, CONFIG_STREAM_BUFFER_BYTES))) {

              ((StreamBuffer_t *) stream_)->length = 0x0u;

              __ReturnOk__();

            } else {

              __AssertOnElse__();

              __HeapFreeMemory__(*data_);

            }

          } else {

            __AssertOnElse__();

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

  FUNCTION_EXIT;

}


Return_t xStreamBytesAvailable(const StreamBuffer_t *stream_, HalfWord_t *bytes_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(stream_) && __PointerIsNotNull__(bytes_)) {

    if(__StreamLengthNonZero__()) {

      *bytes_ = stream_->length;

      __ReturnOk__();

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

  if(__ObjectIsValid__(stream_)) {

    if(__StreamLengthNonZero__()) {

      if(OK(__memset__((Addr_t *) stream_->buffer, 0x0u, CONFIG_STREAM_BUFFER_BYTES))) {

        ((StreamBuffer_t *) stream_)->length = 0x0u;

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


Return_t xStreamIsEmpty(const StreamBuffer_t *stream_, Base_t *res_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(stream_) && __PointerIsNotNull__(res_)) {

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

  FUNCTION_EXIT;

}


Return_t xStreamIsFull(const StreamBuffer_t *stream_, Base_t *res_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(stream_) && __PointerIsNotNull__(res_)) {

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

  FUNCTION_EXIT;

}