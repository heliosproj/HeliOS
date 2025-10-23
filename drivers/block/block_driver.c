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
  Byte_t currentTransferMode; /* Current transfer mode */
} BlockDeviceState_t;



static BlockDeviceState_t state = {
  0
};


/* Forward declarations */
static Return_t __PrepareBlockIORequest__(const Byte_t operation_, BlockIORequest_t **request_, Size_t *configSize_);
static Return_t __BlockDeviceReadBlockRAW__(Byte_t **data_);
static Return_t __BlockDeviceWriteBlockRAW__(const Byte_t *data_);


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

  /* Initialization happens in config after I/O driver is configured */
  state.initialized = false;
  state.ioDriverUID = 0;
  state.protocol = 0;
  state.blockSize = BLOCK_DEFAULT_SECTOR_SIZE;
  state.totalBlocks = 0;
  state.currentBlockNumber = 0;
  state.currentBlockCount = 0;
  state.currentTransferMode = BLOCK_IO_MODE_BLOCKING;

  __ReturnOk__();
  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {
    Byte_t command = *(Byte_t *)config_; /* First byte is always command */

    switch(command) {
      case BLOCK_CMD_CONFIG:
        /* Initial configuration - receive BlockDeviceConfig_t */
        if(*size_ >= sizeof(BlockDeviceConfig_t)) {
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
            /* Return error by default */
            __AssertOnElse__();
          }
        } else {
          /* Return error by default */
          __AssertOnElse__();
        }

        break;

      case BLOCK_CMD_SET_ADDRESS:
        /* Block addressing - set current block for read/write */
        if(*size_ >= sizeof(BlockDeviceCommand_t)) {
          BlockDeviceCommand_t *cmd = (BlockDeviceCommand_t *)config_;

          state.currentBlockNumber = cmd->blockNumber;
          state.currentBlockCount = cmd->blockCount;
          state.currentTransferMode = cmd->transferMode;

          __ReturnOk__();
        } else {
          /* Return error by default */
          __AssertOnElse__();
        }

        break;

      case BLOCK_CMD_GET_INFO:
        /* Get device info */
        if(*size_ >= sizeof(BlockDeviceInfo_t)) {
          BlockDeviceInfo_t *info = (BlockDeviceInfo_t *)config_;

          info->blockSize = state.blockSize;
          info->totalBlocks = state.totalBlocks;
          info->totalBytes = (Word_t)state.blockSize * state.totalBlocks;
          info->protocol = state.protocol;
          info->isInitialized = state.initialized;
          info->isWriteProtected = false;

          __ReturnOk__();
        } else {
          /* Return error by default */
          __AssertOnElse__();
        }

        break;

      default:
        /* Return error by default */
        __AssertOnElse__();
    }
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;

  Byte_t *blockData = null;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    /* Dispatch to protocol-specific read function */
    if(BLOCK_PROTOCOL_RAW == state.protocol) {
      if(OK(__BlockDeviceReadBlockRAW__(&blockData))) {
        *data_ = blockData;
        *size_ = (Size_t)state.blockSize * state.currentBlockCount;
        __ReturnOk__();
      } else {
        /* Return error by default */
        __AssertOnElse__();
      }
    } else {
      /* SD/MMC protocols not implemented yet */
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

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    /* Dispatch to protocol-specific write function */
    if(BLOCK_PROTOCOL_RAW == state.protocol) {
      if(OK(__BlockDeviceWriteBlockRAW__((Byte_t *)data_))) {
        __ReturnOk__();
      } else {
        /* Return error by default */
        __AssertOnElse__();
      }
    } else {
      /* SD/MMC protocols not implemented yet */
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
  /* Block devices don't support simple byte-level operations */
  /* Return error by default */
  __AssertOnElse__();
  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_) {
  FUNCTION_ENTER;
  /* Block devices don't support simple byte-level operations */
  /* Return error by default */
  __AssertOnElse__();
  FUNCTION_EXIT;
}


/* ============================================================================
 * Protocol-Specific Implementation: RAW (Direct I/O)
 * ========================================================================== */

/* Helper function to prepare generic block I/O request */
static Return_t __PrepareBlockIORequest__(const Byte_t operation_,
                                         BlockIORequest_t **request_,
                                         Size_t *configSize_) {
  FUNCTION_ENTER;

  Byte_t *ioConfig = null;

  /* Allocate generic block I/O request from kernel heap */
  if(OK(__KernelAllocateMemory__((volatile Addr_t **)&ioConfig, sizeof(BlockIORequest_t)))) {
    BlockIORequest_t *request = (BlockIORequest_t *)ioConfig;

    /* Fill generic request structure from state */
    request->command = BLOCK_IO_CMD_SET_REQUEST;
    request->operation = operation_;
    request->blockNumber = state.currentBlockNumber;
    request->blockCount = state.currentBlockCount;
    request->blockSize = state.blockSize;
    request->transferMode = state.currentTransferMode;
    request->reserved = 0;

    *request_ = request;
    *configSize_ = sizeof(BlockIORequest_t);

    __ReturnOk__();
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __BlockDeviceReadBlockRAW__(Byte_t **data_) {
  FUNCTION_ENTER;

  Size_t totalSize = (Size_t)state.blockSize * state.currentBlockCount;
  Byte_t *buffer = null;
  BlockIORequest_t *request = null;
  Size_t configSize = 0;

  /* Prepare block I/O request from state */
  if(OK(__PrepareBlockIORequest__(BLOCK_IO_OP_READ, &request, &configSize))) {

    /* Send request to I/O driver - it handles translation to native format */
    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &configSize, (Addr_t *)request))) {

      /* Read data from I/O driver using kernel-level API (returns kernel memory) */
      if(OK(__DeviceRead__(state.ioDriverUID, &totalSize, (Addr_t **)&buffer))) {
        *data_ = buffer;
        __KernelFreeMemory__(request);
        __ReturnOk__();
      } else {
        __KernelFreeMemory__(request);
        /* Return error by default */
        __AssertOnElse__();
      }
    } else {
      __KernelFreeMemory__(request);
      /* Return error by default */
      __AssertOnElse__();
    }
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __BlockDeviceWriteBlockRAW__(const Byte_t *data_) {
  FUNCTION_ENTER;

  Size_t totalSize = (Size_t)state.blockSize * state.currentBlockCount;
  BlockIORequest_t *request = null;
  Size_t configSize = 0;

  /* Prepare block I/O request from state */
  if(OK(__PrepareBlockIORequest__(BLOCK_IO_OP_WRITE, &request, &configSize))) {

    /* Send request to I/O driver - it handles translation to native format */
    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &configSize, (Addr_t *)request))) {

      /* Write data to I/O driver using kernel-level API (data already in kernel memory) */
      if(OK(__DeviceWrite__(state.ioDriverUID, &totalSize, (Addr_t *)data_))) {
        __KernelFreeMemory__(request);
        __ReturnOk__();
      } else {
        __KernelFreeMemory__(request);
        /* Return error by default */
        __AssertOnElse__();
      }
    } else {
      __KernelFreeMemory__(request);
      /* Return error by default */
      __AssertOnElse__();
    }
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


#if defined(POSIX_ARCH_OTHER)


/* For unit testing only! */
void __BlockDeviceStateClear__(void) {
  state.ioDriverUID = 0;
  state.protocol = 0;
  state.blockSize = BLOCK_DEFAULT_SECTOR_SIZE;
  state.totalBlocks = 0;
  state.initialized = false;
  state.currentBlockNumber = 0;
  state.currentBlockCount = 0;
  state.currentTransferMode = BLOCK_IO_MODE_BLOCKING;

  return;
}


#endif /* if defined(POSIX_ARCH_OTHER) */
/*UNCRUSTIFY-ON*/