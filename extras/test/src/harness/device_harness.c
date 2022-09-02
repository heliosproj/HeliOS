/**
 * @file device_harness.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
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

#include "device_harness.h"

xBase device_self_register(void);


void device_harness(void) {

  unit_begin("xDeviceRegisterDevice()");


  unit_try(RETURN_SUCCESS == xDeviceRegisterDevice(device_self_register));

  unit_end();

  return;
}


xBase device_self_register(void) {

  printf("DEVICE HERE!\n");
  return RETURN_SUCCESS;
}


