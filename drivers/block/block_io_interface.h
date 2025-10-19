/*UNCRUSTIFY-OFF*/
/**
 * @file block_io_interface.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Common interface for block I/O drivers
 * @version 0.5.0
 * @date 2025-10-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * @details
 * This header defines a standard interface for communication between block
 * device drivers and underlying I/O drivers (RAM disk, SPI-based SD cards,
 * I2C EEPROMs, etc.). This abstraction allows the block driver to remain
 * hardware-agnostic while I/O drivers handle hardware-specific translation.
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef BLOCK_IO_INTERFACE_H_
  #define BLOCK_IO_INTERFACE_H_

  #include "types.h"


  /* Block I/O operation types */
  #if defined(BLOCK_IO_OP_READ)
    #undef BLOCK_IO_OP_READ
  #endif /* if defined(BLOCK_IO_OP_READ) */
  #define BLOCK_IO_OP_READ 0x01u /* 1 */

  #if defined(BLOCK_IO_OP_WRITE)
    #undef BLOCK_IO_OP_WRITE
  #endif /* if defined(BLOCK_IO_OP_WRITE) */
  #define BLOCK_IO_OP_WRITE 0x02u /* 2 */
  /* Generic configuration commands for block I/O drivers */
  #if defined(BLOCK_IO_CMD_SET_REQUEST)
    #undef BLOCK_IO_CMD_SET_REQUEST
  #endif /* if defined(BLOCK_IO_CMD_SET_REQUEST) */
  #define BLOCK_IO_CMD_SET_REQUEST 0x10u /* 16 */

  #if defined(BLOCK_IO_CMD_GET_INFO)
    #undef BLOCK_IO_CMD_GET_INFO
  #endif /* if defined(BLOCK_IO_CMD_GET_INFO) */
  #define BLOCK_IO_CMD_GET_INFO 0x11u /* 17 */


  /**
   * @brief Generic block I/O request structure
   *
   * Used by block drivers to communicate block-level operations to I/O drivers.
   * Each I/O driver translates this to its native addressing/command format:
   * - RAM disk: converts blockNumber * blockSize to byte offset
   * - SD card: uses blockNumber directly as sector address
   * - I2C EEPROM: translates to page-aligned addresses
   *
   * Usage: Set command to BLOCK_IO_CMD_SET_REQUEST and pass to I/O driver's
   * config function before calling read/write.
   */
  typedef struct BlockIORequest_s {
    Byte_t command;         /* BLOCK_IO_CMD_SET_REQUEST */
    Byte_t operation; /* BLOCK_IO_OP_READ or BLOCK_IO_OP_WRITE */
    Word_t blockNumber; /* Starting block number */
    HalfWord_t blockCount; /* Number of blocks */
    HalfWord_t blockSize; /* Bytes per block */
  } BlockIORequest_t;


  /**
   * @brief Block I/O driver information structure
   *
   * Returned by I/O drivers to inform block driver of capabilities and
   * characteristics. Used to determine optimal block sizes and validate
   * operations.
   *
   * Usage: Set command to BLOCK_IO_CMD_GET_INFO and pass to I/O driver's config
   * function. The driver fills in the remaining fields.
   */
  typedef struct BlockIOInfo_s {
    Byte_t command;              /* BLOCK_IO_CMD_GET_INFO */
    Word_t totalSizeBytes; /* Total capacity in bytes */
    HalfWord_t nativeBlockSize; /* Native block/sector/page size */
    Base_t supportsRandomAccess; /* true for RAM/Flash, false for sequential */
    Base_t requiresErase; /* true for Flash, false for RAM/EEPROM */
  } BlockIOInfo_t;

#endif /* ifndef BLOCK_IO_INTERFACE_H_ */