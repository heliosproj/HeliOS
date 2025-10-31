/*UNCRUSTIFY-OFF*/


/**
 * @file block_driver.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Block device driver implementation
 * @details
 * Implements block device operations for sector-based storage including read, write, address management, and protocol abstraction for SD/MMC devices.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */



/*UNCRUSTIFY-ON*/


#include "block_driver.h"


/**
 * @brief Block device internal state structure
 * @details Maintains runtime state for block device operations including I/O
 * driver binding, protocol settings, and current operation parameters.
 */
typedef struct BlockDeviceState_s {


  HalfWord_t ioDriverUID; /**< UID of underlying I/O driver device */
  Byte_t protocol; /**< Storage protocol in use (SD card, MMC, eMMC, or raw) */
  HalfWord_t blockSize; /**< Block size in bytes */
  Word_t totalBlocks; /**< Total number of blocks on device */
  Base_t initialized; /**< Initialization flag */
  Word_t currentBlockNumber; /**< Current block number for I/O operations */
  HalfWord_t currentBlockCount; /**< Current block count for I/O operations */
  Byte_t currentTransferMode; /**< Current transfer mode (blocking, DMA, etc.)
                               */
} BlockDeviceState_t;


static BlockDeviceState_t state = {


  0


};
/**
 * @brief Prepares block I/O request structure
 * @details Internal helper to create and populate a BlockIORequest_t structure
 * for I/O operations.
 *
 * @param[in]  operation_  Operation type (read or write)
 * @param[out] request_    Pointer to store allocated request structure
 * @param[out] configSize_ Pointer to store size of request structure
 *
 * @return                 ReturnOK if request was prepared successfully
 * @return                 ReturnError if allocation failed
 *
 * @warning Caller is responsible for freeing the allocated request structure
 */
static Return_t __PrepareBlockIORequest__(const Byte_t operation_, BlockIORequest_t **request_, Size_t *configSize_);
/**
 * @brief Reads blocks using RAW protocol
 * @details Internal function to read blocks from the underlying I/O driver
 * using raw protocol.
 *
 * @param[out] data_ Pointer to store allocated data buffer
 *
 * @return           ReturnOK if read was successful
 * @return           ReturnError if read failed
 *
 * @warning Caller is responsible for freeing the allocated data buffer
 */
static Return_t __BlockDeviceReadBlockRAW__(Byte_t **data_);
/**
 * @brief Writes blocks using RAW protocol
 * @details Internal function to write blocks to the underlying I/O driver using
 * raw protocol.
 *
 * @param[in] data_ Pointer to data buffer to write
 *
 * @return          ReturnOK if write was successful
 * @return          ReturnError if write failed
 */
