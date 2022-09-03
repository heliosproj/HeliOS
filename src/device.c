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


Base_t __RegisterDevice__(HWord_t uid_,
                          const char *name_,
                          DeviceState_t state_,
                          DeviceMode_t mode_,
                          Base_t (*init_)(Device_t *device_),
                          Base_t (*config_)(Device_t *device_, void *config_),
                          Base_t (*read_)(Device_t *device_, HWord_t *bytes_, void *data_),
                          Base_t (*write_)(Device_t *device_, HWord_t *bytes_, void *data_)) {
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


      device = (Device_t *)__KernelAllocateMemory__(sizeof(Device_t));



      SYSASSERT(ISNOTNULLPTR(device));



      if (ISNOTNULLPTR(device)) {

        device->uid = uid_;
        __memcpy__(device->name, name_, CONFIG_DEVICE_NAME_BYTES);
        device->state = state_;
        device->mode = mode_;
        device->bytesWritten = zero;
        device->bytesRead = zero;
        device->init = init_;
        device->config = config_;
        device->read = read_;
        device->write = write_;



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

  return ret;
}



Base_t xDeviceWrite(HWord_t uid_, HWord_t *bytes_, Byte_t *data_) {

  Base_t ret = RETURN_FAILURE;

  Device_t *device = NULL;

  Byte_t *data = NULL;

  SYSASSERT(zero < uid_);

  SYSASSERT(zero < *bytes_);

  SYSASSERT(ISNOTNULLPTR(data_));

  SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR));

  if ((zero < uid_) && (zero < *bytes_) && (ISNOTNULLPTR(data_)) && (RETURN_SUCCESS == __MemoryRegionCheckHeap__(data_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {

    device = __DeviceListFind__(uid_);


    SYSASSERT(ISNOTNULLPTR(device));

    if (ISNOTNULLPTR(device)) {

      data = (Byte_t *)__KernelAllocateMemory__(*bytes_);

      SYSASSERT(ISNOTNULLPTR(data));

      if (ISNOTNULLPTR(data)) {

        __memcpy__(data, data_, *bytes_);

        ret = (*device->write)(device, bytes_, data);

        SYSASSERT(RETURN_SUCCESS == ret);
      }


      __KernelFreeMemory__(data);
    }
  }



  return ret;
}


Device_t *__DeviceListFind__(HWord_t uid_) {


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
