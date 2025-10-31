#ifndef DRIVER_H_

#define DRIVER_H_

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

#define DEVICE_NAME DEVICENA

#define DEVICE_UID 0x0u

#define DEVICE_MODE DeviceModeReadWrite

#define DEVICE_STATE DeviceStateRunning

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

#ifdef __cplusplus

  }

#endif // ifdef __cplusplus

#endif // ifndef DRIVER_H_