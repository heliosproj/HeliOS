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



#ifdef __cplusplus
extern "C" {
#endif



Base_t xDeviceRegisterDevice(Base_t (*device_self_register_)());
Base_t __RegisterDevice__(HWord_t uid_,
                        const char *name_,
                        DeviceState_t state_,
                        DeviceMode_t mode_,
                        Byte_t (*init_)(struct Device_s *device_),
                        Byte_t (*config_)(struct Device_s *device_, void *config_),
                        Byte_t (*read_)(struct Device_s *device_, HWord_t *bytes_, void *data_),
                        Byte_t (*write_)(struct Device_s *device_, HWord_t *bytes_, void *data_));


Base_t xDeviceConfigDevice(HWord_t uid_, void *config_);
Base_t xDeviceRead(HWord_t uid_, HWord_t *bytes_, void *data_);
Base_t xDeviceWrite(HWord_t uid_, HWord_t *bytes_, void *data_);


#ifdef __cplusplus
}  // extern "C" {
#endif


#endif