/*UNCRUSTIFY-OFF*/
/**
 * @file usart_stm32_driver.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief STM32 USART driver implementation
 * @details
 * Implements STM32 USART hardware driver with interrupt-based TX/RX, circular buffering, and error handling for serial communication on ARM Cortex-M devices.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#include "usart_stm32_driver.h"

#if !defined(POSIX_ARCH_OTHER)

  #include <unistd.h>

#endif /* if !defined(POSIX_ARCH_OTHER) */

typedef struct USARTDriverState_s {

#if !defined(POSIX_ARCH_OTHER)

    UART_HandleTypeDef huart;

#else  /* if !defined(POSIX_ARCH_OTHER) */

    void *huart;

#endif /* if !defined(POSIX_ARCH_OTHER) */

  Byte_t rxBuffer[USART_RX_BUFFER_SIZE];

  Byte_t txBuffer[USART_TX_BUFFER_SIZE];

  volatile HalfWord_t rxHead;

  volatile HalfWord_t rxTail;

  volatile HalfWord_t txHead;

  volatile HalfWord_t txTail;

  volatile Base_t txBusy;

  volatile Base_t rxBusy;

  volatile Byte_t errorFlags;

  CharIORequest_t currentRequest;

  Base_t initialized;

  Byte_t rxSingleByte;

} USARTDriverState_t;

static USARTDriverState_t state = {

  0

};
static HalfWord_t __CircularBufferSpace__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
static HalfWord_t __CircularBufferAvailable__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
static void __CircularBufferPut__(Byte_t *buffer_, HalfWord_t *head_, const HalfWord_t size_, const Byte_t data_);
static Byte_t __CircularBufferGet__(const Byte_t *buffer_, HalfWord_t *tail_, const HalfWord_t size_);

#if !defined(POSIX_ARCH_OTHER)
  static Return_t __TranslateHALToParity__(const Byte_t parity_, Word_t *halParity_);
  static Return_t __TranslateHALToStopBits__(const Byte_t stopBits_, Word_t *halStopBits_);
  static Return_t __TranslateHALToWordLength__(const Byte_t dataBits_, Word_t *halWordLength_);

