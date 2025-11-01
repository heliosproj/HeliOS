/*UNCRUSTIFY-OFF*/
/**
 * @file fs_internal_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for filesystem internal functions
 * @details
 * Tests internal FAT filesystem utility functions with focus on:
 * - Edge cases (boundary values, zero, maximum values)
 * - Null pointer handling
 * - Data integrity (round-trip conversions)
 * - Endianness correctness
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "fs_internal_harness.h"
#include "../../src/fat.h"
#include "../../src/fs.h"


/* Test constants - Byte Order Conversion */
#define TEST_LE16_ZERO 0x0000u
#define TEST_LE16_ONE 0x0001u
#define TEST_LE16_MAX 0xFFFFu
#define TEST_LE16_PATTERN1 0x1234u
#define TEST_LE16_PATTERN2 0xABCDu
#define TEST_LE16_LOW_BYTE 0x00FFu
#define TEST_LE16_HIGH_BYTE 0xFF00u
#define TEST_LE32_ZERO 0x00000000u
#define TEST_LE32_ONE 0x00000001u
#define TEST_LE32_MAX 0xFFFFFFFFu
#define TEST_LE32_PATTERN1 0x12345678u
#define TEST_LE32_PATTERN2 0xABCDEF01u
#define TEST_LE32_LOW_WORD 0x0000FFFFu
#define TEST_LE32_HIGH_WORD 0xFFFF0000u
/* Test constants - Byte Comparison */
#define BYTE_CMP_LEN_ZERO 0x0u
#define BYTE_CMP_LEN_ONE 0x1u
#define BYTE_CMP_LEN_SMALL 0x5u
#define BYTE_CMP_LEN_MEDIUM 0xBu
#define BYTE_CMP_LEN_LARGE 0x100u
/* Test constants - FAT 8.3 Conversion */
#define FAT83_NAME_LENGTH 11
#define FAT83_BASENAME_LENGTH 8
#define FAT83_EXT_LENGTH 3
/* Test constants - Cluster/Sector Conversion */
#define TEST_BYTES_PER_SECTOR 512
#define TEST_SECTORS_PER_CLUSTER_1 1
#define TEST_SECTORS_PER_CLUSTER_4 4
#define TEST_SECTORS_PER_CLUSTER_8 8
#define TEST_SECTORS_PER_CLUSTER_64 64
#define TEST_RESERVED_SECTORS 32
#define TEST_NUM_FATS 2
#define TEST_SECTORS_PER_FAT 256
#define TEST_CLUSTER_FIRST_DATA 2
#define TEST_CLUSTER_THIRD 3
#define TEST_CLUSTER_TENTH 10
#define TEST_CLUSTER_HUNDREDTH 100
/* Test constants - Mount State Management */
#define TEST_DEVICE_UID_1 0x1000u
#define TEST_DEVICE_UID_2 0x2000u
#define TEST_DEVICE_UID_3 0x3000u
#define TEST_DEVICE_UID_4 0x4000u
#define TEST_DEVICE_UID_5 0x5000u
#define TEST_DEVICE_UID_6 0x6000u
#define TEST_DEVICE_UID_7 0x7000u
#define TEST_DEVICE_UID_8 0x8000u
#define TEST_DEVICE_UID_INVALID 0xFFFFu
/* Cleanup function declaration */
extern void __FSStateClear__(void);
/* Helper function prototypes */
static void test_read_le16(void);
static void test_read_le32(void);
static void test_write_le16(void);
static void test_write_le32(void);
static void test_byte_order_round_trip(void);
static void test_byte_compare_basic(void);
static void test_byte_compare_edge_cases(void);
static void test_byte_compare_null_pointers(void);
static void test_convert_to_fat83_basic(void);
static void test_convert_to_fat83_edge_cases(void);
static void test_convert_to_fat83_null_pointers(void);
static void test_cluster_to_sector_basic(void);
static void test_cluster_to_sector_configurations(void);
static void test_cluster_to_sector_null_pointer(void);
static void test_mount_state_basic(void);
static void test_mount_state_multiple_devices(void);
static void test_mount_state_edge_cases(void);


void fs_internal_harness(void) {
  unit_print("=== FILESYSTEM INTERNAL FUNCTIONS TEST SUITE ===");

  /* Byte order conversion tests */
  test_read_le16();
  test_read_le32();
  test_write_le16();
  test_write_le32();
  test_byte_order_round_trip();

  /* Byte comparison tests */
  test_byte_compare_basic();
  test_byte_compare_edge_cases();
  test_byte_compare_null_pointers();

  /* FAT 8.3 filename conversion tests */
  test_convert_to_fat83_basic();
  test_convert_to_fat83_edge_cases();
  test_convert_to_fat83_null_pointers();

  /* Cluster to sector conversion tests */
  test_cluster_to_sector_basic();
  test_cluster_to_sector_configurations();
  test_cluster_to_sector_null_pointer();

  /* Mount state management tests */
  test_mount_state_basic();
  test_mount_state_multiple_devices();
  test_mount_state_edge_cases();

  unit_print("=== FILESYSTEM INTERNAL FUNCTIONS TEST COMPLETE ===");

  /* Cleanup */
  __FSStateClear__();
}


/* ============================================================================
 * SECTION 1: BYTE ORDER CONVERSION - READ OPERATIONS
 * ============================================================================
 */
