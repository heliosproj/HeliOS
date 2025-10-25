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
#define TEST_DATA_SMALL 0x10u /* 16 bytes */
#define TEST_DATA_MEDIUM 0x100u /* 256 bytes */
#define TEST_DATA_LARGE 0x400u /* 1024 bytes - full buffer */
#define SIMPLE_TEST_BYTE_1 0xFAu /* Test byte 1 */
#define SIMPLE_TEST_BYTE_2 0x5Cu /* Test byte 2 */
#define FILL_PATTERN_ZERO 0x00u /* Zero pattern */
#define FILL_PATTERN_FF 0xFFu /* All ones pattern */
#define FILL_PATTERN_AA 0xAAu /* Alternating pattern */
#define TEST_STRING_1 "LOOPBACK TEST STRING"
#define TEST_STRING_2 "DEVICE DRIVER VALIDATION"
#define TEST_POSITION_START 0x0u /* Start of buffer */
#define TEST_POSITION_MID 0x200u /* Middle of buffer */
#define BLOCK_SIZE_STANDARD 512u /* Standard block size */
#define BLOCK_COUNT_SINGLE 1u /* Single block */
#define BLOCK_COUNT_MULTIPLE 2u /* Multiple blocks */
/* Helper function prototypes */
static void test_device_registration_and_init(void);
static void test_configuration_commands(void);
static void test_basic_write_and_read(void);
static void test_simple_io_operations(void);
static void test_buffer_boundaries(void);
static void test_statistics_tracking(void);
static void test_circular_buffer_wraparound(void);
static void test_error_conditions(void);
static void test_block_io_interface(void);
static void test_large_data_transfers(void);


void device_harness(void) {
  unit_print("=== COMPREHENSIVE DEVICE I/O TEST SUITE ===");
  test_device_registration_and_init();
  test_configuration_commands();
  test_basic_write_and_read();
  test_simple_io_operations();
  test_buffer_boundaries();
  test_statistics_tracking();
  test_circular_buffer_wraparound();
  test_error_conditions();
  test_block_io_interface();
  test_large_data_transfers();
  unit_print("=== DEVICE I/O TEST SUITE COMPLETE ===");
}


/* ============================================================================
 * SECTION 1: DEVICE REGISTRATION AND INITIALIZATION
 * ============================================================================
 */
static void test_device_registration_and_init(void) {
  Base_t isAvailable;


  unit_print("--- Section 1: Device Registration and Initialization ---");


  /* Test 1.1: Device registration */
  unit_begin("Loopback device registration succeeds");
  unit_assert_ok(xDeviceRegisterDevice(LOOPBACK_self_register));
  unit_end();


  /* Test 1.2: Device availability after init */
  unit_begin("Device is not available after initialization");
  unit_assert_ok(xDeviceIsAvailable(LOOPBACK_DEVICE_ID, &isAvailable));
  unit_assert_false(isAvailable);
  unit_end();
}


/* ============================================================================
 * SECTION 2: CONFIGURATION COMMANDS
 * ============================================================================
 */
