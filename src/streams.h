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
  #endif
  #include "mem.h"
  #include "port.h"
  #include "posix.h"
  #include "queue.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"
  #if defined(__StreamLengthNonZero__)
    #undef __StreamLengthNonZero__
  #endif
  #define __StreamLengthNonZero__() (0x0u < stream_->length)
  #if defined(__StreamLengthAtLimit__)
    #undef __StreamLengthAtLimit__
  #endif
  #define __StreamLengthAtLimit__() (stream_->length == CONFIG_STREAM_BUFFER_BYTES)
  #ifdef __cplusplus
    extern "C" {
  #endif
  Return_t xStreamCreate(StreamBuffer_t **stream_);
  Return_t xStreamDelete(const StreamBuffer_t *stream_);
  Return_t xStreamSend(StreamBuffer_t *stream_, const Byte_t byte_);
  Return_t xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_, Byte_t **data_);
  Return_t xStreamBytesAvailable(const StreamBuffer_t *stream_, HalfWord_t *bytes_);
  Return_t xStreamReset(const StreamBuffer_t *stream_);
  Return_t xStreamIsEmpty(const StreamBuffer_t *stream_, Base_t *res_);
  Return_t xStreamIsFull(const StreamBuffer_t *stream_, Base_t *res_);
  #ifdef __cplusplus
    }
  #endif
#endif