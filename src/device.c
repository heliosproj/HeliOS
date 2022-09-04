/**
 * @file device.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel source code for device drivers
 * @version 0.3.5
 * @date 2022-09-01
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
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

#include "device.h"


static DeviceList_t *deviceList = NULL;


Base_t xDeviceRegisterDevice(Base_t (*device_self_register_)()) {

  Base_t ret = RETURN_FAILURE;

  SYSASSERT(ISNOTNULLPTR(device_self_register_));

  if (ISNOTNULLPTR(device_self_register_)) {

    ret = (*device_self_register_)();

    SYSASSERT(RETURN_SUCCESS == ret);
  }


  return ret;
}


Base_t __RegisterDevice__(HalfWord_t uid_,
                          const Char_t *name_,
                          DeviceState_t state_,
                          DeviceMode_t mode_,
                          Base_t (*init_)(Device_t *device_),
                          Base_t (*config_)(Device_t *device_, Size_t *size_, void *config_),
                          Base_t (*read_)(Device_t *device_, Size_t *size_, void *data_),
                          Base_t (*write_)(Device_t *device_, Size_t *size_, void *data_),
                          Base_t (*simple_read_)(Device_t *device_, Word_t *data_),
                          Base_t (*simple_write_)(Device_t *device_, Word_t *data_)) {
  Base_t ret = RETURN_FAILURE;

  Device_t *device = NULL;

  Device_t *cursor = NULL;


  SYSASSERT((zero < uid_) && (ISNOTNULLPTR(name_)) && (ISNOTNULLPTR(init_)) && (ISNOTNULLPTR(config_)) &&
            (ISNOTNULLPTR(read_)) && (ISNOTNULLPTR(write_)));

  if (((zero < uid_) && (ISNOTNULLPTR(name_)) && (ISNOTNULLPTR(init_)) && (ISNOTNULLPTR(config_)) &&
       (ISNOTNULLPTR(read_)) && (ISNOTNULLPTR(write_)))) {


    if (ISNULLPTR(deviceList)) {

      deviceList = (DeviceList_t *)__KernelAllocateMemory__(sizeof(DeviceList_t));
    }


    SYSASSERT(ISNOTNULLPTR(deviceList));

    if (ISNOTNULLPTR(deviceList)) {

      device = __DeviceListFind__(uid_);

      SYSASSERT(ISNULLPTR(device));

      if (ISNULLPTR(device)) {

        device = (Device_t *)__KernelAllocateMemory__(sizeof(Device_t));



        SYSASSERT(ISNOTNULLPTR(device));



        if (ISNOTNULLPTR(device)) {

          device->uid = uid_;
          __memcpy__(device->name, name_, CONFIG_DEVICE_NAME_BYTES);
          device->state = state_;
          device->mode = mode_;
          device->bytesWritten = zero;
          device->bytesRead = zero;
          device->available = false;
          device->init = init_;
          device->config = config_;
          device->read = read_;
          device->write = write_;
          device->simple_read = simple_read_;
          device->simple_write = simple_write_;



          cursor = deviceList->head;


          if (ISNOTNULLPTR(deviceList->head)) {


            while (ISNOTNULLPTR(cursor->next)) {


              cursor = cursor->next;
            }

            cursor->next = device;

          } else {

            deviceList->head = device;
          }

          deviceList->length++;

          ret = RETURN_SUCCESS;
        }
      }
    }
  }

  return ret;
}


Base_t xDeviceIsAvailable(HalfWord_t uid_) {


  Base_t ret = false;

  Device_t *device = NULL;

  SYSASSERT(zero < uid_);



  if (zero < uid_) {

    device = __DeviceListFind__(uid_);


    SYSASSERT(ISNOTNULLPTR(device));

    if (ISNOTNULLPTR(device)) {

      ret = device->available;
    }
  }

  return ret;
}


Base_t xDeviceWrite(HalfWord_t uid_, Size_t *size_, void *data_) {

  Base_t ret = RETURN_FAILURE;

  Device_t *device = NULL;

  Byte_t *data = NULL;

  SYSASSERT(zero < uid_);

  SYSASSERT(ISNOTNULLPTR(size_));

  SYSASSERT(zero < *size_);

  SYSASSERT(ISNOTNULLPTR(data_));

  SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR));

  if ((zero < uid_) && (ISNOTNULLPTR(size_)) && (zero < *size_) && (ISNOTNULLPTR(data_)) && (RETURN_SUCCESS == __MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {


    device = __DeviceListFind__(uid_);


    SYSASSERT(ISNOTNULLPTR(device));

    if (ISNOTNULLPTR(device)) {

      SYSASSERT(((DeviceModeReadWrite == device->mode) || (DeviceModeWriteOnly == device->mode)) && (DeviceStateRunning == device->state));

      if (((DeviceModeReadWrite == device->mode) || (DeviceModeWriteOnly == device->mode)) && (DeviceStateRunning == device->state)) {

        data = (Byte_t *)__KernelAllocateMemory__(*size_);

        SYSASSERT(ISNOTNULLPTR(data));

        if (ISNOTNULLPTR(data)) {

          __memcpy__(data, data_, *size_);

          ret = (*device->write)(device, size_, data);

          device->bytesWritten += *size_;

          SYSASSERT(RETURN_SUCCESS == ret);

          __KernelFreeMemory__(data);
        }
      }
    }
  }



  return ret;
}

Base_t xDeviceRead(HalfWord_t uid_, Size_t *size_, void *data_) {


  Base_t ret = RETURN_FAILURE;

  Device_t *device = NULL;

  void *data = NULL;

  SYSASSERT(zero < uid_);

  SYSASSERT(ISNOTNULLPTR(size_));

  SYSASSERT(zero < *size_);

  SYSASSERT(ISNOTNULLPTR(data_));

  SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR));

  if ((zero < uid_) && (ISNOTNULLPTR(size_)) && (zero < *size_) && (ISNOTNULLPTR(data_)) && (RETURN_SUCCESS == __MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {


    device = __DeviceListFind__(uid_);


    SYSASSERT(ISNOTNULLPTR(device));

    if (ISNOTNULLPTR(device)) {

      SYSASSERT(((DeviceModeReadWrite == device->mode) || (DeviceModeReadOnly == device->mode)) && (DeviceStateRunning == device->state));

      if (((DeviceModeReadWrite == device->mode) || (DeviceModeReadOnly == device->mode)) && (DeviceStateRunning == device->state)) {

        data = (Byte_t *)__KernelAllocateMemory__(*size_);

        SYSASSERT(ISNOTNULLPTR(data));

        if (ISNOTNULLPTR(data)) {

          ret = (*device->read)(device, size_, data);

          __memcpy__(data_, data, *size_);

          device->bytesRead += *size_;

          SYSASSERT(RETURN_SUCCESS == ret);

          __KernelFreeMemory__(data);
        }
      }
    }
  }



  return ret;
}


Device_t *__DeviceListFind__(HalfWord_t uid_) {


  Device_t *ret = NULL;

  Device_t *cursor = NULL;


  SYSASSERT(ISNOTNULLPTR(deviceList));


  SYSASSERT(zero < uid_);


  if ((ISNOTNULLPTR(deviceList)) && (zero < uid_)) {



    cursor = deviceList->head;


    while ((ISNOTNULLPTR(cursor)) && (cursor->uid != uid_)) {

      cursor = cursor->next;
    }


    SYSASSERT(ISNOTNULLPTR(cursor));


    if (ISNOTNULLPTR(cursor)) {


      ret = cursor;
    }
  }

  return ret;
}

Base_t xDeviceInitDevice(HalfWord_t uid_) {

  Base_t ret = RETURN_FAILURE;

  Device_t *device = NULL;

  SYSASSERT(zero < uid_);

  if (zero < uid_) {

    device = __DeviceListFind__(uid_);

    SYSASSERT(ISNOTNULLPTR(device));


    if (ISNOTNULLPTR(device)) {

      ret = (*device->init)(device);

      SYSASSERT(RETURN_SUCCESS == ret);
    }
  }


  return ret;
}
Base_t xDeviceConfigDevice(HalfWord_t uid_, Size_t *size_, void *config_) {


  Base_t ret = RETURN_FAILURE;

  Device_t *device = NULL;

  void *config = NULL;

  SYSASSERT(zero < uid_);

  SYSASSERT(zero < *size_);

  SYSASSERT(ISNOTNULLPTR(config_));

  SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheckHeap__(config_, MEMORY_REGION_CHECK_OPTION_W_ADDR));

  if ((zero < uid_) && (zero < *size_) && (ISNOTNULLPTR(config_)) && (RETURN_SUCCESS == __MemoryRegionCheckHeap__(config_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {

    device = __DeviceListFind__(uid_);

    SYSASSERT(ISNOTNULLPTR(device));


    if (ISNOTNULLPTR(device)) {

      config = (void *)__KernelAllocateMemory__(*size_);

      SYSASSERT(ISNOTNULLPTR(config));

      if (ISNOTNULLPTR(config)) {

        __memcpy__(config, config_, *size_);


        ret = (*device->config)(device, size_, config);


        __memcpy__(config_, config, *size_);


        SYSASSERT(RETURN_SUCCESS == ret);

        __KernelFreeMemory__(config);
      }
    }
  }


  return ret;
}

#if defined(POSIX_ARCH_OTHER)
void __DeviceStateClear__(void) {

  deviceList = NULL;

  return;
}
#endif
