/*UNCRUSTIFY-OFF*/
/**
 * @file device.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Device driver subsystem implementation
 * @details
 * Implements the I/O device registration and management system providing unified interfaces for character, block, and other device drivers.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#include "config.h"
#if defined(CONFIG_ENABLE_IO_SUBSYSTEM)
  #include "device.h"
  static DeviceList_t *dlist = null;
  #define __DeviceUidNonZero__() (0x0u < uid_)
  Return_t xDeviceRegisterDevice(Return_t (*device_self_register_)()) {
    FUNCTION_ENTER;
    if(__PointerIsNotNull__(device_self_register_)) {
      if(OK((*device_self_register_)())) {
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
  Return_t __RegisterDevice__(const HalfWord_t uid_, const Byte_t *name_, const DeviceState_t state_, const DeviceMode_t mode_, Return_t (*init_)(Device_t *
    device_), Return_t (*config_)(Device_t *device_, Size_t *size_, Addr_t *config_), Return_t (*read_)(Device_t *device_, Size_t *size_, Addr_t **data_),
    Return_t (*write_)(Device_t *device_, Size_t *size_, Addr_t *data_), Return_t (*simple_read_)(Device_t *device_, Byte_t *data_), Return_t (*simple_write_)(
    Device_t *device_, Byte_t data_)) {
    FUNCTION_ENTER;
    Device_t *device = null;
    Device_t *cursor = null;
    if((__DeviceUidNonZero__() && __PointerIsNotNull__(name_) && __PointerIsNotNull__(init_) && __PointerIsNotNull__(config_) && __PointerIsNotNull__(read_) &&
      __PointerIsNotNull__(write_) && __PointerIsNotNull__(simple_read_) && __PointerIsNotNull__(simple_write_) && __PointerIsNotNull__(dlist)) || (
      __DeviceUidNonZero__() && __PointerIsNotNull__(name_) && __PointerIsNotNull__(init_) && __PointerIsNotNull__(config_) && __PointerIsNotNull__(read_) &&
      __PointerIsNotNull__(write_) && __PointerIsNotNull__(simple_read_) && __PointerIsNotNull__(simple_write_) && __PointerIsNull__(dlist) && OK(
      __KernelAllocateMemory__((volatile Addr_t **) &dlist, sizeof(DeviceList_t))))) {
      if(__PointerIsNotNull__(dlist) && !__ObjectIsValid__(dlist)) {
        dlist->valid = VALID;
        dlist->length = 0x0u;
        dlist->head = null;
      }
      if(__PointerIsNotNull__(dlist)) {
        if(!OK(__DeviceListFind__(uid_, &device))) {
          if(__PointerIsNull__(device)) {
            if(OK(__KernelAllocateMemory__((volatile Addr_t **) &device, sizeof(Device_t)))) {
              if(__PointerIsNotNull__(device)) {
                if(OK(__memcpy__(device->name, name_, CONFIG_DEVICE_NAME_BYTES))) {
                  device->valid = VALID;
                  device->uid = uid_;
                  device->state = state_;
                  device->mode = mode_;
                  device->bytesWritten = 0x0u;
                  device->bytesRead = 0x0u;
                  device->available = 0x0u;
                  device->init = init_;
                  device->config = config_;
                  device->read = read_;
                  device->write = write_;
                  device->simple_read = simple_read_;
                  device->simple_write = simple_write_;
                  cursor = dlist->head;
                  if(__PointerIsNotNull__(dlist->head)) {
                    while(__PointerIsNotNull__(cursor->next)) {
                      cursor = cursor->next;
                    }
                    cursor->next = device;
                  } else {
                    dlist->head = device;
                  }
                  dlist->length++;
                  __ReturnOk__();
                } else {
                  __AssertOnElse__();
                  __KernelFreeMemory__(device);
                }
              } else {
                __AssertOnElse__();
              }
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    if(__DeviceUidNonZero__() && __PointerIsNotNull__(res_) && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__PointerIsNotNull__(device)) {
          *res_ = device->available;
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Byte_t data_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    if(__DeviceUidNonZero__() && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__PointerIsNotNull__(device)) {
          if(((DeviceModeReadWrite == device->mode) || (DeviceModeWriteOnly == device->mode)) && (DeviceStateRunning == device->state)) {
            if(OK((*device->simple_write)(device, data_))) {
              device->bytesWritten += sizeof(Byte_t);
              __ReturnOk__();
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    Byte_t *data = null;
    if(__DeviceUidNonZero__() && __PointerIsNotNull__(size_) && (0x0u < *size_) && __PointerIsNotNull__(data_) && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__PointerIsNotNull__(device)) {
          if(((DeviceModeReadWrite == device->mode) || (DeviceModeWriteOnly == device->mode)) && (DeviceStateRunning == device->state)) {
            if(OK(__KernelAllocateMemory__((volatile Addr_t **) &data, *size_))) {
              if(__PointerIsNotNull__(data)) {
                if(OK(__memcpy__(data, data_, *size_))) {
                  if(OK((*device->write)(device, size_, data))) {
                    if(OK(__KernelFreeMemory__(data))) {
                      device->bytesWritten += *size_;
                      __ReturnOk__();
                    } else {
                      __AssertOnElse__();
                    }
                  } else {
                    __AssertOnElse__();
                    __KernelFreeMemory__(data);
                  }
                } else {
                  __AssertOnElse__();
                  __KernelFreeMemory__(data);
                }
              } else {
                __AssertOnElse__();
              }
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t __DeviceWrite__(const HalfWord_t uid_, Size_t *size_, Addr_t *data_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    if(__DeviceUidNonZero__() && __PointerIsNotNull__(size_) && (0x0u < *size_) && __PointerIsNotNull__(data_) && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__ObjectIsValid__(device)) {
          if(((DeviceModeReadWrite == device->mode) || (DeviceModeWriteOnly == device->mode)) && (DeviceStateRunning == device->state)) {
            if(OK((*device->write)(device, size_, data_))) {
              device->bytesWritten += *size_;
              __ReturnOk__();
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t xDeviceSimpleRead(const HalfWord_t uid_, Byte_t *data_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    Byte_t data = 0x0u;
    if(__DeviceUidNonZero__() && __PointerIsNotNull__(data_) && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__PointerIsNotNull__(device)) {
          if(((DeviceModeReadWrite == device->mode) || (DeviceModeReadOnly == device->mode)) && (DeviceStateRunning == device->state)) {
            if(OK((*device->simple_read)(device, &data))) {
              *data_ = data;
              device->bytesRead += sizeof(Byte_t);
              __ReturnOk__();
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t **data_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    Addr_t *data = null;
    if(__DeviceUidNonZero__() && __PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__PointerIsNotNull__(device)) {
          if(((DeviceModeReadWrite == device->mode) || (DeviceModeReadOnly == device->mode)) && (DeviceStateRunning == device->state)) {
            if(OK((*device->read)(device, size_, &data))) {
              if((0x0u < *size_) && __PointerIsNotNull__(data)) {
                if(OK(__HeapAllocateMemory__((volatile Addr_t **) data_, *size_))) {
                  if(__PointerIsNotNull__(*data_)) {
                    if(OK(__memcpy__(*data_, data, *size_))) {
                      if(OK(__KernelFreeMemory__(data))) {
                        device->bytesRead += *size_;
                        __ReturnOk__();
                      }
                    } else {
                      __AssertOnElse__();
                      __KernelFreeMemory__(data);
                      __HeapFreeMemory__(*data_);
                    }
                  } else {
                    __AssertOnElse__();
                    __KernelFreeMemory__(data);
                  }
                } else {
                  __AssertOnElse__();
                  __KernelFreeMemory__(data);
                }
              } else {
                __AssertOnElse__();
              }
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t __DeviceRead__(const HalfWord_t uid_, Size_t *size_, Addr_t **data_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    if(__DeviceUidNonZero__() && __PointerIsNotNull__(size_) && __PointerIsNotNull__(data_) && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__ObjectIsValid__(device)) {
          if(((DeviceModeReadWrite == device->mode) || (DeviceModeReadOnly == device->mode)) && (DeviceStateRunning == device->state)) {
            if(OK((*device->read)(device, size_, data_))) {
              if((0x0u < *size_) && __PointerIsNotNull__(*data_)) {
                device->bytesRead += *size_;
                __ReturnOk__();
              } else {
                __AssertOnElse__();
              }
            } else {
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t __DeviceListFind__(const HalfWord_t uid_, Device_t **device_) {
    FUNCTION_ENTER;
    Device_t *cursor = null;
    if(__DeviceUidNonZero__() && __PointerIsNotNull__(device_) && __PointerIsNotNull__(dlist)) {
      cursor = dlist->head;
      while(__PointerIsNotNull__(cursor) && (uid_ != cursor->uid)) {
        cursor = cursor->next;
      }
      if(__PointerIsNotNull__(cursor)) {
        *device_ = cursor;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t xDeviceInitDevice(const HalfWord_t uid_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    if(__DeviceUidNonZero__() && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__PointerIsNotNull__(device)) {
          if(OK((*device->init)(device))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    Addr_t *config = null;
    if(__DeviceUidNonZero__() && (0x0u < *size_) && __PointerIsNotNull__(config_) && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__PointerIsNotNull__(device)) {
          if(OK(__KernelAllocateMemory__((volatile Addr_t **) &config, *size_))) {
            if(__PointerIsNotNull__(config)) {
              if(OK(__memcpy__(config, config_, *size_))) {
                if(OK((*device->config)(device, size_, config))) {
                  if(OK(__memcpy__(config_, config, *size_))) {
                    if(OK(__KernelFreeMemory__(config))) {
                      __ReturnOk__();
                    } else {
                      __AssertOnElse__();
                    }
                  } else {
                    __AssertOnElse__();
                    __KernelFreeMemory__(config);
                  }
                } else {
                  __AssertOnElse__();
                  __KernelFreeMemory__(config);
                }
              } else {
                __AssertOnElse__();
                __KernelFreeMemory__(config);
              }
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
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
  Return_t __DeviceConfigDevice__(const HalfWord_t uid_, Size_t *size_, Addr_t *config_) {
    FUNCTION_ENTER;
    Device_t *device = null;
    if(__DeviceUidNonZero__() && (0x0u < *size_) && __PointerIsNotNull__(config_) && __PointerIsNotNull__(dlist)) {
      if(OK(__DeviceListFind__(uid_, &device))) {
        if(__ObjectIsValid__(device)) {
          if(OK((*device->config)(device, size_, config_))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
    FUNCTION_EXIT;
  }
  #if defined(POSIX_ARCH_OTHER)
    void __DeviceStateClear__(void) {
      dlist = null;
      return;
    }
  #endif /* if defined(POSIX_ARCH_OTHER) */
#endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */