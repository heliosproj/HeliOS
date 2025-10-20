/*UNCRUSTIFY-OFF*/
/**
 * @file usart_stm32_driver.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief STM32 USART HAL I/O driver for HeliOS character devices
 * @version 0.5.0
 * @date 2025-10-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * @details
 * This driver implements the char_io_interface for STM32 USART peripherals
 * using the STM32 HAL library. It handles USB-to-USART communication and can
 * be used with any STM32 USART peripheral (USART1, USART2, etc.).
 *
 * Features:
 * - Blocking, interrupt, and DMA transfer modes
 * - Circular RX/TX buffering for interrupt mode
 * - Configurable baud rate, parity, stop bits
 * - Error detection and reporting
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef USART_STM32_DRIVER_H_
  #define USART_STM32_DRIVER_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "sys.h"
  #include "../char/char_io_interface.h"

  /* Only include STM32 HAL when not in POSIX test mode */
  #if !defined(POSIX_ARCH_OTHER)
    /* Include the appropriate STM32 HAL header for your chip family */
    /* Uncomment the one that matches your target */
    /* #include "stm32f0xx_hal.h" */
    /* #include "stm32f1xx_hal.h" */
    /* #include "stm32f2xx_hal.h" */
    /* #include "stm32f3xx_hal.h" */
    /* #include "stm32f4xx_hal.h" */
    /* #include "stm32f7xx_hal.h" */
    /* #include "stm32h7xx_hal.h" */
    /* #include "stm32l0xx_hal.h" */
    /* #include "stm32l1xx_hal.h" */
    /* #include "stm32l4xx_hal.h" */
    /* #include "stm32l5xx_hal.h" */
    /* #include "stm32g0xx_hal.h" */
    /* #include "stm32g4xx_hal.h" */
    /* #include "stm32wbxx_hal.h" */
    /* #include "stm32wlxx_hal.h" */
  #endif /* if !defined(POSIX_ARCH_OTHER) */


  /* Device name must be exactly CONFIG_DEVICE_NAME_BYTES (8 bytes) */
  #define DEVICE_NAME USART_TX
  #define DEVICE_UID 0x0200u
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning


  /* Buffer sizes - adjust based on your application needs */
  #if defined(USART_RX_BUFFER_SIZE)
    #undef USART_RX_BUFFER_SIZE
  #endif /* if defined(USART_RX_BUFFER_SIZE) */
  #define USART_RX_BUFFER_SIZE 512u

  #if defined(USART_TX_BUFFER_SIZE)
    #undef USART_TX_BUFFER_SIZE
  #endif /* if defined(USART_TX_BUFFER_SIZE) */
  #define USART_TX_BUFFER_SIZE 512u
  /* Error flags - match HAL error definitions */
  #if defined(USART_ERROR_NONE)
    #undef USART_ERROR_NONE
  #endif /* if defined(USART_ERROR_NONE) */
  #define USART_ERROR_NONE 0x00u /* 0 */

  #if defined(USART_ERROR_PARITY)
    #undef USART_ERROR_PARITY
  #endif /* if defined(USART_ERROR_PARITY) */
  #define USART_ERROR_PARITY 0x01u /* 1 */

  #if defined(USART_ERROR_NOISE)
    #undef USART_ERROR_NOISE
  #endif /* if defined(USART_ERROR_NOISE) */
  #define USART_ERROR_NOISE 0x02u /* 2 */

  #if defined(USART_ERROR_FRAME)
    #undef USART_ERROR_FRAME
  #endif /* if defined(USART_ERROR_FRAME) */
  #define USART_ERROR_FRAME 0x04u /* 4 */

  #if defined(USART_ERROR_OVERRUN)
    #undef USART_ERROR_OVERRUN
  #endif /* if defined(USART_ERROR_OVERRUN) */
  #define USART_ERROR_OVERRUN 0x08u /* 8 */


  /**
   * @brief STM32 USART initialization configuration
   *
   * Used during driver initialization to specify which USART peripheral to use
   * and initial communication parameters.
   *
   * Usage: Pass this structure to the init function during driver setup.
   */
  typedef struct USARTSTMInitConfig_s {
    #if !defined(POSIX_ARCH_OTHER)
      USART_TypeDef *usartInstance; /* USART1, USART2, USART3, etc. */
      IRQn_Type irqNumber;          /* USARTx_IRQn from device header */
    #else
      void *usartInstance;          /* Placeholder for POSIX testing */
      int irqNumber;                /* Placeholder for POSIX testing */
    #endif /* if !defined(POSIX_ARCH_OTHER) */
    Word_t baudRate;                /* Initial baud rate (e.g., 115200) */
    Byte_t dataBits;                /* CHAR_IO_DATA_BITS_* */
    Byte_t parity;                  /* CHAR_IO_PARITY_* */
    Byte_t stopBits;                /* CHAR_IO_STOP_BITS_* */
  } USARTSTMInitConfig_t;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  /* Driver interface functions - DO NOT CALL DIRECTLY */
  /* Use xDevice* syscalls instead */
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);

  /* IRQ handler - call this from your STM32 interrupt vector table */
  /* Example: void USART2_IRQHandler(void) { USART_TX_IRQHandler(); } */
  void USART_TX_IRQHandler(void);

  #if defined(POSIX_ARCH_OTHER)
    void __USARTSTMStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef USART_STM32_DRIVER_H_ */