static void test_read_le16(void) {
  Byte_t buffer[2];
  HalfWord_t result;

  unit_print("--- Section 1.1: __ReadLE16__ Tests ---");

  /* Test 1.1.1: Read zero value */
  unit_begin("__ReadLE16__ - Read zero value");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  result = __ReadLE16__(buffer);
  unit_assert_equal(result, TEST_LE16_ZERO);
  unit_end();

  /* Test 1.1.2: Read one */
  unit_begin("__ReadLE16__ - Read value 0x0001");
  buffer[0] = 0x01u;
  buffer[1] = 0x00u;
  result = __ReadLE16__(buffer);
  unit_assert_equal(result, TEST_LE16_ONE);
  unit_end();

  /* Test 1.1.3: Read maximum value */
  unit_begin("__ReadLE16__ - Read maximum value 0xFFFF");
  buffer[0] = 0xFFu;
  buffer[1] = 0xFFu;
  result = __ReadLE16__(buffer);
  unit_assert_equal(result, TEST_LE16_MAX);
  unit_end();

  /* Test 1.1.4: Read pattern 0x1234 */
  unit_begin("__ReadLE16__ - Read pattern 0x1234 (little-endian)");
  buffer[0] = 0x34u; /* Low byte */
  buffer[1] = 0x12u; /* High byte */
  result = __ReadLE16__(buffer);
  unit_assert_equal(result, TEST_LE16_PATTERN1);
  unit_end();

  /* Test 1.1.5: Read pattern 0xABCD */
  unit_begin("__ReadLE16__ - Read pattern 0xABCD (little-endian)");
  buffer[0] = 0xCDu; /* Low byte */
  buffer[1] = 0xABu; /* High byte */
  result = __ReadLE16__(buffer);
  unit_assert_equal(result, TEST_LE16_PATTERN2);
  unit_end();

  /* Test 1.1.6: Read low byte only */
  unit_begin("__ReadLE16__ - Read value with only low byte set");
  buffer[0] = 0xFFu;
  buffer[1] = 0x00u;
  result = __ReadLE16__(buffer);
  unit_assert_equal(result, TEST_LE16_LOW_BYTE);
  unit_end();

  /* Test 1.1.7: Read high byte only */
  unit_begin("__ReadLE16__ - Read value with only high byte set");
  buffer[0] = 0x00u;
  buffer[1] = 0xFFu;
  result = __ReadLE16__(buffer);
  unit_assert_equal(result, TEST_LE16_HIGH_BYTE);
  unit_end();

  /* Test 1.1.8: Null pointer - behavior depends on implementation */
  unit_begin("__ReadLE16__ - Null pointer parameter");
  /* Note: Passing null will likely crash, but documenting expected behavior */
  /* In production code, this should be validated by caller */
  unit_print("Null pointer test skipped - undefined behavior");
  unit_end();
}


static void test_read_le32(void) {
  Byte_t buffer[4];
  Word_t result;

  unit_print("--- Section 1.2: __ReadLE32__ Tests ---");

  /* Test 1.2.1: Read zero value */
  unit_begin("__ReadLE32__ - Read zero value");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  buffer[2] = 0x00u;
  buffer[3] = 0x00u;
  result = __ReadLE32__(buffer);
  unit_assert_equal(result, TEST_LE32_ZERO);
  unit_end();

  /* Test 1.2.2: Read one */
  unit_begin("__ReadLE32__ - Read value 0x00000001");
  buffer[0] = 0x01u;
  buffer[1] = 0x00u;
  buffer[2] = 0x00u;
  buffer[3] = 0x00u;
  result = __ReadLE32__(buffer);
  unit_assert_equal(result, TEST_LE32_ONE);
  unit_end();

  /* Test 1.2.3: Read maximum value */
  unit_begin("__ReadLE32__ - Read maximum value 0xFFFFFFFF");
  buffer[0] = 0xFFu;
  buffer[1] = 0xFFu;
  buffer[2] = 0xFFu;
  buffer[3] = 0xFFu;
  result = __ReadLE32__(buffer);
  unit_assert_equal(result, TEST_LE32_MAX);
  unit_end();

  /* Test 1.2.4: Read pattern 0x12345678 */
  unit_begin("__ReadLE32__ - Read pattern 0x12345678 (little-endian)");
  buffer[0] = 0x78u; /* Byte 0 - lowest */
  buffer[1] = 0x56u; /* Byte 1 */
  buffer[2] = 0x34u; /* Byte 2 */
  buffer[3] = 0x12u; /* Byte 3 - highest */
  result = __ReadLE32__(buffer);
  unit_assert_equal(result, TEST_LE32_PATTERN1);
  unit_end();

  /* Test 1.2.5: Read pattern 0xABCDEF01 */
  unit_begin("__ReadLE32__ - Read pattern 0xABCDEF01 (little-endian)");
  buffer[0] = 0x01u; /* Byte 0 - lowest */
  buffer[1] = 0xEFu; /* Byte 1 */
  buffer[2] = 0xCDu; /* Byte 2 */
  buffer[3] = 0xABu; /* Byte 3 - highest */
  result = __ReadLE32__(buffer);
  unit_assert_equal(result, TEST_LE32_PATTERN2);
  unit_end();

  /* Test 1.2.6: Read low word only */
  unit_begin("__ReadLE32__ - Read value with only low word set");
  buffer[0] = 0xFFu;
  buffer[1] = 0xFFu;
  buffer[2] = 0x00u;
  buffer[3] = 0x00u;
  result = __ReadLE32__(buffer);
  unit_assert_equal(result, TEST_LE32_LOW_WORD);
  unit_end();

  /* Test 1.2.7: Read high word only */
  unit_begin("__ReadLE32__ - Read value with only high word set");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  buffer[2] = 0xFFu;
  buffer[3] = 0xFFu;
  result = __ReadLE32__(buffer);
  unit_assert_equal(result, TEST_LE32_HIGH_WORD);
  unit_end();

  /* Test 1.2.8: Null pointer */
  unit_begin("__ReadLE32__ - Null pointer parameter");
  unit_print("Null pointer test skipped - undefined behavior");
  unit_end();
}


