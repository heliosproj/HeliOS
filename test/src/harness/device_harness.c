/*UNCRUSTIFY-OFF*/
/**
 * @file device_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#include "device_harness.h"


/* Test constants */
#define LOOPBACK_DEVICE_ID 0xFFu /* Loopback device identifier */
#define TEST_DATA_SIZE 0x26u /* 38 bytes - size of test string */
#define TEST_MESSAGE "THIS IS A TEST OF THE LOOPBACK DEVICE\0"
#define SIMPLE_TEST_BYTE 0xFAu /* Test byte for simple read/write */


void device_harness(void) {
  Size_t bytes1;
  Addr_t *data1;
  Size_t bytes2;
  Addr_t *data2;
  Byte_t data3;
  Byte_t data4;
  Base_t res;


  unit_begin("Loopback device registration succeeds");
  unit_assert_ok(xDeviceRegisterDevice(LOOPBACK_self_register));
  unit_end();
  unit_begin("Device write transmits test data successfully");
  bytes1 = TEST_DATA_SIZE;
  data1 = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &data1, bytes1));
  memcpy(data1, TEST_MESSAGE, bytes1);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytes1, data1));
  unit_assert_ok(xMemFree(data1));
  unit_end();
  unit_begin("Device availability check returns true for active device");
  unit_assert_ok(xDeviceIsAvailable(LOOPBACK_DEVICE_ID, &res));
  unit_assert_true(res);
  unit_end();
  unit_begin("Device read retrieves previously written data");
  bytes2 = nil;
  data2 = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytes2, &data2));
  unit_assert_equal(bytes2, TEST_DATA_SIZE);
  unit_assert_equal(strncmp((char *) data2, TEST_MESSAGE, bytes2), nil);
  unit_assert_ok(xMemFree(data2));
  unit_end();
  unit_begin("Simple device write sends single byte");
  data3 = SIMPLE_TEST_BYTE;
  unit_assert_ok(xDeviceSimpleWrite(LOOPBACK_DEVICE_ID, data3));
  unit_end();
  unit_begin("Simple device read retrieves single byte");
  data4 = nil;
  unit_assert_ok(xDeviceSimpleRead(LOOPBACK_DEVICE_ID, &data4));
  unit_assert_equal(data4, SIMPLE_TEST_BYTE);
  unit_end();

  return;
}