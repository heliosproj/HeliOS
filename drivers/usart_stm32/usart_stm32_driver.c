/*UNCRUSTIFY-OFF*/
/**
 * @file usart_stm32_driver.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief STM32 USART HAL I/O driver implementation
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
#include "usart_stm32_driver.h"

#if !defined(POSIX_ARCH_OTHER)
  #include <unistd.h> /* For POSIX write() in simulation mode */
#endif /* if !defined(POSIX_ARCH_OTHER) */
/* Driver state */
typedef struct USARTDriverState_s {
#if !defined(POSIX_ARCH_OTHER)
    UART_HandleTypeDef huart; /* STM32 HAL UART handle */



#else  /* if !defined(POSIX_ARCH_OTHER) */
    void *huart; /* Placeholder for testing */



#endif /* if !defined(POSIX_ARCH_OTHER) */
  Byte_t rxBuffer[USART_RX_BUFFER_SIZE]; /* Circular RX buffer */
  Byte_t txBuffer[USART_TX_BUFFER_SIZE]; /* Circular TX buffer */
  volatile HalfWord_t rxHead; /* RX buffer write position */
  volatile HalfWord_t rxTail; /* RX buffer read position */
  volatile HalfWord_t txHead; /* TX buffer write position */
  volatile HalfWord_t txTail; /* TX buffer read position */
  volatile Base_t txBusy; /* TX operation in progress */
  volatile Base_t rxBusy; /* RX operation in progress */
  volatile Byte_t errorFlags; /* Accumulated error flags */
  CharIORequest_t currentRequest; /* Current I/O request */
  Base_t initialized; /* Initialization flag */
  Byte_t rxSingleByte; /* Single byte for interrupt RX */
} USARTDriverState_t;



static USARTDriverState_t state = {
  0
};


/* Forward declarations */
static HalfWord_t __CircularBufferSpace__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
static HalfWord_t __CircularBufferAvailable__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
static void __CircularBufferPut__(Byte_t *buffer_, HalfWord_t *head_, const HalfWord_t size_, const Byte_t data_);
static Byte_t __CircularBufferGet__(const Byte_t *buffer_, HalfWord_t *tail_, const HalfWord_t size_);

#if !defined(POSIX_ARCH_OTHER)
  static Return_t __TranslateHALToParity__(const Byte_t parity_, Word_t *halParity_);
  static Return_t __TranslateHALToStopBits__(const Byte_t stopBits_, Word_t *halStopBits_);
  static Return_t __TranslateHALToWordLength__(const Byte_t dataBits_, Word_t *halWordLength_);
