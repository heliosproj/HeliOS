/*UNCRUSTIFY-OFF*/
/**
 * @file ramdisk_driver.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief RAM disk driver implementation
 * @details
 * Implements an in-memory block device with configurable size, position control, and usage statistics for filesystem testing and volatile caching.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#include "ramdisk_driver.h"

static Byte_t ramdisk[RAMDISK_SIZE_BYTES] = {

  0

};

/**
 * @brief RAM disk internal state structure
 * @details Maintains runtime statistics and position tracking for the RAM disk device.
 */
typedef struct RAMDiskState_s {

  Word_t currentPosition;       /**< Current read/write position in the disk */

  Word_t bytesRead;             /**< Total bytes read from the disk */

  Word_t bytesWritten;          /**< Total bytes written to the disk */

  Word_t readOperations;        /**< Count of read operations performed */

  Word_t writeOperations;       /**< Count of write operations performed */

  Base_t initialized;           /**< Initialization flag */

} RAMDiskState_t;

static RAMDiskState_t state = {

  0

};

#define __UpdateReadStats__(bytes_) \
        do { \
          state.currentPosition += (bytes_); \
          state.bytesRead += (bytes_); \
          state.readOperations++; \
        } while (0)

#define __UpdateWriteStats__(bytes_) \
        do { \
          state.currentPosition += (bytes_); \
          state.bytesWritten += (bytes_); \
          state.writeOperations++; \
        } while (0)

#define __ValidateBufferParams__(size_, data_) \
        (__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && (0x0u < *(size_)))

/**
 * @brief Validates and truncates requested I/O size
 * @details Internal helper to ensure I/O operations don't exceed disk boundaries.
 *
 * @param[in] requested_ Requested size in bytes
 * @param[out] actual_ Pointer to store actual size that can be transferred
 *
 * @return ReturnOK if validation was successful
 * @return ReturnError if invalid parameters
 */
