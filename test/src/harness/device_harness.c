/*UNCRUSTIFY-OFF*/
/**
 * @file device_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for device I/O
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


/* Helper function prototypes */
static void test_device_registration(void);
static void test_device_write_and_availability(void);
static void test_device_read(void);
static void test_device_simple_io(void);


void device_harness(void) {
  unit_print("=== COMPREHENSIVE DEVICE I/O TEST SUITE ===");

  test_device_registration();
  test_device_write_and_availability();
  test_device_read();
  test_device_simple_io();

  unit_print("=== DEVICE I/O TEST SUITE COMPLETE ===");
}


/* ============================================================================
 * SECTION 1: DEVICE REGISTRATION
 * ============================================================================ */
static void test_device_registration(void) {
  unit_print("--- Section 1: Device Registration ---");

  /* Test 1.1: Device registration */
  unit_begin("Loopback device registration succeeds");
  unit_assert_ok(xDeviceRegisterDevice(LOOPBACK_self_register));
  unit_end();
}


/* ============================================================================
 * SECTION 2: DEVICE WRITE AND AVAILABILITY
 * ============================================================================ */
static void test_device_write_and_availability(void) {
  Size_t bytesToWrite;
  Addr_t *writeData;
  Base_t isAvailable;


  unit_print("--- Section 2: Device Write and Availability ---");

  /* Test 2.1: Device write */
  unit_begin("Device write transmits test data successfully");
  bytesToWrite = TEST_DATA_SIZE;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  memcpy(writeData, TEST_MESSAGE, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  unit_end();

  /* Test 2.2: Device availability check */
  unit_begin("Device availability check returns true for active device");
  unit_assert_ok(xDeviceIsAvailable(LOOPBACK_DEVICE_ID, &isAvailable));
  unit_assert_true(isAvailable);
  unit_end();
}


/* ============================================================================
 * SECTION 3: DEVICE READ
 * ============================================================================ */
static void test_device_read(void) {
  Size_t bytesRead;
  Addr_t *readData;


  unit_print("--- Section 3: Device Read ---");

  /* Test 3.1: Device read */
  unit_begin("Device read retrieves previously written data");
  bytesRead = nil;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(bytesRead, TEST_DATA_SIZE);
  unit_assert_equal(strncmp((char *) readData, TEST_MESSAGE, bytesRead), nil);
  unit_assert_ok(xMemFree(readData));
  unit_end();
}


/* ============================================================================
 * SECTION 4: DEVICE SIMPLE I/O
 * ============================================================================ */
static void test_device_simple_io(void) {
  Byte_t writeByte;
  Byte_t readByte;


  unit_print("--- Section 4: Device Simple I/O ---");

  /* Test 4.1: Simple device write */
  unit_begin("Simple device write sends single byte");
  writeByte = SIMPLE_TEST_BYTE;
  unit_assert_ok(xDeviceSimpleWrite(LOOPBACK_DEVICE_ID, writeByte));
  unit_end();

  /* Test 4.2: Simple device read */
  unit_begin("Simple device read retrieves single byte");
  readByte = nil;
  unit_assert_ok(xDeviceSimpleRead(LOOPBACK_DEVICE_ID, &readByte));
  unit_assert_equal(readByte, SIMPLE_TEST_BYTE);
  unit_end();
}
