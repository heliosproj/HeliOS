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
  #endif
  #include "mem.h"
  #include "port.h"
  #include "posix.h"
  #include "queue.h"
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"
  #include "../block/block_io_interface.h"
  #if defined(DEVICE_NAME)
    #undef DEVICE_NAME
  #endif
  #define DEVICE_NAME RAMDISK0
  #if defined(DEVICE_UID)
    #undef DEVICE_UID
  #endif
  #define DEVICE_UID 0x0100u
  #if defined(DEVICE_MODE)
    #undef DEVICE_MODE
  #endif
  #define DEVICE_MODE DeviceModeReadWrite
  #if defined(DEVICE_STATE)
    #undef DEVICE_STATE
  #endif
  #define DEVICE_STATE DeviceStateRunning
  #if defined(RAMDISK_CMD_SET_POSITION)
    #undef RAMDISK_CMD_SET_POSITION
  #endif
  #define RAMDISK_CMD_SET_POSITION 0x01u
  #if defined(RAMDISK_CMD_CLEAR_DISK)
    #undef RAMDISK_CMD_CLEAR_DISK
  #endif
  #define RAMDISK_CMD_CLEAR_DISK 0x02u
  #if defined(RAMDISK_CMD_GET_STATS)
    #undef RAMDISK_CMD_GET_STATS
  #endif
  #define RAMDISK_CMD_GET_STATS 0x03u
  typedef struct RAMDiskPositionConfig_s {
    Byte_t command;
    Word_t position;
  } RAMDiskPositionConfig_t;
  typedef struct RAMDiskClearConfig_s {
    Byte_t command;
    Byte_t fillPattern;
  } RAMDiskClearConfig_t;
  typedef struct RAMDiskStats_s {
    Byte_t command;
    Word_t totalSize;
    Word_t currentPosition;
    Word_t bytesRead;
    Word_t bytesWritten;
    Word_t readOperations;
    Word_t writeOperations;
  } RAMDiskStats_t;
typedef struct RAMDiskState_s {
  Word_t currentPosition;
  Word_t bytesRead;
  Word_t bytesWritten;
  Word_t readOperations;
  Word_t writeOperations;
  Base_t initialized;
} RAMDiskState_t;
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
Return_t __ValidateAndTruncateSize__(Size_t requested_, Size_t *actual_);
  #if defined(POSIX_ARCH_OTHER)
    void __RAMDiskStateClear__(void);
  #endif
  #ifdef __cplusplus
    }
  #endif
#endif