static void test_configuration_commands(void) {
  Size_t configSize;
  LoopbackClearConfig_t *clearCfg;
  LoopbackPositionConfig_t *posCfg;
  LoopbackStats_t *stats;
  LoopbackModeConfig_t *modeCfg;


  unit_print("--- Section 2: Configuration Commands ---");


  /* Test 2.1: Clear buffer command */
  unit_begin("Clear buffer with zero pattern");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &clearCfg, sizeof(LoopbackClearConfig_t)));
  clearCfg->command = LOOPBACK_CMD_CLEAR_BUFFER;
  clearCfg->fillPattern = FILL_PATTERN_ZERO;
  configSize = sizeof(LoopbackClearConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) clearCfg));
  xMemFree((Addr_t *) clearCfg);
  unit_end();


  /* Test 2.2: Clear buffer with 0xFF pattern */
  unit_begin("Clear buffer with 0xFF pattern");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &clearCfg, sizeof(LoopbackClearConfig_t)));
  clearCfg->command = LOOPBACK_CMD_CLEAR_BUFFER;
  clearCfg->fillPattern = FILL_PATTERN_FF;
  configSize = sizeof(LoopbackClearConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) clearCfg));
  xMemFree((Addr_t *) clearCfg);
  unit_end();


  /* Test 2.3: Get statistics command */
  unit_begin("Get device statistics");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &stats, sizeof(LoopbackStats_t)));
  stats->command = LOOPBACK_CMD_GET_STATS;
  configSize = sizeof(LoopbackStats_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) stats));
  unit_assert_equal(stats->bufferSize, TEST_DATA_LARGE);
  unit_assert_equal(stats->bytesAvailable, 0x0u);
  xMemFree((Addr_t *) stats);
  unit_end();


  /* Test 2.4: Set position command */
  unit_begin("Set read/write position to middle of buffer");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &posCfg, sizeof(LoopbackPositionConfig_t)));
  posCfg->command = LOOPBACK_CMD_SET_POSITION;
  posCfg->position = TEST_POSITION_MID;
  configSize = sizeof(LoopbackPositionConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) posCfg));
  xMemFree((Addr_t *) posCfg);
  unit_end();


  /* Test 2.5: Reset position to start */
  unit_begin("Reset position to start of buffer");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &posCfg, sizeof(LoopbackPositionConfig_t)));
  posCfg->command = LOOPBACK_CMD_SET_POSITION;
  posCfg->position = TEST_POSITION_START;
  configSize = sizeof(LoopbackPositionConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) posCfg));
  xMemFree((Addr_t *) posCfg);
  unit_end();


  /* Test 2.6: Set FIFO mode */
  unit_begin("Set device to FIFO mode");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &modeCfg, sizeof(LoopbackModeConfig_t)));
  modeCfg->command = LOOPBACK_CMD_SET_MODE;
  modeCfg->mode = LOOPBACK_MODE_FIFO;
  configSize = sizeof(LoopbackModeConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) modeCfg));
  xMemFree((Addr_t *) modeCfg);
  unit_end();


  /* Test 2.7: Set ECHO mode */
  unit_begin("Set device to ECHO mode");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &modeCfg, sizeof(LoopbackModeConfig_t)));
  modeCfg->command = LOOPBACK_CMD_SET_MODE;
  modeCfg->mode = LOOPBACK_MODE_ECHO;
  configSize = sizeof(LoopbackModeConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) modeCfg));
  xMemFree((Addr_t *) modeCfg);
  unit_end();


  /* Test 2.8: Reset to FIFO mode for subsequent tests */
  unit_begin("Reset device to FIFO mode");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &modeCfg, sizeof(LoopbackModeConfig_t)));
  modeCfg->command = LOOPBACK_CMD_SET_MODE;
  modeCfg->mode = LOOPBACK_MODE_FIFO;
  configSize = sizeof(LoopbackModeConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) modeCfg));
  xMemFree((Addr_t *) modeCfg);
  unit_end();
}


/* ============================================================================
 * SECTION 3: BASIC WRITE AND READ OPERATIONS
 * ============================================================================
 */