#endif /* if !defined(POSIX_ARCH_OTHER) */
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

  state.rxHead = 0x0u;

  state.rxTail = 0x0u;

  state.txHead = 0x0u;

  state.txTail = 0x0u;

  state.txBusy = false;

  state.rxBusy = false;

  state.errorFlags = USART_ERROR_NONE;

  state.initialized = false;

  state.rxSingleByte = 0x0u;

  state.currentRequest.command = 0x0u;

  state.currentRequest.operation = 0x0u;

  state.currentRequest.byteCount = 0x0u;

  state.currentRequest.transferMode = CHAR_IO_MODE_BLOCKING;

  state.currentRequest.timeoutMs = 0x0u;

  state.initialized = true;

  __ReturnOk__();

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {

  FUNCTION_ENTER;

  if(__PointerIsNotNull__(config_) && __PointerIsNotNull__(size_)) {

    if(*size_ >= sizeof(CharIORequest_t)) {

      CharIORequest_t *req = (CharIORequest_t *) config_;

      if(CHAR_IO_CMD_SET_REQUEST == req->command) {

        state.currentRequest.command = req->command;

        state.currentRequest.operation = req->operation;

        state.currentRequest.byteCount = req->byteCount;

        state.currentRequest.transferMode = req->transferMode;

        state.currentRequest.timeoutMs = req->timeoutMs;

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

    } else if(*size_ >= sizeof(CharIOInfo_t)) {

      CharIOInfo_t *info = (CharIOInfo_t *) config_;

      if(CHAR_IO_CMD_GET_INFO == info->command) {

        info->rxBufferSize = USART_RX_BUFFER_SIZE;

        info->txBufferSize = USART_TX_BUFFER_SIZE;

        info->supportsDMA = true;

        info->supportsInterrupt = true;

        info->isFullDuplex = true;

        info->maxBaudRate = 115200;

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

    } else if(*size_ >= sizeof(CharIOUARTParams_t)) {

      CharIOUARTParams_t *params = (CharIOUARTParams_t *) config_;

      if(CHAR_IO_CMD_SET_PARAMS == params->command) {

#if !defined(POSIX_ARCH_OTHER)

          Word_t halParity = 0x0u;

          Word_t halStopBits = 0x0u;

          Word_t halWordLength = 0x0u;

          if(OK(__TranslateHALToParity__(params->parity, &halParity)) &&

            OK(__TranslateHALToStopBits__(params->stopBits, &halStopBits)) &&

            OK(__TranslateHALToWordLength__(params->dataBits, &halWordLength))) {

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

              __AssertOnElse__();

            }

          } else {

            __AssertOnElse__();

          }

#else  /* if !defined(POSIX_ARCH_OTHER) */

          __ReturnOk__();

#endif /* if !defined(POSIX_ARCH_OTHER) */

      } else {

        __AssertOnElse__();

      }

    } else if(*size_ >= sizeof(CharIOStatus_t)) {

      CharIOStatus_t *status = (CharIOStatus_t *) config_;

      if(CHAR_IO_CMD_GET_STATUS == status->command) {

        status->rxBytesAvailable = __CircularBufferAvailable__(state.rxHead, state.rxTail, USART_RX_BUFFER_SIZE);

        status->txBytesFree = __CircularBufferSpace__(state.txHead, state.txTail, USART_TX_BUFFER_SIZE);

        status->isTransmitting = state.txBusy;

        status->isReceiving = state.rxBusy;

        status->errorFlags = state.errorFlags;

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

    } else if(*size_ >= sizeof(USARTSTMInitConfig_t)) {

      USARTSTMInitConfig_t *initCfg = (USARTSTMInitConfig_t *) config_;

#if !defined(POSIX_ARCH_OTHER)

        Word_t halParity = 0x0u;

        Word_t halStopBits = 0x0u;

        Word_t halWordLength = 0x0u;

        state.huart.Instance = initCfg->usartInstance;

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

          if(HAL_OK == HAL_UART_Init(&state.huart)) {

            HAL_NVIC_SetPriority(initCfg->irqNumber, 5, 0);

            HAL_NVIC_EnableIRQ(initCfg->irqNumber);

            HAL_UART_Receive_IT(&state.huart, &state.rxSingleByte, 1);

            state.initialized = true;

            __ReturnOk__();

          } else {

            __AssertOnElse__();

          }

        } else {

          __AssertOnElse__();

        }

#else  /* if !defined(POSIX_ARCH_OTHER) */

        state.initialized = true;

        __ReturnOk__();

#endif /* if !defined(POSIX_ARCH_OTHER) */

    } else {

      __AssertOnElse__();

    }

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {

  FUNCTION_ENTER;

  Byte_t *buffer = null;

  HalfWord_t bytesToRead = 0x0u;

  HalfWord_t bytesRead = 0x0u;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    bytesToRead = (HalfWord_t) *size_;

    if(CHAR_IO_MODE_INTERRUPT == state.currentRequest.transferMode) {

      HalfWord_t available = __CircularBufferAvailable__(state.rxHead, state.rxTail, USART_RX_BUFFER_SIZE);

      if(bytesToRead > available) {

        bytesToRead = available;

      }

      if(OK(__KernelAllocateMemory__((volatile Addr_t **) &buffer, bytesToRead))) {

        for(bytesRead = 0x0u; bytesRead < bytesToRead; bytesRead++) {

          buffer[bytesRead] = __CircularBufferGet__(state.rxBuffer, &state.rxTail, USART_RX_BUFFER_SIZE);

        }

        *data_ = buffer;

        *size_ = bytesRead;

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

    } else if(CHAR_IO_MODE_BLOCKING == state.currentRequest.transferMode) {

#if !defined(POSIX_ARCH_OTHER)

        if(OK(__KernelAllocateMemory__((volatile Addr_t **) &buffer, bytesToRead))) {

          if(HAL_OK == HAL_UART_Receive(&state.huart, buffer, bytesToRead, state.currentRequest.timeoutMs)) {

            *data_ = buffer;

            *size_ = bytesToRead;

            __ReturnOk__();

          } else {

            __KernelFreeMemory__(buffer);

            __AssertOnElse__();

          }

        } else {

          __AssertOnElse__();

        }

#else  /* if !defined(POSIX_ARCH_OTHER) */

        if(OK(__KernelAllocateMemory__((volatile Addr_t **) &buffer, bytesToRead))) {

          *data_ = buffer;

          *size_ = 0x0u;

          __ReturnOk__();

        } else {

          __AssertOnElse__();

        }

#endif /* if !defined(POSIX_ARCH_OTHER) */

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

  HalfWord_t bytesToWrite = 0x0u;

  if(__PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && state.initialized) {

    bytesToWrite = (HalfWord_t) *size_;

#if !defined(POSIX_ARCH_OTHER)

      if(CHAR_IO_MODE_DMA == state.currentRequest.transferMode) {

        state.txBusy = true;

        if(HAL_OK == HAL_UART_Transmit_DMA(&state.huart, (Byte_t *) data_, bytesToWrite)) {

          __ReturnOk__();

        } else {

          state.txBusy = false;

          __AssertOnElse__();

        }

      } else if(CHAR_IO_MODE_INTERRUPT == state.currentRequest.transferMode) {

        state.txBusy = true;

        if(HAL_OK == HAL_UART_Transmit_IT(&state.huart, (Byte_t *) data_, bytesToWrite)) {

          __ReturnOk__();

        } else {

          state.txBusy = false;

          __AssertOnElse__();

        }

      } else {

        if(HAL_OK == HAL_UART_Transmit(&state.huart, (Byte_t *) data_, bytesToWrite, state.currentRequest.timeoutMs)) {

          __ReturnOk__();

        } else {

          __AssertOnElse__();

        }

      }

#else  /* if !defined(POSIX_ARCH_OTHER) */

      write(STDOUT_FILENO, data_, bytesToWrite);

      __ReturnOk__();

#endif /* if !defined(POSIX_ARCH_OTHER) */

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_) {

  FUNCTION_ENTER;

  if(__PointerIsNotNull__(data_) && state.initialized) {

    if(__CircularBufferAvailable__(state.rxHead, state.rxTail, USART_RX_BUFFER_SIZE) > 0) {

      *data_ = __CircularBufferGet__(state.rxBuffer, &state.rxTail, USART_RX_BUFFER_SIZE);

      __ReturnOk__();

    } else {

#if !defined(POSIX_ARCH_OTHER)

        if(HAL_OK == HAL_UART_Receive(&state.huart, data_, 1, 1000)) {

          __ReturnOk__();

        } else {

          __AssertOnElse__();

        }

#else  /* if !defined(POSIX_ARCH_OTHER) */

        __AssertOnElse__();

#endif /* if !defined(POSIX_ARCH_OTHER) */

    }

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_) {

  FUNCTION_ENTER;

  if(state.initialized) {

#if !defined(POSIX_ARCH_OTHER)

      if(HAL_OK == HAL_UART_Transmit(&state.huart, &data_, 1, 1000)) {

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

#else  /* if !defined(POSIX_ARCH_OTHER) */

      write(STDOUT_FILENO, &data_, 1);

      __ReturnOk__();

#endif /* if !defined(POSIX_ARCH_OTHER) */

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}
void USART_TX_IRQHandler(void) {

#if !defined(POSIX_ARCH_OTHER)

    HAL_UART_IRQHandler(&state.huart);

#endif /* if !defined(POSIX_ARCH_OTHER) */

}


#if !defined(POSIX_ARCH_OTHER)
  void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {

    if(huart == &state.huart) {

      state.txBusy = false;

    }

  }


  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

    if(huart == &state.huart) {

      __CircularBufferPut__(state.rxBuffer, &state.rxHead, USART_RX_BUFFER_SIZE, state.rxSingleByte);

      HAL_UART_Receive_IT(&state.huart, &state.rxSingleByte, 1);

    }

  }


  void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {

    if(huart == &state.huart) {

      Word_t halError = HAL_UART_GetError(huart);

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

      HAL_UART_Receive_IT(&state.huart, &state.rxSingleByte, 1);

    }

  }


#endif /* if !defined(POSIX_ARCH_OTHER) */
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


static void __CircularBufferPut__(Byte_t *buffer_,

  HalfWord_t *head_,

  const HalfWord_t size_,

  const Byte_t data_) {

  buffer_[*head_] = data_;

  *head_ = (*head_ + 1) % size_;

}


static Byte_t __CircularBufferGet__(const Byte_t *buffer_,

  HalfWord_t *tail_,

  const HalfWord_t size_) {

  Byte_t data = buffer_[*tail_];

  *tail_ = (*tail_ + 1) % size_;

  return(data);

}


#if !defined(POSIX_ARCH_OTHER)
  /**
   * @brief Translates HAL parity setting
   * @details Internal helper that converts STM32 HAL parity constants to driver parity constants.
   *
   * @param[in] halParity_ HAL parity value
   * @param[in] parity_ Pointer to store translated parity
   *
   * @return ReturnOK if translation was successful
   * @return ReturnError if invalid parity value
   */
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

      __AssertOnElse__();

    }

    __ReturnOk__();

    FUNCTION_EXIT;

  }


  /**
   * @brief Translates HAL stop bits setting
   * @details Internal helper that converts STM32 HAL stop bits constants to driver stop bits constants.
   *
   * @param[in] halStopBits_ HAL stop bits value
   * @param[in] stopBits_ Pointer to store translated stop bits
   *
   * @return ReturnOK if translation was successful
   * @return ReturnError if invalid stop bits value
   */
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

      __AssertOnElse__();

    }

    __ReturnOk__();

    FUNCTION_EXIT;

  }


  /**
   * @brief Translates data bits to HAL word length setting
   * @details Internal helper that converts driver data bits setting to STM32 HAL word length constants.
   *
   * @param[in]  dataBits_      Data bits setting (8 or 9)
   * @param[out] halWordLength_ Pointer to store HAL word length constant
   *
   * @return                    ReturnOK if translation was successful
   * @return                    ReturnError if invalid data bits value
   */
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

      __AssertOnElse__();

    }

    __ReturnOk__();

    FUNCTION_EXIT;

  }


#endif /* if !defined(POSIX_ARCH_OTHER) */

#if defined(POSIX_ARCH_OTHER)
  void __USARTSTMStateClear__(void) {

    state.rxHead = 0x0u;

    state.rxTail = 0x0u;

    state.txHead = 0x0u;

    state.txTail = 0x0u;

    state.txBusy = false;

    state.rxBusy = false;

    state.errorFlags = USART_ERROR_NONE;

    state.initialized = false;

    state.rxSingleByte = 0x0u;

    return;

  }


#endif /* if defined(POSIX_ARCH_OTHER) */