/* ============================================================================
 * SECTION 2: BYTE ORDER CONVERSION - WRITE OPERATIONS
 * ============================================================================
 */
static void test_write_le16(void) {
  Byte_t buffer[2];

  unit_print("--- Section 2.1: __WriteLE16__ Tests ---");

  /* Test 2.1.1: Write zero value */
  unit_begin("__WriteLE16__ - Write zero value");
  buffer[0] = 0xFFu;
  buffer[1] = 0xFFu;
  __WriteLE16__(buffer, TEST_LE16_ZERO);
  unit_assert_equal(buffer[0], 0x00u);
  unit_assert_equal(buffer[1], 0x00u);
  unit_end();

  /* Test 2.1.2: Write one */
  unit_begin("__WriteLE16__ - Write value 0x0001");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  __WriteLE16__(buffer, TEST_LE16_ONE);
  unit_assert_equal(buffer[0], 0x01u);
  unit_assert_equal(buffer[1], 0x00u);
  unit_end();

  /* Test 2.1.3: Write maximum value */
  unit_begin("__WriteLE16__ - Write maximum value 0xFFFF");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  __WriteLE16__(buffer, TEST_LE16_MAX);
  unit_assert_equal(buffer[0], 0xFFu);
  unit_assert_equal(buffer[1], 0xFFu);
  unit_end();

  /* Test 2.1.4: Write pattern 0x1234 */
  unit_begin("__WriteLE16__ - Write pattern 0x1234 (little-endian)");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  __WriteLE16__(buffer, TEST_LE16_PATTERN1);
  unit_assert_equal(buffer[0], 0x34u); /* Low byte */
  unit_assert_equal(buffer[1], 0x12u); /* High byte */
  unit_end();

  /* Test 2.1.5: Write pattern 0xABCD */
  unit_begin("__WriteLE16__ - Write pattern 0xABCD (little-endian)");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  __WriteLE16__(buffer, TEST_LE16_PATTERN2);
  unit_assert_equal(buffer[0], 0xCDu); /* Low byte */
  unit_assert_equal(buffer[1], 0xABu); /* High byte */
  unit_end();

  /* Test 2.1.6: Write low byte only */
  unit_begin("__WriteLE16__ - Write value with only low byte set");
  buffer[0] = 0xFFu;
  buffer[1] = 0xFFu;
  __WriteLE16__(buffer, TEST_LE16_LOW_BYTE);
  unit_assert_equal(buffer[0], 0xFFu);
  unit_assert_equal(buffer[1], 0x00u);
  unit_end();

  /* Test 2.1.7: Write high byte only */
  unit_begin("__WriteLE16__ - Write value with only high byte set");
  buffer[0] = 0xFFu;
  buffer[1] = 0xFFu;
  __WriteLE16__(buffer, TEST_LE16_HIGH_BYTE);
  unit_assert_equal(buffer[0], 0x00u);
  unit_assert_equal(buffer[1], 0xFFu);
  unit_end();

  /* Test 2.1.8: Null pointer */
  unit_begin("__WriteLE16__ - Null pointer parameter");
  unit_print("Null pointer test skipped - undefined behavior");
  unit_end();
}


static void test_write_le32(void) {
  Byte_t buffer[4];

  unit_print("--- Section 2.2: __WriteLE32__ Tests ---");

  /* Test 2.2.1: Write zero value */
  unit_begin("__WriteLE32__ - Write zero value");
  buffer[0] = 0xFFu;
  buffer[1] = 0xFFu;
  buffer[2] = 0xFFu;
  buffer[3] = 0xFFu;
  __WriteLE32__(buffer, TEST_LE32_ZERO);
  unit_assert_equal(buffer[0], 0x00u);
  unit_assert_equal(buffer[1], 0x00u);
  unit_assert_equal(buffer[2], 0x00u);
  unit_assert_equal(buffer[3], 0x00u);
  unit_end();

  /* Test 2.2.2: Write one */
  unit_begin("__WriteLE32__ - Write value 0x00000001");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  buffer[2] = 0x00u;
  buffer[3] = 0x00u;
  __WriteLE32__(buffer, TEST_LE32_ONE);
  unit_assert_equal(buffer[0], 0x01u);
  unit_assert_equal(buffer[1], 0x00u);
  unit_assert_equal(buffer[2], 0x00u);
  unit_assert_equal(buffer[3], 0x00u);
  unit_end();

  /* Test 2.2.3: Write maximum value */
  unit_begin("__WriteLE32__ - Write maximum value 0xFFFFFFFF");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  buffer[2] = 0x00u;
  buffer[3] = 0x00u;
  __WriteLE32__(buffer, TEST_LE32_MAX);
  unit_assert_equal(buffer[0], 0xFFu);
  unit_assert_equal(buffer[1], 0xFFu);
  unit_assert_equal(buffer[2], 0xFFu);
  unit_assert_equal(buffer[3], 0xFFu);
  unit_end();

  /* Test 2.2.4: Write pattern 0x12345678 */
  unit_begin("__WriteLE32__ - Write pattern 0x12345678 (little-endian)");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  buffer[2] = 0x00u;
  buffer[3] = 0x00u;
  __WriteLE32__(buffer, TEST_LE32_PATTERN1);
  unit_assert_equal(buffer[0], 0x78u); /* Byte 0 - lowest */
  unit_assert_equal(buffer[1], 0x56u); /* Byte 1 */
  unit_assert_equal(buffer[2], 0x34u); /* Byte 2 */
  unit_assert_equal(buffer[3], 0x12u); /* Byte 3 - highest */
  unit_end();

  /* Test 2.2.5: Write pattern 0xABCDEF01 */
  unit_begin("__WriteLE32__ - Write pattern 0xABCDEF01 (little-endian)");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  buffer[2] = 0x00u;
  buffer[3] = 0x00u;
  __WriteLE32__(buffer, TEST_LE32_PATTERN2);
  unit_assert_equal(buffer[0], 0x01u); /* Byte 0 - lowest */
  unit_assert_equal(buffer[1], 0xEFu); /* Byte 1 */
  unit_assert_equal(buffer[2], 0xCDu); /* Byte 2 */
  unit_assert_equal(buffer[3], 0xABu); /* Byte 3 - highest */
  unit_end();

  /* Test 2.2.6: Write low word only */
  unit_begin("__WriteLE32__ - Write value with only low word set");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  buffer[2] = 0x00u;
  buffer[3] = 0x00u;
  __WriteLE32__(buffer, TEST_LE32_LOW_WORD);
  unit_assert_equal(buffer[0], 0xFFu);
  unit_assert_equal(buffer[1], 0xFFu);
  unit_assert_equal(buffer[2], 0x00u);
  unit_assert_equal(buffer[3], 0x00u);
  unit_end();

  /* Test 2.2.7: Write high word only */
  unit_begin("__WriteLE32__ - Write value with only high word set");
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  buffer[2] = 0x00u;
  buffer[3] = 0x00u;
  __WriteLE32__(buffer, TEST_LE32_HIGH_WORD);
  unit_assert_equal(buffer[0], 0x00u);
  unit_assert_equal(buffer[1], 0x00u);
  unit_assert_equal(buffer[2], 0xFFu);
  unit_assert_equal(buffer[3], 0xFFu);
  unit_end();

  /* Test 2.2.8: Null pointer */
  unit_begin("__WriteLE32__ - Null pointer parameter");
  unit_print("Null pointer test skipped - undefined behavior");
  unit_end();
}


