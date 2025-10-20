/*UNCRUSTIFY-OFF*/
/**
 * @file char_driver.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Generic character device driver for HeliOS
 * @version 0.5.0
 * @date 2025-10-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef CHAR_DRIVER_H_
  #define CHAR_DRIVER_H_

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
  #include "char_io_interface.h"


  /* Device name must be exactly CONFIG_DEVICE_NAME_BYTES (8 bytes) */
  #define DEVICE_NAME CHARDEV0
  #define DEVICE_UID 0x2000u
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning


  /* Character device commands - for config() function discrimination */
  #define CHAR_CMD_CONFIG 0x01u /* 1 - Initial configuration */
  #define CHAR_CMD_SET_PARAMS 0x02u /* 2 - Set parameters for read/write */
  #define CHAR_CMD_GET_INFO 0x03u /* 3 - Query device info */
  /* Character device protocols - determines command sequences */
  #define CHAR_PROTOCOL_UART 0x01u /* 1 - Standard UART */
  #define CHAR_PROTOCOL_USART 0x02u /* 2 - USART (synchronous capable) */
  #define CHAR_PROTOCOL_USB_CDC 0x03u /* 3 - USB CDC (Virtual COM) */
  #define CHAR_PROTOCOL_RAW 0xFFu /* 255 - Direct I/O, no protocol */
  /* Line discipline modes */
  #define CHAR_LINE_RAW 0x00u /* 0 - Raw mode, no processing */
  #define CHAR_LINE_COOKED 0x01u /* 1 - Cooked mode, line buffering with echo */
  /* Default buffer sizes */
  #define CHAR_DEFAULT_RX_BUFFER_SIZE 256u
  #define CHAR_DEFAULT_TX_BUFFER_SIZE 256u


  /**
   * @brief Character device configuration structure
   *
   * Configures the character device with I/O driver details and protocol.
   * All hardware-specific details are handled by the I/O driver.
   */
  typedef struct CharDeviceConfig_s {
    Byte_t command;              /* CHAR_CMD_CONFIG */
    HalfWord_t ioDriverUID;      /* UID of I/O driver (USART/UART/etc.) */
    Byte_t protocol;             /* CHAR_PROTOCOL_* constant */
    Byte_t lineMode;             /* CHAR_LINE_* constant */
    Word_t baudRate;             /* Baud rate for UART/USART */
    HalfWord_t rxBufferSize;     /* Receive buffer size (0 = use default) */
    HalfWord_t txBufferSize;     /* Transmit buffer size (0 = use default) */
  } CharDeviceConfig_t;


  /**
   * @brief Character device command structure
   *
   * Specifies character-level operations. Used to set parameters before read/write.
   */
  typedef struct CharDeviceCommand_s {
    Byte_t command;              /* CHAR_CMD_SET_PARAMS */
    HalfWord_t byteCount;        /* Number of bytes for operation */
    Byte_t transferMode;         /* CHAR_IO_MODE_* constant */
  } CharDeviceCommand_t;


  /**
   * @brief Character device information structure
   *
   * Returns device capabilities and statistics.
   */
  typedef struct CharDeviceInfo_s {
    Byte_t command;              /* CHAR_CMD_GET_INFO */
    Byte_t protocol;             /* CHAR_PROTOCOL_* constant */
    Byte_t lineMode;             /* Current line discipline mode */
    Word_t baudRate;             /* Current baud rate */
    HalfWord_t rxBytesAvailable; /* Bytes available to read */
    HalfWord_t txBytesFree;      /* Free space in TX buffer */
    Base_t isInitialized;        /* Initialization status */
    Base_t isConnected;          /* Connection status (for USB CDC, etc.) */
  } CharDeviceInfo_t;

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
    void __CharDeviceStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef CHAR_DRIVER_H_ */
