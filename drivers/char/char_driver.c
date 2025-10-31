/*UNCRUSTIFY-OFF*/
/**
 * @file char_driver.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Character device driver implementation
 * @details
 * Implements character device operations including buffered read/write, line modes (raw/cooked), and protocol abstraction for serial communications.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#include "char_driver.h"

/**
 * @brief Character device internal state structure
 * @details Maintains the runtime state of a character device including buffers and communication parameters.
 */
typedef struct CharDeviceState_s {

  HalfWord_t ioDriverUID;           /**< UID of underlying I/O driver device */

  Byte_t protocol;                  /**< Communication protocol (UART, USART, USB CDC) */

  Byte_t lineMode;                  /**< Line mode (raw or cooked) */

  Word_t baudRate;                  /**< Baud rate for serial communication */

  Base_t initialized;               /**< Initialization flag */

  HalfWord_t rxBufferSize;          /**< Size of receive buffer in bytes */

  HalfWord_t txBufferSize;          /**< Size of transmit buffer in bytes */

  Byte_t *rxBuffer;                 /**< Pointer to receive circular buffer */

  Byte_t *txBuffer;                 /**< Pointer to transmit circular buffer */

  HalfWord_t rxHead;                /**< Receive buffer head pointer */

  HalfWord_t rxTail;                /**< Receive buffer tail pointer */

  HalfWord_t txHead;                /**< Transmit buffer head pointer */

  HalfWord_t txTail;                /**< Transmit buffer tail pointer */

  HalfWord_t currentByteCount;      /**< Current transfer byte count */

  Byte_t currentTransferMode;       /**< Current transfer mode setting */

} CharDeviceState_t;

static CharDeviceState_t state = {

  0

};

/**
 * @brief Prepares a character I/O request structure
 * @details Internal helper to allocate and initialize a CharIORequest_t for I/O operations.
 *
 * @param[in] operation_ I/O operation type
 * @param[out] request_ Pointer to store allocated request structure
 * @param[out] configSize_ Pointer to store config size
 *
 * @return ReturnOK if request was prepared successfully
 * @return ReturnError if allocation failed or invalid parameters
 */
static Return_t __PrepareCharIORequest__(const Byte_t operation_, CharIORequest_t **request_, Size_t *configSize_);

/**
 * @brief Reads raw data from character device
 * @details Internal function to read bytes from the receive buffer without line processing.
 *
 * @param[out] data_ Pointer to store allocated data buffer
 * @param[out] bytesRead_ Pointer to store number of bytes read
 *
 * @return ReturnOK if read was successful
 * @return ReturnError if read failed or allocation failed
 */
static Return_t __CharDeviceReadRAW__(Byte_t **data_, Size_t *bytesRead_);

/**
 * @brief Writes raw data to character device
 * @details Internal function to write a single byte to the transmit buffer.
 *
 * @param[in] data_ Pointer to byte data
 *
 * @return ReturnOK if write was successful
 * @return ReturnError if buffer full or write failed
 */
static Return_t __CharDeviceWriteRAW__(const Byte_t *data_);

/**
 * @brief Calculates free space in circular buffer
 * @details Internal helper to determine available space for writing.
 *
 * @param[in] head_ Buffer head position
 * @param[in] tail_ Buffer tail position
 * @param[in] size_ Total buffer size
 *
 * @return Number of free bytes in the buffer
 */
static HalfWord_t __CircularBufferSpace__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);

/**
 * @brief Calculates available data in circular buffer
 * @details Internal helper to determine bytes available for reading.
 *
 * @param[in] head_ Buffer head position
 * @param[in] tail_ Buffer tail position
 * @param[in] size_ Total buffer size
 *
 * @return Number of bytes available to read
 */
