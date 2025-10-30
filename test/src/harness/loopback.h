/*UNCRUSTIFY-OFF*/
/**
 * @file loopback.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Loopback device driver for comprehensive device I/O testing
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef LOOPBACK_DRIVER_H_
  #define LOOPBACK_DRIVER_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"
  #include "../../drivers/block/block_io_interface.h"


  /* Device name must be exactly CONFIG_DEVICE_NAME_BYTES (8 bytes) */
  #define DEVICE_NAME LOOPBACK
  #define DEVICE_UID 0xFFu
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning


  /* Loopback buffer size - 1KB for testing */
  #define LOOPBACK_BUFFER_SIZE 0x400u /* 1024 bytes */
  /* Configuration commands - used with xDeviceConfigDevice() */
  #define LOOPBACK_CMD_SET_POSITION 0x01u /* 1 */
  #define LOOPBACK_CMD_CLEAR_BUFFER 0x02u /* 2 */
  #define LOOPBACK_CMD_GET_STATS 0x03u /* 3 */
  #define LOOPBACK_CMD_SET_MODE 0x04u /* 4 */
  /* Loopback modes */
  #define LOOPBACK_MODE_FIFO 0x00u /* 0 - FIFO mode (default) */
  #define LOOPBACK_MODE_ECHO 0x01u /* 1 - Echo mode (data immediately available)
                                    */
  /**
   * @brief Loopback position configuration
   *
   * Used with xDeviceConfigDevice() to set read/write position.
   */
  typedef struct LoopbackPositionConfig_s {
    Byte_t command;       /* LOOPBACK_CMD_SET_POSITION */
    HalfWord_t position; /* Byte offset to set */
  } LoopbackPositionConfig_t;


  /**
   * @brief Loopback clear configuration
   *
   * Used with xDeviceConfigDevice() to clear buffer contents.
   */
  typedef struct LoopbackClearConfig_s {
    Byte_t command;       /* LOOPBACK_CMD_CLEAR_BUFFER */
    Byte_t fillPattern; /* Pattern to fill with */
  } LoopbackClearConfig_t;


  /**
   * @brief Loopback mode configuration
   *
   * Used with xDeviceConfigDevice() to set operation mode.
   */
  typedef struct LoopbackModeConfig_s {
    Byte_t command;       /* LOOPBACK_CMD_SET_MODE */
    Byte_t mode; /* LOOPBACK_MODE_* constant */
  } LoopbackModeConfig_t;


  /**
   * @brief Loopback statistics structure
   *
   * Used with xDeviceConfigDevice() to get statistics.
   */
  typedef struct LoopbackStats_s {
    Byte_t command;           /* LOOPBACK_CMD_GET_STATS */
    HalfWord_t bufferSize; /* Total buffer size in bytes */
    HalfWord_t currentPosition; /* Current read/write position */
    HalfWord_t bytesAvailable; /* Bytes available to read */
    Word_t bytesRead; /* Total bytes read since init */
    Word_t bytesWritten; /* Total bytes written since init */
    Word_t readOperations; /* Number of read operations */
    Word_t writeOperations; /* Number of write operations */
    Byte_t mode; /* Current operation mode */
  } LoopbackStats_t;

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
    void __LoopbackStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef LOOPBACK_DRIVER_H_ */