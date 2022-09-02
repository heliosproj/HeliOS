/**
 * @file device.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel source code for device drivers
 * @version 0.3.5
 * @date 2022-09-01
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef DEVICE_H_
#define DEVICE_H_

#include "config.h"
#include "defines.h"
#include "types.h"
#include "port.h"
#include "mem.h"
#include "queue.h"
#include "stream.h"
#include "sys.h"
#include "task.h"
#include "timer.h"

#if !defined(CONFIG_DEVICE_NAME_BYTES)
#define CONFIG_DEVICE_NAME_BYTES 0x8u /* 8 */
#endif

typedef enum {
  DeviceStateError,
  DeviceStateSuspended,
  DeviceStateRunning
} DeviceState_t;

typedef enum {
  DeviceModeReadOnly,
  DeviceModeWriteOnly,
  DeviceModeReadWrite
} DeviceMode_t;

typedef struct Device_s {
  HWord_t uid;
  char name[CONFIG_DEVICE_NAME_BYTES];
  DeviceState_t state;
  DeviceMode_t mode;
  Word_t bytesWritten;
  Word_t bytesRead;
  Byte_t (*init)(struct Device_s *device_);
  Byte_t (*config)(struct Device_s *device_, void *config_);
  Byte_t (*read)(struct Device_s *device_, HWord_t *bytes_, void *data_);
  Byte_t (*write)(struct Device_s *device_, HWord_t *bytes_, void *data_);
} Device_t;

typedef Device_t *xDevice;

/*

  The only parameters the kernel will respect are state and mode,
  everything else is up to the device driver.

  xDeviceInitDevice()
  xDeviceConfigDevice()
  xDeviceRead()
  xDeviceWrite()


*/

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}  // extern "C" {
#endif


#endif