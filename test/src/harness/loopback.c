/*UNCRUSTIFY-OFF*/
/**
 * @file loopback.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Loopback device driver implementation
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "loopback.h"


/* Loopback circular buffer storage */
static Byte_t loopbackBuffer[LOOPBACK_BUFFER_SIZE] = {
  0
};



/* Driver state - tracks position, availability, and statistics */
typedef struct LoopbackState_s {
  HalfWord_t writePosition;    /* Current write position */
  HalfWord_t readPosition; /* Current read position */
  HalfWord_t bytesAvailable; /* Bytes available to read */
  Word_t bytesRead; /* Total bytes read */
  Word_t bytesWritten; /* Total bytes written */
  Word_t readOperations; /* Number of read ops */
  Word_t writeOperations; /* Number of write ops */
  Byte_t mode; /* Operation mode (FIFO/ECHO) */
  Base_t initialized; /* Initialization flag */
} LoopbackState_t;



static LoopbackState_t state = {
  0
};


/* Helper macros for statistics tracking */
#define __UpdateReadStats__(bytes_) \
        do { \
          state.bytesRead += (bytes_); \
          state.readOperations++; \
        } while (0)

#define __UpdateWriteStats__(bytes_) \
        do { \
          state.bytesWritten += (bytes_); \
          state.writeOperations++; \
        } while (0)


/* Helper macro for parameter validation */
#define __ValidateBufferParams__(size_, data_) \
        (__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && (0x0u < *(size_)))


/* Forward declarations for helper functions */
static Return_t __ValidateAndTruncateSize__(Size_t requested_, Size_t *actual_);
static void __UpdateAvailability__(Device_t *device_);


