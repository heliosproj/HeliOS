/*UNCRUSTIFY-OFF*/
/**
 * @file block_driver.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Block device driver header
 * @details
 * Defines structures, commands, and function prototypes for block device drivers supporting SD cards, MMC, and eMMC storage with sector-based I/O.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef BLOCK_DRIVER_H_
  #define BLOCK_DRIVER_H_
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
  #include "block_io_interface.h"
  #define DEVICE_NAME BLOCKDEV
  #define DEVICE_UID 0x1000u
  #define DEVICE_MODE DeviceModeReadWrite
  #define DEVICE_STATE DeviceStateRunning
  #define BLOCK_CMD_CONFIG 0x01u
  #define BLOCK_CMD_SET_ADDRESS 0x02u
  #define BLOCK_CMD_GET_INFO 0x03u
  #define BLOCK_PROTOCOL_SD_CARD 0x01u
  #define BLOCK_PROTOCOL_MMC 0x02u
  #define BLOCK_PROTOCOL_EMMC 0x03u
  #define BLOCK_PROTOCOL_RAW 0xFFu
  #define BLOCK_CMD_READ_SINGLE 0x01u
  #define BLOCK_CMD_READ_MULTIPLE 0x02u
  #define BLOCK_CMD_WRITE_SINGLE 0x03u
  #define BLOCK_CMD_WRITE_MULTIPLE 0x04u
  #define BLOCK_DEFAULT_SECTOR_SIZE 512u
  /**
   * @brief Block device configuration structure
   * @details Configuration command structure for initializing block device
   * parameters including protocol and geometry.
   */
  typedef struct BlockDeviceConfig_s {
    Byte_t command; /**< Command type identifier (BLOCK_CMD_CONFIG) */
    HalfWord_t ioDriverUID; /**< UID of underlying I/O driver */
    Byte_t protocol; /**< Storage protocol (SD card, MMC, eMMC, or raw) */
    HalfWord_t blockSize; /**< Block size in bytes */
    Word_t totalBlocks; /**< Total number of blocks on device */
  } BlockDeviceConfig_t;
  /**
   * @brief Block device information structure
   * @details Returned information about block device capabilities, geometry,
   * and current state.
   */
  typedef struct BlockDeviceInfo_s {
    Byte_t command; /**< Command type identifier (BLOCK_CMD_GET_INFO) */
    HalfWord_t blockSize; /**< Block size in bytes */
    Word_t totalBlocks; /**< Total number of blocks on device */
    Word_t totalBytes; /**< Total device capacity in bytes */
    Byte_t protocol; /**< Storage protocol in use */
    Base_t isWriteProtected; /**< Flag indicating if device is write-protected
                              */
    Base_t isInitialized; /**< Flag indicating if device is initialized */
  } BlockDeviceInfo_t;
  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  /**
   * @brief Self-registers block device driver
   * @details Registers the block device driver with the device manager during
   * system initialization.
   *
   * @return ReturnOK if registration was successful
   * @return ReturnError if registration failed
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);
  /**
   * @brief Initializes block device driver
   * @details Initializes internal state and prepares the block device for
   * operation.
   *
   * @param[in,out] device_ Pointer to device structure
   *
   * @return                ReturnOK if initialization was successful
   * @return                ReturnError if initialization failed
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);
  /**
   * @brief Configures block device
   * @details Configures block device parameters, sets address for I/O
   * operations, or retrieves device information.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in,out] size_   Pointer to configuration data size
   * @param[in]     config_ Pointer to configuration data (BlockDeviceConfig_t,
   *                        BlockIORequest_t, or BlockDeviceInfo_t)
   *
   * @return                ReturnOK if configuration was successful
   * @return                ReturnError if configuration failed or invalid
   *                        parameters
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);
  /**
   * @brief Reads data from block device
   * @details Reads data from the current address position, allocates memory for
   * the data.
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
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);
  /**
   * @brief Writes data to block device
   * @details Writes data to the current address position.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in,out] size_   Pointer to size of data to write (updated with
   *                        actual bytes written)
   * @param[in]     data_   Pointer to data buffer to write
   *
   * @return                ReturnOK if write was successful
   * @return                ReturnError if write failed or invalid parameters
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);
  /**
   * @brief Reads single byte from block device
   * @details Simple interface for reading one byte from the current position.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[out]    data_   Pointer to store the read byte
   *
   * @return                ReturnOK if read was successful
   * @return                ReturnError if read failed or invalid parameters
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);
  /**
   * @brief Writes single byte to block device
   * @details Simple interface for writing one byte to the current position.
   *
   * @param[in,out] device_ Pointer to device structure
   * @param[in]     data_   Byte to write
   *
   * @return                ReturnOK if write was successful
   * @return                ReturnError if write failed
   */
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);
  #if defined(POSIX_ARCH_OTHER)
    /**
     * @brief Clears block device driver state
     * @details Internal function for POSIX platforms to reset driver state for
     * testing.
     */
    void __BlockDeviceStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */
  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef BLOCK_DRIVER_H_ */