static void test_basic_write_and_read(void) {
  Size_t bytesToWrite;
  Size_t bytesRead;
  Addr_t *writeData;
  Addr_t *readData;
  Base_t isAvailable;


  unit_print("--- Section 3: Basic Write and Read Operations ---");


  /* Test 3.1: Write small data */
  unit_begin("Write small data block");
  bytesToWrite = TEST_DATA_SMALL;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  memcpy(writeData, TEST_STRING_1, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_equal(bytesToWrite, TEST_DATA_SMALL);
  unit_assert_ok(xMemFree(writeData));
  unit_end();


  /* Test 3.2: Device availability after write */
  unit_begin("Device is available after write");
  unit_assert_ok(xDeviceIsAvailable(LOOPBACK_DEVICE_ID, &isAvailable));
  unit_assert_true(isAvailable);
  unit_end();


  /* Test 3.3: Read small data */
  unit_begin("Read small data block");
  bytesRead = TEST_DATA_SMALL;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(bytesRead, TEST_DATA_SMALL);
  unit_assert_equal(strncmp((char *) readData, TEST_STRING_1, bytesRead), 0x0u);
  unit_assert_ok(xMemFree(readData));
  unit_end();


  /* Test 3.4: Device not available after read */
  unit_begin("Device is not available after read");
  unit_assert_ok(xDeviceIsAvailable(LOOPBACK_DEVICE_ID, &isAvailable));
  unit_assert_false(isAvailable);
  unit_end();


  /* Test 3.5: Write and read medium data */
  unit_begin("Write and read medium data block");
  bytesToWrite = TEST_DATA_MEDIUM;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, FILL_PATTERN_AA, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  bytesRead = TEST_DATA_MEDIUM;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(bytesRead, TEST_DATA_MEDIUM);
  unit_assert_equal(((Byte_t *) readData)[0], FILL_PATTERN_AA);
  unit_assert_equal(((Byte_t *) readData)[TEST_DATA_MEDIUM - 1], FILL_PATTERN_AA);
  unit_assert_ok(xMemFree(readData));
  unit_end();
}


/* ============================================================================
 * SECTION 4: SIMPLE I/O OPERATIONS
 * ============================================================================
 */
static void test_simple_io_operations(void) {
  Byte_t writeByte;
  Byte_t readByte;


  unit_print("--- Section 4: Simple I/O Operations ---");


  /* Test 4.1: Simple write single byte */
  unit_begin("Simple write sends single byte");
  writeByte = SIMPLE_TEST_BYTE_1;
  unit_assert_ok(xDeviceSimpleWrite(LOOPBACK_DEVICE_ID, writeByte));
  unit_end();


  /* Test 4.2: Simple read single byte */
  unit_begin("Simple read retrieves single byte");
  readByte = 0x0u;
  unit_assert_ok(xDeviceSimpleRead(LOOPBACK_DEVICE_ID, &readByte));
  unit_assert_equal(readByte, SIMPLE_TEST_BYTE_1);
  unit_end();


  /* Test 4.3: Write and read sequence */
  unit_begin("Simple write and read byte sequence");
  unit_assert_ok(xDeviceSimpleWrite(LOOPBACK_DEVICE_ID, SIMPLE_TEST_BYTE_1));
  unit_assert_ok(xDeviceSimpleWrite(LOOPBACK_DEVICE_ID, SIMPLE_TEST_BYTE_2));
  unit_assert_ok(xDeviceSimpleRead(LOOPBACK_DEVICE_ID, &readByte));
  unit_assert_equal(readByte, SIMPLE_TEST_BYTE_1);
  unit_assert_ok(xDeviceSimpleRead(LOOPBACK_DEVICE_ID, &readByte));
  unit_assert_equal(readByte, SIMPLE_TEST_BYTE_2);
  unit_end();
}


/* ============================================================================
 * SECTION 5: BUFFER BOUNDARIES
 * ============================================================================
 */
static void test_buffer_boundaries(void) {
  Size_t bytesToWrite;
  Size_t bytesRead;
  Addr_t *writeData;
  Addr_t *readData;
  LoopbackClearConfig_t *clearCfg;
  Size_t configSize;


  unit_print("--- Section 5: Buffer Boundaries ---");


  /* Test 5.1: Fill buffer to capacity */
  unit_begin("Write full buffer capacity");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &clearCfg, sizeof(LoopbackClearConfig_t)));
  clearCfg->command = LOOPBACK_CMD_CLEAR_BUFFER;
  clearCfg->fillPattern = FILL_PATTERN_ZERO;
  configSize = sizeof(LoopbackClearConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) clearCfg));
  xMemFree((Addr_t *) clearCfg);
  bytesToWrite = TEST_DATA_LARGE;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, FILL_PATTERN_AA, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_equal(bytesToWrite, TEST_DATA_LARGE);
  unit_assert_ok(xMemFree(writeData));
  unit_end();


  /* Test 5.2: Attempt write when buffer full */
  unit_begin("Write fails when buffer is full");
  bytesToWrite = TEST_DATA_SMALL;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, FILL_PATTERN_FF, bytesToWrite);
  unit_assert_not_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  unit_end();


  /* Test 5.3: Read full buffer */
  unit_begin("Read full buffer capacity");
  bytesRead = TEST_DATA_LARGE;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(bytesRead, TEST_DATA_LARGE);
  unit_assert_equal(((Byte_t *) readData)[0], FILL_PATTERN_AA);
  unit_assert_ok(xMemFree(readData));
  unit_end();


  /* Test 5.4: Attempt read when buffer empty */
  unit_begin("Read fails when buffer is empty");
  bytesRead = TEST_DATA_SMALL;
  readData = null;
  unit_assert_not_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_end();


  /* Test 5.5: Partial read from buffer */
  unit_begin("Partial read from buffer");
  bytesToWrite = TEST_DATA_MEDIUM;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, FILL_PATTERN_AA, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  bytesRead = TEST_DATA_SMALL;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(bytesRead, TEST_DATA_SMALL);
  unit_assert_ok(xMemFree(readData));
  unit_end();


  /* Test 5.6: Read remaining data */
  unit_begin("Read remaining data from buffer");
  bytesRead = TEST_DATA_LARGE;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(bytesRead, TEST_DATA_MEDIUM - TEST_DATA_SMALL);
  unit_assert_ok(xMemFree(readData));
  unit_end();
}


