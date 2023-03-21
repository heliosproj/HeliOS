/*UNCRUSTIFY-OFF*/
/**
 * @file device.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for device I/O
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
  #endif /* ifdef __cplusplus */
  Return_t xDeviceRegisterDevice(Return_t (*device_self_register_)());
  Return_t __RegisterDevice__(const HalfWord_t uid_, const Byte_t *name_, const DeviceState_t state_, const DeviceMode_t mode_, Return_t (*init_)(
      Device_t *device_), Return_t (*config_)(Device_t *device_, Size_t *size_, Addr_t *config_), Return_t (*read_)(Device_t *device_, Size_t *size_,
    Addr_t **data_), Return_t (*write_)(Device_t *device_, Size_t *size_, Addr_t *data_), Return_t (*simple_read_)(Device_t *device_, Byte_t *data_),
    Return_t (*simple_write_)(Device_t *device_, Byte_t data_));
  Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_);
  Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Byte_t data_);
  Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
  Return_t xDeviceSimpleRead(const HalfWord_t uid_, Byte_t *data_);
  Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);
  Return_t xDeviceInitDevice(const HalfWord_t uid_);
  Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);

  #if defined(POSIX_ARCH_OTHER)
    void __DeviceStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef DEVICE_H_ */