/* ============================================================================
 * SECTION 3: BYTE ORDER CONVERSION - ROUND TRIP TESTS
 * ============================================================================
 */
static void test_byte_order_round_trip(void) {
  Byte_t buffer16[2];
  Byte_t buffer32[4];
  HalfWord_t value16;
  Word_t value32;

  unit_print("--- Section 3: Byte Order Round-Trip Tests ---");

  /* Test 3.1: 16-bit round trip - zero */
  unit_begin("Round-trip 16-bit - Zero value");
  __WriteLE16__(buffer16, TEST_LE16_ZERO);
  value16 = __ReadLE16__(buffer16);
  unit_assert_equal(value16, TEST_LE16_ZERO);
  unit_end();

  /* Test 3.2: 16-bit round trip - maximum */
  unit_begin("Round-trip 16-bit - Maximum value");
  __WriteLE16__(buffer16, TEST_LE16_MAX);
  value16 = __ReadLE16__(buffer16);
  unit_assert_equal(value16, TEST_LE16_MAX);
  unit_end();

  /* Test 3.3: 16-bit round trip - pattern */
  unit_begin("Round-trip 16-bit - Pattern 0x1234");
  __WriteLE16__(buffer16, TEST_LE16_PATTERN1);
  value16 = __ReadLE16__(buffer16);
  unit_assert_equal(value16, TEST_LE16_PATTERN1);
  unit_end();

  /* Test 3.4: 32-bit round trip - zero */
  unit_begin("Round-trip 32-bit - Zero value");
  __WriteLE32__(buffer32, TEST_LE32_ZERO);
  value32 = __ReadLE32__(buffer32);
  unit_assert_equal(value32, TEST_LE32_ZERO);
  unit_end();

  /* Test 3.5: 32-bit round trip - maximum */
  unit_begin("Round-trip 32-bit - Maximum value");
  __WriteLE32__(buffer32, TEST_LE32_MAX);
  value32 = __ReadLE32__(buffer32);
  unit_assert_equal(value32, TEST_LE32_MAX);
  unit_end();

  /* Test 3.6: 32-bit round trip - pattern */
  unit_begin("Round-trip 32-bit - Pattern 0x12345678");
  __WriteLE32__(buffer32, TEST_LE32_PATTERN1);
  value32 = __ReadLE32__(buffer32);
  unit_assert_equal(value32, TEST_LE32_PATTERN1);
  unit_end();
}


/* ============================================================================
 * SECTION 4: BYTE COMPARISON TESTS
 * ============================================================================
 */
static void test_byte_compare_basic(void) {
  const Byte_t pattern1[] = "TESTDATA";
  const Byte_t pattern2[] = "TESTDATA";
  const Byte_t pattern3[] = "TESTDATX";
  const Byte_t pattern4[] = "XESTDATA";

  unit_print("--- Section 4.1: __ByteCompare__ Basic Tests ---");

  /* Test 4.1.1: Identical strings */
  unit_begin("__ByteCompare__ - Identical byte sequences");
  unit_assert_true(__ByteCompare__(pattern1, pattern2, BYTE_CMP_LEN_MEDIUM));
  unit_end();

  /* Test 4.1.2: Different at end */
  unit_begin("__ByteCompare__ - Different byte at end");
  unit_assert_false(__ByteCompare__(pattern1, pattern3, BYTE_CMP_LEN_MEDIUM));
  unit_end();

  /* Test 4.1.3: Different at start */
  unit_begin("__ByteCompare__ - Different byte at start");
  unit_assert_false(__ByteCompare__(pattern1, pattern4, BYTE_CMP_LEN_MEDIUM));
  unit_end();

  /* Test 4.1.4: Partial match - compare only matching portion */
  unit_begin("__ByteCompare__ - Partial match within longer strings");
  /* pattern1 = "TESTDATA", pattern3 = "TESTDATX" - first 7 chars match */
  unit_assert_true(__ByteCompare__(pattern1, pattern3, 7));
  unit_end();

  /* Test 4.1.5: Single byte comparison - equal */
  unit_begin("__ByteCompare__ - Single byte comparison (equal)");
  unit_assert_true(__ByteCompare__(pattern1, pattern2, BYTE_CMP_LEN_ONE));
  unit_end();

  /* Test 4.1.6: Single byte comparison - not equal */
  unit_begin("__ByteCompare__ - Single byte comparison (not equal)");
  unit_assert_false(__ByteCompare__(pattern1, pattern4, BYTE_CMP_LEN_ONE));
  unit_end();
}


