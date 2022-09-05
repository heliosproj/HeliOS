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
Base_t xDeviceInitDevice(HalfWord_t uid_);
Base_t xDeviceConfigDevice(HalfWord_t uid_, Size_t *size_, Addr_t *config_);
Base_t xDeviceWrite(HalfWord_t uid_, Size_t *size_, Addr_t *data_);
Base_t xDeviceRead(HalfWord_t uid_, Size_t *size_, Addr_t *data_);
Base_t xDeviceSimpleWrite(HalfWord_t uid_, Word_t *data_);
Base_t xDeviceSimpleRead(HalfWord_t uid_, Word_t *data_);
Base_t xDeviceIsAvailable(HalfWord_t uid_);
Base_t __RegisterDevice__(HalfWord_t uid_,
                          const Char_t *name_,
                          DeviceState_t state_,
                          DeviceMode_t mode_,
                          Base_t (*init_)(Device_t *device_),
                          Base_t (*config_)(Device_t *device_, Size_t *size_, Addr_t *config_),
                          Base_t (*read_)(Device_t *device_, Size_t *size_, Addr_t *data_),
                          Base_t (*write_)(Device_t *device_, Size_t *size_, Addr_t *data_),
                          Base_t (*simple_read_)(Device_t *device_, Word_t *data_),
                          Base_t (*simple_write_)(Device_t *device_, Word_t *data_));

#if defined(POSIX_ARCH_OTHER)
void __DeviceStateClear__(void);
#endif

#ifdef __cplusplus
}
#endif
#endif