static Return_t __BlockDeviceWriteBlockRAW__(const Byte_t *data_);
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


  state.initialized = false;


  state.ioDriverUID = 0x0u;


  state.protocol = 0x0u;


  state.blockSize = BLOCK_DEFAULT_SECTOR_SIZE;


  state.totalBlocks = 0x0u;


  state.currentBlockNumber = 0x0u;


  state.currentBlockCount = 0x0u;


  state.currentTransferMode = BLOCK_IO_MODE_BLOCKING;


  __ReturnOk__();


  FUNCTION_EXIT;


}
Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {


  FUNCTION_ENTER;


  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {


    Byte_t command = *(Byte_t *) config_;


    switch(command) {


    case BLOCK_CMD_CONFIG:


      if(*size_ >= sizeof(BlockDeviceConfig_t)) {


        BlockDeviceConfig_t *cfg = (BlockDeviceConfig_t *) config_;


        state.ioDriverUID = cfg->ioDriverUID;


        state.protocol = cfg->protocol;


        state.blockSize = cfg->blockSize;


        state.totalBlocks = cfg->totalBlocks;


        if(BLOCK_PROTOCOL_RAW == state.protocol) {


          state.initialized = true;


          cfg->blockSize = state.blockSize;


          cfg->totalBlocks = state.totalBlocks;


          __ReturnOk__();


        } else {


          __AssertOnElse__();


        }


      } else {


        __AssertOnElse__();


      }


      break;


    case BLOCK_CMD_SET_ADDRESS:


      if(*size_ >= sizeof(BlockDeviceCommand_t)) {


        BlockDeviceCommand_t *cmd = (BlockDeviceCommand_t *) config_;


        state.currentBlockNumber = cmd->blockNumber;


        state.currentBlockCount = cmd->blockCount;


        state.currentTransferMode = cmd->transferMode;


        __ReturnOk__();


      } else {


        __AssertOnElse__();


      }


      break;


    case BLOCK_CMD_GET_INFO:


      if(*size_ >= sizeof(BlockDeviceInfo_t)) {


        BlockDeviceInfo_t *info = (BlockDeviceInfo_t *) config_;


        info->blockSize = state.blockSize;


        info->totalBlocks = state.totalBlocks;


        info->totalBytes = (Word_t) state.blockSize * state.totalBlocks;


        info->protocol = state.protocol;


        info->isInitialized = state.initialized;


        info->isWriteProtected = false;


        __ReturnOk__();


      } else {


        __AssertOnElse__();


      }


      break;


    default:


      __AssertOnElse__();


    }


  } else {


    __AssertOnElse__();


  }


  FUNCTION_EXIT;


}
Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {


  FUNCTION_ENTER;


  Byte_t *blockData = null;


  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {


    if(BLOCK_PROTOCOL_RAW == state.protocol) {


      if(OK(__BlockDeviceReadBlockRAW__(&blockData))) {


        *data_ = blockData;


        *size_ = (Size_t) state.blockSize * state.currentBlockCount;


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


  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {


    if(BLOCK_PROTOCOL_RAW == state.protocol) {


      if(OK(__BlockDeviceWriteBlockRAW__((Byte_t *) data_))) {


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
Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_) {


  FUNCTION_ENTER;


  __AssertOnElse__();


  FUNCTION_EXIT;


}
Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_) {


  FUNCTION_ENTER;


  __AssertOnElse__();


  FUNCTION_EXIT;


}
static Return_t __PrepareBlockIORequest__(const Byte_t operation_,


  BlockIORequest_t **request_,


  Size_t *configSize_) {

  FUNCTION_ENTER;

  Byte_t *ioConfig = null;

  if(OK(__KernelAllocateMemory__((volatile Addr_t **) &ioConfig, sizeof(BlockIORequest_t)))) {

    BlockIORequest_t *request = (BlockIORequest_t *) ioConfig;

    request->command = BLOCK_IO_CMD_SET_REQUEST;

    request->operation = operation_;

    request->blockNumber = state.currentBlockNumber;

    request->blockCount = state.currentBlockCount;

    request->blockSize = state.blockSize;

    request->transferMode = state.currentTransferMode;

    request->reserved = 0x0u;

    *request_ = request;

    *configSize_ = sizeof(BlockIORequest_t);

    __ReturnOk__();

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}


/**
 * @brief Raw block read operation
 * @details Internal helper that performs a raw block read from the current
 * block position.
 *
 * @param[out] data_ Pointer to store read data
 *
 * @return           ReturnOK if block was read successfully
 * @return           ReturnError if read operation failed
 */
static Return_t __BlockDeviceReadBlockRAW__(Byte_t **data_) {

  FUNCTION_ENTER;

  Size_t totalSize = (Size_t) state.blockSize * state.currentBlockCount;

  Byte_t *buffer = null;

  BlockIORequest_t *request = null;

  Size_t configSize = 0x0u;

  if(OK(__PrepareBlockIORequest__(BLOCK_IO_OP_READ, &request, &configSize))) {

    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &configSize, (Addr_t *) request))) {

      if(OK(__DeviceRead__(state.ioDriverUID, &totalSize, (Addr_t **) &buffer))) {

        *data_ = buffer;

        __KernelFreeMemory__(request);

        __ReturnOk__();

      } else {

        __KernelFreeMemory__(request);

        __AssertOnElse__();

      }

    } else {

      __KernelFreeMemory__(request);

      __AssertOnElse__();

    }

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}


/**
 * @brief Raw block write operation
 * @details Internal helper that performs a raw block write to the current block
 * position.
 *
 * @param[in] data_ Pointer to data to write
 *
 * @return          ReturnOK if block was written successfully
 * @return          ReturnError if write operation failed
 */
static Return_t __BlockDeviceWriteBlockRAW__(const Byte_t *data_) {

  FUNCTION_ENTER;

  Size_t totalSize = (Size_t) state.blockSize * state.currentBlockCount;

  BlockIORequest_t *request = null;

  Size_t configSize = 0x0u;

  if(OK(__PrepareBlockIORequest__(BLOCK_IO_OP_WRITE, &request, &configSize))) {

    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &configSize, (Addr_t *) request))) {

      if(OK(__DeviceWrite__(state.ioDriverUID, &totalSize, (Addr_t *) data_))) {

        __KernelFreeMemory__(request);

        __ReturnOk__();

      } else {

        __KernelFreeMemory__(request);

        __AssertOnElse__();

      }

    } else {

      __KernelFreeMemory__(request);

      __AssertOnElse__();

    }

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}


#if defined(POSIX_ARCH_OTHER)
  void __BlockDeviceStateClear__(void) {

    state.ioDriverUID = 0x0u;

    state.protocol = 0x0u;

    state.blockSize = BLOCK_DEFAULT_SECTOR_SIZE;

    state.totalBlocks = 0x0u;

    state.initialized = false;

    state.currentBlockNumber = 0x0u;

    state.currentBlockCount = 0x0u;

    state.currentTransferMode = BLOCK_IO_MODE_BLOCKING;

    return;

  }


#endif /* if defined(POSIX_ARCH_OTHER) */