static HalfWord_t __CircularBufferAvailable__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
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

  state.lineMode = CHAR_LINE_RAW;

  state.baudRate = 0x0u;

  state.rxBufferSize = 0x0u;

  state.txBufferSize = 0x0u;

  state.rxBuffer = null;

  state.txBuffer = null;

  state.rxHead = 0x0u;

  state.rxTail = 0x0u;

  state.txHead = 0x0u;

  state.txTail = 0x0u;

  state.currentByteCount = 0x0u;

  state.currentTransferMode = CHAR_IO_MODE_BLOCKING;

  __ReturnOk__();

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {

  FUNCTION_ENTER;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {

    Byte_t command = *(Byte_t *) config_;

    switch(command) {

    case CHAR_CMD_CONFIG:

      if(*size_ >= sizeof(CharDeviceConfig_t)) {

        CharDeviceConfig_t *cfg = (CharDeviceConfig_t *) config_;

        state.ioDriverUID = cfg->ioDriverUID;

        state.protocol = cfg->protocol;

        state.lineMode = cfg->lineMode;

        state.baudRate = cfg->baudRate;

        state.rxBufferSize = (0 == cfg->rxBufferSize) ? CHAR_DEFAULT_RX_BUFFER_SIZE : cfg->rxBufferSize;

        state.txBufferSize = (0 == cfg->txBufferSize) ? CHAR_DEFAULT_TX_BUFFER_SIZE : cfg->txBufferSize;

        if((CHAR_PROTOCOL_RAW == state.protocol) && (CHAR_LINE_RAW == state.lineMode)) {

          state.initialized = true;

          cfg->rxBufferSize = state.rxBufferSize;

          cfg->txBufferSize = state.txBufferSize;

          __ReturnOk__();

        } else if(CHAR_LINE_COOKED == state.lineMode) {

          if(OK(__KernelAllocateMemory__((volatile Addr_t **) &state.rxBuffer, state.rxBufferSize))) {

            if(OK(__KernelAllocateMemory__((volatile Addr_t **) &state.txBuffer, state.txBufferSize))) {

              state.rxHead = 0x0u;

              state.rxTail = 0x0u;

              state.txHead = 0x0u;

              state.txTail = 0x0u;

              state.initialized = true;

              cfg->rxBufferSize = state.rxBufferSize;

              cfg->txBufferSize = state.txBufferSize;

              __ReturnOk__();

            } else {

              __KernelFreeMemory__(state.rxBuffer);

              state.rxBuffer = null;

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

      break;

    case CHAR_CMD_SET_PARAMS:

      if(*size_ >= sizeof(CharDeviceCommand_t)) {

        CharDeviceCommand_t *cmd = (CharDeviceCommand_t *) config_;

        state.currentByteCount = cmd->byteCount;

        state.currentTransferMode = cmd->transferMode;

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

      break;

    case CHAR_CMD_GET_INFO:

      if(*size_ >= sizeof(CharDeviceInfo_t)) {

        CharDeviceInfo_t *info = (CharDeviceInfo_t *) config_;

        info->protocol = state.protocol;

        info->lineMode = state.lineMode;

        info->baudRate = state.baudRate;

        if(CHAR_LINE_COOKED == state.lineMode) {

          info->rxBytesAvailable = __CircularBufferAvailable__(state.rxHead, state.rxTail, state.rxBufferSize);

          info->txBytesFree = __CircularBufferSpace__(state.txHead, state.txTail, state.txBufferSize);

        } else {

          CharIOStatus_t ioStatus;

          Size_t statusSize = sizeof(CharIOStatus_t);

          ioStatus.command = CHAR_IO_CMD_GET_STATUS;

          if(OK(__DeviceConfigDevice__(state.ioDriverUID, &statusSize, (Addr_t *) &ioStatus))) {

            info->rxBytesAvailable = ioStatus.rxBytesAvailable;

            info->txBytesFree = ioStatus.txBytesFree;

          } else {

            info->rxBytesAvailable = 0x0u;

            info->txBytesFree = 0x0u;

          }

        }

        info->isInitialized = state.initialized;

        info->isConnected = true;

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

  Byte_t *charData = null;

  Size_t bytesRead = 0x0u;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    if(CHAR_PROTOCOL_RAW == state.protocol) {

      if(OK(__CharDeviceReadRAW__(&charData, &bytesRead))) {

        *data_ = charData;

        *size_ = bytesRead;

        __ReturnOk__();

      } else {

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

    if(CHAR_PROTOCOL_RAW == state.protocol) {

      if(OK(__CharDeviceWriteRAW__((Byte_t *) data_))) {

        __ReturnOk__();

      } else {

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

  if(__PointerIsNotNull__(data_) && state.initialized) {

    Byte_t *byteData = null;

    Size_t bytesRead = 0x0u;

    state.currentByteCount = 1;

    if(OK(__CharDeviceReadRAW__(&byteData, &bytesRead))) {

      if(bytesRead > 0) {

        *data_ = byteData[0];

        __KernelFreeMemory__(byteData);

        __ReturnOk__();

      } else {

        __KernelFreeMemory__(byteData);

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
Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_) {

  FUNCTION_ENTER;

  if(state.initialized) {

    state.currentByteCount = 1;

    if(OK(__CharDeviceWriteRAW__(&data_))) {

      __ReturnOk__();

    } else {

      __AssertOnElse__();

    }

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
static Return_t __PrepareCharIORequest__(const Byte_t operation_,

  CharIORequest_t **request_,

  Size_t *configSize_) {

  FUNCTION_ENTER;

  Byte_t *ioConfig = null;

  if(OK(__KernelAllocateMemory__((volatile Addr_t **) &ioConfig, sizeof(CharIORequest_t)))) {

    CharIORequest_t *request = (CharIORequest_t *) ioConfig;

    request->command = CHAR_IO_CMD_SET_REQUEST;

    request->operation = operation_;

    request->byteCount = state.currentByteCount;

    request->transferMode = state.currentTransferMode;

    request->timeoutMs = 1000;

    *request_ = request;

    *configSize_ = sizeof(CharIORequest_t);

    __ReturnOk__();

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}


static Return_t __CharDeviceReadRAW__(Byte_t **data_,

  Size_t *bytesRead_) {

  FUNCTION_ENTER;

  Size_t requestSize = (Size_t) state.currentByteCount;

  Byte_t *buffer = null;

  CharIORequest_t *request = null;

  Size_t configSize = 0x0u;

  if(OK(__PrepareCharIORequest__(CHAR_IO_OP_READ, &request, &configSize))) {

    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &configSize, (Addr_t *) request))) {

      if(OK(__DeviceRead__(state.ioDriverUID, &requestSize, (Addr_t **) &buffer))) {

        *data_ = buffer;

        *bytesRead_ = requestSize;

        __KernelFreeMemory__(request);

        __ReturnOk__();

      } else {

        __KernelFreeMemory__(request);

      }

    } else {

      __KernelFreeMemory__(request);

    }

  } else {

  }

  FUNCTION_EXIT;

}


static Return_t __CharDeviceWriteRAW__(const Byte_t *data_) {

  FUNCTION_ENTER;

  Size_t writeSize = (Size_t) state.currentByteCount;

  CharIORequest_t *request = null;

  Size_t configSize = 0x0u;

  if(OK(__PrepareCharIORequest__(CHAR_IO_OP_WRITE, &request, &configSize))) {

    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &configSize, (Addr_t *) request))) {

      if(OK(__DeviceWrite__(state.ioDriverUID, &writeSize, (Addr_t *) data_))) {

        __KernelFreeMemory__(request);

        __ReturnOk__();

      } else {

        __KernelFreeMemory__(request);

      }

    } else {

      __KernelFreeMemory__(request);

    }

  } else {

  }

  FUNCTION_EXIT;

}


static HalfWord_t __CircularBufferSpace__(const HalfWord_t head_,

  const HalfWord_t tail_,

  const HalfWord_t size_) {

  if(head_ >= tail_) {

    return (size_ - (head_ - tail_) - 1);

  } else {

    return (tail_ - head_ - 1);

  }

}


static HalfWord_t __CircularBufferAvailable__(const HalfWord_t head_,

  const HalfWord_t tail_,

  const HalfWord_t size_) {

  if(head_ >= tail_) {

    return (head_ - tail_);

  } else {

    return (size_ - (tail_ - head_));

  }

}


#if defined(POSIX_ARCH_OTHER)
  void __CharDeviceStateClear__(void) {

    if(null != state.rxBuffer) {

      __KernelFreeMemory__(state.rxBuffer);

    }

    if(null != state.txBuffer) {

      __KernelFreeMemory__(state.txBuffer);

    }

    state.ioDriverUID = 0x0u;

    state.protocol = 0x0u;

    state.lineMode = CHAR_LINE_RAW;

    state.baudRate = 0x0u;

    state.initialized = false;

    state.rxBufferSize = 0x0u;

    state.txBufferSize = 0x0u;

    state.rxBuffer = null;

    state.txBuffer = null;

    state.rxHead = 0x0u;

    state.rxTail = 0x0u;

    state.txHead = 0x0u;

    state.txTail = 0x0u;

    state.currentByteCount = 0x0u;

    state.currentTransferMode = CHAR_IO_MODE_BLOCKING;

    return;

  }


#endif /* if defined(POSIX_ARCH_OTHER) */