/*UNCRUSTIFY-OFF*/
/**
 * @file block_driver.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Generic block device driver implementation
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
#include "block_driver.h"


/* Driver state - NO hardware-specific fields */
typedef struct BlockDeviceState_s {
  HalfWord_t ioDriverUID;           /* I/O driver to use for communication */
  Byte_t protocol; /* Storage protocol (SD/MMC/RAW) */
  HalfWord_t blockSize; /* Bytes per block */
  Word_t totalBlocks; /* Device capacity */
  Base_t initialized; /* Initialization flag */
  Word_t currentBlockNumber; /* Last addressed block */
  HalfWord_t currentBlockCount; /* Blocks in current operation */
} BlockDeviceState_t;



static BlockDeviceState_t state = {
  0
};


/* Forward declarations */
static Return_t __BlockDeviceReadBlockRAW__(const Word_t blockNum_, const HalfWord_t blockCount_, Byte_t **data_);
static Return_t __BlockDeviceWriteBlockRAW__(const Word_t blockNum_, const HalfWord_t blockCount_, const Byte_t *data_);


/*UNCRUSTIFY-OFF*/
Return_t BLOCKDEV_self_register(void) {
  FUNCTION_ENTER;

  if(OK(__RegisterDevice__(DEVICE_UID,
                          (Byte_t *)TO_LITERAL(DEVICE_NAME),
                          DEVICE_STATE,
                          DEVICE_MODE,
                          BLOCKDEV_init,
                          BLOCKDEV_config,
                          BLOCKDEV_read,
                          BLOCKDEV_write,
                          BLOCKDEV_simple_read,
                          BLOCKDEV_simple_write))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t BLOCKDEV_init(Device_t *device_) {
  FUNCTION_ENTER;

  /* Initialization happens in config after I/O driver is configured */
  state.initialized = false;
  state.ioDriverUID = 0;
  state.protocol = 0;
  state.blockSize = 512;  /* Default */
  state.totalBlocks = 0;
  state.currentBlockNumber = 0;
  state.currentBlockCount = 0;

  __ReturnOk__();
  FUNCTION_EXIT;
}


Return_t BLOCKDEV_config(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {

    /* Initial configuration - receive BlockDeviceConfig_t */
    if(*size_ == sizeof(BlockDeviceConfig_t)) {
      BlockDeviceConfig_t *cfg = (BlockDeviceConfig_t *)config_;

      /* Store I/O driver UID and protocol type */
      state.ioDriverUID = cfg->ioDriverUID;
      state.protocol = cfg->protocol;
      state.blockSize = cfg->blockSize;
      state.totalBlocks = cfg->totalBlocks;

      /* For RAW protocol, no initialization needed */
      if(BLOCK_PROTOCOL_RAW == state.protocol) {
        state.initialized = true;

        /* Return configured values back to caller */
        cfg->blockSize = state.blockSize;
        cfg->totalBlocks = state.totalBlocks;

        __ReturnOk__();
      }
      /* SD/MMC protocols would initialize here */
      else {
        /* Not implemented yet */
        __AssertOnElse__();
      }
    }

    /* Block addressing - set current block for read/write */
    else if(*size_ == sizeof(BlockDeviceCommand_t)) {
      BlockDeviceCommand_t *cmd = (BlockDeviceCommand_t *)config_;

      state.currentBlockNumber = cmd->blockNumber;
      state.currentBlockCount = cmd->blockCount;

      __ReturnOk__();
    }

    /* Get device info */
    else if(*size_ == sizeof(BlockDeviceInfo_t)) {
      BlockDeviceInfo_t *info = (BlockDeviceInfo_t *)config_;

      info->blockSize = state.blockSize;
      info->totalBlocks = state.totalBlocks;
      info->totalBytes = (Word_t)state.blockSize * state.totalBlocks;
      info->protocol = state.protocol;
      info->isInitialized = state.initialized;
      info->isWriteProtected = false;

      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t BLOCKDEV_read(Device_t *device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;

  Byte_t *blockData = null;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    /* Dispatch to protocol-specific read function */
    if(BLOCK_PROTOCOL_RAW == state.protocol) {
      if(OK(__BlockDeviceReadBlockRAW__(state.currentBlockNumber,
                                       state.currentBlockCount,
                                       &blockData))) {
        *data_ = blockData;
        *size_ = (Size_t)state.blockSize * state.currentBlockCount;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      /* SD/MMC protocols not implemented yet */
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t BLOCKDEV_write(Device_t *device_, Size_t *size_, Addr_t *data_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    /* Dispatch to protocol-specific write function */
    if(BLOCK_PROTOCOL_RAW == state.protocol) {
      if(OK(__BlockDeviceWriteBlockRAW__(state.currentBlockNumber,
                                        state.currentBlockCount,
                                        (Byte_t *)data_))) {
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      /* SD/MMC protocols not implemented yet */
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t BLOCKDEV_simple_read(Device_t *device_, Byte_t *data_) {
  FUNCTION_ENTER;
  /* Not typically used for block devices */
  FUNCTION_EXIT;
}


Return_t BLOCKDEV_simple_write(Device_t *device_, Byte_t data_) {
  FUNCTION_ENTER;
  /* Not typically used for block devices */
  FUNCTION_EXIT;
}


/* ============================================================================
 * Protocol-Specific Implementation: RAW (Direct I/O)
 * ========================================================================== */

static Return_t __BlockDeviceReadBlockRAW__(const Word_t blockNum_,
                                           const HalfWord_t blockCount_,
                                           Byte_t **data_) {
  FUNCTION_ENTER;

  Size_t totalSize = (Size_t)state.blockSize * blockCount_;
  Word_t byteOffset = (Word_t)state.blockSize * blockNum_;
  Byte_t *buffer = null;
  Byte_t *posConfig = null;
  Size_t posSize = 0;

  /* For RAW protocol, we need to tell the I/O driver where to seek */
  typedef struct RAMDiskPositionConfig_s {
    Byte_t command;
    Word_t position;
  } RAMDiskPositionConfig_t;

  /* Allocate position config from kernel heap (drivers use kernel memory) */
  if(OK(__KernelAllocateMemory__((volatile Addr_t **)&posConfig, sizeof(RAMDiskPositionConfig_t)))) {
    RAMDiskPositionConfig_t *cfg = (RAMDiskPositionConfig_t *)posConfig;
    cfg->command = 0x01u;  /* RAMDISK_CMD_SET_POSITION */
    cfg->position = byteOffset;

    posSize = sizeof(RAMDiskPositionConfig_t);

    /* Set I/O driver position using kernel-level device API (no heap memory copy) */
    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &posSize, (Addr_t *)posConfig))) {

      /* Read data from I/O driver using kernel-level API (returns kernel memory) */
      if(OK(__DeviceRead__(state.ioDriverUID, &totalSize, (Addr_t **)&buffer))) {
        *data_ = buffer;
        __KernelFreeMemory__(posConfig);
        __ReturnOk__();
      } else {
        __KernelFreeMemory__(posConfig);
        __AssertOnElse__();
      }
    } else {
      __KernelFreeMemory__(posConfig);
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __BlockDeviceWriteBlockRAW__(const Word_t blockNum_,
                                            const HalfWord_t blockCount_,
                                            const Byte_t *data_) {
  FUNCTION_ENTER;

  Size_t totalSize = (Size_t)state.blockSize * blockCount_;
  Word_t byteOffset = (Word_t)state.blockSize * blockNum_;
  Byte_t *posConfig = null;
  Size_t posSize = 0;

  /* For RAW protocol, we need to tell the I/O driver where to seek */
  typedef struct RAMDiskPositionConfig_s {
    Byte_t command;
    Word_t position;
  } RAMDiskPositionConfig_t;

  /* Allocate position config from kernel heap (drivers use kernel memory) */
  if(OK(__KernelAllocateMemory__((volatile Addr_t **)&posConfig, sizeof(RAMDiskPositionConfig_t)))) {
    RAMDiskPositionConfig_t *cfg = (RAMDiskPositionConfig_t *)posConfig;
    cfg->command = 0x01u;  /* RAMDISK_CMD_SET_POSITION */
    cfg->position = byteOffset;

    posSize = sizeof(RAMDiskPositionConfig_t);

    /* Set I/O driver position using kernel-level device API (no heap memory copy) */
    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &posSize, (Addr_t *)posConfig))) {

      /* Write data to I/O driver using kernel-level API (data already in kernel memory) */
      if(OK(__DeviceWrite__(state.ioDriverUID, &totalSize, (Addr_t *)data_))) {
        __KernelFreeMemory__(posConfig);
        __ReturnOk__();
      } else {
        __KernelFreeMemory__(posConfig);
        __AssertOnElse__();
      }
    } else {
      __KernelFreeMemory__(posConfig);
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


#if defined(POSIX_ARCH_OTHER)


/* For unit testing only! */
void __BlockDeviceStateClear__(void) {
  state.ioDriverUID = 0;
  state.protocol = 0;
  state.blockSize = 512;
  state.totalBlocks = 0;
  state.initialized = false;
  state.currentBlockNumber = 0;
  state.currentBlockCount = 0;

  return;
}


#endif /* if defined(POSIX_ARCH_OTHER) */
/*UNCRUSTIFY-ON*/