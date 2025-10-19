/*UNCRUSTIFY-OFF*/
/**
 * @file device.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for device I/O
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 * @details
 * HeliOS Device Driver Framework
 * ==============================
 *
 * HeliOS supports two types of device drivers:
 *
 * BLOCK DEVICES:
 *   - Use drivers/block/block_driver.c as an abstraction layer
 *   - Underlying I/O drivers implement block_io_interface.h
 *   - Examples: RAM disk, SD card, EEPROM, Flash memory
 *   - Addressed by block/sector number
 *   - Primary operations: read/write (buffer-based)
 *   - Block driver translates block operations to I/O driver commands
 *
 * CHARACTER DEVICES:
 *   - Register directly with the device framework (no abstraction layer)
 *   - Implement simple_read/simple_write for byte-stream operations
 *   - Examples: UART, USB CDC, console, pipes
 *   - Sequential byte access
 *   - Primary operations: simple_read/simple_write (byte-based)
 *   - May also implement read/write for buffered operations
 *
 * Device Function Pointers:
 *   init()         - Initialize device hardware and state
 *   config()       - Configure device settings or query capabilities
 *   read()         - Read buffer of data (returns kernel memory)
 *   write()        - Write buffer of data (accepts kernel memory)
 *   simple_read()  - Read single byte (character devices)
 *   simple_write() - Write single byte (character devices)
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
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"


  #if defined(CONCAT)
    #undef CONCAT
  #endif /* if defined(CONCAT) */
  #define CONCAT(a_, b_) a_ ## b_


  #if defined(QUOTE)
    #undef QUOTE
  #endif /* if defined(QUOTE) */
  #define QUOTE(a_) #a_


  #if defined(TO_FUNCTION)
    #undef TO_FUNCTION
  #endif /* if defined(TO_FUNCTION) */
  #define TO_FUNCTION(a_, b_) CONCAT(a_, b_)


  #if defined(TO_LITERAL)
    #undef TO_LITERAL
  #endif /* if defined(TO_LITERAL) */
  #define TO_LITERAL(a_) QUOTE(a_)


  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  Return_t xDeviceRegisterDevice(Return_t (*device_self_register_)());
  Return_t __RegisterDevice__(const HalfWord_t uid_, const Byte_t *name_, const DeviceState_t state_, const DeviceMode_t mode_, Return_t (*init_)(Device_t *
    device_), Return_t (*config_)(Device_t *device_, Size_t *size_, Addr_t *config_), Return_t (*read_)(Device_t *device_, Size_t *size_, Addr_t **data_),
    Return_t (*write_)(Device_t *device_, Size_t *size_, Addr_t *data_), Return_t (*simple_read_)(Device_t *device_, Byte_t *data_), Return_t (*simple_write_)(
    Device_t *device_, Byte_t data_));
  Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_);
  Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Byte_t data_);
  Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
  Return_t xDeviceSimpleRead(const HalfWord_t uid_, Byte_t *data_);
  Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);
  Return_t xDeviceInitDevice(const HalfWord_t uid_);
  Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);
  /* Internal kernel-level device APIs (for driver-to-driver communication) */
  Return_t __DeviceWrite__(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
  Return_t __DeviceRead__(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);
  Return_t __DeviceConfigDevice__(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);

  #if defined(POSIX_ARCH_OTHER)
    void __DeviceStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef DEVICE_H_ */