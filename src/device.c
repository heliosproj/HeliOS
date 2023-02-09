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

  if(ISNOTNULLPTR(device_self_register_)) {
    if(ISSUCCESSFUL((*device_self_register_)())) {
      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t __RegisterDevice__(const HalfWord_t uid_, const Char_t *name_, const DeviceState_t state_, const DeviceMode_t mode_, Return_t (*init_)(
    Device_t *device_), Return_t (*config_)(Device_t *device_, Size_t *size_, Addr_t *config_), Return_t (*read_)(Device_t *device_, Size_t *size_,
  Addr_t *data_), Return_t (*write_)(Device_t *device_, Size_t *size_, Addr_t *data_), Return_t (*simple_read_)(Device_t *device_, Word_t *data_),
  Return_t (*simple_write_)(Device_t *device_, Word_t *data_)) {
  RET_DEFINE;


  Device_t *device = null;
  Device_t *cursor = null;


  if(((zero < uid_) && (ISNOTNULLPTR(name_)) && (ISNOTNULLPTR(init_)) && (ISNOTNULLPTR(config_)) && (ISNOTNULLPTR(read_)) && (ISNOTNULLPTR(write_)) &&
    (ISNOTNULLPTR(simple_read_)) && (ISNOTNULLPTR(simple_write_)) && (ISNOTNULLPTR(deviceList))) || ((zero < uid_) && (ISNOTNULLPTR(name_)) && (ISNOTNULLPTR(
      init_)) && (ISNOTNULLPTR(config_)) && (ISNOTNULLPTR(read_)) && (ISNOTNULLPTR(write_)) && (ISNOTNULLPTR(simple_read_)) && (ISNOTNULLPTR(simple_write_)) &&
    (ISNULLPTR(deviceList)) && (ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &deviceList, sizeof(DeviceList_t)))))) {
    if(ISNOTNULLPTR(deviceList)) {
      if(ISSUCCESSFUL(__DeviceListFind__(uid_, &device))) {
        if(ISNULLPTR(device)) {
          if(ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &device, sizeof(Device_t)))) {
            if(ISNOTNULLPTR(device)) {
              if(ISSUCCESSFUL(__memcpy__(device->name, name_, CONFIG_DEVICE_NAME_BYTES))) {
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

                if(ISNOTNULLPTR(deviceList->head)) {
                  while(ISNOTNULLPTR(cursor->next)) {
                    cursor = cursor->next;
                  }

                  cursor->next = device;
                } else {
                  deviceList->head = device;
                }

                deviceList->length++;
                RET_SUCCESS;
              } else {
                SYSASSERT(false);
              }
            } else {
              SYSASSERT(false);
            }
          } else {
            SYSASSERT(false);
          }
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_) {
  RET_DEFINE;


  Device_t *device = null;


  if((zero < uid_) && ISNOTNULLPTR(res_)) {
    if(ISSUCCESSFUL(__DeviceListFind__(uid_, &device))) {
      if(ISNOTNULLPTR(device)) {
        *res_ = device->available;
        RET_SUCCESS;
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Word_t *data_) {
  RET_DEFINE;


  Device_t *device = null;
  Word_t *data = null;


  if((zero < uid_) && (ISNOTNULLPTR(data_)) && (ISSUCCESSFUL(__MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISSUCCESSFUL(__DeviceListFind__(uid_, &device))) {
      if(ISNOTNULLPTR(device)) {
        if(((DeviceModeReadWrite == device->mode) || (DeviceModeWriteOnly == device->mode)) && (DeviceStateRunning == device->state)) {
          if(ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &data, sizeof(Word_t)))) {
            if(ISNOTNULLPTR(data)) {
              if(ISSUCCESSFUL(__memcpy__(data, data_, sizeof(Word_t)))) {
                if(ISSUCCESSFUL((*device->simple_write)(device, data))) {
                  if(ISSUCCESSFUL(__KernelFreeMemory__(data))) {
                    device->bytesWritten += sizeof(Word_t);
                    RET_SUCCESS;
                  } else {
                    SYSASSERT(false);
                  }
                } else {
                  SYSASSERT(false);
                }
              } else {
                SYSASSERT(false);
              }
            } else {
              SYSASSERT(false);
            }
          } else {
            SYSASSERT(false);
          }
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_) {
  RET_DEFINE;


  Device_t *device = null;
  Byte_t *data = null;


  if((zero < uid_) && (ISNOTNULLPTR(size_)) && (zero < *size_) && (ISNOTNULLPTR(data_)) && (ISSUCCESSFUL(__MemoryRegionCheckHeap__(data_,
    MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISSUCCESSFUL(__DeviceListFind__(uid_, &device))) {
      if(ISNOTNULLPTR(device)) {
        if(((DeviceModeReadWrite == device->mode) || (DeviceModeWriteOnly == device->mode)) && (DeviceStateRunning == device->state)) {
          if(ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &data, *size_))) {
            if(ISNOTNULLPTR(data)) {
              if(ISSUCCESSFUL(__memcpy__(data, data_, *size_))) {
                if(ISSUCCESSFUL((*device->write)(device, size_, data))) {
                  if(ISSUCCESSFUL(__KernelFreeMemory__(data))) {
                    device->bytesWritten += *size_;
                    RET_SUCCESS;
                  } else {
                    SYSASSERT(false);
                  }
                } else {
                  SYSASSERT(false);
                }
              } else {
                SYSASSERT(false);
              }
            } else {
              SYSASSERT(false);
            }
          } else {
            SYSASSERT(false);
          }
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t xDeviceSimpleRead(const HalfWord_t uid_, Word_t *data_) {
  RET_DEFINE;


  Device_t *device = null;
  Word_t *data = null;


  if((zero < uid_) && (ISNOTNULLPTR(data_)) && (ISSUCCESSFUL(__MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISSUCCESSFUL(__DeviceListFind__(uid_, &device))) {
      if(ISNOTNULLPTR(device)) {
        if(((DeviceModeReadWrite == device->mode) || (DeviceModeReadOnly == device->mode)) && (DeviceStateRunning == device->state)) {
          if(ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &data, sizeof(Word_t)))) {
            if(ISNOTNULLPTR(data)) {
              if(ISSUCCESSFUL((*device->simple_read)(device, data))) {
                if(ISSUCCESSFUL(__memcpy__(data_, data, sizeof(Word_t)))) {
                  if(ISSUCCESSFUL(__KernelFreeMemory__(data))) {
                    device->bytesRead += sizeof(Word_t);
                    RET_SUCCESS;
                  } else {
                    SYSASSERT(false);
                  }
                } else {
                  SYSASSERT(false);
                }
              } else {
                SYSASSERT(false);
              }
            } else {
              SYSASSERT(false);
            }
          } else {
            SYSASSERT(false);
          }
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t *data_) {
  RET_DEFINE;


  Device_t *device = null;
  Byte_t *data = null;


  if((zero < uid_) && (ISNOTNULLPTR(size_)) && (zero < *size_) && (ISNOTNULLPTR(data_)) && (ISSUCCESSFUL(__MemoryRegionCheckHeap__(data_,
    MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISSUCCESSFUL(__DeviceListFind__(uid_, &device))) {
      if(ISNOTNULLPTR(device)) {
        if(((DeviceModeReadWrite == device->mode) || (DeviceModeReadOnly == device->mode)) && (DeviceStateRunning == device->state)) {
          if(ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &data, *size_))) {
            if(ISNOTNULLPTR(data)) {
              if(ISSUCCESSFUL((*device->read)(device, size_, data))) {
                if(ISSUCCESSFUL(__memcpy__(data_, data, *size_))) {
                  if(ISSUCCESSFUL(__KernelFreeMemory__(data))) {
                    device->bytesRead += *size_;
                    RET_SUCCESS;
                  } else {
                    SYSASSERT(false);
                  }
                } else {
                  SYSASSERT(false);
                }
              } else {
                SYSASSERT(false);
              }
            } else {
              SYSASSERT(false);
            }
          } else {
            SYSASSERT(false);
          }
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


static Return_t __DeviceListFind__(const HalfWord_t uid_, Device_t **device_) {
  RET_DEFINE;


  Device_t *cursor = null;


  if(ISNOTNULLPTR(device_) && ISNOTNULLPTR(deviceList) && (zero < uid_)) {
    cursor = deviceList->head;

    while((ISNOTNULLPTR(cursor)) && (cursor->uid != uid_)) {
      cursor = cursor->next;
    }

    if(ISNOTNULLPTR(cursor)) {
      *device_ = cursor;
      RET_SUCCESS;
    } else {
      *device_ = null;
      RET_SUCCESS;
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t xDeviceInitDevice(const HalfWord_t uid_) {
  RET_DEFINE;


  Device_t *device = null;


  if(zero < uid_) {
    if(ISSUCCESSFUL(__DeviceListFind__(uid_, &device))) {
      if(ISNOTNULLPTR(device)) {
        if(ISSUCCESSFUL((*device->init)(device))) {
          RET_SUCCESS;
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_) {
  RET_DEFINE;


  Device_t *device = null;
  Addr_t *config = null;


  if((zero < uid_) && (zero < *size_) && (ISNOTNULLPTR(config_)) && (ISSUCCESSFUL(__MemoryRegionCheckHeap__(config_, MEMORY_REGION_CHECK_OPTION_W_ADDR)))) {
    if(ISSUCCESSFUL(__DeviceListFind__(uid_, &device))) {
      if(ISNOTNULLPTR(device)) {
        if(ISSUCCESSFUL(__KernelAllocateMemory__((volatile Addr_t **) &config, *size_))) {
          if(ISNOTNULLPTR(config)) {
            if(ISSUCCESSFUL(__memcpy__(config, config_, *size_))) {
              if(ISSUCCESSFUL((*device->config)(device, size_, config))) {
                if(ISSUCCESSFUL(__memcpy__(config_, config, *size_))) {
                  if(ISSUCCESSFUL(__KernelFreeMemory__(config))) {
                    RET_SUCCESS;
                  } else {
                    SYSASSERT(false);
                  }
                } else {
                  SYSASSERT(false);
                }
              } else {
                SYSASSERT(false);
              }
            } else {
              SYSASSERT(false);
            }
          } else {
            SYSASSERT(false);
          }
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  return (ret);
}


#if defined(POSIX_ARCH_OTHER)


  void __DeviceStateClear__(void) {
    deviceList = null;

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */