/*UNCRUSTIFY-OFF*/
/**
 * @file char_io_interface.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Common interface for character I/O drivers
 * @version 0.5.0
 * @date 2025-10-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * @details
 * This header defines a standard interface for communication between character
 * device drivers and underlying I/O drivers (UART/USART, SPI, I2C, USB CDC, etc.).
 * This abstraction allows the character driver to remain hardware-agnostic while
 * I/O drivers handle hardware-specific translation.
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef CHAR_IO_INTERFACE_H_
  #define CHAR_IO_INTERFACE_H_

  #include "types.h"


  /* Character I/O operation types */
  #if defined(CHAR_IO_OP_READ)
    #undef CHAR_IO_OP_READ
  #endif /* if defined(CHAR_IO_OP_READ) */
  #define CHAR_IO_OP_READ 0x01u /* 1 */

  #if defined(CHAR_IO_OP_WRITE)
    #undef CHAR_IO_OP_WRITE
  #endif /* if defined(CHAR_IO_OP_WRITE) */
  #define CHAR_IO_OP_WRITE 0x02u /* 2 */

  #if defined(CHAR_IO_OP_FLUSH_RX)
    #undef CHAR_IO_OP_FLUSH_RX
  #endif /* if defined(CHAR_IO_OP_FLUSH_RX) */
  #define CHAR_IO_OP_FLUSH_RX 0x03u /* 3 */

  #if defined(CHAR_IO_OP_FLUSH_TX)
    #undef CHAR_IO_OP_FLUSH_TX
  #endif /* if defined(CHAR_IO_OP_FLUSH_TX) */
  #define CHAR_IO_OP_FLUSH_TX 0x04u /* 4 */
  /* Generic configuration commands for character I/O drivers */
  #if defined(CHAR_IO_CMD_SET_REQUEST)
    #undef CHAR_IO_CMD_SET_REQUEST
  #endif /* if defined(CHAR_IO_CMD_SET_REQUEST) */
  #define CHAR_IO_CMD_SET_REQUEST 0x10u /* 16 */

  #if defined(CHAR_IO_CMD_GET_INFO)
    #undef CHAR_IO_CMD_GET_INFO
  #endif /* if defined(CHAR_IO_CMD_GET_INFO) */
  #define CHAR_IO_CMD_GET_INFO 0x11u /* 17 */

  #if defined(CHAR_IO_CMD_SET_BAUDRATE)
    #undef CHAR_IO_CMD_SET_BAUDRATE
  #endif /* if defined(CHAR_IO_CMD_SET_BAUDRATE) */
  #define CHAR_IO_CMD_SET_BAUDRATE 0x12u /* 18 */

  #if defined(CHAR_IO_CMD_SET_PARAMS)
    #undef CHAR_IO_CMD_SET_PARAMS
  #endif /* if defined(CHAR_IO_CMD_SET_PARAMS) */
  #define CHAR_IO_CMD_SET_PARAMS 0x13u /* 19 */

  #if defined(CHAR_IO_CMD_GET_STATUS)
    #undef CHAR_IO_CMD_GET_STATUS
  #endif /* if defined(CHAR_IO_CMD_GET_STATUS) */
  #define CHAR_IO_CMD_GET_STATUS 0x14u /* 20 */
  /* Character I/O transfer modes */
  #if defined(CHAR_IO_MODE_BLOCKING)
    #undef CHAR_IO_MODE_BLOCKING
  #endif /* if defined(CHAR_IO_MODE_BLOCKING) */
  #define CHAR_IO_MODE_BLOCKING 0x00u /* 0 */

  #if defined(CHAR_IO_MODE_NONBLOCKING)
    #undef CHAR_IO_MODE_NONBLOCKING
  #endif /* if defined(CHAR_IO_MODE_NONBLOCKING) */
  #define CHAR_IO_MODE_NONBLOCKING 0x01u /* 1 */

  #if defined(CHAR_IO_MODE_DMA)
    #undef CHAR_IO_MODE_DMA
  #endif /* if defined(CHAR_IO_MODE_DMA) */
  #define CHAR_IO_MODE_DMA 0x02u /* 2 */

  #if defined(CHAR_IO_MODE_INTERRUPT)
    #undef CHAR_IO_MODE_INTERRUPT
  #endif /* if defined(CHAR_IO_MODE_INTERRUPT) */
  #define CHAR_IO_MODE_INTERRUPT 0x03u /* 3 */
  /* UART parity configuration */
  #if defined(CHAR_IO_PARITY_NONE)
    #undef CHAR_IO_PARITY_NONE
  #endif /* if defined(CHAR_IO_PARITY_NONE) */
  #define CHAR_IO_PARITY_NONE 0x00u /* 0 */

  #if defined(CHAR_IO_PARITY_EVEN)
    #undef CHAR_IO_PARITY_EVEN
  #endif /* if defined(CHAR_IO_PARITY_EVEN) */
  #define CHAR_IO_PARITY_EVEN 0x01u /* 1 */

  #if defined(CHAR_IO_PARITY_ODD)
    #undef CHAR_IO_PARITY_ODD
  #endif /* if defined(CHAR_IO_PARITY_ODD) */
  #define CHAR_IO_PARITY_ODD 0x02u /* 2 */
  /* UART stop bits configuration */
  #if defined(CHAR_IO_STOP_BITS_1)
    #undef CHAR_IO_STOP_BITS_1
  #endif /* if defined(CHAR_IO_STOP_BITS_1) */
  #define CHAR_IO_STOP_BITS_1 0x00u /* 0 */

  #if defined(CHAR_IO_STOP_BITS_1_5)
    #undef CHAR_IO_STOP_BITS_1_5
  #endif /* if defined(CHAR_IO_STOP_BITS_1_5) */
  #define CHAR_IO_STOP_BITS_1_5 0x01u /* 1 */

  #if defined(CHAR_IO_STOP_BITS_2)
    #undef CHAR_IO_STOP_BITS_2
  #endif /* if defined(CHAR_IO_STOP_BITS_2) */
  #define CHAR_IO_STOP_BITS_2 0x02u /* 2 */
  /* UART data bits configuration */
  #if defined(CHAR_IO_DATA_BITS_7)
    #undef CHAR_IO_DATA_BITS_7
  #endif /* if defined(CHAR_IO_DATA_BITS_7) */
  #define CHAR_IO_DATA_BITS_7 0x07u /* 7 */

  #if defined(CHAR_IO_DATA_BITS_8)
    #undef CHAR_IO_DATA_BITS_8
  #endif /* if defined(CHAR_IO_DATA_BITS_8) */
  #define CHAR_IO_DATA_BITS_8 0x08u /* 8 */

  #if defined(CHAR_IO_DATA_BITS_9)
    #undef CHAR_IO_DATA_BITS_9
  #endif /* if defined(CHAR_IO_DATA_BITS_9) */
  #define CHAR_IO_DATA_BITS_9 0x09u /* 9 */


  /**
   * @brief Generic character I/O request structure
   *
   * Used by character drivers to communicate operations to I/O drivers. Each
   * I/O driver translates this to its native hardware commands:
   * - UART: configures baud rate, starts TX/RX
   * - USB CDC: sends to USB endpoint
   * - SPI: configures SPI and sends data
   *
   * Usage: Set command to CHAR_IO_CMD_SET_REQUEST and pass to I/O driver's
   * config function before calling read/write.
   */
  typedef struct CharIORequest_s {
    Byte_t command;        /* CHAR_IO_CMD_SET_REQUEST */
    Byte_t operation; /* CHAR_IO_OP_READ or CHAR_IO_OP_WRITE */
    HalfWord_t byteCount; /* Number of bytes to transfer */
    Byte_t transferMode; /* CHAR_IO_MODE_* constant */
    Word_t timeoutMs; /* Timeout in milliseconds (0 = no timeout) */
  } CharIORequest_t;


  /**
   * @brief Character I/O driver information structure
   *
   * Returned by I/O drivers to inform character driver of capabilities and
   * characteristics. Used to determine optimal buffer sizes and validate
   * operations.
   *
   * Usage: Set command to CHAR_IO_CMD_GET_INFO and pass to I/O driver's config
   * function. The driver fills in the remaining fields.
   */
  typedef struct CharIOInfo_s {
    Byte_t command;                /* CHAR_IO_CMD_GET_INFO */
    HalfWord_t rxBufferSize; /* Receive buffer size in bytes */
    HalfWord_t txBufferSize; /* Transmit buffer size in bytes */
    Base_t supportsDMA; /* true if DMA supported */
    Base_t supportsInterrupt; /* true if interrupt-driven I/O supported */
    Base_t isFullDuplex; /* true for full-duplex, false for half-duplex */
    Word_t maxBaudRate; /* Maximum supported baud rate */
  } CharIOInfo_t;


  /**
   * @brief UART/USART parameter configuration
   *
   * Used to configure serial port parameters.
   *
   * Usage: Set command to CHAR_IO_CMD_SET_PARAMS and pass to I/O driver's
   * config function to change UART settings.
   */
  typedef struct CharIOUARTParams_s {
    Byte_t command;          /* CHAR_IO_CMD_SET_PARAMS */
    Word_t baudRate; /* Baud rate (9600, 115200, etc.) */
    Byte_t dataBits; /* CHAR_IO_DATA_BITS_* */
    Byte_t parity; /* CHAR_IO_PARITY_* */
    Byte_t stopBits; /* CHAR_IO_STOP_BITS_* */
    Base_t hardwareFlowCtrl; /* Hardware flow control enable (RTS/CTS) */
  } CharIOUARTParams_t;


  /**
   * @brief Character I/O status structure
   *
   * Returns current status of the I/O driver including buffer levels and error
   * conditions.
   *
   * Usage: Set command to CHAR_IO_CMD_GET_STATUS and pass to I/O driver's
   * config function. The driver fills in the status fields.
   */
  typedef struct CharIOStatus_s {
    Byte_t command;              /* CHAR_IO_CMD_GET_STATUS */
    HalfWord_t rxBytesAvailable; /* Bytes available in RX buffer */
    HalfWord_t txBytesFree; /* Free space in TX buffer */
    Base_t isTransmitting; /* Transmission in progress */
    Base_t isReceiving; /* Reception in progress */
    Byte_t errorFlags; /* Error flags (overrun, parity, framing) */
  } CharIOStatus_t;

#endif /* ifndef CHAR_IO_INTERFACE_H_ */