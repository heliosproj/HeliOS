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

  #define DEVICE_NAME RAMDISK0

  #define DEVICE_UID 0x0100u

  #define DEVICE_MODE DeviceModeReadWrite

  #define DEVICE_STATE DeviceStateRunning

  #define RAMDISK_SIZE_BYTES 0x100000u 

  #define RAMDISK_CMD_SET_POSITION 0x01u 

  #define RAMDISK_CMD_CLEAR_DISK 0x02u 

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

  #if defined(POSIX_ARCH_OTHER)

    void __RAMDiskStateClear__(void);

  #endif 

  #ifdef __cplusplus

    }

  #endif 

#endif 

