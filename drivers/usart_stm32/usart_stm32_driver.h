#ifndef USART_STM32_DRIVER_H_
  #define USART_STM32_DRIVER_H_
  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)
    #include "console.h"
    #include "device.h"
    #include "fat.h"
    #include "fs.h"
  #endif
  #include "mem.h"
  #include "port.h"
  #include "posix.h"
  #include "queue.h"
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"
  #include "../char/char_io_interface.h"
  #if !defined(POSIX_ARCH_OTHER)
  #endif
  #define DEVICE_NAME USART_TX
  #define DEVICE_UID 0x0200u
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning
  #if defined(USART_RX_BUFFER_SIZE)
    #undef USART_RX_BUFFER_SIZE
  #endif
  #define USART_RX_BUFFER_SIZE 0x200u
  #if defined(USART_TX_BUFFER_SIZE)
    #undef USART_TX_BUFFER_SIZE
  #endif
  #define USART_TX_BUFFER_SIZE 0x200u
  #if defined(USART_ERROR_NONE)
    #undef USART_ERROR_NONE
  #endif
  #define USART_ERROR_NONE 0x00u
  #if defined(USART_ERROR_PARITY)
    #undef USART_ERROR_PARITY
  #endif
  #define USART_ERROR_PARITY 0x01u
  #if defined(USART_ERROR_NOISE)
    #undef USART_ERROR_NOISE
  #endif
  #define USART_ERROR_NOISE 0x02u
  #if defined(USART_ERROR_FRAME)
    #undef USART_ERROR_FRAME
  #endif
  #define USART_ERROR_FRAME 0x04u
  #if defined(USART_ERROR_OVERRUN)
    #undef USART_ERROR_OVERRUN
  #endif
  #define USART_ERROR_OVERRUN 0x08u
  #define USART_MAX_BAUD_RATE 0x1C200u
  #define USART_IRQ_PRIORITY 0x5u
  #define USART_SINGLE_BYTE_TRANSFER 0x1u
  #define USART_DEFAULT_TIMEOUT_MS 0x3E8u
  typedef struct USARTSTMInitConfig_s {
  #if !defined(POSIX_ARCH_OTHER)
      USART_TypeDef *usartInstance;
      IRQn_Type irqNumber;
  #else
      void *usartInstance;
      int irqNumber;
  #endif
    Word_t baudRate;
    Byte_t dataBits;
    Byte_t parity;
    Byte_t stopBits;
  } USARTSTMInitConfig_t;
typedef struct USARTDriverState_s {
#if !defined(POSIX_ARCH_OTHER)
    UART_HandleTypeDef huart;
#else
    void *huart;
#endif
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
  #ifdef __cplusplus
    extern "C" {
  #endif
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);
  void USART_TX_IRQHandler(void);
  HalfWord_t __CircularBufferSpace__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
HalfWord_t __CircularBufferAvailable__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
void __CircularBufferPut__(Byte_t *buffer_, HalfWord_t *head_, const HalfWord_t size_, const Byte_t data_);
Byte_t __CircularBufferGet__(const Byte_t *buffer_, HalfWord_t *tail_, const HalfWord_t size_);
#if !defined(POSIX_ARCH_OTHER)
  Return_t __TranslateHALToParity__(const Byte_t parity_, Word_t *halParity_);
  Return_t __TranslateHALToStopBits__(const Byte_t stopBits_, Word_t *halStopBits_);
  Return_t __TranslateHALToWordLength__(const Byte_t dataBits_, Word_t *halWordLength_);
#endif
  #if defined(POSIX_ARCH_OTHER)
    void __USARTSTMStateClear__(void);
  #endif
  #ifdef __cplusplus
    }
  #endif
#endif