/* ============================================================================
 * SECTION 6: STATISTICS TRACKING
 * ============================================================================
 */
static void test_statistics_tracking(void) {
  LoopbackStats_t *stats;
  LoopbackClearConfig_t *clearCfg;
  Size_t configSize;
  Size_t bytesToWrite;
  Size_t bytesRead;
  Addr_t *writeData;
  Addr_t *readData;


  unit_print("--- Section 6: Statistics Tracking ---");


  /* Test 6.1: Initial statistics after clear */
  unit_begin("Statistics show zero after clear");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &clearCfg, sizeof(LoopbackClearConfig_t)));
  clearCfg->command = LOOPBACK_CMD_CLEAR_BUFFER;
  clearCfg->fillPattern = FILL_PATTERN_ZERO;
  configSize = sizeof(LoopbackClearConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) clearCfg));
  xMemFree((Addr_t *) clearCfg);
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &stats, sizeof(LoopbackStats_t)));
  stats->command = LOOPBACK_CMD_GET_STATS;
  configSize = sizeof(LoopbackStats_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) stats));
  unit_assert_equal(stats->bytesWritten, 0x0u);
  unit_assert_equal(stats->bytesRead, 0x0u);
  unit_assert_equal(stats->writeOperations, 0x0u);
  unit_assert_equal(stats->readOperations, 0x0u);
  xMemFree((Addr_t *) stats);
  unit_end();


  /* Test 6.2: Statistics after write */
  unit_begin("Statistics track write operations");
  bytesToWrite = TEST_DATA_SMALL;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, FILL_PATTERN_AA, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &stats, sizeof(LoopbackStats_t)));
  stats->command = LOOPBACK_CMD_GET_STATS;
  configSize = sizeof(LoopbackStats_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) stats));
  unit_assert_equal(stats->bytesWritten, TEST_DATA_SMALL);
  unit_assert_equal(stats->writeOperations, 0x1u);
  unit_assert_equal(stats->bytesAvailable, TEST_DATA_SMALL);
  xMemFree((Addr_t *) stats);
  unit_end();


  /* Test 6.3: Statistics after read */
  unit_begin("Statistics track read operations");
  bytesRead = TEST_DATA_SMALL;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_ok(xMemFree(readData));
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &stats, sizeof(LoopbackStats_t)));
  stats->command = LOOPBACK_CMD_GET_STATS;
  configSize = sizeof(LoopbackStats_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) stats));
  unit_assert_equal(stats->bytesRead, TEST_DATA_SMALL);
  unit_assert_equal(stats->readOperations, 0x1u);
  unit_assert_equal(stats->bytesAvailable, 0x0u);
  xMemFree((Addr_t *) stats);
  unit_end();


  /* Test 6.4: Statistics after multiple operations */
  unit_begin("Statistics track multiple operations");
  bytesToWrite = TEST_DATA_MEDIUM;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, FILL_PATTERN_FF, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  bytesRead = TEST_DATA_SMALL;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_ok(xMemFree(readData));
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &stats, sizeof(LoopbackStats_t)));
  stats->command = LOOPBACK_CMD_GET_STATS;
  configSize = sizeof(LoopbackStats_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) stats));
  unit_assert_equal(stats->bytesWritten, TEST_DATA_SMALL + TEST_DATA_MEDIUM);
  unit_assert_equal(stats->bytesRead, TEST_DATA_SMALL + TEST_DATA_SMALL);
  unit_assert_equal(stats->writeOperations, 0x2u);
  unit_assert_equal(stats->readOperations, 0x2u);
  xMemFree((Addr_t *) stats);
  unit_end();
}


