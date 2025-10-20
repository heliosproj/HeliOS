/*UNCRUSTIFY-OFF*/
/**
 * @file char_driver.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Generic character device driver implementation
 * @version 0.5.0
 * @date 2025-10-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "char_driver.h"


/* Driver state - NO hardware-specific fields */
typedef struct CharDeviceState_s {
  HalfWord_t ioDriverUID;           /* I/O driver to use for communication */
  Byte_t protocol;                  /* Character protocol (UART/USART/USB/RAW) */
  Byte_t lineMode;                  /* Line discipline mode */
  Word_t baudRate;                  /* Current baud rate */
  Base_t initialized;               /* Initialization flag */
  HalfWord_t rxBufferSize;          /* RX buffer size */
  HalfWord_t txBufferSize;          /* TX buffer size */
  Byte_t *rxBuffer;                 /* Receive buffer (for cooked mode) */
  Byte_t *txBuffer;                 /* Transmit buffer (for cooked mode) */
  HalfWord_t rxHead;                /* RX buffer head index */
  HalfWord_t rxTail;                /* RX buffer tail index */
  HalfWord_t txHead;                /* TX buffer head index */
  HalfWord_t txTail;                /* TX buffer tail index */
} CharDeviceState_t;



static CharDeviceState_t state = {
  0
};
/* Forward declarations */
static Return_t __PrepareCharIORequest__(const HalfWord_t byteCount_, const Byte_t operation_, const Byte_t transferMode_, CharIORequest_t **request_, Size_t *
  configSize_);
static Return_t __CharDeviceReadRAW__(const HalfWord_t byteCount_, Byte_t **data_, Size_t *bytesRead_);
static Return_t __CharDeviceWriteRAW__(const HalfWord_t byteCount_, const Byte_t *data_);
static HalfWord_t __CircularBufferSpace__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
static HalfWord_t __CircularBufferAvailable__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);


