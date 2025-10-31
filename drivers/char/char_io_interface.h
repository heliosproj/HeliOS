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

typedef struct CharIORequest_s {

  Byte_t command;

  Byte_t operation;

  HalfWord_t byteCount;

  Byte_t transferMode;

  Word_t timeoutMs;

} CharIORequest_t;

typedef struct CharIOInfo_s {

  Byte_t command;

  HalfWord_t rxBufferSize;

  HalfWord_t txBufferSize;

  Base_t supportsDMA;

  Base_t supportsInterrupt;

  Base_t isFullDuplex;

  Word_t maxBaudRate;

} CharIOInfo_t;

typedef struct CharIOUARTParams_s {

  Byte_t command;

  Word_t baudRate;

  Byte_t dataBits;

  Byte_t parity;

  Byte_t stopBits;

  Base_t hardwareFlowCtrl;

} CharIOUARTParams_t;

typedef struct CharIOStatus_s {

  Byte_t command;

  HalfWord_t rxBytesAvailable;

  HalfWord_t txBytesFree;

  Base_t isTransmitting;

  Base_t isReceiving;

  Byte_t errorFlags;

} CharIOStatus_t;

#endif /* ifndef CHAR_IO_INTERFACE_H_ */