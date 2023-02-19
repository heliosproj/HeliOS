/*UNCRUSTIFY-OFF*/
/**
 * @file device.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel source code for device drivers
 * @version 0.4.0
 * @date 2022-09-01
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
#include "device.h"

static DeviceList_t *deviceList = null;
static Return_t __DeviceListFind__(const HalfWord_t uid_, Device_t **device_);


Return_t xDeviceRegisterDevice(Return_t (*device_self_register_)()) {
  RET_DEFINE;

  if(NOTNULLPTR(device_self_register_)) {
    if(ISOK((*device_self_register_)())) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t __RegisterDevice__(const HalfWord_t uid_, const Byte_t *name_, const DeviceState_t state_, const DeviceMode_t mode_, Return_t (*init_)(
    Device_t *device_), Return_t (*config_)(Device_t *device_, Size_t *size_, Addr_t *config_), Return_t (*read_)(Device_t *device_, Size_t *size_,
  Addr_t *data_), Return_t (*write_)(Device_t *device_, Size_t *size_, Addr_t *data_), Return_t (*simple_read_)(Device_t *device_, Word_t *data_),
  Return_t (*simple_write_)(Device_t *device_, Word_t *data_)) {
  RET_DEFINE;


  Device_t *device = null;
  Device_t *cursor = null;


  if(((zero < uid_) && (NOTNULLPTR(name_)) && (NOTNULLPTR(init_)) && (NOTNULLPTR(config_)) && (NOTNULLPTR(read_)) && (NOTNULLPTR(write_)) && (NOTNULLPTR(
      simple_read_)) && (NOTNULLPTR(simple_write_)) && (NOTNULLPTR(deviceList))) || ((zero < uid_) && (NOTNULLPTR(name_)) && (NOTNULLPTR(init_)) && (NOTNULLPTR(
      config_)) && (NOTNULLPTR(read_)) && (NOTNULLPTR(write_)) && (NOTNULLPTR(simple_read_)) && (NOTNULLPTR(simple_write_)) && (NULLPTR(deviceList)) && (ISOK(
      __KernelAllocateMemory__((volatile Addr_t **) &deviceList, sizeof(DeviceList_t)))))) {
    if(NOTNULLPTR(deviceList)) {
      /* We are expecting *NOT* to find the device UID in the device list. This
       * is to confirm there isn't already a device with the same UID already
       * registered. */
      if(!ISOK(__DeviceListFind__(uid_, &device))) {
        /* Likewise this should be null since we expected __DeviceListFind()
         * will *NOT* find a device by that UID. */
        if(NULLPTR(device)) {
          if(ISOK(__KernelAllocateMemory__((volatile Addr_t **) &device, sizeof(Device_t)))) {
            if(NOTNULLPTR(device)) {
              if(ISOK(__memcpy__(device->name, name_, CONFIG_DEVICE_NAME_BYTES))) {
                device->uid = uid_;
                device->state = state_;
                device->mode = mode_;
                device->bytesWritten = zero;
                device->bytesRead = zero;
                device->available = zero;
                device->init = init_;
                device->config = config_;
                device->read = read_;
                device->write = write_;
                device->simple_read = simple_read_;
                device->simple_write = simple_write_;
                cursor = deviceList->head;

                if(NOTNULLPTR(deviceList->head)) {
                  while(NOTNULLPTR(cursor->next)) {
                    cursor = cursor->next;
                  }

                  cursor->next = device;
                } else {
                  deviceList->head = device;
                }

                deviceList->length++;
                RET_OK;
              } else {
                ASSERT;
              }
            } else {
              ASSERT;
            }
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_) {
  RET_DEFINE;


  Device_t *device = null;


  if((zero < uid_) && NOTNULLPTR(res_)) {
    if(ISOK(__DeviceListFind__(uid_, &device))) {
      if(NOTNULLPTR(device)) {
        *res_ = device->available;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Word_t *data_) {
  RET_DEFINE;


  Device_t *device = null;
  Word_t *data = null;


  if((zero < uid_) && (NOTNULLPTR(data_)) && (ISOK(__MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISOK(__DeviceListFind__(uid_, &device))) {
      if(NOTNULLPTR(device)) {
        if(((DeviceModeReadWrite == device->mode) || (DeviceModeWriteOnly == device->mode)) && (DeviceStateRunning == device->state)) {
          if(ISOK(__KernelAllocateMemory__((volatile Addr_t **) &data, sizeof(Word_t)))) {
            if(NOTNULLPTR(data)) {
              if(ISOK(__memcpy__(data, data_, sizeof(Word_t)))) {
                if(ISOK((*device->simple_write)(device, data))) {
                  if(ISOK(__KernelFreeMemory__(data))) {
                    device->bytesWritten += sizeof(Word_t);
                    RET_OK;
                  } else {
                    ASSERT;
                  }
                } else {
                  ASSERT;
                }
              } else {
                ASSERT;
              }
            } else {
              ASSERT;
            }
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_) {
  RET_DEFINE;


  Device_t *device = null;
  Byte_t *data = null;


  if((zero < uid_) && (NOTNULLPTR(size_)) && (zero < *size_) && (NOTNULLPTR(data_)) && (ISOK(__MemoryRegionCheckHeap__(data_,
    MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISOK(__DeviceListFind__(uid_, &device))) {
      if(NOTNULLPTR(device)) {
        if(((DeviceModeReadWrite == device->mode) || (DeviceModeWriteOnly == device->mode)) && (DeviceStateRunning == device->state)) {
          if(ISOK(__KernelAllocateMemory__((volatile Addr_t **) &data, *size_))) {
            if(NOTNULLPTR(data)) {
              if(ISOK(__memcpy__(data, data_, *size_))) {
                if(ISOK((*device->write)(device, size_, data))) {
                  if(ISOK(__KernelFreeMemory__(data))) {
                    device->bytesWritten += *size_;
                    RET_OK;
                  } else {
                    ASSERT;
                  }
                } else {
                  ASSERT;
                }
              } else {
                ASSERT;
              }
            } else {
              ASSERT;
            }
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xDeviceSimpleRead(const HalfWord_t uid_, Word_t *data_) {
  RET_DEFINE;


  Device_t *device = null;
  Word_t *data = null;


  if((zero < uid_) && (NOTNULLPTR(data_)) && (ISOK(__MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISOK(__DeviceListFind__(uid_, &device))) {
      if(NOTNULLPTR(device)) {
        if(((DeviceModeReadWrite == device->mode) || (DeviceModeReadOnly == device->mode)) && (DeviceStateRunning == device->state)) {
          if(ISOK(__KernelAllocateMemory__((volatile Addr_t **) &data, sizeof(Word_t)))) {
            if(NOTNULLPTR(data)) {
              if(ISOK((*device->simple_read)(device, data))) {
                if(ISOK(__memcpy__(data_, data, sizeof(Word_t)))) {
                  if(ISOK(__KernelFreeMemory__(data))) {
                    device->bytesRead += sizeof(Word_t);
                    RET_OK;
                  } else {
                    ASSERT;
                  }
                } else {
                  ASSERT;
                }
              } else {
                ASSERT;
              }
            } else {
              ASSERT;
            }
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t *data_) {
  RET_DEFINE;


  Device_t *device = null;
  Byte_t *data = null;


  if((zero < uid_) && (NOTNULLPTR(size_)) && (zero < *size_) && (NOTNULLPTR(data_)) && (ISOK(__MemoryRegionCheckHeap__(data_,
    MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISOK(__DeviceListFind__(uid_, &device))) {
      if(NOTNULLPTR(device)) {
        if(((DeviceModeReadWrite == device->mode) || (DeviceModeReadOnly == device->mode)) && (DeviceStateRunning == device->state)) {
          if(ISOK(__KernelAllocateMemory__((volatile Addr_t **) &data, *size_))) {
            if(NOTNULLPTR(data)) {
              if(ISOK((*device->read)(device, size_, data))) {
                if(ISOK(__memcpy__(data_, data, *size_))) {
                  if(ISOK(__KernelFreeMemory__(data))) {
                    device->bytesRead += *size_;
                    RET_OK;
                  } else {
                    ASSERT;
                  }
                } else {
                  ASSERT;
                }
              } else {
                ASSERT;
              }
            } else {
              ASSERT;
            }
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __DeviceListFind__(const HalfWord_t uid_, Device_t **device_) {
  RET_DEFINE;


  Device_t *cursor = null;


  if(NOTNULLPTR(device_) && NOTNULLPTR(deviceList) && (zero < uid_)) {
    cursor = deviceList->head;

    while((NOTNULLPTR(cursor)) && (cursor->uid != uid_)) {
      cursor = cursor->next;
    }

    if(NOTNULLPTR(cursor)) {
      *device_ = cursor;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xDeviceInitDevice(const HalfWord_t uid_) {
  RET_DEFINE;


  Device_t *device = null;


  if(zero < uid_) {
    if(ISOK(__DeviceListFind__(uid_, &device))) {
      if(NOTNULLPTR(device)) {
        if(ISOK((*device->init)(device))) {
          RET_OK;
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_) {
  RET_DEFINE;


  Device_t *device = null;
  Addr_t *config = null;


  if((zero < uid_) && (zero < *size_) && (NOTNULLPTR(config_)) && (ISOK(__MemoryRegionCheckHeap__(config_, MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISOK(__DeviceListFind__(uid_, &device))) {
      if(NOTNULLPTR(device)) {
        if(ISOK(__KernelAllocateMemory__((volatile Addr_t **) &config, *size_))) {
          if(NOTNULLPTR(config)) {
            if(ISOK(__memcpy__(config, config_, *size_))) {
              if(ISOK((*device->config)(device, size_, config))) {
                if(ISOK(__memcpy__(config_, config, *size_))) {
                  if(ISOK(__KernelFreeMemory__(config))) {
                    RET_OK;
                  } else {
                    ASSERT;
                  }
                } else {
                  ASSERT;
                }
              } else {
                ASSERT;
              }
            } else {
              ASSERT;
            }
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  return (ret);
}


#if defined(POSIX_ARCH_OTHER)


  void __DeviceStateClear__(void) {
    deviceList = null;

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */