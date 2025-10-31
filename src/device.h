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
    /**
     * @brief Checks if a device is available for I/O
     * @details Returns true if the device is registered and ready for
     * operations.
     *
     * @param[in]  uid_ Device unique identifier
     * @param[out] res_ Pointer to store the result (true if available, false
     *                  otherwise)
     *
     * @return          ReturnOK if check was successful
     * @return          ReturnError if device not found or invalid parameter
     */
    Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_);
    /**
     * @brief Writes a single byte to a device
     * @details Simple write operation for byte-oriented devices.
     *
     * @param[in] uid_  Device unique identifier
     * @param[in] data_ Byte value to write
     *
     * @return          ReturnOK if write was successful
     * @return          ReturnError if device not found, unavailable, or write
     *                  failed
     */
    Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Byte_t data_);
    /**
     * @brief Writes data to a device
     * @details Block write operation for transferring multiple bytes.
     *
     * @param[in]     uid_  Device unique identifier
     * @param[in,out] size_ Pointer to size of data to write (may be updated by
     *                      driver)
     * @param[in]     data_ Pointer to data buffer
     *
     * @return              ReturnOK if write was successful
     * @return              ReturnError if device not found, unavailable, or
     *                      write failed
     */
    Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
    /**
     * @brief Reads a single byte from a device
     * @details Simple read operation for byte-oriented devices.
     *
     * @param[in]  uid_  Device unique identifier
     * @param[out] data_ Pointer to store the read byte
     *
     * @return           ReturnOK if read was successful
     * @return           ReturnError if device not found, unavailable, or read
     *                   failed
     */
    Return_t xDeviceSimpleRead(const HalfWord_t uid_, Byte_t *data_);
    /**
     * @brief Reads data from a device
     * @details Block read operation for receiving multiple bytes.
     *
     * @param[in]     uid_  Device unique identifier
     * @param[in,out] size_ Pointer to size of data to read (may be updated by
     *                      driver)
     * @param[out]    data_ Pointer to store data buffer pointer
     *
     * @return              ReturnOK if read was successful
     * @return              ReturnError if device not found, unavailable, or
     *                      read failed
     */
    Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);
    /**
     * @brief Initializes a device
     * @details Calls the device's initialization function to prepare it for
     * operation.
     *
     * @param[in] uid_ Device unique identifier
     *
     * @return         ReturnOK if initialization was successful
     * @return         ReturnError if device not found or initialization failed
     */
    Return_t xDeviceInitDevice(const HalfWord_t uid_);
    /**
     * @brief Configures a device
     * @details Sends configuration data to the device.
     *
     * @param[in]     uid_    Device unique identifier
     * @param[in,out] size_   Pointer to size of configuration data
     * @param[in]     config_ Pointer to configuration data
     *
     * @return                ReturnOK if configuration was successful
     * @return                ReturnError if device not found or configuration
     *                        failed
     */
    Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);
    /**
     * @brief Finds a device in the device list
     * @details Internal function to locate a device structure by UID.
     *
     * @param[in]  uid_    Device unique identifier
     * @param[out] device_ Pointer to store the found device structure pointer
     *
     * @return             ReturnOK if device was found
     * @return             ReturnError if device not found or invalid parameter
     *
     * @note This is an internal function for device subsystem use
     */
    Return_t __DeviceListFind__(const HalfWord_t uid_, Device_t **device_);
    /**
     * @brief Internal device write function
     * @details Lower-level write operation called by xDeviceWrite.
     *
     * @param[in]     uid_  Device unique identifier
     * @param[in,out] size_ Pointer to size of data to write
     * @param[in]     data_ Pointer to data buffer
     *
     * @return              ReturnOK if write was successful
     * @return              ReturnError if write failed
     *
     * @note This is an internal function for device subsystem use
     */
    Return_t __DeviceWrite__(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
    /**
     * @brief Internal device read function
     * @details Lower-level read operation called by xDeviceRead.
     *
     * @param[in]     uid_  Device unique identifier
     * @param[in,out] size_ Pointer to size of data to read
     * @param[out]    data_ Pointer to store data buffer pointer
     *
     * @return              ReturnOK if read was successful
     * @return              ReturnError if read failed
     *
     * @note This is an internal function for device subsystem use
     */
    Return_t __DeviceRead__(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);
    /**
     * @brief Internal device configuration function
     * @details Lower-level configuration operation called by
     * xDeviceConfigDevice.
     *
     * @param[in]     uid_    Device unique identifier
     * @param[in,out] size_   Pointer to size of configuration data
     * @param[in]     config_ Pointer to configuration data
     *
     * @return                ReturnOK if configuration was successful
     * @return                ReturnError if configuration failed
     *
     * @note This is an internal function for device subsystem use
     */
    Return_t __DeviceConfigDevice__(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);

    #if defined(POSIX_ARCH_OTHER)
      void __DeviceStateClear__(void);

    #endif /* if defined(POSIX_ARCH_OTHER) */

    #ifdef __cplusplus

      }

    #endif /* ifdef __cplusplus */

  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

#endif /* ifndef DEVICE_H_ */