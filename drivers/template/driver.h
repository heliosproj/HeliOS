/*UNCRUSTIFY-OFF*/
/**
 * @file driver.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Device driver template header
 * @details
 * Template header defining the standard device driver interface with stub function prototypes for creating new device drivers.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef DRIVER_H_

  #define DRIVER_H_

  #include "config.h"

  #include "defines.h"

  #include "types.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "console.h"

    #include "device.h"

    #include "fat.h"

    #include "fs.h"

  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

  #include "mem.h"

  #include "port.h"

  #include "posix.h"

  #include "queue.h"

  #include "streams.h"

  #include "sys.h"

  #include "task.h"

  #include "timer.h"

  #define DEVICE_NAME DEVICENA

  #define DEVICE_UID 0x0u

  #define DEVICE_MODE DeviceModeReadWrite

  #define DEVICE_STATE DeviceStateRunning

  #ifdef __cplusplus

    extern "C" {

  #endif /* ifdef __cplusplus */
  /**
   * @brief Self-registers device driver
   * @details Template function for registering the device driver with the
   * device manager during system initialization.
   *
   * @return ReturnOK if registration was successful
   * @return ReturnError if registration failed
   *
   * @note Implement this function to call __RegisterDevice__ with appropriate
   * parameters
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);

  /**
   * @brief Initializes device driver
   * @details Template function for initializing internal state and preparing
   * the device for operation.
   *
   * @param[in,out] device_ Pointer to device structure
   *
   * @return                ReturnOK if initialization was successful
   * @return                ReturnError if initialization failed
   *
   * @note Implement device-specific initialization logic here
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);

  /**
   * @brief Configures device
   * @details Template function for configuring device parameters.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in,out] size_   Pointer to configuration data size
   * @param[in]     config_ Pointer to configuration data
   *
   * @return                ReturnOK if configuration was successful
   * @return                ReturnError if configuration failed or invalid
   *                        parameters
   *
   * @note Implement device-specific configuration logic here
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);

  /**
   * @brief Reads data from device
   * @details Template function for reading data from the device, allocates
   * memory for the data.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in,out] size_   Pointer to size of data to read (updated with actual
   *                        bytes read)
   * @param[out]    data_   Pointer to store allocated data buffer
   *
   * @return                ReturnOK if read was successful
   * @return                ReturnError if read failed or allocation failed
   *
   * @warning Caller is responsible for freeing the allocated data buffer
   * @note Implement device-specific read logic here
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);

  /**
   * @brief Writes data to device
   * @details Template function for writing data to the device.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in,out] size_   Pointer to size of data to write (updated with
   *                        actual bytes written)
   * @param[in]     data_   Pointer to data buffer to write
   *
   * @return                ReturnOK if write was successful
   * @return                ReturnError if write failed or invalid parameters
   *
   * @note Implement device-specific write logic here
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);

  /**
   * @brief Reads single byte from device
   * @details Template function for reading one byte from the device.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[out]    data_   Pointer to store the read byte
   *
   * @return                ReturnOK if read was successful
   * @return                ReturnError if read failed or invalid parameters
   *
   * @note Implement device-specific simple read logic here
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);

  /**
   * @brief Writes single byte to device
   * @details Template function for writing one byte to the device.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in]     data_   Byte to write
   *
   * @return                ReturnOK if write was successful
   * @return                ReturnError if write failed
   *
   * @note Implement device-specific simple write logic here
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);

  #ifdef __cplusplus

    }

  #endif /* ifdef __cplusplus */

#endif /* ifndef DRIVER_H_ */