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

#endif // if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

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

#endif // if !defined(POSIX_ARCH_OTHER)

#define DEVICE_NAME USART_TX

#define DEVICE_UID 0x0200u

#define DEVICE_MODE DeviceModeReadWrite

#define DEVICE_STATE DeviceStateRunning

#if defined(USART_RX_BUFFER_SIZE)

  #undef USART_RX_BUFFER_SIZE

#endif // if defined(USART_RX_BUFFER_SIZE)

#define USART_RX_BUFFER_SIZE 512u

#if defined(USART_TX_BUFFER_SIZE)

  #undef USART_TX_BUFFER_SIZE

#endif // if defined(USART_TX_BUFFER_SIZE)

#define USART_TX_BUFFER_SIZE 512u

#if defined(USART_ERROR_NONE)

  #undef USART_ERROR_NONE

#endif // if defined(USART_ERROR_NONE)

#define USART_ERROR_NONE 0x00u

#if defined(USART_ERROR_PARITY)

  #undef USART_ERROR_PARITY

#endif // if defined(USART_ERROR_PARITY)

#define USART_ERROR_PARITY 0x01u

#if defined(USART_ERROR_NOISE)

  #undef USART_ERROR_NOISE

#endif // if defined(USART_ERROR_NOISE)

#define USART_ERROR_NOISE 0x02u

#if defined(USART_ERROR_FRAME)

  #undef USART_ERROR_FRAME

#endif // if defined(USART_ERROR_FRAME)

#define USART_ERROR_FRAME 0x04u

#if defined(USART_ERROR_OVERRUN)

  #undef USART_ERROR_OVERRUN

#endif // if defined(USART_ERROR_OVERRUN)

#define USART_ERROR_OVERRUN 0x08u

typedef struct USARTSTMInitConfig_s {

#if !defined(POSIX_ARCH_OTHER)

    USART_TypeDef *usartInstance;

    IRQn_Type irqNumber;

#else // if !defined(POSIX_ARCH_OTHER)

    void *usartInstance;

    int irqNumber;

#endif // if !defined(POSIX_ARCH_OTHER)

  Word_t baudRate;

  Byte_t dataBits;

  Byte_t parity;

  Byte_t stopBits;

} USARTSTMInitConfig_t;

#ifdef __cplusplus

  extern "C" {

#endif // ifdef __cplusplus

Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);

Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);

Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);

Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);

Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);

Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);

Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);
void USART_TX_IRQHandler(void);

#if defined(POSIX_ARCH_OTHER)
  void __USARTSTMStateClear__(void);

#endif // if defined(POSIX_ARCH_OTHER)

#ifdef __cplusplus

  }

#endif // ifdef __cplusplus

#endif // ifndef USART_STM32_DRIVER_H_