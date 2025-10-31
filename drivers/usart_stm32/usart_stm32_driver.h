/*UNCRUSTIFY-OFF*/
/**
 * @file usart_stm32_driver.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief STM32 USART driver header
 * @details
 * Defines STM32-specific USART configuration structures and function prototypes for interrupt-driven serial communication with hardware flow control support.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
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
  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */
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
  #endif /* if !defined(POSIX_ARCH_OTHER) */
  #define DEVICE_NAME USART_TX
  #define DEVICE_UID 0x0200u
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning
  #if defined(USART_RX_BUFFER_SIZE)
    #undef USART_RX_BUFFER_SIZE
  #endif /* if defined(USART_RX_BUFFER_SIZE) */
  #define USART_RX_BUFFER_SIZE 512u
  #if defined(USART_TX_BUFFER_SIZE)
    #undef USART_TX_BUFFER_SIZE
  #endif /* if defined(USART_TX_BUFFER_SIZE) */
  #define USART_TX_BUFFER_SIZE 512u
  #if defined(USART_ERROR_NONE)
    #undef USART_ERROR_NONE
  #endif /* if defined(USART_ERROR_NONE) */
  #define USART_ERROR_NONE 0x00u
  #if defined(USART_ERROR_PARITY)
    #undef USART_ERROR_PARITY
  #endif /* if defined(USART_ERROR_PARITY) */
  #define USART_ERROR_PARITY 0x01u
  #if defined(USART_ERROR_NOISE)
    #undef USART_ERROR_NOISE
  #endif /* if defined(USART_ERROR_NOISE) */
  #define USART_ERROR_NOISE 0x02u
  #if defined(USART_ERROR_FRAME)
    #undef USART_ERROR_FRAME
  #endif /* if defined(USART_ERROR_FRAME) */
  #define USART_ERROR_FRAME 0x04u
  #if defined(USART_ERROR_OVERRUN)
    #undef USART_ERROR_OVERRUN
  #endif /* if defined(USART_ERROR_OVERRUN) */
  #define USART_ERROR_OVERRUN 0x08u
  /**
   * @brief STM32 USART initialization configuration structure
   * @details Configuration structure for initializing an STM32 USART peripheral
   * with hardware instance and communication parameters.
   */
  typedef struct USARTSTMInitConfig_s {
  #if !defined(POSIX_ARCH_OTHER)
      USART_TypeDef *usartInstance; /**< Pointer to STM32 USART hardware
                                     * instance (e.g., USART1, USART2) */
      IRQn_Type irqNumber; /**< IRQ number for USART interrupt */
  #else /* if !defined(POSIX_ARCH_OTHER) */
      void *usartInstance; /**< Placeholder for POSIX platforms */
      int irqNumber; /**< Placeholder for POSIX platforms */
  #endif /* if !defined(POSIX_ARCH_OTHER) */
    Word_t baudRate; /**< Baud rate for serial communication (e.g., 9600,
                      * 115200) */
    Byte_t dataBits; /**< Number of data bits (7 or 8) */
    Byte_t parity; /**< Parity setting (none, even, odd) */
    Byte_t stopBits; /**< Number of stop bits (1 or 2) */
  } USARTSTMInitConfig_t;
  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  /**
   * @brief Self-registers STM32 USART driver
   * @details Registers the STM32 USART driver with the device manager during
   * system initialization.
   *
   * @return ReturnOK if registration was successful
   * @return ReturnError if registration failed
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);
  /**
   * @brief Initializes STM32 USART driver
   * @details Initializes internal buffers, configures USART hardware, and
   * enables interrupts.
   *
   * @param[in,out] device_ Pointer to device structure
   *
   * @return                ReturnOK if initialization was successful
   * @return                ReturnError if initialization failed
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);
  /**
   * @brief Configures STM32 USART driver
   * @details Configures USART parameters such as baud rate, data bits, parity,
   * and stop bits.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in,out] size_   Pointer to configuration data size
   * @param[in]     config_ Pointer to configuration data (USARTSTMInitConfig_t
   *                        or CharIOConfig_t)
   *
   * @return                ReturnOK if configuration was successful
   * @return                ReturnError if configuration failed or invalid
   *                        parameters
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);
  /**
   * @brief Reads data from STM32 USART
   * @details Reads data from the receive buffer, allocates memory for the data.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in,out] size_   Pointer to size of data to read (updated with actual
   *                        bytes read)
   * @param[out]    data_   Pointer to store allocated data buffer
   *
   * @return                ReturnOK if read was successful
   * @return                ReturnError if read failed or allocation failed
   *
   * @warning Caller is responsible for freeing the allocated data buffer
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);
  /**
   * @brief Writes data to STM32 USART
   * @details Writes data to the transmit buffer and initiates transmission.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in,out] size_   Pointer to size of data to write (updated with
   *                        actual bytes written)
   * @param[in]     data_   Pointer to data buffer to write
   *
   * @return                ReturnOK if write was successful
   * @return                ReturnError if write failed or invalid parameters
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);
  /**
   * @brief Reads single byte from STM32 USART
   * @details Simple interface for reading one byte from the receive buffer.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[out]    data_   Pointer to store the read byte
   *
   * @return                ReturnOK if read was successful
   * @return                ReturnError if read failed or no data available
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);
  /**
   * @brief Writes single byte to STM32 USART
   * @details Simple interface for writing one byte to the transmit buffer.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in]     data_   Byte to write
   *
   * @return                ReturnOK if write was successful
   * @return                ReturnError if write failed or buffer full
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);
  /**
   * @brief STM32 USART interrupt handler
   * @details Handles USART transmit and receive interrupts, manages circular
   * buffers.
   *
   * @note This function should be called from the appropriate USART IRQ handler
   */
  void USART_TX_IRQHandler(void);
  #if defined(POSIX_ARCH_OTHER)
    /**
     * @brief Clears STM32 USART driver state
     * @details Internal function for POSIX platforms to reset driver state for
     * testing.
     */
    void __USARTSTMStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */
  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef USART_STM32_DRIVER_H_ */