static void test_byte_compare_edge_cases(void) {
  const Byte_t zeros[16] = {
    0
  };
  const Byte_t ones[16] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  };
  const Byte_t ffs[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  const Byte_t mixed1[16] = {
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
  };
  const Byte_t mixed2[16] = {
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
  };

  unit_print("--- Section 4.2: __ByteCompare__ Edge Cases ---");

  /* Test 4.2.1: Zero length comparison */
  unit_begin("__ByteCompare__ - Zero length comparison");
  unit_assert_true(__ByteCompare__(zeros, ones, BYTE_CMP_LEN_ZERO));
  unit_end();

  /* Test 4.2.2: All zeros */
  unit_begin("__ByteCompare__ - All zero bytes");
  unit_assert_true(__ByteCompare__(zeros, zeros, sizeof(zeros)));
  unit_end();

  /* Test 4.2.3: All 0xFF */
  unit_begin("__ByteCompare__ - All 0xFF bytes");
  unit_assert_true(__ByteCompare__(ffs, ffs, sizeof(ffs)));
  unit_end();

  /* Test 4.2.4: Zeros vs 0xFF */
  unit_begin("__ByteCompare__ - All zeros vs all 0xFF");
  unit_assert_false(__ByteCompare__(zeros, ffs, sizeof(zeros)));
  unit_end();

  /* Test 4.2.5: Pattern match */
  unit_begin("__ByteCompare__ - Alternating pattern match");
  unit_assert_true(__ByteCompare__(mixed1, mixed2, sizeof(mixed1)));
  unit_end();

  /* Test 4.2.6: Large comparison */
  unit_begin("__ByteCompare__ - Large byte sequence");
  Byte_t large1[256];
  Byte_t large2[256];
  Word_t i;

  for(i = 0x0u; i < BYTE_CMP_LEN_LARGE; i++) {
    large1[i] = (Byte_t) (i & 0xFFu);
    large2[i] = (Byte_t) (i & 0xFFu);
  }

  unit_assert_true(__ByteCompare__(large1, large2, BYTE_CMP_LEN_LARGE));
  unit_end();

  /* Test 4.2.7: Large comparison with one difference */
  unit_begin("__ByteCompare__ - Large sequence with one byte different");
  large2[128] ^= 0x01u; /* Flip one bit in middle */
  unit_assert_false(__ByteCompare__(large1, large2, BYTE_CMP_LEN_LARGE));
  unit_end();
}


static void test_byte_compare_null_pointers(void) {
  const Byte_t pattern[] = "TEST";

  unit_print("--- Section 4.3: __ByteCompare__ Null Pointer Tests ---");

  /* Test 4.3.1: First parameter null */
  unit_begin("__ByteCompare__ - First parameter null");
  unit_print("Null pointer test skipped - would cause assertion/crash");
  unit_end();

  /* Test 4.3.2: Second parameter null */
  unit_begin("__ByteCompare__ - Second parameter null");
  unit_print("Null pointer test skipped - would cause assertion/crash");
  unit_end();

  /* Test 4.3.3: Both parameters null */
  unit_begin("__ByteCompare__ - Both parameters null");
  unit_print("Null pointer test skipped - would cause assertion/crash");
  unit_end();

  /* Test 4.3.4: Valid comparison for reference */
  unit_begin("__ByteCompare__ - Valid parameters (reference test)");
  unit_assert_true(__ByteCompare__(pattern, pattern, sizeof(pattern)));
  unit_end();
}


/* ============================================================================
 * SECTION 5: FAT 8.3 FILENAME CONVERSION TESTS
 * ============================================================================
 */