/* ============================================================================
 * SECTION 7: CIRCULAR BUFFER WRAPAROUND
 * ============================================================================
 */
static void test_circular_buffer_wraparound(void) {
  LoopbackClearConfig_t *clearCfg;
  Size_t configSize;
  Size_t bytesToWrite;
  Size_t bytesRead;
  Addr_t *writeData;
  Addr_t *readData;
  HalfWord_t i;


  unit_print("--- Section 7: Circular Buffer Wraparound ---");


  /* Test 7.1: Write and read beyond buffer boundary */
  unit_begin("Circular buffer wraps around correctly");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &clearCfg, sizeof(LoopbackClearConfig_t)));
  clearCfg->command = LOOPBACK_CMD_CLEAR_BUFFER;
  clearCfg->fillPattern = FILL_PATTERN_ZERO;
  configSize = sizeof(LoopbackClearConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) clearCfg));
  xMemFree((Addr_t *) clearCfg);
  bytesToWrite = TEST_DATA_MEDIUM;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));

  for(i = 0x0u; i < TEST_DATA_MEDIUM; i++) {
    ((Byte_t *) writeData)[i] = (Byte_t) (i & 0xFFu);
  }

  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  bytesRead = TEST_DATA_SMALL;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(((Byte_t *) readData)[0], 0x0u);
  unit_assert_equal(((Byte_t *) readData)[TEST_DATA_SMALL - 1], TEST_DATA_SMALL - 1);
  unit_assert_ok(xMemFree(readData));
  unit_assert_ok(xMemFree(writeData));
  unit_end();


  /* Test 7.2: Sequential writes and reads */
  unit_begin("Sequential operations maintain order");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &clearCfg, sizeof(LoopbackClearConfig_t)));
  clearCfg->command = LOOPBACK_CMD_CLEAR_BUFFER;
  clearCfg->fillPattern = FILL_PATTERN_ZERO;
  configSize = sizeof(LoopbackClearConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) clearCfg));
  xMemFree((Addr_t *) clearCfg);
  bytesToWrite = TEST_DATA_SMALL;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, SIMPLE_TEST_BYTE_1, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  bytesRead = bytesToWrite;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(((Byte_t *) readData)[0], SIMPLE_TEST_BYTE_1);
  unit_assert_ok(xMemFree(readData));
  unit_end();
}


/* ============================================================================
 * SECTION 8: ERROR CONDITIONS
 * ============================================================================
 */
