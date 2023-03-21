/*UNCRUSTIFY-OFF*/
/**
 * @file driver.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief A template for kernel mode device drivers
 * @version 0.4.1
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
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
 * IMPORTANT: THE DEVICE NAME MUST BE SUPPLIED HERE AND MUST BE EXACTLY
 * CONFIG_DEVICE_NAME_BYTES (DEFAULT IS 8) IN LENGTH, IT MAY BE NECESSARY TO PAD
 * SHORTER DEVICE NAMES. */
  #define DEVICE_NAME DEVICENA
  #define DEVICE_UID 0x00u
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning


/* END: DEVICE DRIVER PARAMATER BLOCK */


  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t * device_);
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t * device_, Size_t *size_, Addr_t *config_);
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t * device_, Size_t *size_, Addr_t **data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t * device_, Size_t *size_, Addr_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t * device_, Byte_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t * device_, Byte_t data_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef DRIVER_H_ */