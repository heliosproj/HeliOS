/*UNCRUSTIFY-OFF*/
/**
 * @file driver.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief A template for HeliOS device drivers
 * @version 0.4.0
 * @date 2022-09-02
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
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
/*UNCRUSTIFY-ON*/
#ifndef DRIVER_H_
  #define DRIVER_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "stream.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"


/* START: DEVICE DRIVER PARAMATER BLOCK
 *
 *  IMPORTANT: THE DEVICE NAME MUST BE SUPPLIED HERE AND MUST BE EXACTLY
 * CONFIG_DEVICE_NAME_BYTES (DEFAULT IS 8) IN LENGTH, IT MAY BE NECESSARY TO PAD
 * SHORTER DEVICE NAMES. */
  #define DEVICE_NAME NAMEHERE
  #define DEVICE_UID 0x01u
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning


/* END: DEVICE DRIVER PARAMATER BLOCK */


  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t * device_);
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t * device_, Size_t *size_, Addr_t *config_);
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t * device_, Size_t *size_, Addr_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t * device_, Size_t *size_, Addr_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t * device_, Word_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t * device_, Word_t *data_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef DRIVER_H_ */