static void test_error_conditions(void) {
  Size_t bytesToWrite;
  Size_t bytesRead;
  Addr_t *writeData;
  Addr_t *readData;
  Byte_t readByte;
  LoopbackClearConfig_t *clearCfg;
  Size_t configSize;


  unit_print("--- Section 8: Error Conditions ---");


  /* Test 8.1: Write with NULL data pointer */
  unit_begin("Write rejects NULL data pointer");
  bytesToWrite = TEST_DATA_SMALL;
  unit_assert_not_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, null));
  unit_end();


  /* Test 8.2: Read with NULL data pointer */
  unit_begin("Read rejects NULL data pointer");
  bytesRead = TEST_DATA_SMALL;
  unit_assert_not_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, null));
  unit_end();


  /* Test 8.3: Simple read with NULL pointer */
  unit_begin("Simple read rejects NULL pointer");
  unit_assert_not_ok(xDeviceSimpleRead(LOOPBACK_DEVICE_ID, null));
  unit_end();


  /* Test 8.4: Read from empty buffer */
  unit_begin("Read from empty buffer fails");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &clearCfg, sizeof(LoopbackClearConfig_t)));
  clearCfg->command = LOOPBACK_CMD_CLEAR_BUFFER;
  clearCfg->fillPattern = FILL_PATTERN_ZERO;
  configSize = sizeof(LoopbackClearConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) clearCfg));
  xMemFree((Addr_t *) clearCfg);
  bytesRead = TEST_DATA_SMALL;
  readData = null;
  unit_assert_not_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_end();


  /* Test 8.5: Simple read from empty buffer */
  unit_begin("Simple read from empty buffer fails");
  readByte = 0x0u;
  unit_assert_not_ok(xDeviceSimpleRead(LOOPBACK_DEVICE_ID, &readByte));
  unit_end();


  /* Test 8.6: Write beyond buffer capacity */
  unit_begin("Write truncates at buffer capacity");
  bytesToWrite = TEST_DATA_LARGE;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, FILL_PATTERN_AA, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_equal(bytesToWrite, TEST_DATA_LARGE);
  bytesToWrite = TEST_DATA_SMALL;
  unit_assert_not_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  bytesRead = TEST_DATA_LARGE;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_ok(xMemFree(readData));
  unit_end();
}


/* ============================================================================
 * SECTION 9: BLOCK I/O INTERFACE
 * ============================================================================
 */
static void test_block_io_interface(void) {
  BlockIOInfo_t *info;
  BlockIORequest_t *request;
  LoopbackClearConfig_t *clearCfg;
  Size_t configSize;
  Size_t bytesToWrite;
  Size_t bytesRead;
  Addr_t *writeData;
  Addr_t *readData;


  unit_print("--- Section 9: Block I/O Interface ---");


  /* Test 9.1: Get block I/O info */
  unit_begin("Get block I/O capabilities");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &info, sizeof(BlockIOInfo_t)));
  info->command = BLOCK_IO_CMD_GET_INFO;
  configSize = sizeof(BlockIOInfo_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) info));
  unit_assert_equal(info->totalSizeBytes, TEST_DATA_LARGE);
  unit_assert_equal(info->nativeBlockSize, 0x1u);
  unit_assert_true(info->supportsRandomAccess);
  unit_assert_false(info->requiresErase);
  xMemFree((Addr_t *) info);
  unit_end();


  /* Test 9.2: Block I/O write request */
  unit_begin("Set block I/O write request");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &clearCfg, sizeof(LoopbackClearConfig_t)));
  clearCfg->command = LOOPBACK_CMD_CLEAR_BUFFER;
  clearCfg->fillPattern = FILL_PATTERN_ZERO;
  configSize = sizeof(LoopbackClearConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) clearCfg));
  xMemFree((Addr_t *) clearCfg);
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &request, sizeof(BlockIORequest_t)));
  request->command = BLOCK_IO_CMD_SET_REQUEST;
  request->operation = BLOCK_IO_OP_WRITE;
  request->blockNumber = 0x0u;
  request->blockCount = BLOCK_COUNT_SINGLE;
  request->blockSize = BLOCK_SIZE_STANDARD;
  configSize = sizeof(BlockIORequest_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) request));
  xMemFree((Addr_t *) request);
  unit_end();


  /* Test 9.3: Write block data */
  unit_begin("Write block data");
  bytesToWrite = BLOCK_SIZE_STANDARD;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, FILL_PATTERN_AA, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  unit_end();


  /* Test 9.4: Block I/O read request */
  unit_begin("Set block I/O read request");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &request, sizeof(BlockIORequest_t)));
  request->command = BLOCK_IO_CMD_SET_REQUEST;
  request->operation = BLOCK_IO_OP_READ;
  request->blockNumber = 0x0u;
  request->blockCount = BLOCK_COUNT_SINGLE;
  request->blockSize = BLOCK_SIZE_STANDARD;
  configSize = sizeof(BlockIORequest_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) request));
  xMemFree((Addr_t *) request);
  unit_end();


  /* Test 9.5: Read block data */
  unit_begin("Read block data");
  bytesRead = BLOCK_SIZE_STANDARD;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(bytesRead, BLOCK_SIZE_STANDARD);
  unit_assert_equal(((Byte_t *) readData)[0], FILL_PATTERN_AA);
  unit_assert_ok(xMemFree(readData));
  unit_end();
}


