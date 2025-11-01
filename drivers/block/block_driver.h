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
  #endif
  #include "mem.h"
  #include "port.h"
  #include "posix.h"
  #include "queue.h"
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"
  #include "block_io_interface.h"
  #if defined(DEVICE_NAME)
    #undef DEVICE_NAME
  #endif
  #define DEVICE_NAME BLOCKDEV
  #if defined(DEVICE_UID)
    #undef DEVICE_UID
  #endif
  #define DEVICE_UID 0x1000u
  #if defined(DEVICE_MODE)
    #undef DEVICE_MODE
  #endif
  #define DEVICE_MODE DeviceModeReadWrite
  #if defined(DEVICE_STATE)
    #undef DEVICE_STATE
  #endif
  #define DEVICE_STATE DeviceStateRunning
  #if defined(BLOCK_CMD_CONFIG)
    #undef BLOCK_CMD_CONFIG
  #endif
  #define BLOCK_CMD_CONFIG 0x01u
  #if defined(BLOCK_CMD_SET_ADDRESS)
    #undef BLOCK_CMD_SET_ADDRESS
  #endif
  #define BLOCK_CMD_SET_ADDRESS 0x02u
  #if defined(BLOCK_CMD_GET_INFO)
    #undef BLOCK_CMD_GET_INFO
  #endif
  #define BLOCK_CMD_GET_INFO 0x03u
  #if defined(BLOCK_PROTOCOL_SD_CARD)
    #undef BLOCK_PROTOCOL_SD_CARD
  #endif
  #define BLOCK_PROTOCOL_SD_CARD 0x01u
  #if defined(BLOCK_PROTOCOL_MMC)
    #undef BLOCK_PROTOCOL_MMC
  #endif
  #define BLOCK_PROTOCOL_MMC 0x02u
  #if defined(BLOCK_PROTOCOL_EMMC)
    #undef BLOCK_PROTOCOL_EMMC
  #endif
  #define BLOCK_PROTOCOL_EMMC 0x03u
  #if defined(BLOCK_PROTOCOL_RAW)
    #undef BLOCK_PROTOCOL_RAW
  #endif
  #define BLOCK_PROTOCOL_RAW 0xFFu
  #if defined(BLOCK_CMD_READ_SINGLE)
    #undef BLOCK_CMD_READ_SINGLE
  #endif
  #define BLOCK_CMD_READ_SINGLE 0x01u
  #if defined(BLOCK_CMD_READ_MULTIPLE)
    #undef BLOCK_CMD_READ_MULTIPLE
  #endif
  #define BLOCK_CMD_READ_MULTIPLE 0x02u
  #if defined(BLOCK_CMD_WRITE_SINGLE)
    #undef BLOCK_CMD_WRITE_SINGLE
  #endif
  #define BLOCK_CMD_WRITE_SINGLE 0x03u
  #if defined(BLOCK_CMD_WRITE_MULTIPLE)
    #undef BLOCK_CMD_WRITE_MULTIPLE
  #endif
  #define BLOCK_CMD_WRITE_MULTIPLE 0x04u
  typedef struct BlockDeviceState_s {
  HalfWord_t ioDriverUID;
  Byte_t protocol;
  HalfWord_t blockSize;
  Word_t totalBlocks;
  Base_t initialized;
  Word_t currentBlockNumber;
  HalfWord_t currentBlockCount;
  Byte_t currentTransferMode;
} BlockDeviceState_t;
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
  #endif
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);
  Return_t __PrepareBlockIORequest__(const Byte_t operation_, BlockIORequest_t **request_, Size_t *configSize_);
  Return_t __BlockDeviceReadBlockRAW__(Byte_t **data_);
  Return_t __BlockDeviceWriteBlockRAW__(const Byte_t *data_);
  #if defined(POSIX_ARCH_OTHER)
    void __BlockDeviceStateClear__(void);
  #endif
  #ifdef __cplusplus
    }
  #endif
#endif