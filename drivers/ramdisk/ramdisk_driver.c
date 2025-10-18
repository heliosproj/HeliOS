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



/*UNCRUSTIFY-OFF*/
Return_t RAMDISK0_self_register(void) {
  FUNCTION_ENTER;

  if(OK(__RegisterDevice__(DEVICE_UID,
                          (Byte_t *)TO_LITERAL(DEVICE_NAME),
                          DEVICE_STATE,
                          DEVICE_MODE,
                          RAMDISK0_init,
                          RAMDISK0_config,
                          RAMDISK0_read,
                          RAMDISK0_write,
                          RAMDISK0_simple_read,
                          RAMDISK0_simple_write))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t RAMDISK0_init(Device_t *device_) {
  FUNCTION_ENTER;

  /* Initialize state */
  state.currentPosition = 0;
  state.bytesRead = 0;
  state.bytesWritten = 0;
  state.readOperations = 0;
  state.writeOperations = 0;
  state.initialized = true;

  /* Clear RAM disk to zeros */
  __memset__(ramdisk, 0x00u, RAMDISK_SIZE_BYTES);

  __ReturnOk__();
  FUNCTION_EXIT;
}


Return_t RAMDISK0_config(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {

    /* All config structures start with command byte */
    Byte_t command = *(Byte_t *)config_;

    /* Set read/write position */
    if(RAMDISK_CMD_SET_POSITION == command) {
      if(*size_ == sizeof(RAMDiskPositionConfig_t)) {
        RAMDiskPositionConfig_t *cfg = (RAMDiskPositionConfig_t *)config_;

        if(cfg->position < RAMDISK_SIZE_BYTES) {
          state.currentPosition = cfg->position;
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      }
    }

    /* Clear disk with pattern */
    else if(RAMDISK_CMD_CLEAR_DISK == command) {
      if(*size_ == sizeof(RAMDiskClearConfig_t)) {
        RAMDiskClearConfig_t *cfg = (RAMDiskClearConfig_t *)config_;

        __memset__(ramdisk, cfg->fillPattern, RAMDISK_SIZE_BYTES);
        state.currentPosition = 0;
        state.bytesRead = 0;
        state.bytesWritten = 0;
        state.readOperations = 0;
        state.writeOperations = 0;

        __ReturnOk__();
      }
    }

    /* Get statistics - bidirectional config */
    else if(RAMDISK_CMD_GET_STATS == command) {
      if(*size_ == sizeof(RAMDiskStats_t)) {
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
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t RAMDISK0_read(Device_t *device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;

  Byte_t *buffer = null;
  Size_t bytesToRead = *size_;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && (nil < *size_)) {

    /* Validate read is within disk bounds */
    if((state.currentPosition + bytesToRead) > RAMDISK_SIZE_BYTES) {
      /* Truncate read to disk size */
      bytesToRead = RAMDISK_SIZE_BYTES - state.currentPosition;

      if(nil == bytesToRead) {
        /* Already at end of disk */
        __AssertOnElse__();
        FUNCTION_EXIT;
      }
    }

    /* Allocate kernel memory for read data */
    if(OK(__KernelAllocateMemory__((volatile Addr_t **)&buffer, bytesToRead))) {

      /* Copy data from RAM disk to buffer */
      __memcpy__(buffer, &ramdisk[state.currentPosition], bytesToRead);

      /* Update state */
      state.currentPosition += bytesToRead;
      state.bytesRead += bytesToRead;
      state.readOperations++;

      /* Return buffer and actual size read */
      *data_ = buffer;
      *size_ = bytesToRead;

      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t RAMDISK0_write(Device_t *device_, Size_t *size_, Addr_t *data_) {
  FUNCTION_ENTER;

  Size_t bytesToWrite = *size_;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && (nil < *size_)) {

    /* Validate write is within disk bounds */
    if((state.currentPosition + bytesToWrite) > RAMDISK_SIZE_BYTES) {
      /* Truncate write to disk size */
      bytesToWrite = RAMDISK_SIZE_BYTES - state.currentPosition;

      if(nil == bytesToWrite) {
        /* Already at end of disk */
        __AssertOnElse__();
        FUNCTION_EXIT;
      }
    }

    /* Copy data from buffer to RAM disk */
    __memcpy__(&ramdisk[state.currentPosition], data_, bytesToWrite);

    /* Update state */
    state.currentPosition += bytesToWrite;
    state.bytesWritten += bytesToWrite;
    state.writeOperations++;

    /* Update actual size written */
    *size_ = bytesToWrite;

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t RAMDISK0_simple_read(Device_t *device_, Byte_t *data_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(data_)) {

    /* Check bounds */
    if(state.currentPosition < RAMDISK_SIZE_BYTES) {

      /* Read single byte from current position */
      *data_ = ramdisk[state.currentPosition];

      /* Update state */
      state.currentPosition++;
      state.bytesRead++;
      state.readOperations++;

      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t RAMDISK0_simple_write(Device_t *device_, Byte_t data_) {
  FUNCTION_ENTER;

  /* Check bounds */
  if(state.currentPosition < RAMDISK_SIZE_BYTES) {

    /* Write single byte to current position */
    ramdisk[state.currentPosition] = data_;

    /* Update state */
    state.currentPosition++;
    state.bytesWritten++;
    state.writeOperations++;

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


#if defined(POSIX_ARCH_OTHER)


/* For unit testing only! */
void __RAMDiskStateClear__(void) {
  /* Clear state */
  state.currentPosition = 0;
  state.bytesRead = 0;
  state.bytesWritten = 0;
  state.readOperations = 0;
  state.writeOperations = 0;
  state.initialized = false;

  /* Clear RAM disk contents */
  __memset__(ramdisk, 0x00u, RAMDISK_SIZE_BYTES);

  return;
}


#endif /* if defined(POSIX_ARCH_OTHER) */
/*UNCRUSTIFY-ON*/