#endif /* if !defined(POSIX_ARCH_OTHER) */
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
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_) {
  FUNCTION_ENTER;

  /* Initialize state */
  state.rxHead = 0;
  state.rxTail = 0;
  state.txHead = 0;
  state.txTail = 0;
  state.txBusy = false;
  state.rxBusy = false;
  state.errorFlags = USART_ERROR_NONE;
  state.initialized = false;
  state.rxSingleByte = 0;

  /* Clear current request */
  state.currentRequest.command = 0;
  state.currentRequest.operation = 0;
  state.currentRequest.byteCount = 0;
  state.currentRequest.transferMode = CHAR_IO_MODE_BLOCKING;
  state.currentRequest.timeoutMs = 0;

  /* Hardware initialization would happen here with USARTSTMInitConfig_t */
  /* For now, mark as initialized - actual HAL init happens in config */
  state.initialized = true;

  __ReturnOk__();
  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {

    /* Handle CharIORequest - set current I/O operation parameters */
    if(*size_ >= sizeof(CharIORequest_t)) {
      CharIORequest_t *req = (CharIORequest_t *)config_;

      if(CHAR_IO_CMD_SET_REQUEST == req->command) {
        /* Store the request for subsequent read/write operations */
        state.currentRequest.command = req->command;
        state.currentRequest.operation = req->operation;
        state.currentRequest.byteCount = req->byteCount;
        state.currentRequest.transferMode = req->transferMode;
        state.currentRequest.timeoutMs = req->timeoutMs;

        __ReturnOk__();
      } else {
        __ReturnError__();
        __AssertOnElse__();
      }
    }

    /* Handle CharIOInfo - return driver capabilities */
    else if(*size_ >= sizeof(CharIOInfo_t)) {
      CharIOInfo_t *info = (CharIOInfo_t *)config_;

      if(CHAR_IO_CMD_GET_INFO == info->command) {
        info->rxBufferSize = USART_RX_BUFFER_SIZE;
        info->txBufferSize = USART_TX_BUFFER_SIZE;
        info->supportsDMA = true;
        info->supportsInterrupt = true;
        info->isFullDuplex = true;
        info->maxBaudRate = 115200; /* Adjust based on your clock config */

        __ReturnOk__();
      } else {
        __ReturnError__();
        __AssertOnElse__();
      }
    }

    /* Handle CharIOUARTParams - configure UART parameters */
    else if(*size_ >= sizeof(CharIOUARTParams_t)) {
      CharIOUARTParams_t *params = (CharIOUARTParams_t *)config_;

      if(CHAR_IO_CMD_SET_PARAMS == params->command) {
        #if !defined(POSIX_ARCH_OTHER)
          Word_t halParity = 0;
          Word_t halStopBits = 0;
          Word_t halWordLength = 0;

          /* Translate parameters to HAL format */
          if(OK(__TranslateHALToParity__(params->parity, &halParity)) &&
             OK(__TranslateHALToStopBits__(params->stopBits, &halStopBits)) &&
             OK(__TranslateHALToWordLength__(params->dataBits, &halWordLength))) {

            /* Reconfigure UART with new parameters */
            state.huart.Init.BaudRate = params->baudRate;
            state.huart.Init.WordLength = halWordLength;
            state.huart.Init.StopBits = halStopBits;
            state.huart.Init.Parity = halParity;
            state.huart.Init.Mode = UART_MODE_TX_RX;
            state.huart.Init.HwFlowCtl = params->hardwareFlowCtrl ? UART_HWCONTROL_RTS_CTS : UART_HWCONTROL_NONE;
            state.huart.Init.OverSampling = UART_OVERSAMPLING_16;

            if(HAL_OK == HAL_UART_Init(&state.huart)) {
              __ReturnOk__();
            } else {
              __ReturnError__();
              __AssertOnElse__();
            }
          } else {
            __ReturnError__();
            __AssertOnElse__();
          }
        #else
          /* POSIX simulation - just accept the parameters */
          __ReturnOk__();
        #endif /* if !defined(POSIX_ARCH_OTHER) */
      } else {
        __ReturnError__();
        __AssertOnElse__();
      }
    }

    /* Handle CharIOStatus - return current status */
    else if(*size_ >= sizeof(CharIOStatus_t)) {
      CharIOStatus_t *status = (CharIOStatus_t *)config_;

      if(CHAR_IO_CMD_GET_STATUS == status->command) {
        status->rxBytesAvailable = __CircularBufferAvailable__(state.rxHead, state.rxTail, USART_RX_BUFFER_SIZE);
        status->txBytesFree = __CircularBufferSpace__(state.txHead, state.txTail, USART_TX_BUFFER_SIZE);
        status->isTransmitting = state.txBusy;
        status->isReceiving = state.rxBusy;
        status->errorFlags = state.errorFlags;

        __ReturnOk__();
      } else {
        __ReturnError__();
        __AssertOnElse__();
      }
    }

    /* Handle USARTSTMInitConfig - initialize hardware */
    else if(*size_ >= sizeof(USARTSTMInitConfig_t)) {
      USARTSTMInitConfig_t *initCfg = (USARTSTMInitConfig_t *)config_;

      #if !defined(POSIX_ARCH_OTHER)
        Word_t halParity = 0;
        Word_t halStopBits = 0;
        Word_t halWordLength = 0;

        /* Configure HAL UART handle */
        state.huart.Instance = initCfg->usartInstance;

        /* Translate parameters */
        if(OK(__TranslateHALToParity__(initCfg->parity, &halParity)) &&
           OK(__TranslateHALToStopBits__(initCfg->stopBits, &halStopBits)) &&
           OK(__TranslateHALToWordLength__(initCfg->dataBits, &halWordLength))) {

          state.huart.Init.BaudRate = initCfg->baudRate;
          state.huart.Init.WordLength = halWordLength;
          state.huart.Init.StopBits = halStopBits;
          state.huart.Init.Parity = halParity;
          state.huart.Init.Mode = UART_MODE_TX_RX;
          state.huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
          state.huart.Init.OverSampling = UART_OVERSAMPLING_16;

          /* Initialize UART */
          if(HAL_OK == HAL_UART_Init(&state.huart)) {
            /* Enable UART interrupt */
            HAL_NVIC_SetPriority(initCfg->irqNumber, 5, 0);
            HAL_NVIC_EnableIRQ(initCfg->irqNumber);

            /* Start continuous reception */
            HAL_UART_Receive_IT(&state.huart, &state.rxSingleByte, 1);

            state.initialized = true;
            __ReturnOk__();
          } else {
            __ReturnError__();
            __AssertOnElse__();
          }
        } else {
          __ReturnError__();
          __AssertOnElse__();
        }
      #else
        /* POSIX simulation - just mark as initialized */
        state.initialized = true;
        __ReturnOk__();
      #endif /* if !defined(POSIX_ARCH_OTHER) */
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

  Byte_t *buffer = null;
  HalfWord_t bytesToRead = 0;
  HalfWord_t bytesRead = 0;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    bytesToRead = (HalfWord_t)*size_;

    /* For interrupt mode, read from circular buffer */
    if(CHAR_IO_MODE_INTERRUPT == state.currentRequest.transferMode) {
      HalfWord_t available = __CircularBufferAvailable__(state.rxHead, state.rxTail, USART_RX_BUFFER_SIZE);

      /* Limit read to available data */
      if(bytesToRead > available) {
        bytesToRead = available;
      }

      /* Allocate buffer for read data */
      if(OK(__KernelAllocateMemory__((volatile Addr_t **)&buffer, bytesToRead))) {
        /* Copy data from circular buffer */
        for(bytesRead = 0; bytesRead < bytesToRead; bytesRead++) {
          buffer[bytesRead] = __CircularBufferGet__(state.rxBuffer, &state.rxTail, USART_RX_BUFFER_SIZE);
        }

        *data_ = buffer;
        *size_ = bytesRead;
        __ReturnOk__();
      } else {
        __ReturnError__();
        __AssertOnElse__();
      }
    }
    /* For blocking mode, use HAL blocking receive */
    else if(CHAR_IO_MODE_BLOCKING == state.currentRequest.transferMode) {
      #if !defined(POSIX_ARCH_OTHER)
        /* Allocate buffer for read data */
        if(OK(__KernelAllocateMemory__((volatile Addr_t **)&buffer, bytesToRead))) {
          if(HAL_OK == HAL_UART_Receive(&state.huart, buffer, bytesToRead, state.currentRequest.timeoutMs)) {
            *data_ = buffer;
            *size_ = bytesToRead;
            __ReturnOk__();
          } else {
            __KernelFreeMemory__(buffer);
            __ReturnError__();
            __AssertOnElse__();
          }
        } else {
          __ReturnError__();
          __AssertOnElse__();
        }
      #else
        /* POSIX simulation - return empty buffer */
        if(OK(__KernelAllocateMemory__((volatile Addr_t **)&buffer, bytesToRead))) {
          *data_ = buffer;
          *size_ = 0; /* No data available in simulation */
          __ReturnOk__();
        } else {
          __ReturnError__();
          __AssertOnElse__();
        }
      #endif /* if !defined(POSIX_ARCH_OTHER) */
    } else {
      /* DMA and other modes not implemented yet */
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

  HalfWord_t bytesToWrite = 0;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    bytesToWrite = (HalfWord_t)*size_;

    /* Use STM32 HAL based on transfer mode */
    #if !defined(POSIX_ARCH_OTHER)
      if(CHAR_IO_MODE_DMA == state.currentRequest.transferMode) {
        /* DMA transfer */
        state.txBusy = true;
        if(HAL_OK == HAL_UART_Transmit_DMA(&state.huart, (Byte_t *)data_, bytesToWrite)) {
          __ReturnOk__();
        } else {
          state.txBusy = false;
          __ReturnError__();
          __AssertOnElse__();
        }
      } else if(CHAR_IO_MODE_INTERRUPT == state.currentRequest.transferMode) {
        /* Interrupt-driven transfer */
        state.txBusy = true;
        if(HAL_OK == HAL_UART_Transmit_IT(&state.huart, (Byte_t *)data_, bytesToWrite)) {
          __ReturnOk__();
        } else {
          state.txBusy = false;
          __ReturnError__();
          __AssertOnElse__();
        }
      } else {
        /* Blocking transfer */
        if(HAL_OK == HAL_UART_Transmit(&state.huart, (Byte_t *)data_, bytesToWrite, state.currentRequest.timeoutMs)) {
          __ReturnOk__();
        } else {
          __ReturnError__();
          __AssertOnElse__();
        }
      }
    #else
      /* POSIX simulation - write to stdout */
      write(STDOUT_FILENO, data_, bytesToWrite);
      __ReturnOk__();
    #endif /* if !defined(POSIX_ARCH_OTHER) */
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(data_) && state.initialized) {
    /* Check if data available in RX buffer */
    if(__CircularBufferAvailable__(state.rxHead, state.rxTail, USART_RX_BUFFER_SIZE) > 0) {
      *data_ = __CircularBufferGet__(state.rxBuffer, &state.rxTail, USART_RX_BUFFER_SIZE);
      __ReturnOk__();
    } else {
      #if !defined(POSIX_ARCH_OTHER)
        /* Blocking read of single byte */
        if(HAL_OK == HAL_UART_Receive(&state.huart, data_, 1, 1000)) {
          __ReturnOk__();
        } else {
          __ReturnError__();
          __AssertOnElse__();
        }
      #else
        /* POSIX simulation - no data */
        __ReturnError__();
        __AssertOnElse__();
      #endif /* if !defined(POSIX_ARCH_OTHER) */
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
    #if !defined(POSIX_ARCH_OTHER)
      /* Blocking write of single byte */
      if(HAL_OK == HAL_UART_Transmit(&state.huart, &data_, 1, 1000)) {
        __ReturnOk__();
      } else {
        __ReturnError__();
        __AssertOnElse__();
      }
    #else
      /* POSIX simulation - write to stdout */
      write(STDOUT_FILENO, &data_, 1);
      __ReturnOk__();
    #endif /* if !defined(POSIX_ARCH_OTHER) */
  } else {
    __ReturnError__();
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* ============================================================================
 * IRQ Handler and HAL Callbacks
 * ========================================================================== */

void USART_TX_IRQHandler(void) {
  #if !defined(POSIX_ARCH_OTHER)
    HAL_UART_IRQHandler(&state.huart);
  #endif /* if !defined(POSIX_ARCH_OTHER) */
}


#if !defined(POSIX_ARCH_OTHER)


/* STM32 HAL callback - called when TX complete */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if(huart == &state.huart) {
    state.txBusy = false;
    /* Could signal a HeliOS task here if needed */
  }
}


/* STM32 HAL callback - called when RX complete */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if(huart == &state.huart) {
    /* Put received byte into circular buffer */
    __CircularBufferPut__(state.rxBuffer, &state.rxHead, USART_RX_BUFFER_SIZE, state.rxSingleByte);

    /* Restart reception for next byte */
    HAL_UART_Receive_IT(&state.huart, &state.rxSingleByte, 1);
  }
}


/* STM32 HAL callback - called on UART error */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  if(huart == &state.huart) {
    Word_t halError = HAL_UART_GetError(huart);

    /* Translate HAL errors to our error flags */
    if(halError & HAL_UART_ERROR_PE) {
      state.errorFlags |= USART_ERROR_PARITY;
    }

    if(halError & HAL_UART_ERROR_NE) {
      state.errorFlags |= USART_ERROR_NOISE;
    }

    if(halError & HAL_UART_ERROR_FE) {
      state.errorFlags |= USART_ERROR_FRAME;
    }

    if(halError & HAL_UART_ERROR_ORE) {
      state.errorFlags |= USART_ERROR_OVERRUN;
    }

    state.txBusy = false;
    state.rxBusy = false;

    /* Restart reception after error */
    HAL_UART_Receive_IT(&state.huart, &state.rxSingleByte, 1);
  }
}


#endif /* if !defined(POSIX_ARCH_OTHER) */


/* ============================================================================
 * Circular Buffer Utilities
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


/* Put byte into circular buffer */
static void __CircularBufferPut__(Byte_t *buffer_,
                                 HalfWord_t *head_,
                                 const HalfWord_t size_,
                                 const Byte_t data_) {
  buffer_[*head_] = data_;
  *head_ = (*head_ + 1) % size_;
}


/* Get byte from circular buffer */
static Byte_t __CircularBufferGet__(const Byte_t *buffer_,
                                   HalfWord_t *tail_,
                                   const HalfWord_t size_) {
  Byte_t data = buffer_[*tail_];
  *tail_ = (*tail_ + 1) % size_;
  return data;
}


/* ============================================================================
 * HAL Parameter Translation
 * ========================================================================== */

#if !defined(POSIX_ARCH_OTHER)


static Return_t __TranslateHALToParity__(const Byte_t parity_, Word_t *halParity_) {
  FUNCTION_ENTER;

  switch(parity_) {
    case CHAR_IO_PARITY_NONE:
      *halParity_ = UART_PARITY_NONE;
      break;

    case CHAR_IO_PARITY_EVEN:
      *halParity_ = UART_PARITY_EVEN;
      break;

    case CHAR_IO_PARITY_ODD:
      *halParity_ = UART_PARITY_ODD;
      break;

    default:
      __ReturnError__();
      __AssertOnElse__();
  }

  __ReturnOk__();
  FUNCTION_EXIT;
}


static Return_t __TranslateHALToStopBits__(const Byte_t stopBits_, Word_t *halStopBits_) {
  FUNCTION_ENTER;

  switch(stopBits_) {
    case CHAR_IO_STOP_BITS_1:
      *halStopBits_ = UART_STOPBITS_1;
      break;

    case CHAR_IO_STOP_BITS_2:
      *halStopBits_ = UART_STOPBITS_2;
      break;

    default:
      __ReturnError__();
      __AssertOnElse__();
  }

  __ReturnOk__();
  FUNCTION_EXIT;
}


static Return_t __TranslateHALToWordLength__(const Byte_t dataBits_, Word_t *halWordLength_) {
  FUNCTION_ENTER;

  switch(dataBits_) {
    case CHAR_IO_DATA_BITS_8:
      *halWordLength_ = UART_WORDLENGTH_8B;
      break;

    case CHAR_IO_DATA_BITS_9:
      *halWordLength_ = UART_WORDLENGTH_9B;
      break;

    default:
      __ReturnError__();
      __AssertOnElse__();
  }

  __ReturnOk__();
  FUNCTION_EXIT;
}


#endif /* if !defined(POSIX_ARCH_OTHER) */


#if defined(POSIX_ARCH_OTHER)


/* For unit testing only! */
void __USARTSTMStateClear__(void) {
  state.rxHead = 0;
  state.rxTail = 0;
  state.txHead = 0;
  state.txTail = 0;
  state.txBusy = false;
  state.rxBusy = false;
  state.errorFlags = USART_ERROR_NONE;
  state.initialized = false;
  state.rxSingleByte = 0;

  return;
}


#endif /* if defined(POSIX_ARCH_OTHER) */
/*UNCRUSTIFY-ON*/