/*UNCRUSTIFY-OFF*/
Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void) {
  FUNCTION_ENTER;

  if(OK(__RegisterDevice__(DEVICE_UID,
                          (Byte_t *)TO_LITERAL(DEVICE_NAME),
                          DEVICE_STATE,
                          DEVICE_MODE,
                          CHARDEV0_init,
                          CHARDEV0_config,
                          CHARDEV0_read,
                          CHARDEV0_write,
                          CHARDEV0_simple_read,
                          CHARDEV0_simple_write))) {
    __ReturnOk__();
  } else {
    __ReturnError__();
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
  state.lineMode = CHAR_LINE_RAW;
  state.baudRate = 0;
  state.rxBufferSize = 0;
  state.txBufferSize = 0;
  state.rxBuffer = null;
  state.txBuffer = null;
  state.rxHead = 0;
  state.rxTail = 0;
  state.txHead = 0;
  state.txTail = 0;

  __ReturnOk__();
  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {

    /* Initial configuration - receive CharDeviceConfig_t */
    if(*size_ >= sizeof(CharDeviceConfig_t)) {
      CharDeviceConfig_t *cfg = (CharDeviceConfig_t *)config_;

      /* Store I/O driver UID and protocol type */
      state.ioDriverUID = cfg->ioDriverUID;
      state.protocol = cfg->protocol;
      state.lineMode = cfg->lineMode;
      state.baudRate = cfg->baudRate;

      /* Set buffer sizes (use defaults if not specified) */
      state.rxBufferSize = (0 == cfg->rxBufferSize) ? CHAR_DEFAULT_RX_BUFFER_SIZE : cfg->rxBufferSize;
      state.txBufferSize = (0 == cfg->txBufferSize) ? CHAR_DEFAULT_TX_BUFFER_SIZE : cfg->txBufferSize;

      /* For RAW protocol with RAW line mode, no buffering needed */
      if((CHAR_PROTOCOL_RAW == state.protocol) && (CHAR_LINE_RAW == state.lineMode)) {
        state.initialized = true;

        /* Return configured values back to caller */
        cfg->rxBufferSize = state.rxBufferSize;
        cfg->txBufferSize = state.txBufferSize;

        __ReturnOk__();
      }
      /* For cooked mode, allocate buffers */
      else if(CHAR_LINE_COOKED == state.lineMode) {
        /* Allocate RX buffer */
        if(OK(__KernelAllocateMemory__((volatile Addr_t **)&state.rxBuffer, state.rxBufferSize))) {
          /* Allocate TX buffer */
          if(OK(__KernelAllocateMemory__((volatile Addr_t **)&state.txBuffer, state.txBufferSize))) {
            state.rxHead = 0;
            state.rxTail = 0;
            state.txHead = 0;
            state.txTail = 0;
            state.initialized = true;

            /* Return configured values back to caller */
            cfg->rxBufferSize = state.rxBufferSize;
            cfg->txBufferSize = state.txBufferSize;

            __ReturnOk__();
          } else {
            /* Failed to allocate TX buffer, free RX buffer */
            __KernelFreeMemory__(state.rxBuffer);
            state.rxBuffer = null;
            __ReturnError__();
            __AssertOnElse__();
          }
        } else {
          __ReturnError__();
          __AssertOnElse__();
        }
      }
      /* Other protocols or modes not implemented yet */
      else {
        /* Not implemented yet */
        __ReturnError__();
        __AssertOnElse__();
      }
    }

    /* Get device info */
    else if(*size_ >= sizeof(CharDeviceInfo_t)) {
      CharDeviceInfo_t *info = (CharDeviceInfo_t *)config_;

      info->protocol = state.protocol;
      info->lineMode = state.lineMode;
      info->baudRate = state.baudRate;

      /* Calculate available bytes in buffers */
      if(CHAR_LINE_COOKED == state.lineMode) {
        info->rxBytesAvailable = __CircularBufferAvailable__(state.rxHead, state.rxTail, state.rxBufferSize);
        info->txBytesFree = __CircularBufferSpace__(state.txHead, state.txTail, state.txBufferSize);
      } else {
        /* For RAW mode, query I/O driver status */
        CharIOStatus_t ioStatus;
        Size_t statusSize = sizeof(CharIOStatus_t);
        ioStatus.command = CHAR_IO_CMD_GET_STATUS;

        if(OK(__DeviceConfigDevice__(state.ioDriverUID, &statusSize, (Addr_t *)&ioStatus))) {
          info->rxBytesAvailable = ioStatus.rxBytesAvailable;
          info->txBytesFree = ioStatus.txBytesFree;
        } else {
          info->rxBytesAvailable = 0;
          info->txBytesFree = 0;
        }
      }

      info->isInitialized = state.initialized;
      info->isConnected = true; /* TODO: Implement proper connection detection */

      __ReturnOk__();
    } else {
      __ReturnError__();
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;

  Byte_t *charData = null;
  Size_t bytesRead = 0;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    /* Dispatch to protocol-specific read function */
    if(CHAR_PROTOCOL_RAW == state.protocol) {
      if(OK(__CharDeviceReadRAW__((HalfWord_t)*size_, &charData, &bytesRead))) {
        *data_ = charData;
        *size_ = bytesRead;
        __ReturnOk__();
      } else {
        __ReturnError__();
        __AssertOnElse__();
      }
    } else {
      /* UART/USART/USB protocols not implemented yet */
      __ReturnError__();
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    /* Dispatch to protocol-specific write function */
    if(CHAR_PROTOCOL_RAW == state.protocol) {
      if(OK(__CharDeviceWriteRAW__((HalfWord_t)*size_, (Byte_t *)data_))) {
        __ReturnOk__();
      } else {
        __ReturnError__();
        __AssertOnElse__();
      }
    } else {
      /* UART/USART/USB protocols not implemented yet */
      __ReturnError__();
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(data_) && state.initialized) {
    Byte_t *byteData = null;
    Size_t bytesRead = 0;

    /* Read single byte */
    if(OK(__CharDeviceReadRAW__(1, &byteData, &bytesRead))) {
      if(bytesRead > 0) {
        *data_ = byteData[0];
        __KernelFreeMemory__(byteData);
        __ReturnOk__();
      } else {
        __KernelFreeMemory__(byteData);
        __ReturnError__();
        __AssertOnElse__();
      }
    } else {
      __ReturnError__();
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_) {
  FUNCTION_ENTER;

  if(state.initialized) {
    /* Write single byte - perfect for printf integration */
    if(OK(__CharDeviceWriteRAW__(1, &data_))) {
      __ReturnOk__();
    } else {
      __ReturnError__();
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* ============================================================================
 * Protocol-Specific Implementation: RAW (Direct I/O)
 * ========================================================================== */

/* Helper function to prepare generic character I/O request */
static Return_t __PrepareCharIORequest__(const HalfWord_t byteCount_,
                                        const Byte_t operation_,
                                        const Byte_t transferMode_,
                                        CharIORequest_t **request_,
                                        Size_t *configSize_) {
  FUNCTION_ENTER;

  Byte_t *ioConfig = null;

  /* Allocate generic character I/O request from kernel heap */
  if(OK(__KernelAllocateMemory__((volatile Addr_t **)&ioConfig, sizeof(CharIORequest_t)))) {
    CharIORequest_t *request = (CharIORequest_t *)ioConfig;

    /* Fill generic request structure */
    request->command = CHAR_IO_CMD_SET_REQUEST;
    request->operation = operation_;
    request->byteCount = byteCount_;
    request->transferMode = transferMode_;
    request->timeoutMs = 1000; /* Default 1 second timeout */

    *request_ = request;
    *configSize_ = sizeof(CharIORequest_t);

    __ReturnOk__();
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __CharDeviceReadRAW__(const HalfWord_t byteCount_,
                                     Byte_t **data_,
                                     Size_t *bytesRead_) {
  FUNCTION_ENTER;

  Size_t requestSize = (Size_t)byteCount_;
  Byte_t *buffer = null;
  CharIORequest_t *request = null;
  Size_t configSize = 0;

  /* Prepare character I/O request */
  if(OK(__PrepareCharIORequest__(byteCount_, CHAR_IO_OP_READ, CHAR_IO_MODE_BLOCKING, &request, &configSize))) {

    /* Send request to I/O driver - it handles translation to native format */
    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &configSize, (Addr_t *)request))) {

      /* Read data from I/O driver using kernel-level API (returns kernel memory) */
      if(OK(__DeviceRead__(state.ioDriverUID, &requestSize, (Addr_t **)&buffer))) {
        *data_ = buffer;
        *bytesRead_ = requestSize;
        __KernelFreeMemory__(request);
        __ReturnOk__();
      } else {
        __KernelFreeMemory__(request);
        __ReturnError__();
        __AssertOnElse__();
      }
    } else {
      __KernelFreeMemory__(request);
      __ReturnError__();
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __CharDeviceWriteRAW__(const HalfWord_t byteCount_,
                                      const Byte_t *data_) {
  FUNCTION_ENTER;

  Size_t writeSize = (Size_t)byteCount_;
  CharIORequest_t *request = null;
  Size_t configSize = 0;

  /* Prepare character I/O request */
  if(OK(__PrepareCharIORequest__(byteCount_, CHAR_IO_OP_WRITE, CHAR_IO_MODE_BLOCKING, &request, &configSize))) {

    /* Send request to I/O driver - it handles translation to native format */
    if(OK(__DeviceConfigDevice__(state.ioDriverUID, &configSize, (Addr_t *)request))) {

      /* Write data to I/O driver using kernel-level API (data already in kernel memory) */
      if(OK(__DeviceWrite__(state.ioDriverUID, &writeSize, (Addr_t *)data_))) {
        __KernelFreeMemory__(request);
        __ReturnOk__();
      } else {
        __KernelFreeMemory__(request);
        __ReturnError__();
        __AssertOnElse__();
      }
    } else {
      __KernelFreeMemory__(request);
      __ReturnError__();
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* ============================================================================
 * Circular Buffer Utilities (for cooked mode)
 * ========================================================================== */

/* Calculate free space in circular buffer */
static HalfWord_t __CircularBufferSpace__(const HalfWord_t head_,
                                         const HalfWord_t tail_,
                                         const HalfWord_t size_) {
  if(head_ >= tail_) {
    return (size_ - (head_ - tail_) - 1);
  } else {
    return (tail_ - head_ - 1);
  }
}


/* Calculate available data in circular buffer */
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


/* For unit testing only! */
void __CharDeviceStateClear__(void) {
  /* Free buffers if allocated */
  if(null != state.rxBuffer) {
    __KernelFreeMemory__(state.rxBuffer);
  }

  if(null != state.txBuffer) {
    __KernelFreeMemory__(state.txBuffer);
  }

  state.ioDriverUID = 0;
  state.protocol = 0;
  state.lineMode = CHAR_LINE_RAW;
  state.baudRate = 0;
  state.initialized = false;
  state.rxBufferSize = 0;
  state.txBufferSize = 0;
  state.rxBuffer = null;
  state.txBuffer = null;
  state.rxHead = 0;
  state.rxTail = 0;
  state.txHead = 0;
  state.txTail = 0;

  return;
}


#endif /* if defined(POSIX_ARCH_OTHER) */
/*UNCRUSTIFY-ON*/