static Return_t __ValidateAndTruncateSize__(Size_t requested_, Size_t *actual_);
Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void) {

  FUNCTION_ENTER;

  if(OK(__RegisterDevice__(DEVICE_UID,

    (Byte_t *) TO_LITERAL(DEVICE_NAME),

    DEVICE_STATE,

    DEVICE_MODE,

    TO_FUNCTION(DEVICE_NAME, _init),

    TO_FUNCTION(DEVICE_NAME, _config),

    TO_FUNCTION(DEVICE_NAME, _read),

    TO_FUNCTION(DEVICE_NAME, _write),

    TO_FUNCTION(DEVICE_NAME, _simple_read),

    TO_FUNCTION(DEVICE_NAME, _simple_write)))) {

    __ReturnOk__();

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_) {

  FUNCTION_ENTER;

  state.currentPosition = 0x0u;

  state.bytesRead = 0x0u;

  state.bytesWritten = 0x0u;

  state.readOperations = 0x0u;

  state.writeOperations = 0x0u;

  state.initialized = true;

  __memset__(ramdisk, 0x00u, RAMDISK_SIZE_BYTES);

  __ReturnOk__();

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {

  FUNCTION_ENTER;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {

    Byte_t command = *(Byte_t *) config_;

    if(BLOCK_IO_CMD_SET_REQUEST == command) {

      if(*size_ >= sizeof(BlockIORequest_t)) {

        BlockIORequest_t *request = (BlockIORequest_t *) config_;

        Word_t byteOffset = request->blockNumber * request->blockSize;

        Word_t totalBytes = (Word_t) request->blockCount * request->blockSize;

        if((byteOffset + totalBytes) <= RAMDISK_SIZE_BYTES) {

          state.currentPosition = byteOffset;

          __ReturnOk__();

        } else {

          __AssertOnElse__();

        }

      }

    } else if(BLOCK_IO_CMD_GET_INFO == command) {

      if(*size_ >= sizeof(BlockIOInfo_t)) {

        BlockIOInfo_t *info = (BlockIOInfo_t *) config_;

        info->command = BLOCK_IO_CMD_GET_INFO;

        info->totalSizeBytes = RAMDISK_SIZE_BYTES;

        info->nativeBlockSize = 1;

        info->supportsRandomAccess = true;

        info->requiresErase = false;

        __ReturnOk__();

      }

    } else if(RAMDISK_CMD_SET_POSITION == command) {

      if(*size_ >= sizeof(RAMDiskPositionConfig_t)) {

        RAMDiskPositionConfig_t *cfg = (RAMDiskPositionConfig_t *) config_;

        if(cfg->position < RAMDISK_SIZE_BYTES) {

          state.currentPosition = cfg->position;

          __ReturnOk__();

        } else {

          __AssertOnElse__();

        }

      }

    } else if(RAMDISK_CMD_CLEAR_DISK == command) {

      if(*size_ >= sizeof(RAMDiskClearConfig_t)) {

        RAMDiskClearConfig_t *cfg = (RAMDiskClearConfig_t *) config_;

        __memset__(ramdisk, cfg->fillPattern, RAMDISK_SIZE_BYTES);

        state.currentPosition = 0x0u;

        state.bytesRead = 0x0u;

        state.bytesWritten = 0x0u;

        state.readOperations = 0x0u;

        state.writeOperations = 0x0u;

        __ReturnOk__();

      }

    } else if(RAMDISK_CMD_GET_STATS == command) {

      if(*size_ >= sizeof(RAMDiskStats_t)) {

        RAMDiskStats_t *stats = (RAMDiskStats_t *) config_;

        stats->command = RAMDISK_CMD_GET_STATS;

        stats->totalSize = RAMDISK_SIZE_BYTES;

        stats->currentPosition = state.currentPosition;

        stats->bytesRead = state.bytesRead;

        stats->bytesWritten = state.bytesWritten;

        stats->readOperations = state.readOperations;

        stats->writeOperations = state.writeOperations;

        __ReturnOk__();

      }

    }

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {

  FUNCTION_ENTER;

  Byte_t *buffer = null;

  Size_t bytesToRead = 0x0u;

  if(__ValidateBufferParams__(size_, data_)) {

    if(OK(__ValidateAndTruncateSize__(*size_, &bytesToRead))) {

      if(OK(__KernelAllocateMemory__((volatile Addr_t **) &buffer, bytesToRead))) {

        __memcpy__(buffer, &ramdisk[state.currentPosition], bytesToRead);

        __UpdateReadStats__(bytesToRead);

        *data_ = buffer;

        *size_ = bytesToRead;

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
Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_) {

  FUNCTION_ENTER;

  Size_t bytesToWrite = 0x0u;

  if(__ValidateBufferParams__(size_, data_)) {

    if(OK(__ValidateAndTruncateSize__(*size_, &bytesToWrite))) {

      __memcpy__(&ramdisk[state.currentPosition], data_, bytesToWrite);

      __UpdateWriteStats__(bytesToWrite);

      *size_ = bytesToWrite;

      __ReturnOk__();

    } else {

      __AssertOnElse__();

    }

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_) {

  FUNCTION_ENTER;

  if(__PointerIsNotNull__(data_)) {

    if(state.currentPosition < RAMDISK_SIZE_BYTES) {

      *data_ = ramdisk[state.currentPosition];

      __UpdateReadStats__(1);

      __ReturnOk__();

    } else {

      __AssertOnElse__();

    }

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_) {

  FUNCTION_ENTER;

  if(state.currentPosition < RAMDISK_SIZE_BYTES) {

    ramdisk[state.currentPosition] = data_;

    __UpdateWriteStats__(1);

    __ReturnOk__();

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
static Return_t __ValidateAndTruncateSize__(Size_t requested_, Size_t *actual_) {

  FUNCTION_ENTER;

  if((state.currentPosition + requested_) > RAMDISK_SIZE_BYTES) {

    *actual_ = RAMDISK_SIZE_BYTES - state.currentPosition;

    if(0x0u == *actual_) {

      __AssertOnElse__();

    }

  } else {

    *actual_ = requested_;

  }

  __ReturnOk__();

  FUNCTION_EXIT;

}


#if defined(POSIX_ARCH_OTHER)
  void __RAMDiskStateClear__(void) {

    state.currentPosition = 0x0u;

    state.bytesRead = 0x0u;

    state.bytesWritten = 0x0u;

    state.readOperations = 0x0u;

    state.writeOperations = 0x0u;

    state.initialized = false;

    __memset__(ramdisk, 0x00u, RAMDISK_SIZE_BYTES);

    return;

  }


#endif /* if defined(POSIX_ARCH_OTHER) */