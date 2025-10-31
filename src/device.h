/*UNCRUSTIFY-OFF*/
/**
 * @file device.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Device driver subsystem API header
 * @details
 * Defines device structures, states, modes, and function prototypes for the unified device driver registration and management interface.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef DEVICE_H_

  #define DEVICE_H_

  #include "config.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "defines.h"

    #include "types.h"

    #include "console.h"

    #include "fat.h"

    #include "fs.h"

    #include "mem.h"

    #include "port.h"

    #include "posix.h"

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
    /**
     * @brief Registers a device driver with the system
     * @details Calls the device's self-registration function to add it to the
     * device list.
     *
     * @param[in] device_self_register_ Function pointer to device's
     *                                  self-registration function
     *
     * @return                          ReturnOK if device was registered
     *                                  successfully
     * @return                          ReturnError if registration failed or
     *                                  invalid parameter
     */
    Return_t xDeviceRegisterDevice(Return_t (*device_self_register_)());
    /**
     * @brief Internal device registration function
     * @details Allocates and initializes a device structure with all required
     * function pointers.
     *
     * @param[in] uid_          Unique device identifier
     * @param[in] name_         Device name string
     * @param[in] state_        Initial device state
     * @param[in] mode_         Device access mode
     * @param[in] init_         Device initialization function pointer
     * @param[in] config_       Device configuration function pointer
     * @param[in] read_         Device read function pointer
     * @param[in] write_        Device write function pointer
     * @param[in] simple_read_  Device simple read function pointer
     * @param[in] simple_write_ Device simple write function pointer
     *
     * @return                  ReturnOK if device was registered successfully
     * @return                  ReturnError if allocation failed or invalid
     *                          parameters
     *
     * @note This is an internal function called by device self-registration
     * routines
     */
    Return_t __RegisterDevice__(const HalfWord_t uid_, const Byte_t *name_, const DeviceState_t state_, const DeviceMode_t mode_, Return_t (*init_)(Device_t *

      device_), Return_t (*config_)(Device_t *device_, Size_t *size_, Addr_t *config_), Return_t (*read_)(Device_t *device_, Size_t *size_, Addr_t **data_),

      Return_t (*write_)(Device_t *device_, Size_t *size_, Addr_t *data_), Return_t (*simple_read_)(Device_t *device_, Byte_t *data_), Return_t (*simple_write_)

      (Device_t *device_, Byte_t data_));
    Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_);
    Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Byte_t data_);
    Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
    Return_t xDeviceSimpleRead(const HalfWord_t uid_, Byte_t *data_);
    Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);
    Return_t xDeviceInitDevice(const HalfWord_t uid_);
    Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);
    Return_t __DeviceListFind__(const HalfWord_t uid_, Device_t **device_);
    Return_t __DeviceWrite__(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
    Return_t __DeviceRead__(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);
    Return_t __DeviceConfigDevice__(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);

    #if defined(POSIX_ARCH_OTHER)
      void __DeviceStateClear__(void);

    #endif /* if defined(POSIX_ARCH_OTHER) */

    #ifdef __cplusplus

      }

    #endif /* ifdef __cplusplus */

  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

#endif /* ifndef DEVICE_H_ */