/*UNCRUSTIFY-OFF*/
/**
 * @file block_io_interface.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Block I/O interface definitions
 * @details
 * Defines standard interface structures and command constants for low-level block I/O drivers including transfer modes and device information.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/


#ifndef BLOCK_IO_INTERFACE_H_


  #define BLOCK_IO_INTERFACE_H_


  #include "types.h"


  #if defined(BLOCK_IO_OP_READ)


    #undef BLOCK_IO_OP_READ


  #endif /* if defined(BLOCK_IO_OP_READ) */


  #define BLOCK_IO_OP_READ 0x01u


  #if defined(BLOCK_IO_OP_WRITE)


    #undef BLOCK_IO_OP_WRITE


  #endif /* if defined(BLOCK_IO_OP_WRITE) */


  #define BLOCK_IO_OP_WRITE 0x02u


  #if defined(BLOCK_IO_MODE_BLOCKING)


    #undef BLOCK_IO_MODE_BLOCKING


  #endif /* if defined(BLOCK_IO_MODE_BLOCKING) */


  #define BLOCK_IO_MODE_BLOCKING 0x00u


  #if defined(BLOCK_IO_MODE_NONBLOCKING)


    #undef BLOCK_IO_MODE_NONBLOCKING


  #endif /* if defined(BLOCK_IO_MODE_NONBLOCKING) */


  #define BLOCK_IO_MODE_NONBLOCKING 0x01u


  #if defined(BLOCK_IO_MODE_DMA)


    #undef BLOCK_IO_MODE_DMA


  #endif /* if defined(BLOCK_IO_MODE_DMA) */


  #define BLOCK_IO_MODE_DMA 0x02u


  #if defined(BLOCK_IO_MODE_INTERRUPT)


    #undef BLOCK_IO_MODE_INTERRUPT


  #endif /* if defined(BLOCK_IO_MODE_INTERRUPT) */


  #define BLOCK_IO_MODE_INTERRUPT 0x03u


  #if defined(BLOCK_IO_CMD_SET_REQUEST)


    #undef BLOCK_IO_CMD_SET_REQUEST


  #endif /* if defined(BLOCK_IO_CMD_SET_REQUEST) */


  #define BLOCK_IO_CMD_SET_REQUEST 0x10u


  #if defined(BLOCK_IO_CMD_GET_INFO)


    #undef BLOCK_IO_CMD_GET_INFO


  #endif /* if defined(BLOCK_IO_CMD_GET_INFO) */


  #define BLOCK_IO_CMD_GET_INFO 0x11u


  /**
   * @brief Block I/O request structure
   * @details Command structure for block device I/O operations specifying the operation, blocks, and transfer mode.
   */
  typedef struct BlockIORequest_s {


    Byte_t command; /**< Command type identifier */
    Byte_t operation; /**< Operation (read or write) */
    Word_t blockNumber; /**< Starting block number */
    HalfWord_t blockCount; /**< Number of blocks to transfer */
    HalfWord_t blockSize; /**< Size of each block in bytes */
    Byte_t transferMode; /**< Transfer mode (blocking, DMA, interrupt, etc.) */
    Byte_t reserved; /**< Reserved for future use */
  } BlockIORequest_t;


  /**
   * @brief Block I/O device information structure
   * @details Contains device capabilities and characteristics for block I/O devices.
   */
  typedef struct BlockIOInfo_s {


    Byte_t command; /**< Command type identifier */
    Word_t totalSizeBytes; /**< Total device capacity in bytes */
    HalfWord_t nativeBlockSize; /**< Native block size in bytes */
    Base_t supportsRandomAccess; /**< Flag indicating if device supports random access */
    Base_t requiresErase; /**< Flag indicating if device requires erase before write */
  } BlockIOInfo_t;


#endif /* ifndef BLOCK_IO_INTERFACE_H_ */