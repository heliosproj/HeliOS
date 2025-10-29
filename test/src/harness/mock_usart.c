/*UNCRUSTIFY-OFF*/
/**
 * @file mock_usart.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Mock USART device for console unit testing
 * @version 0.5.0
 * @date 2025-01-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "mock_usart.h"


/* Device state */
static MockUSARTState_t state;
/* Forward declarations */
static HalfWord_t __GetRxAvailable__(void);
static HalfWord_t __GetTxFree__(void);
static void __RxWrite__(Byte_t byte_);
static Base_t __RxRead__(Byte_t *byte_);
static void __TxWrite__(Byte_t byte_);
static Base_t __TxRead__(Byte_t *byte_);


/**
 * @brief Register mock USART device
 * @return Return_t OK or error
 */
Return_t MOCKUSRT_self_register(void) {
  FUNCTION_ENTER;

  if(OK(__RegisterDevice__(MOCK_USART_DEVICE_UID, (Byte_t *) MOCK_USART_DEVICE_NAME, DeviceStateRunning, DeviceModeReadWrite, MOCKUSRT_init, MOCKUSRT_config,
    MOCKUSRT_read, MOCKUSRT_write, MOCKUSRT_simple_read, MOCKUSRT_simple_write))) {
    __ReturnOk__();
  } else {
    /* Return error by default */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Initialize mock USART device
 * @param  device_ Device structure
 * @return         Return_t OK
 */
Return_t MOCKUSRT_init(Device_t *device_) {
  FUNCTION_ENTER;
  (void) device_;


  /* Initialize state */
  __memset__(&state, 0x00u, sizeof(MockUSARTState_t));
  state.rxHead = 0x0u;
  state.rxTail = 0x0u;
  state.txHead = 0x0u;
  state.txTail = 0x0u;
  state.initialized = true;


  /* Initialize current request to defaults */
  state.currentRequest.command = CHAR_IO_CMD_SET_REQUEST;
  state.currentRequest.operation = CHAR_IO_OP_READ;
  state.currentRequest.byteCount = 0x0u;
  state.currentRequest.transferMode = CHAR_IO_MODE_BLOCKING;
  state.currentRequest.timeoutMs = 0x0u;
  __ReturnOk__();
  FUNCTION_EXIT;
}


/**
 * @brief Configure mock USART device
 * @param  device_ Device structure
 * @param  size_   Size of configuration data
 * @param  config_ Configuration data
 * @return         Return_t OK or error
 */
Return_t MOCKUSRT_config(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;


  Byte_t *cmd = null;


  (void) device_;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_) && (*size_ > 0x0u)) {
    cmd = (Byte_t *) config_;

    switch(*cmd) {
    case CHAR_IO_CMD_SET_REQUEST: {
      CharIORequest_t *request = (CharIORequest_t *) config_;


      if(*size_ >= sizeof(CharIORequest_t)) {
        /* Store request for next read/write operation */
        __memcpy__(&state.currentRequest, request, sizeof(CharIORequest_t));
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }

      break;
    } case CHAR_IO_CMD_GET_INFO: {
      CharIOInfo_t *info = (CharIOInfo_t *) config_;


      if(*size_ >= sizeof(CharIOInfo_t)) {
        /* Return device capabilities */
        info->command = CHAR_IO_CMD_GET_INFO;
        info->rxBufferSize = MOCK_USART_RX_BUFFER_SIZE;
        info->txBufferSize = MOCK_USART_TX_BUFFER_SIZE;
        info->supportsDMA = false;
        info->supportsInterrupt = false;
        info->isFullDuplex = true;
        info->maxBaudRate = 115200u;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }

      break;
    } case CHAR_IO_CMD_GET_STATUS: {
      CharIOStatus_t *status = (CharIOStatus_t *) config_;


      if(*size_ >= sizeof(CharIOStatus_t)) {
        /* Return current status */
        status->command = CHAR_IO_CMD_GET_STATUS;
        status->rxBytesAvailable = __GetRxAvailable__();
        status->txBytesFree = __GetTxFree__();
        status->isTransmitting = false;
        status->isReceiving = false;
        status->errorFlags = 0x00u;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }

      break;
    } case CHAR_IO_CMD_SET_BAUDRATE: {
      /* Accept but ignore baud rate changes */
      __ReturnOk__();
      break;
    } case CHAR_IO_CMD_SET_PARAMS: {
      /* Accept but ignore UART parameter changes */
      __ReturnOk__();
      break;
    } default: __AssertOnElse__();
      break;
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Read data from mock USART device
 * @param  device_ Device structure
 * @param  size_   Requested bytes (in), actual bytes (out)
 * @param  data_   Pointer to receive allocated buffer
 * @return         Return_t OK or error
 */
Return_t MOCKUSRT_read(Device_t *device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;


  Byte_t *buffer = null;
  HalfWord_t bytesToRead = 0x0u;
  HalfWord_t available = 0x0u;
  HalfWord_t i = 0x0u;
  Byte_t byte = 0x00u;


  (void) device_;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_)) {
    /* Determine how many bytes to read */
    available = __GetRxAvailable__();

    if(0x0u == available) {
      /* No data available */
      *size_ = 0x0u;
      *data_ = null;


      /* Return error by default */
      FUNCTION_EXIT;
    }

    /* Read up to requested amount or what's available */
    bytesToRead = (*size_ < available) ? (HalfWord_t) *size_ : available;

    /* Allocate kernel memory for return data */
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) &buffer, bytesToRead))) {
      /* Copy data from RX buffer */
      for(i = 0x0u; i < bytesToRead; i++) {
        if(__RxRead__(&byte)) {
          buffer[i] = byte;
        } else {
          break;
        }
      }

      *data_ = (Addr_t *) buffer;
      *size_ = bytesToRead;


      /* Update device availability */
      device_->available = (__GetRxAvailable__() > 0x0u) ? true : false;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Write data to mock USART device
 * @param  device_ Device structure
 * @param  size_   Bytes to write (in), actual bytes written (out)
 * @param  data_   Data buffer
 * @return         Return_t OK or error
 */
Return_t MOCKUSRT_write(Device_t *device_, Size_t *size_, Addr_t *data_) {
  FUNCTION_ENTER;


  Byte_t *buffer = null;
  HalfWord_t bytesToWrite = 0x0u;
  HalfWord_t freeSpace = 0x0u;
  HalfWord_t i = 0x0u;


  (void) device_;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && (*size_ > 0x0u)) {
    buffer = (Byte_t *) data_;
    freeSpace = __GetTxFree__();

    if(0x0u == freeSpace) {
      /* No space available */
      *size_ = 0x0u;


      /* Return error by default */
      FUNCTION_EXIT;
    }

    /* Write up to requested amount or available space */
    bytesToWrite = (*size_ < freeSpace) ? (HalfWord_t) *size_ : freeSpace;

    /* Copy data to TX buffer */
    for(i = 0x0u; i < bytesToWrite; i++) {
      __TxWrite__(buffer[i]);
    }

    *size_ = bytesToWrite;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Read single byte from mock USART device
 * @param  device_ Device structure
 * @param  data_   Pointer to receive byte
 * @return         Return_t OK or error
 */
Return_t MOCKUSRT_simple_read(Device_t *device_, Byte_t *data_) {
  FUNCTION_ENTER;
  (void) device_;

  if(__PointerIsNotNull__(data_)) {
    if(__RxRead__(data_)) {
      /* Update device availability */
      device_->available = (__GetRxAvailable__() > 0x0u) ? true : false;
      __ReturnOk__();
    } else {
      /* Return error by default */
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Write single byte to mock USART device
 * @param  device_ Device structure
 * @param  data_   Byte to write
 * @return         Return_t OK or error
 */
Return_t MOCKUSRT_simple_write(Device_t *device_, Byte_t data_) {
  FUNCTION_ENTER;
  (void) device_;

  if(__GetTxFree__() > 0x0u) {
    __TxWrite__(data_);
    __ReturnOk__();
  } else {
    /* Return error by default */
  }

  FUNCTION_EXIT;
}


/**
 * @brief Get number of bytes available in RX buffer
 * @return HalfWord_t Bytes available
 */
static HalfWord_t __GetRxAvailable__(void) {
  if(state.rxHead >= state.rxTail) {
    return(state.rxHead - state.rxTail);
  } else {
    return(MOCK_USART_RX_BUFFER_SIZE - (state.rxTail - state.rxHead));
  }
}


/**
 * @brief Get number of free bytes in TX buffer
 * @return HalfWord_t Free bytes
 */
static HalfWord_t __GetTxFree__(void) {
  HalfWord_t used = 0x0u;


  if(state.txHead >= state.txTail) {
    used = state.txHead - state.txTail;
  } else {
    used = MOCK_USART_TX_BUFFER_SIZE - (state.txTail - state.txHead);
  }

  return(MOCK_USART_TX_BUFFER_SIZE - used - 0x1u);
}


/**
 * @brief Write byte to RX buffer (for test injection)
 * @param byte_ Byte to write
 */
static void __RxWrite__(Byte_t byte_) {
  HalfWord_t nextHead = (state.rxHead + 0x1u) % MOCK_USART_RX_BUFFER_SIZE;


  if(nextHead != state.rxTail) {
    state.rxBuffer[state.rxHead] = byte_;
    state.rxHead = nextHead;
  }
}


/**
 * @brief Read byte from RX buffer
 * @param  byte_ Pointer to receive byte
 * @return       Base_t true if byte read, false if empty
 */
static Base_t __RxRead__(Byte_t *byte_) {
  if(state.rxHead != state.rxTail) {
    *byte_ = state.rxBuffer[state.rxTail];
    state.rxTail = (state.rxTail + 0x1u) % MOCK_USART_RX_BUFFER_SIZE;

    return(true);
  }

  return(false);
}


/**
 * @brief Write byte to TX buffer
 * @param byte_ Byte to write
 */
static void __TxWrite__(Byte_t byte_) {
  HalfWord_t nextHead = (state.txHead + 0x1u) % MOCK_USART_TX_BUFFER_SIZE;


  if(nextHead != state.txTail) {
    state.txBuffer[state.txHead] = byte_;
    state.txHead = nextHead;
  }
}


/**
 * @brief Read byte from TX buffer (for test verification)
 * @param  byte_ Pointer to receive byte
 * @return       Base_t true if byte read, false if empty
 */
static Base_t __TxRead__(Byte_t *byte_) {
  if(state.txHead != state.txTail) {
    *byte_ = state.txBuffer[state.txTail];
    state.txTail = (state.txTail + 0x1u) % MOCK_USART_TX_BUFFER_SIZE;

    return(true);
  }

  return(false);
}


/* ============================================================================
 * TEST HELPER FUNCTIONS
 * ============================================================================
 */


/**
 * @brief Inject input into RX buffer for console to read
 * @param input_ Null-terminated string to inject
 */
void xMockUSARTInjectInput(const Byte_t *input_) {
  HalfWord_t i = 0x0u;


  if(__PointerIsNotNull__(input_)) {
    while(0x00u != input_[i]) {
      __RxWrite__(input_[i]);
      i++;
    }
  }
}


/**
 * @brief Get output from TX buffer (console output)
 * @param buffer_    Buffer to receive output
 * @param maxLen_    Maximum bytes to read
 * @param actualLen_ Actual bytes read
 */
void xMockUSARTGetOutput(Byte_t *buffer_, HalfWord_t maxLen_, HalfWord_t *actualLen_) {
  HalfWord_t i = 0x0u;
  Byte_t byte = 0x00u;


  if(__PointerIsNotNull__(buffer_) && __PointerIsNotNull__(actualLen_)) {
    while((i < maxLen_) && __TxRead__(&byte)) {
      buffer_[i] = byte;
      i++;
    }

    *actualLen_ = i;
  }
}


/**
 * @brief Reset both RX and TX buffers
 */
void xMockUSARTReset(void) {
  state.rxHead = 0x0u;
  state.rxTail = 0x0u;
  state.txHead = 0x0u;
  state.txTail = 0x0u;
  __memset__(state.rxBuffer, 0x00u, MOCK_USART_RX_BUFFER_SIZE);
  __memset__(state.txBuffer, 0x00u, MOCK_USART_TX_BUFFER_SIZE);
}


/**
 * @brief Get number of bytes available in RX buffer
 * @return HalfWord_t Bytes available
 */
HalfWord_t xMockUSARTGetRxAvailable(void) {
  return(__GetRxAvailable__());
}


/**
 * @brief Get number of bytes in TX buffer
 * @return HalfWord_t Bytes in TX buffer
 */
HalfWord_t xMockUSARTGetTxCount(void) {
  HalfWord_t count = 0x0u;


  if(state.txHead >= state.txTail) {
    count = state.txHead - state.txTail;
  } else {
    count = MOCK_USART_TX_BUFFER_SIZE - (state.txTail - state.txHead);
  }

  return(count);
}


/**
 * @brief Clear TX buffer only (keep RX intact)
 */
void xMockUSARTClearOutput(void) {
  state.txHead = 0x0u;
  state.txTail = 0x0u;
  __memset__(state.txBuffer, 0x00u, MOCK_USART_TX_BUFFER_SIZE);
}