static void test_convert_to_fat83_basic(void) {
  Byte_t fat83[FAT83_NAME_LENGTH];
  Return_t result;

  unit_print("--- Section 5.1: __ConvertToFAT83__ Basic Tests ---");

  /* Test 5.1.1: Simple filename with extension */
  unit_begin("__ConvertToFAT83__ - Simple filename 'test.txt'");
  result = __ConvertToFAT83__((const Byte_t *) "test.txt", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "TEST    TXT", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.1.2: Filename without extension */
  unit_begin("__ConvertToFAT83__ - Filename without extension 'readme'");
  result = __ConvertToFAT83__((const Byte_t *) "readme", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "README     ", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.1.3: Already uppercase */
  unit_begin("__ConvertToFAT83__ - Already uppercase 'FILE.DAT'");
  result = __ConvertToFAT83__((const Byte_t *) "FILE.DAT", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "FILE    DAT", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.1.4: Lowercase to uppercase conversion */
  unit_begin("__ConvertToFAT83__ - Lowercase 'data.bin'");
  result = __ConvertToFAT83__((const Byte_t *) "data.bin", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "DATA    BIN", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.1.5: Mixed case */
  unit_begin("__ConvertToFAT83__ - Mixed case 'MyFile.Txt'");
  result = __ConvertToFAT83__((const Byte_t *) "MyFile.Txt", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "MYFILE  TXT", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.1.6: Short extension */
  unit_begin("__ConvertToFAT83__ - Short extension 'file.c'");
  result = __ConvertToFAT83__((const Byte_t *) "file.c", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "FILE    C  ", FAT83_NAME_LENGTH));
  unit_end();
}


static void test_convert_to_fat83_edge_cases(void) {
  Byte_t fat83[FAT83_NAME_LENGTH];
  Return_t result;

  unit_print("--- Section 5.2: __ConvertToFAT83__ Edge Cases ---");

  /* Test 5.2.1: Maximum length basename (8 chars) */
  unit_begin("__ConvertToFAT83__ - Maximum basename length 'abcdefgh.txt'");
  result = __ConvertToFAT83__((const Byte_t *) "abcdefgh.txt", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "ABCDEFGHTXT", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.2.2: Maximum length extension (3 chars) */
  unit_begin("__ConvertToFAT83__ - Maximum extension length 'file.abc'");
  result = __ConvertToFAT83__((const Byte_t *) "file.abc", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "FILE    ABC", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.2.3: Basename truncation (more than 8 chars) */
  unit_begin("__ConvertToFAT83__ - Basename truncation 'verylongname.txt'");
  result = __ConvertToFAT83__((const Byte_t *) "verylongname.txt", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "VERYLONGTXT", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.2.4: Extension truncation (more than 3 chars) */
  unit_begin("__ConvertToFAT83__ - Extension truncation 'file.longext'");
  result = __ConvertToFAT83__((const Byte_t *) "file.longext", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "FILE    LON", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.2.5: Single character filename */
  unit_begin("__ConvertToFAT83__ - Single character 'a.b'");
  result = __ConvertToFAT83__((const Byte_t *) "a.b", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "A       B  ", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.2.6: Multiple dots (use last one) */
  unit_begin("__ConvertToFAT83__ - Multiple dots 'my.file.txt'");
  result = __ConvertToFAT83__((const Byte_t *) "my.file.txt", fat83);
  unit_assert_ok(result);
  /* Should use last dot, so basename is "my.file" (truncated to "MY.FILE" then "MY") */
  /* and extension is "txt" */
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "MY      TXT", FAT83_NAME_LENGTH) ||
    __ByteCompare__(fat83, (const Byte_t *) "MY.FILE TXT", FAT83_NAME_LENGTH));
  unit_end();

  /* Test 5.2.7: Dot at start (hidden file in Unix) */
  unit_begin("__ConvertToFAT83__ - Dot at start '.hidden'");
  result = __ConvertToFAT83__((const Byte_t *) ".hidden", fat83);
  unit_assert_ok(result);
  /* Behavior may vary - basename could be empty or "." */
  unit_print("Edge case - implementation specific behavior");
  unit_end();

  /* Test 5.2.8: Numbers and special chars */
  unit_begin("__ConvertToFAT83__ - Numbers 'file123.456'");
  result = __ConvertToFAT83__((const Byte_t *) "file123.456", fat83);
  unit_assert_ok(result);
  unit_assert_true(__ByteCompare__(fat83, (const Byte_t *) "FILE123 456", FAT83_NAME_LENGTH));
  unit_end();
}


static void test_convert_to_fat83_null_pointers(void) {
  Byte_t fat83[FAT83_NAME_LENGTH];
  Return_t result;

  unit_print("--- Section 5.3: __ConvertToFAT83__ Null Pointer Tests ---");

  /* Test 5.3.1: Null input path */
  unit_begin("__ConvertToFAT83__ - Null input path parameter");
  result = __ConvertToFAT83__(null, fat83);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 5.3.2: Null output buffer */
  unit_begin("__ConvertToFAT83__ - Null output buffer parameter");
  result = __ConvertToFAT83__((const Byte_t *) "test.txt", null);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 5.3.3: Both parameters null */
  unit_begin("__ConvertToFAT83__ - Both parameters null");
  result = __ConvertToFAT83__(null, null);
  unit_assert_not_ok(result);
  unit_end();

  /* Test 5.3.4: Valid parameters for reference */
  unit_begin("__ConvertToFAT83__ - Valid parameters (reference test)");
  result = __ConvertToFAT83__((const Byte_t *) "valid.txt", fat83);
  unit_assert_ok(result);
  unit_end();
}


/* ============================================================================
 * SECTION 6: CLUSTER TO SECTOR CONVERSION TESTS
 * ============================================================================
 */
static void test_cluster_to_sector_basic(void) {
  Volume_t vol;
  Word_t sector;

  unit_print("--- Section 6.1: __ClusterToSector__ Basic Tests ---");

  /* Setup test volume configuration */
  vol.bytesPerSector = TEST_BYTES_PER_SECTOR;
  vol.sectorsPerCluster = TEST_SECTORS_PER_CLUSTER_8;
  vol.reservedSectors = TEST_RESERVED_SECTORS;
  vol.numFATs = TEST_NUM_FATS;
  vol.sectorsPerFAT = TEST_SECTORS_PER_FAT;
  vol.dataStartSector = vol.reservedSectors + (vol.numFATs * vol.sectorsPerFAT);

  /* Test 6.1.1: First data cluster (cluster 2) */
  unit_begin("__ClusterToSector__ - First data cluster (2)");
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_FIRST_DATA);
  unit_assert_equal(sector, vol.dataStartSector);
  unit_end();

  /* Test 6.1.2: Cluster 3 */
  unit_begin("__ClusterToSector__ - Cluster 3");
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_THIRD);
  unit_assert_equal(sector, vol.dataStartSector + TEST_SECTORS_PER_CLUSTER_8);
  unit_end();

  /* Test 6.1.3: Cluster 10 */
  unit_begin("__ClusterToSector__ - Cluster 10");
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_TENTH);
  unit_assert_equal(sector, vol.dataStartSector + (8 * TEST_SECTORS_PER_CLUSTER_8));
  unit_end();

  /* Test 6.1.4: Cluster 100 */
  unit_begin("__ClusterToSector__ - Cluster 100");
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_HUNDREDTH);
  unit_assert_equal(sector, vol.dataStartSector + (98 * TEST_SECTORS_PER_CLUSTER_8));
  unit_end();

  /* Test 6.1.5: Verify formula - dataStartSector + ((cluster - 2) * sectorsPerCluster) */
  unit_begin("__ClusterToSector__ - Verify calculation formula");
  Word_t testCluster = 50;
  Word_t expectedSector = vol.dataStartSector + ((testCluster - 2) * vol.sectorsPerCluster);
  sector = __ClusterToSector__(&vol, testCluster);
  unit_assert_equal(sector, expectedSector);
  unit_end();
}


static void test_cluster_to_sector_configurations(void) {
  Volume_t vol;
  Word_t sector;

  unit_print("--- Section 6.2: __ClusterToSector__ Different Configurations ---");

  /* Base setup */
  vol.bytesPerSector = TEST_BYTES_PER_SECTOR;
  vol.reservedSectors = TEST_RESERVED_SECTORS;
  vol.numFATs = TEST_NUM_FATS;
  vol.sectorsPerFAT = TEST_SECTORS_PER_FAT;

  /* Test 6.2.1: Configuration with 1 sector per cluster */
  unit_begin("__ClusterToSector__ - 1 sector per cluster");
  vol.sectorsPerCluster = TEST_SECTORS_PER_CLUSTER_1;
  vol.dataStartSector = vol.reservedSectors + (vol.numFATs * vol.sectorsPerFAT);
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_TENTH);
  unit_assert_equal(sector, vol.dataStartSector + 8); /* (10 - 2) * 1 */
  unit_end();

  /* Test 6.2.2: Configuration with 4 sectors per cluster */
  unit_begin("__ClusterToSector__ - 4 sectors per cluster");
  vol.sectorsPerCluster = TEST_SECTORS_PER_CLUSTER_4;
  vol.dataStartSector = vol.reservedSectors + (vol.numFATs * vol.sectorsPerFAT);
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_TENTH);
  unit_assert_equal(sector, vol.dataStartSector + 32); /* (10 - 2) * 4 */
  unit_end();

  /* Test 6.2.3: Configuration with 8 sectors per cluster */
  unit_begin("__ClusterToSector__ - 8 sectors per cluster");
  vol.sectorsPerCluster = TEST_SECTORS_PER_CLUSTER_8;
  vol.dataStartSector = vol.reservedSectors + (vol.numFATs * vol.sectorsPerFAT);
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_TENTH);
  unit_assert_equal(sector, vol.dataStartSector + 64); /* (10 - 2) * 8 */
  unit_end();

  /* Test 6.2.4: Configuration with 64 sectors per cluster */
  unit_begin("__ClusterToSector__ - 64 sectors per cluster");
  vol.sectorsPerCluster = TEST_SECTORS_PER_CLUSTER_64;
  vol.dataStartSector = vol.reservedSectors + (vol.numFATs * vol.sectorsPerFAT);
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_TENTH);
  unit_assert_equal(sector, vol.dataStartSector + 512); /* (10 - 2) * 64 */
  unit_end();

  /* Test 6.2.5: Different data start sector */
  unit_begin("__ClusterToSector__ - Different data start sector");
  vol.sectorsPerCluster = TEST_SECTORS_PER_CLUSTER_8;
  vol.reservedSectors = 64; /* Different reserved sectors */
  vol.dataStartSector = vol.reservedSectors + (vol.numFATs * vol.sectorsPerFAT);
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_FIRST_DATA);
  unit_assert_equal(sector, vol.dataStartSector);
  unit_end();
}


