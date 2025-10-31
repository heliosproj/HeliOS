/*UNCRUSTIFY-OFF*/


/**
 * @file char_io_interface.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Character I/O interface definitions
 * @details
 * Defines standard interface structures and command constants for low-level character I/O drivers including transfer modes, UART parameters, and status reporting.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */



/*UNCRUSTIFY-ON*/


#ifndef CHAR_IO_INTERFACE_H_


  #define CHAR_IO_INTERFACE_H_


  #include "types.h"


  #if defined(CHAR_IO_OP_READ)


    #undef CHAR_IO_OP_READ


  #endif /* if defined(CHAR_IO_OP_READ) */


  #define CHAR_IO_OP_READ 0x01u


  #if defined(CHAR_IO_OP_WRITE)


    #undef CHAR_IO_OP_WRITE


  #endif /* if defined(CHAR_IO_OP_WRITE) */


  #define CHAR_IO_OP_WRITE 0x02u


  #if defined(CHAR_IO_OP_FLUSH_RX)


    #undef CHAR_IO_OP_FLUSH_RX


  #endif /* if defined(CHAR_IO_OP_FLUSH_RX) */


  #define CHAR_IO_OP_FLUSH_RX 0x03u


  #if defined(CHAR_IO_OP_FLUSH_TX)


    #undef CHAR_IO_OP_FLUSH_TX


  #endif /* if defined(CHAR_IO_OP_FLUSH_TX) */


  #define CHAR_IO_OP_FLUSH_TX 0x04u


  #if defined(CHAR_IO_CMD_SET_REQUEST)


    #undef CHAR_IO_CMD_SET_REQUEST


  #endif /* if defined(CHAR_IO_CMD_SET_REQUEST) */


  #define CHAR_IO_CMD_SET_REQUEST 0x10u


  #if defined(CHAR_IO_CMD_GET_INFO)


    #undef CHAR_IO_CMD_GET_INFO


  #endif /* if defined(CHAR_IO_CMD_GET_INFO) */


  #define CHAR_IO_CMD_GET_INFO 0x11u


  #if defined(CHAR_IO_CMD_SET_BAUDRATE)


    #undef CHAR_IO_CMD_SET_BAUDRATE


  #endif /* if defined(CHAR_IO_CMD_SET_BAUDRATE) */


  #define CHAR_IO_CMD_SET_BAUDRATE 0x12u


  #if defined(CHAR_IO_CMD_SET_PARAMS)


    #undef CHAR_IO_CMD_SET_PARAMS


  #endif /* if defined(CHAR_IO_CMD_SET_PARAMS) */


  #define CHAR_IO_CMD_SET_PARAMS 0x13u


  #if defined(CHAR_IO_CMD_GET_STATUS)


    #undef CHAR_IO_CMD_GET_STATUS


  #endif /* if defined(CHAR_IO_CMD_GET_STATUS) */


  #define CHAR_IO_CMD_GET_STATUS 0x14u


  #if defined(CHAR_IO_MODE_BLOCKING)


    #undef CHAR_IO_MODE_BLOCKING


  #endif /* if defined(CHAR_IO_MODE_BLOCKING) */


  #define CHAR_IO_MODE_BLOCKING 0x00u


  #if defined(CHAR_IO_MODE_NONBLOCKING)


    #undef CHAR_IO_MODE_NONBLOCKING


  #endif /* if defined(CHAR_IO_MODE_NONBLOCKING) */


  #define CHAR_IO_MODE_NONBLOCKING 0x01u


  #if defined(CHAR_IO_MODE_DMA)


    #undef CHAR_IO_MODE_DMA


  #endif /* if defined(CHAR_IO_MODE_DMA) */


  #define CHAR_IO_MODE_DMA 0x02u


  #if defined(CHAR_IO_MODE_INTERRUPT)


    #undef CHAR_IO_MODE_INTERRUPT


  #endif /* if defined(CHAR_IO_MODE_INTERRUPT) */


  #define CHAR_IO_MODE_INTERRUPT 0x03u


  #if defined(CHAR_IO_PARITY_NONE)


    #undef CHAR_IO_PARITY_NONE


  #endif /* if defined(CHAR_IO_PARITY_NONE) */


  #define CHAR_IO_PARITY_NONE 0x00u


  #if defined(CHAR_IO_PARITY_EVEN)


    #undef CHAR_IO_PARITY_EVEN


  #endif /* if defined(CHAR_IO_PARITY_EVEN) */


  #define CHAR_IO_PARITY_EVEN 0x01u


  #if defined(CHAR_IO_PARITY_ODD)


    #undef CHAR_IO_PARITY_ODD


  #endif /* if defined(CHAR_IO_PARITY_ODD) */


  #define CHAR_IO_PARITY_ODD 0x02u


  #if defined(CHAR_IO_STOP_BITS_1)


    #undef CHAR_IO_STOP_BITS_1


  #endif /* if defined(CHAR_IO_STOP_BITS_1) */


  #define CHAR_IO_STOP_BITS_1 0x00u


  #if defined(CHAR_IO_STOP_BITS_1_5)


    #undef CHAR_IO_STOP_BITS_1_5


  #endif /* if defined(CHAR_IO_STOP_BITS_1_5) */


  #define CHAR_IO_STOP_BITS_1_5 0x01u


  #if defined(CHAR_IO_STOP_BITS_2)


    #undef CHAR_IO_STOP_BITS_2


  #endif /* if defined(CHAR_IO_STOP_BITS_2) */


  #define CHAR_IO_STOP_BITS_2 0x02u


  #if defined(CHAR_IO_DATA_BITS_7)


    #undef CHAR_IO_DATA_BITS_7


  #endif /* if defined(CHAR_IO_DATA_BITS_7) */


  #define CHAR_IO_DATA_BITS_7 0x07u


  #if defined(CHAR_IO_DATA_BITS_8)


    #undef CHAR_IO_DATA_BITS_8


  #endif /* if defined(CHAR_IO_DATA_BITS_8) */


  #define CHAR_IO_DATA_BITS_8 0x08u


  #if defined(CHAR_IO_DATA_BITS_9)


    #undef CHAR_IO_DATA_BITS_9


  #endif /* if defined(CHAR_IO_DATA_BITS_9) */


  #define CHAR_IO_DATA_BITS_9 0x09u


  /**
   * @brief Character I/O request structure
   * @details Command structure for character device I/O operations specifying
   * the operation type, transfer mode, and timeout.
   */
  typedef struct CharIORequest_s {


    Byte_t command; /**< Command type identifier */
    Byte_t operation; /**< Operation (read, write, flush) */
    HalfWord_t byteCount; /**< Number of bytes to transfer */
    Byte_t transferMode; /**< Transfer mode (blocking, non-blocking, DMA,
                          * interrupt) */
    Word_t timeoutMs; /**< Timeout in milliseconds */
  } CharIORequest_t;


  /**
   * @brief Character I/O device information structure
   * @details Contains device capabilities and characteristics for character I/O
   * devices.
   */
  typedef struct CharIOInfo_s {


    Byte_t command; /**< Command type identifier */
    HalfWord_t rxBufferSize; /**< Receive buffer size in bytes */
    HalfWord_t txBufferSize; /**< Transmit buffer size in bytes */
    Base_t supportsDMA; /**< Flag indicating if device supports DMA */
    Base_t supportsInterrupt; /**< Flag indicating if device supports
                               * interrupt-driven I/O */
    Base_t isFullDuplex; /**< Flag indicating if device supports full-duplex
                          * communication */
    Word_t maxBaudRate; /**< Maximum supported baud rate */
  } CharIOInfo_t;


  /**
   * @brief Character I/O UART parameters structure
   * @details Configuration structure for UART/USART communication parameters.
   */
  typedef struct CharIOUARTParams_s {


    Byte_t command; /**< Command type identifier */
    Word_t baudRate; /**< Baud rate for serial communication */
    Byte_t dataBits; /**< Number of data bits (7, 8, or 9) */
    Byte_t parity; /**< Parity setting (none, even, odd) */
    Byte_t stopBits; /**< Number of stop bits (1, 1.5, or 2) */
    Base_t hardwareFlowCtrl; /**< Flag indicating if hardware flow control is
                              * enabled */
  } CharIOUARTParams_t;


  /**
   * @brief Character I/O status structure
   * @details Runtime status information for character device including buffer
   * levels and error flags.
   */
  typedef struct CharIOStatus_s {


    Byte_t command; /**< Command type identifier */
    HalfWord_t rxBytesAvailable; /**< Number of bytes available in receive
                                  * buffer */
    HalfWord_t txBytesFree; /**< Number of free bytes in transmit buffer */
    Base_t isTransmitting; /**< Flag indicating if device is currently
                            * transmitting */
    Base_t isReceiving; /**< Flag indicating if device is currently receiving */
    Byte_t errorFlags; /**< Error flags (parity, noise, frame, overrun) */
  } CharIOStatus_t;


#endif /* ifndef CHAR_IO_INTERFACE_H_ */