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

#endif // if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

#include "mem.h"

#include "port.h"

#include "posix.h"

#include "queue.h"

#include "streams.h"

#include "sys.h"

#include "task.h"

#include "timer.h"

#include "block_io_interface.h"

#define DEVICE_NAME BLOCKDEV

#define DEVICE_UID 0x1000u

#define DEVICE_MODE DeviceModeReadWrite

#define DEVICE_STATE DeviceStateRunning

#define BLOCK_CMD_CONFIG 0x01u

#define BLOCK_CMD_SET_ADDRESS 0x02u

#define BLOCK_CMD_GET_INFO 0x03u

#define BLOCK_PROTOCOL_SD_CARD 0x01u

#define BLOCK_PROTOCOL_MMC 0x02u

#define BLOCK_PROTOCOL_EMMC 0x03u

#define BLOCK_PROTOCOL_RAW 0xFFu

#define BLOCK_CMD_READ_SINGLE 0x01u

#define BLOCK_CMD_READ_MULTIPLE 0x02u

#define BLOCK_CMD_WRITE_SINGLE 0x03u

#define BLOCK_CMD_WRITE_MULTIPLE 0x04u

#define BLOCK_DEFAULT_SECTOR_SIZE 512u

typedef struct BlockDeviceConfig_s {

  Byte_t command;

  HalfWord_t ioDriverUID;

  Byte_t protocol;

  HalfWord_t blockSize;

  Word_t totalBlocks;

} BlockDeviceConfig_t;

typedef struct BlockDeviceInfo_s {

  Byte_t command;

  HalfWord_t blockSize;

  Word_t totalBlocks;

  Word_t totalBytes;

  Byte_t protocol;

  Base_t isWriteProtected;

  Base_t isInitialized;

} BlockDeviceInfo_t;

#ifdef __cplusplus

  extern "C" {

#endif // ifdef __cplusplus

Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);

Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);

Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);

Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);

Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);

Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);

Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);

#if defined(POSIX_ARCH_OTHER)
  void __BlockDeviceStateClear__(void);

#endif // if defined(POSIX_ARCH_OTHER)

#ifdef __cplusplus

  }

#endif // ifdef __cplusplus

#endif // ifndef BLOCK_DRIVER_H_