static void test_cluster_to_sector_null_pointer(void) {
  unit_print("--- Section 6.3: __ClusterToSector__ Null Pointer Tests ---");

  /* Test 6.3.1: Null volume pointer */
  unit_begin("__ClusterToSector__ - Null volume pointer");
  unit_print("Null pointer test skipped - would cause crash/undefined behavior");
  unit_end();

  /* Test 6.3.2: Valid volume for reference */
  unit_begin("__ClusterToSector__ - Valid volume pointer (reference test)");
  Volume_t vol;
  Word_t sector;
  vol.sectorsPerCluster = TEST_SECTORS_PER_CLUSTER_8;
  vol.dataStartSector = 544; /* 32 + (2 * 256) */
  sector = __ClusterToSector__(&vol, TEST_CLUSTER_FIRST_DATA);
  unit_assert_equal(sector, 544);
  unit_end();
}


/* ============================================================================
 * SECTION 7: MOUNT STATE MANAGEMENT TESTS
 * ============================================================================
 */
static void test_mount_state_basic(void) {
  Base_t isMounted;
  Return_t result;

  unit_print("--- Section 7.1: Mount State Management Basic Tests ---");

  /* Clear state before tests */
  __FSStateClear__();

  /* Test 7.1.1: Check unmounted device */
  unit_begin("Mount State - Check unmounted device");
  isMounted = __IsDeviceMounted__(TEST_DEVICE_UID_1);
  unit_assert_false(isMounted);
  unit_end();

  /* Test 7.1.2: Add device to mounted list */
  unit_begin("Mount State - Add device to mounted list");
  result = __AddMountedDevice__(TEST_DEVICE_UID_1);
  unit_assert_ok(result);
  unit_end();

  /* Test 7.1.3: Verify device is now mounted */
  unit_begin("Mount State - Verify device is mounted after add");
  isMounted = __IsDeviceMounted__(TEST_DEVICE_UID_1);
  unit_assert_true(isMounted);
  unit_end();

  /* Test 7.1.4: Check different device still unmounted */
  unit_begin("Mount State - Check different device still unmounted");
  isMounted = __IsDeviceMounted__(TEST_DEVICE_UID_2);
  unit_assert_false(isMounted);
  unit_end();

  /* Test 7.1.5: Remove mounted device */
  unit_begin("Mount State - Remove mounted device");
  result = __RemoveMountedDevice__(TEST_DEVICE_UID_1);
  unit_assert_ok(result);
  unit_end();

  /* Test 7.1.6: Verify device is no longer mounted */
  unit_begin("Mount State - Verify device unmounted after remove");
  isMounted = __IsDeviceMounted__(TEST_DEVICE_UID_1);
  unit_assert_false(isMounted);
  unit_end();

  /* Test 7.1.7: Remove non-existent device */
  unit_begin("Mount State - Remove non-existent device returns error");
  result = __RemoveMountedDevice__(TEST_DEVICE_UID_INVALID);
  unit_assert_not_ok(result);
  unit_end();

  /* Cleanup */
  __FSStateClear__();
}