/* ============================================================================
 * SECTION 10: LARGE DATA TRANSFERS
 * ============================================================================
 */
static void test_large_data_transfers(void) {
  LoopbackClearConfig_t *clearCfg;
  Size_t configSize;
  Size_t bytesToWrite;
  Size_t bytesRead;
  Addr_t *writeData;
  Addr_t *readData;
  HalfWord_t i;


  unit_print("--- Section 10: Large Data Transfers ---");


  /* Test 10.1: Maximum size write */
  unit_begin("Write maximum buffer size");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &clearCfg, sizeof(LoopbackClearConfig_t)));
  clearCfg->command = LOOPBACK_CMD_CLEAR_BUFFER;
  clearCfg->fillPattern = FILL_PATTERN_ZERO;
  configSize = sizeof(LoopbackClearConfig_t);
  unit_assert_ok(xDeviceConfigDevice(LOOPBACK_DEVICE_ID, &configSize, (Addr_t *) clearCfg));
  xMemFree((Addr_t *) clearCfg);
  bytesToWrite = TEST_DATA_LARGE;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));

  for(i = 0x0u; i < TEST_DATA_LARGE; i++) {
    ((Byte_t *) writeData)[i] = (Byte_t) (i & 0xFFu);
  }

  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_equal(bytesToWrite, TEST_DATA_LARGE);
  unit_assert_ok(xMemFree(writeData));
  unit_end();


  /* Test 10.2: Maximum size read */
  unit_begin("Read maximum buffer size");
  bytesRead = TEST_DATA_LARGE;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(bytesRead, TEST_DATA_LARGE);

  for(i = 0x0u; i < TEST_DATA_LARGE; i++) {
    if(((Byte_t *) readData)[i] != (Byte_t) (i & 0xFFu)) {
      unit_assert_true(false);
      break;
    }
  }

  unit_assert_ok(xMemFree(readData));
  unit_end();


  /* Test 10.3: Multiple large transfers */
  unit_begin("Multiple sequential large transfers");
  bytesToWrite = TEST_DATA_MEDIUM;
  writeData = null;
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, bytesToWrite));
  __memset__(writeData, SIMPLE_TEST_BYTE_1, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  __memset__(writeData, SIMPLE_TEST_BYTE_2, bytesToWrite);
  unit_assert_ok(xDeviceWrite(LOOPBACK_DEVICE_ID, &bytesToWrite, writeData));
  unit_assert_ok(xMemFree(writeData));
  bytesRead = TEST_DATA_MEDIUM;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(((Byte_t *) readData)[0], SIMPLE_TEST_BYTE_1);
  unit_assert_ok(xMemFree(readData));
  bytesRead = TEST_DATA_MEDIUM;
  readData = null;
  unit_assert_ok(xDeviceRead(LOOPBACK_DEVICE_ID, &bytesRead, &readData));
  unit_assert_equal(((Byte_t *) readData)[0], SIMPLE_TEST_BYTE_2);
  unit_assert_ok(xMemFree(readData));
  unit_end();
}