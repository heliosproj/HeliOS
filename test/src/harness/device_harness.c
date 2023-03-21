/*UNCRUSTIFY-OFF*/
/**
 * @file device_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.4.1
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#include "device_harness.h"


void device_harness(void) {
  Size_t bytes1;
  Addr_t *data1;
  Size_t bytes2;
  Addr_t *data2;
  Byte_t data3;
  Byte_t data4;
  Base_t res;


  unit_begin("xDeviceRegisterDevice()");
  unit_try(OK(xDeviceRegisterDevice(LOOPBACK_self_register)));
  unit_end();
  unit_begin("xDeviceWrite()");
  bytes1 = 0x26u;
  data1 = null;
  unit_try(OK(xMemAlloc((volatile Addr_t **) &data1, bytes1)));
  memcpy(data1, "THIS IS A TEST OF THE LOOPBACK DEVICE\0", bytes1);
  unit_try(OK(xDeviceWrite(0xFFu, &bytes1, data1)));
  unit_try(OK(xMemFree(data1)));
  unit_end();
  unit_begin("xDeviceIsAvailable()");
  unit_try(OK(xDeviceIsAvailable(0xFFu, &res)));
  unit_try(true == res);
  unit_end();
  unit_begin("xDeviceRead()");
  bytes2 = zero;
  data2 = null;
  unit_try(OK(xDeviceRead(0xFFu, &bytes2, &data2)));
  unit_try(0x26u == bytes2);
  unit_try(zero == strncmp((char *) data2, "THIS IS A TEST OF THE LOOPBACK DEVICE\0", bytes2));
  unit_try(OK(xMemFree(data2)));
  unit_end();
  unit_begin("xDeviceSimpleWrite()");
  data3 = 0xFAu;
  unit_try(OK(xDeviceSimpleWrite(0xFFu, data3)));
  unit_end();
  unit_begin("xDeviceSimpleRead()");
  data4 = zero;
  unit_try(OK(xDeviceSimpleRead(0xFFu, &data4)));
  unit_try(0xFAu == data4);
  unit_end();

  return;
}