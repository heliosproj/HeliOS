/*UNCRUSTIFY-OFF*/
/**
 * @file ramdisk_driver.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief RAM disk driver implementation
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "ramdisk_driver.h"


/* RAM disk storage - 1MB static array */
static Byte_t ramdisk[RAMDISK_SIZE_BYTES] = {
  0
};



/* Driver state - tracks position and statistics */
typedef struct RAMDiskState_s {
  Word_t currentPosition;      /* Current read/write position */
  Word_t bytesRead; /* Total bytes read */
  Word_t bytesWritten; /* Total bytes written */
  Word_t readOperations; /* Number of read ops */
  Word_t writeOperations; /* Number of write ops */
  Base_t initialized; /* Initialization flag */
} RAMDiskState_t;



static RAMDiskState_t state = {
  0
};


/* Helper macros for statistics tracking */
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


/* Helper macro for parameter validation */
#define __ValidateBufferParams__(size_, data_) \
        (__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && (nil < *(size_)))


/* Forward declaration for bounds checking helper */
static Return_t __ValidateAndTruncateSize__(Size_t requested_, Size_t *actual_);


/*UNCRUSTIFY-OFF*/
Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void) {
  FUNCTION_ENTER;

  if(OK(__RegisterDevice__(DEVICE_UID,
                          (Byte_t *)TO_LITERAL(DEVICE_NAME),
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
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_) {
  FUNCTION_ENTER;

  /* Initialize state */
  state.currentPosition = nil;
  state.bytesRead = nil;
  state.bytesWritten = nil;
  state.readOperations = nil;
  state.writeOperations = nil;
  state.initialized = true;

  /* Clear RAM disk to zeros */
  __memset__(ramdisk, 0x00u, RAMDISK_SIZE_BYTES);

  __ReturnOk__();
  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {

    /* All config structures start with command byte */
    Byte_t command = *(Byte_t *)config_;

    /* Handle generic block I/O request (NEW - preferred interface) */
    if(BLOCK_IO_CMD_SET_REQUEST == command) {
      if(*size_ >= sizeof(BlockIORequest_t)) {
        BlockIORequest_t *request = (BlockIORequest_t *)config_;

        /* Translate block address to byte offset */
        Word_t byteOffset = request->blockNumber * request->blockSize;

        /* Validate bounds */
        Word_t totalBytes = (Word_t)request->blockCount * request->blockSize;

        if((byteOffset + totalBytes) <= RAMDISK_SIZE_BYTES) {
          /* Set position for subsequent read/write */
          state.currentPosition = byteOffset;
          __ReturnOk__();
        } else {
          /* Return error by default */
          __AssertOnElse__();
        }
      }
    }

    /* Get block I/O info (NEW - optional capability query) */
    else if(BLOCK_IO_CMD_GET_INFO == command) {
      if(*size_ >= sizeof(BlockIOInfo_t)) {
        BlockIOInfo_t *info = (BlockIOInfo_t *)config_;

        info->command = BLOCK_IO_CMD_GET_INFO;
        info->totalSizeBytes = RAMDISK_SIZE_BYTES;
        info->nativeBlockSize = 1; /* Byte-addressable */
        info->supportsRandomAccess = true;
        info->requiresErase = false;

        __ReturnOk__();
      }
    }

    /* Set read/write position (LEGACY - for backward compatibility) */
    else if(RAMDISK_CMD_SET_POSITION == command) {
      if(*size_ >= sizeof(RAMDiskPositionConfig_t)) {
        RAMDiskPositionConfig_t *cfg = (RAMDiskPositionConfig_t *)config_;

        if(cfg->position < RAMDISK_SIZE_BYTES) {
          state.currentPosition = cfg->position;
          __ReturnOk__();
        } else {
          /* Return error by default */
          __AssertOnElse__();
        }
      }
    }

    /* Clear disk with pattern */
    else if(RAMDISK_CMD_CLEAR_DISK == command) {
      if(*size_ >= sizeof(RAMDiskClearConfig_t)) {
        RAMDiskClearConfig_t *cfg = (RAMDiskClearConfig_t *)config_;

        __memset__(ramdisk, cfg->fillPattern, RAMDISK_SIZE_BYTES);
        state.currentPosition = nil;
        state.bytesRead = nil;
        state.bytesWritten = nil;
        state.readOperations = nil;
        state.writeOperations = nil;

        __ReturnOk__();
      }
    }

    /* Get statistics - bidirectional config */
    else if(RAMDISK_CMD_GET_STATS == command) {
      if(*size_ >= sizeof(RAMDiskStats_t)) {
        RAMDiskStats_t *stats = (RAMDiskStats_t *)config_;

        /* Fill in statistics (config is bidirectional) */
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
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;

  Byte_t *buffer = null;
  Size_t bytesToRead = nil;

  if(__ValidateBufferParams__(size_, data_)) {

    /* Validate and truncate size to disk bounds */
    if(OK(__ValidateAndTruncateSize__(*size_, &bytesToRead))) {

      /* Allocate kernel memory for read data */
      if(OK(__KernelAllocateMemory__((volatile Addr_t **)&buffer, bytesToRead))) {

        /* Copy data from RAM disk to buffer */
        __memcpy__(buffer, &ramdisk[state.currentPosition], bytesToRead);

        /* Update statistics */
        __UpdateReadStats__(bytesToRead);

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
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_) {
  FUNCTION_ENTER;

  Size_t bytesToWrite = nil;

  if(__ValidateBufferParams__(size_, data_)) {

    /* Validate and truncate size to disk bounds */
    if(OK(__ValidateAndTruncateSize__(*size_, &bytesToWrite))) {

      /* Copy data from buffer to RAM disk */
      __memcpy__(&ramdisk[state.currentPosition], data_, bytesToWrite);

      /* Update statistics */
      __UpdateWriteStats__(bytesToWrite);

      /* Update actual size written */
      *size_ = bytesToWrite;

      __ReturnOk__();
    } else {
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

  if(__PointerIsNotNull__(data_)) {

    /* Check bounds */
    if(state.currentPosition < RAMDISK_SIZE_BYTES) {

      /* Read single byte from current position */
      *data_ = ramdisk[state.currentPosition];

      /* Update statistics */
      __UpdateReadStats__(1);

      __ReturnOk__();
    } else {
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

  /* Check bounds */
  if(state.currentPosition < RAMDISK_SIZE_BYTES) {

    /* Write single byte to current position */
    ramdisk[state.currentPosition] = data_;

    /* Update statistics */
    __UpdateWriteStats__(1);

    __ReturnOk__();
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* Helper function to validate and truncate size to disk bounds */
static Return_t __ValidateAndTruncateSize__(Size_t requested_, Size_t *actual_) {
  FUNCTION_ENTER;

  if((state.currentPosition + requested_) > RAMDISK_SIZE_BYTES) {
    /* Truncate to remaining space */
    *actual_ = RAMDISK_SIZE_BYTES - state.currentPosition;

    if(nil == *actual_) {
      /* Already at end of disk */
      /* Return error by default */
      __AssertOnElse__();
    }
  } else {
    *actual_ = requested_;
  }

  __ReturnOk__();
  FUNCTION_EXIT;
}


#if defined(POSIX_ARCH_OTHER)


/* For unit testing only! */
void __RAMDiskStateClear__(void) {
  /* Clear state */
  state.currentPosition = nil;
  state.bytesRead = nil;
  state.bytesWritten = nil;
  state.readOperations = nil;
  state.writeOperations = nil;
  state.initialized = false;

  /* Clear RAM disk contents */
  __memset__(ramdisk, 0x00u, RAMDISK_SIZE_BYTES);

  return;
}


#endif /* if defined(POSIX_ARCH_OTHER) */
/*UNCRUSTIFY-ON*/