/*UNCRUSTIFY-OFF*/
Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void) {
  FUNCTION_ENTER;

  if(OK(__RegisterDevice__(DEVICE_UID,
                          (Byte_t *)TO_LITERAL(DEVICE_NAME),
                          DEVICE_STATE,
                          DEVICE_MODE,
                          LOOPBACK_init,
                          LOOPBACK_config,
                          LOOPBACK_read,
                          LOOPBACK_write,
                          LOOPBACK_simple_read,
                          LOOPBACK_simple_write))) {
    __ReturnOk__();
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(device_)) {

    /* Initialize state */
    state.writePosition = 0x0u;
    state.readPosition = 0x0u;
    state.bytesAvailable = 0x0u;
    state.bytesRead = 0x0u;
    state.bytesWritten = 0x0u;
    state.readOperations = 0x0u;
    state.writeOperations = 0x0u;
    state.mode = LOOPBACK_MODE_FIFO;
    state.initialized = true;

    /* Clear buffer to zeros */
    __memset__(loopbackBuffer, 0x00u, LOOPBACK_BUFFER_SIZE);

    /* Set device as not available (no data to read) */
    device_->available = false;

    __ReturnOk__();
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(device_) && __PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {

    /* All config structures start with command byte */
    Byte_t command = *(Byte_t *)config_;

    /* Handle generic block I/O request (standard interface) */
    if(BLOCK_IO_CMD_SET_REQUEST == command) {
      if(*size_ >= sizeof(BlockIORequest_t)) {
        BlockIORequest_t *request = (BlockIORequest_t *)config_;

        /* Translate block address to byte offset */
        HalfWord_t byteOffset = (HalfWord_t)(request->blockNumber * request->blockSize);

        /* Validate bounds */
        HalfWord_t totalBytes = (HalfWord_t)(request->blockCount * request->blockSize);

        if((byteOffset + totalBytes) <= LOOPBACK_BUFFER_SIZE) {
          /* Set position for subsequent read/write */
          if(BLOCK_IO_OP_READ == request->operation) {
            state.readPosition = byteOffset;
          } else if(BLOCK_IO_OP_WRITE == request->operation) {
            state.writePosition = byteOffset;
          }

          __ReturnOk__();
        } else {
          /* Return error by default */
          __AssertOnElse__();
        }
      }
    }

    /* Get block I/O info (capability query) */
    else if(BLOCK_IO_CMD_GET_INFO == command) {
      if(*size_ >= sizeof(BlockIOInfo_t)) {
        BlockIOInfo_t *info = (BlockIOInfo_t *)config_;

        info->command = BLOCK_IO_CMD_GET_INFO;
        info->totalSizeBytes = LOOPBACK_BUFFER_SIZE;
        info->nativeBlockSize = 1; /* Byte-addressable */
        info->supportsRandomAccess = true;
        info->requiresErase = false;

        __ReturnOk__();
      }
    }

    /* Set read/write position */
    else if(LOOPBACK_CMD_SET_POSITION == command) {
      if(*size_ >= sizeof(LoopbackPositionConfig_t)) {
        LoopbackPositionConfig_t *cfg = (LoopbackPositionConfig_t *)config_;

        if(cfg->position < LOOPBACK_BUFFER_SIZE) {
          state.readPosition = cfg->position;
          state.writePosition = cfg->position;
          __ReturnOk__();
        } else {
          /* Return error by default */
          __AssertOnElse__();
        }
      }
    }

    /* Clear buffer with pattern */
    else if(LOOPBACK_CMD_CLEAR_BUFFER == command) {
      if(*size_ >= sizeof(LoopbackClearConfig_t)) {
        LoopbackClearConfig_t *cfg = (LoopbackClearConfig_t *)config_;

        __memset__(loopbackBuffer, cfg->fillPattern, LOOPBACK_BUFFER_SIZE);
        state.writePosition = 0x0u;
        state.readPosition = 0x0u;
        state.bytesAvailable = 0x0u;
        state.bytesRead = 0x0u;
        state.bytesWritten = 0x0u;
        state.readOperations = 0x0u;
        state.writeOperations = 0x0u;
        device_->available = false;

        __ReturnOk__();
      }
    }

    /* Set operation mode */
    else if(LOOPBACK_CMD_SET_MODE == command) {
      if(*size_ >= sizeof(LoopbackModeConfig_t)) {
        LoopbackModeConfig_t *cfg = (LoopbackModeConfig_t *)config_;

        if((LOOPBACK_MODE_FIFO == cfg->mode) || (LOOPBACK_MODE_ECHO == cfg->mode)) {
          state.mode = cfg->mode;
          __ReturnOk__();
        } else {
          /* Return error by default */
          __AssertOnElse__();
        }
      }
    }

    /* Get statistics - bidirectional config */
    else if(LOOPBACK_CMD_GET_STATS == command) {
      if(*size_ >= sizeof(LoopbackStats_t)) {
        LoopbackStats_t *stats = (LoopbackStats_t *)config_;

        /* Fill in statistics (config is bidirectional) */
        stats->command = LOOPBACK_CMD_GET_STATS;
        stats->bufferSize = LOOPBACK_BUFFER_SIZE;
        stats->currentPosition = state.readPosition;
        stats->bytesAvailable = state.bytesAvailable;
        stats->bytesRead = state.bytesRead;
        stats->bytesWritten = state.bytesWritten;
        stats->readOperations = state.readOperations;
        stats->writeOperations = state.writeOperations;
        stats->mode = state.mode;

        __ReturnOk__();
      }
    }
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;

  Byte_t *buffer = null;
  Size_t bytesToRead = 0x0u;
  Size_t i;

  if(__PointerIsNotNull__(device_) && __ValidateBufferParams__(size_, data_)) {

    /* Check if data is available */
    if(state.bytesAvailable > 0) {

      /* Validate and truncate size to available data */
      if(OK(__ValidateAndTruncateSize__(*size_, &bytesToRead))) {

        /* Allocate kernel memory for read data */
        if(OK(__KernelAllocateMemory__((volatile Addr_t **)&buffer, bytesToRead))) {

          /* Copy data from circular buffer */
          for(i = 0x0u; i < bytesToRead; i++) {
            buffer[i] = loopbackBuffer[state.readPosition];
            state.readPosition = (state.readPosition + 1) % LOOPBACK_BUFFER_SIZE;
          }

          /* Update availability */
          state.bytesAvailable -= bytesToRead;

          /* Update statistics */
          __UpdateReadStats__(bytesToRead);

          /* Update device availability flag */
          __UpdateAvailability__(device_);

          /* Return buffer and actual size read */
          *data_ = buffer;
          *size_ = bytesToRead;

          __ReturnOk__();
        } else {
          /* Return error by default */
          __AssertOnElse__();
        }
      } else {
        /* Return error by default */
        __AssertOnElse__();
      }
    } else {
      /* No data available */
      /* Return error by default */
      __AssertOnElse__();
    }
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_) {
  FUNCTION_ENTER;

  Size_t bytesToWrite = 0x0u;
  Size_t i;

  if(__PointerIsNotNull__(device_) && __ValidateBufferParams__(size_, data_)) {

    /* Check if buffer has space */
    Size_t spaceAvailable = LOOPBACK_BUFFER_SIZE - state.bytesAvailable;

    if(spaceAvailable > 0) {

      /* Limit write to available space */
      bytesToWrite = (*size_ > spaceAvailable) ? spaceAvailable : *size_;

      /* Copy data to circular buffer */
      for(i = 0x0u; i < bytesToWrite; i++) {
        loopbackBuffer[state.writePosition] = ((Byte_t *)data_)[i];
        state.writePosition = (state.writePosition + 1) % LOOPBACK_BUFFER_SIZE;
      }

      /* Update availability */
      state.bytesAvailable += bytesToWrite;

      /* Update statistics */
      __UpdateWriteStats__(bytesToWrite);

      /* Update device availability flag */
      __UpdateAvailability__(device_);

      /* Update actual size written */
      *size_ = bytesToWrite;

      __ReturnOk__();
    } else {
      /* Buffer full */
      /* Return error by default */
      __AssertOnElse__();
    }
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(device_) && __PointerIsNotNull__(data_)) {

    /* Check if data is available */
    if(state.bytesAvailable > 0) {

      /* Read single byte from current read position */
      *data_ = loopbackBuffer[state.readPosition];
      state.readPosition = (state.readPosition + 1) % LOOPBACK_BUFFER_SIZE;

      /* Update availability */
      state.bytesAvailable--;

      /* Update statistics */
      __UpdateReadStats__(1);

      /* Update device availability flag */
      __UpdateAvailability__(device_);

      __ReturnOk__();
    } else {
      /* No data available */
      /* Return error by default */
      __AssertOnElse__();
    }
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(device_)) {

    /* Check if buffer has space */
    if(state.bytesAvailable < LOOPBACK_BUFFER_SIZE) {

      /* Write single byte to current write position */
      loopbackBuffer[state.writePosition] = data_;
      state.writePosition = (state.writePosition + 1) % LOOPBACK_BUFFER_SIZE;

      /* Update availability */
      state.bytesAvailable++;

      /* Update statistics */
      __UpdateWriteStats__(1);

      /* Update device availability flag */
      __UpdateAvailability__(device_);

      __ReturnOk__();
    } else {
      /* Buffer full */
      /* Return error by default */
      __AssertOnElse__();
    }
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* Helper function to validate and truncate size to available data */
static Return_t __ValidateAndTruncateSize__(Size_t requested_, Size_t *actual_) {
  FUNCTION_ENTER;

  if(requested_ > state.bytesAvailable) {
    /* Truncate to available data */
    *actual_ = state.bytesAvailable;

    if(0x0u == *actual_) {
      /* No data available */
      /* Return error by default */
      __AssertOnElse__();
    }
  } else {
    *actual_ = requested_;
  }

  __ReturnOk__();
  FUNCTION_EXIT;
}


/* Helper function to update device availability flag */
static void __UpdateAvailability__(Device_t *device_) {
  if(LOOPBACK_MODE_ECHO == state.mode) {
    /* In echo mode, data is immediately available if written */
    device_->available = (state.bytesAvailable > 0) ? true : false;
  } else {
    /* In FIFO mode, data is available if present */
    device_->available = (state.bytesAvailable > 0) ? true : false;
  }

  return;
}


#if defined(POSIX_ARCH_OTHER)


/* For unit testing only! */
void __LoopbackStateClear__(void) {
  /* Clear state */
  state.writePosition = 0x0u;
  state.readPosition = 0x0u;
  state.bytesAvailable = 0x0u;
  state.bytesRead = 0x0u;
  state.bytesWritten = 0x0u;
  state.readOperations = 0x0u;
  state.writeOperations = 0x0u;
  state.mode = LOOPBACK_MODE_FIFO;
  state.initialized = false;

  /* Clear buffer contents */
  __memset__(loopbackBuffer, 0x00u, LOOPBACK_BUFFER_SIZE);

  return;
}


#endif /* if defined(POSIX_ARCH_OTHER) */
/*UNCRUSTIFY-ON*/