/*UNCRUSTIFY-OFF*/
/**
 * @file block_driver.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Generic block device driver for HeliOS
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef BLOCK_DRIVER_H_
  #define BLOCK_DRIVER_H_

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
  #include "block_io_interface.h"


  /* Device name must be exactly CONFIG_DEVICE_NAME_BYTES (8 bytes) */
  #define DEVICE_NAME BLOCKDEV
  #define DEVICE_UID 0x1000u
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning


  /* Block device commands - for config() function discrimination */
  #define BLOCK_CMD_CONFIG 0x01u /* 1 - Initial configuration */
  #define BLOCK_CMD_SET_ADDRESS 0x02u /* 2 - Set addressing for read/write */
  #define BLOCK_CMD_GET_INFO 0x03u /* 3 - Query device info */
  /* Storage device protocols - determines command sequences */
  #define BLOCK_PROTOCOL_SD_CARD 0x01u /* 1 */
  #define BLOCK_PROTOCOL_MMC 0x02u /* 2 */
  #define BLOCK_PROTOCOL_EMMC 0x03u /* 3 */
  #define BLOCK_PROTOCOL_RAW 0xFFu /* 255 - Direct I/O, no protocol */
  /* Block operation commands - internal use */
  #define BLOCK_CMD_READ_SINGLE 0x01u /* 1 */
  #define BLOCK_CMD_READ_MULTIPLE 0x02u /* 2 */
  #define BLOCK_CMD_WRITE_SINGLE 0x03u /* 3 */
  #define BLOCK_CMD_WRITE_MULTIPLE 0x04u /* 4 */
  /* Default block size - standard disk sector size */
  #define BLOCK_DEFAULT_SECTOR_SIZE 512u


  /**
   * @brief Block device configuration structure
   *
   * Configures the block device with I/O driver details and storage protocol.
   * All hardware-specific details are handled by the I/O driver.
   */
  typedef struct BlockDeviceConfig_s {
    Byte_t command;                  /* BLOCK_CMD_CONFIG */
    HalfWord_t ioDriverUID; /* UID of I/O driver (SPI/I2C/etc.) */
    Byte_t protocol; /* BLOCK_PROTOCOL_* constant */
    HalfWord_t blockSize; /* Block/sector size (typically 512) */
    Word_t totalBlocks; /* Total capacity in blocks (0 = auto-detect) */
  } BlockDeviceConfig_t;


  /**
   * @brief Block device information structure
   *
   * Returns device capabilities and statistics.
   */
  typedef struct BlockDeviceInfo_s {
    Byte_t command;                   /* BLOCK_CMD_GET_INFO */
    HalfWord_t blockSize; /* Block/sector size in bytes */
    Word_t totalBlocks; /* Total capacity in blocks */
    Word_t totalBytes; /* Total capacity in bytes */
    Byte_t protocol; /* BLOCK_PROTOCOL_* constant */
    Base_t isWriteProtected; /* Write protection status */
    Base_t isInitialized; /* Initialization status */
  } BlockDeviceInfo_t;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  /* Driver interface functions */
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);

  #if defined(POSIX_ARCH_OTHER)
    void __BlockDeviceStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef BLOCK_DRIVER_H_ */