/*UNCRUSTIFY-OFF*/
/**
 * @file ramdisk_driver.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief RAM disk driver for testing and simulation
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
#ifndef RAMDISK_DRIVER_H_
  #define RAMDISK_DRIVER_H_

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
  #include "../block/block_io_interface.h"


  /* Device name must be exactly CONFIG_DEVICE_NAME_BYTES (8 bytes) */
  #define DEVICE_NAME RAMDISK0
  #define DEVICE_UID 0x0100u
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning


  /* RAM disk size - 1MB for testing */
  #define RAMDISK_SIZE_BYTES 0x100000u /* 1048576 (1MB) */
  /* Configuration commands - used with xDeviceConfigDevice() */
  #define RAMDISK_CMD_SET_POSITION 0x01u /* 1 */
  #define RAMDISK_CMD_CLEAR_DISK 0x02u /* 2 */
  #define RAMDISK_CMD_GET_STATS 0x03u /* 3 */


  /**
   * @brief RAM disk position configuration
   *
   * Used with xDeviceConfigDevice() to set read/write position.
   */
  typedef struct RAMDiskPositionConfig_s {
    Byte_t command;            /* RAMDISK_CMD_SET_POSITION */
    Word_t position; /* Byte offset to set */
  } RAMDiskPositionConfig_t;


  /**
   * @brief RAM disk clear configuration
   *
   * Used with xDeviceConfigDevice() to clear disk contents.
   */
  typedef struct RAMDiskClearConfig_s {
    Byte_t command;            /* RAMDISK_CMD_CLEAR_DISK */
    Byte_t fillPattern; /* Pattern to fill with */
  } RAMDiskClearConfig_t;


  /**
   * @brief RAM disk statistics structure
   *
   * Used with xDeviceConfigDevice() to get statistics. Call with command =
   * RAMDISK_CMD_GET_STATS, then read back.
   */
  typedef struct RAMDiskStats_s {
    Byte_t command;            /* RAMDISK_CMD_GET_STATS */
    Word_t totalSize; /* Total size in bytes */
    Word_t currentPosition; /* Current read/write position */
    Word_t bytesRead; /* Total bytes read since init */
    Word_t bytesWritten; /* Total bytes written since init */
    Word_t readOperations; /* Number of read operations */
    Word_t writeOperations; /* Number of write operations */
  } RAMDiskStats_t;

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
  #if defined(POSIX_ARCH_OTHER)
    void __RAMDiskStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef RAMDISK_DRIVER_H_ */