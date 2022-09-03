/**
 * @file driver.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Template for a HeliOS device driver.
 * @version 0.3.5
 * @date 2022-09-02
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

#include "driver.h"

Base_t device_self_register(void) {
  Base_t ret = RETURN_FAILURE;

  ret = __RegisterDevice__(0x1u, "TEMPLATE", DeviceStateRunning, DeviceModeReadWrite, device_init, device_config, device_read, device_write);

  return ret;
}



Base_t device_init(Device_t *device_) {
  Base_t ret = RETURN_FAILURE;


  return ret;
}



Base_t device_config(Device_t *device_, void *config_) {
  Base_t ret = RETURN_FAILURE;


  return ret;
}



Base_t device_read(Device_t *device_, HWord_t *bytes_, void *data_) {

  Base_t ret = RETURN_FAILURE;


  return ret;
}



Base_t device_write(Device_t *device_, HWord_t *bytes_, void *data_) {
  Base_t ret = RETURN_FAILURE;


  return ret;
}