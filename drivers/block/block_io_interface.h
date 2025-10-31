#ifndef BLOCK_IO_INTERFACE_H_

  #define BLOCK_IO_INTERFACE_H_

  #include "types.h"

  #if defined(BLOCK_IO_OP_READ)

    #undef BLOCK_IO_OP_READ

  #endif 

  #define BLOCK_IO_OP_READ 0x01u 

  #if defined(BLOCK_IO_OP_WRITE)

    #undef BLOCK_IO_OP_WRITE

  #endif 

  #define BLOCK_IO_OP_WRITE 0x02u 

  #if defined(BLOCK_IO_MODE_BLOCKING)

    #undef BLOCK_IO_MODE_BLOCKING

  #endif 

  #define BLOCK_IO_MODE_BLOCKING 0x00u 

  #if defined(BLOCK_IO_MODE_NONBLOCKING)

    #undef BLOCK_IO_MODE_NONBLOCKING

  #endif 

  #define BLOCK_IO_MODE_NONBLOCKING 0x01u 

  #if defined(BLOCK_IO_MODE_DMA)

    #undef BLOCK_IO_MODE_DMA

  #endif 

  #define BLOCK_IO_MODE_DMA 0x02u 

  #if defined(BLOCK_IO_MODE_INTERRUPT)

    #undef BLOCK_IO_MODE_INTERRUPT

  #endif 

  #define BLOCK_IO_MODE_INTERRUPT 0x03u 

  #if defined(BLOCK_IO_CMD_SET_REQUEST)

    #undef BLOCK_IO_CMD_SET_REQUEST

  #endif 

  #define BLOCK_IO_CMD_SET_REQUEST 0x10u 

  #if defined(BLOCK_IO_CMD_GET_INFO)

    #undef BLOCK_IO_CMD_GET_INFO

  #endif 

  #define BLOCK_IO_CMD_GET_INFO 0x11u 

  typedef struct BlockIORequest_s {

    Byte_t command;         

    Byte_t operation; 

    Word_t blockNumber; 

    HalfWord_t blockCount; 

    HalfWord_t blockSize; 

    Byte_t transferMode; 

    Byte_t reserved; 

  } BlockIORequest_t;

  typedef struct BlockIOInfo_s {

    Byte_t command;              

    Word_t totalSizeBytes; 

    HalfWord_t nativeBlockSize; 

    Base_t supportsRandomAccess; 

    Base_t requiresErase; 

  } BlockIOInfo_t;

#endif 