static void test_mount_state_multiple_devices(void) {
  Base_t isMounted;
  Return_t result;

  unit_print("--- Section 7.2: Mount State Multiple Devices ---");

  /* Clear state before tests */
  __FSStateClear__();

  /* Test 7.2.1: Add multiple devices */
  unit_begin("Mount State - Add three devices");
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_1));
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_2));
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_3));
  unit_end();

  /* Test 7.2.2: Verify all three are mounted */
  unit_begin("Mount State - Verify all three devices mounted");
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_1));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_2));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_3));
  unit_end();

  /* Test 7.2.3: Remove middle device */
  unit_begin("Mount State - Remove middle device from list");
  result = __RemoveMountedDevice__(TEST_DEVICE_UID_2);
  unit_assert_ok(result);
  unit_end();

  /* Test 7.2.4: Verify only middle device removed */
  unit_begin("Mount State - Verify only removed device is unmounted");
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_1));
  unit_assert_false(__IsDeviceMounted__(TEST_DEVICE_UID_2));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_3));
  unit_end();

  /* Test 7.2.5: Remove first device */
  unit_begin("Mount State - Remove first device");
  result = __RemoveMountedDevice__(TEST_DEVICE_UID_1);
  unit_assert_ok(result);
  unit_assert_false(__IsDeviceMounted__(TEST_DEVICE_UID_1));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_3));
  unit_end();

  /* Test 7.2.6: Remove last remaining device */
  unit_begin("Mount State - Remove last device");
  result = __RemoveMountedDevice__(TEST_DEVICE_UID_3);
  unit_assert_ok(result);
  unit_assert_false(__IsDeviceMounted__(TEST_DEVICE_UID_3));
  unit_end();

  /* Cleanup */
  __FSStateClear__();
}


static void test_mount_state_edge_cases(void) {
  Return_t result;

  unit_print("--- Section 7.3: Mount State Edge Cases ---");

  /* Clear state before tests */
  __FSStateClear__();

  /* Test 7.3.1: Add same device twice */
  unit_begin("Mount State - Add same device twice");
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_1));
  /* Adding same device again - implementation specific behavior */
  result = __AddMountedDevice__(TEST_DEVICE_UID_1);
  unit_print("Duplicate mount - implementation specific behavior");
  /* Device should still be mounted regardless */
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_1));
  unit_end();

  /* Cleanup and restart */
  __FSStateClear__();

  /* Test 7.3.2: Fill to maximum capacity (MAX_MOUNTED_VOLUMES = 8) */
  unit_begin("Mount State - Fill to maximum capacity");
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_1));
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_2));
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_3));
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_4));
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_5));
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_6));
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_7));
  unit_assert_ok(__AddMountedDevice__(TEST_DEVICE_UID_8));
  unit_end();

  /* Test 7.3.3: Try to add beyond maximum */
  unit_begin("Mount State - Attempt to exceed maximum capacity");
  result = __AddMountedDevice__(0x9000u); /* 9th device */
  unit_assert_not_ok(result); /* Should fail - array full */
  unit_end();

  /* Test 7.3.4: Verify all 8 devices still mounted */
  unit_begin("Mount State - Verify all 8 devices still mounted");
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_1));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_2));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_3));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_4));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_5));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_6));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_7));
  unit_assert_true(__IsDeviceMounted__(TEST_DEVICE_UID_8));
  unit_end();

  /* Test 7.3.5: Remove all and verify cleanup */
  unit_begin("Mount State - Remove all devices and verify cleanup");
  __RemoveMountedDevice__(TEST_DEVICE_UID_1);
  __RemoveMountedDevice__(TEST_DEVICE_UID_2);
  __RemoveMountedDevice__(TEST_DEVICE_UID_3);
  __RemoveMountedDevice__(TEST_DEVICE_UID_4);
  __RemoveMountedDevice__(TEST_DEVICE_UID_5);
  __RemoveMountedDevice__(TEST_DEVICE_UID_6);
  __RemoveMountedDevice__(TEST_DEVICE_UID_7);
  __RemoveMountedDevice__(TEST_DEVICE_UID_8);
  unit_assert_false(__IsDeviceMounted__(TEST_DEVICE_UID_1));
  unit_assert_false(__IsDeviceMounted__(TEST_DEVICE_UID_8));
  unit_end();

  /* Test 7.3.6: State clear functionality */
  unit_begin("Mount State - State clear resets all mounts");
  __AddMountedDevice__(TEST_DEVICE_UID_1);
  __AddMountedDevice__(TEST_DEVICE_UID_2);
  __FSStateClear__();
  unit_assert_false(__IsDeviceMounted__(TEST_DEVICE_UID_1));
  unit_assert_false(__IsDeviceMounted__(TEST_DEVICE_UID_2));
  unit_end();

  /* Final cleanup */
  __FSStateClear__();
}
