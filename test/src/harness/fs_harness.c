/*UNCRUSTIFY-OFF*/
/**
 * @file fs_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for filesystem
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "fs_harness.h"
#include "../../src/fs.h"


/* Block device structures - forward declarations to avoid duplicate includes */
#define BLOCK_PROTOCOL_RAW 0xFFu
#define BLOCK_CMD_CONFIG 0x01u

typedef struct BlockDeviceConfig_s {
  Byte_t command;          /* BLOCK_CMD_CONFIG */
  HalfWord_t ioDriverUID;
  Byte_t protocol;
  HalfWord_t blockSize;
  Word_t totalBlocks;
} BlockDeviceConfig_t;


/* Test constants - Device Configuration */
#define RAMDISK_UID 0x0100u /* RAM disk device UID */
#define BLOCKDEV_UID 0x1000u /* Block device UID */
#define TEST_BLOCK_SIZE 512 /* 512 bytes per sector */
#define TEST_TOTAL_BLOCKS 2048 /* 1MB / 512 = 2048 blocks */
#define TEST_VOLUME_LABEL "HELIOS     " /* FAT32 volume label */
/* Test constants - File and Directory Operations */
#define TEST_FILE_DATA "Hello, HeliOS Filesystem!"
#define TEST_FILE_SIZE 26 /* Including null terminator */
#define TEST_SEEK_OFFSET_7 7 /* Seek test offset */
#define TEST_TRUNCATE_SIZE 10 /* File truncation size */
#define TEST_APPEND_DATA "MORE!!"
#define TEST_APPEND_SIZE 6
#define EXPECTED_APPEND_TOTAL 16 /* 10 + 6 */
#define TEST_DIR_FILE1_DATA "Test File 1\0"
#define TEST_DIR_FILE1_SIZE 12
#define TEST_DIR_FILE2_DATA "Test File 2\0"
#define TEST_DIR_FILE2_SIZE 12
#define MIN_DIR_ENTRIES 2 /* Minimum entries in test directory */
#define TEST_MANAGE_DATA "Management!!\0"
#define TEST_MANAGE_SIZE 13


/* Test constants - Large File Operations */
#define LARGE_FILE_SIZE 8192 /* 8KB - spans multiple clusters */
#define CLUSTER_SIZE 4096 /* 512 bytes/sector * 8 sectors/cluster */
#define CLUSTER_BOUNDARY_OFFSET 100 /* Offset from cluster boundary */
#define CLUSTER_SPAN_SIZE 200 /* Size crossing cluster boundary */
#define CLUSTER_BOUNDARY_READ_OFFSET 50 /* Read offset from boundary */
#define CLUSTER_BOUNDARY_READ_SIZE 100 /* Read size crossing boundary */
/* Test constants - Partial I/O Operations */
#define PARTIAL_IO_DATA "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define PARTIAL_IO_SIZE 36
#define PARTIAL_READ_FIRST 10 /* First 10 bytes */
#define PARTIAL_READ_SECOND 10 /* Next 10 bytes */
#define PARTIAL_READ_POSITION_20 20 /* Expected position after two reads */
#define PARTIAL_SEEK_OFFSET_15 15 /* Seek to middle */
#define PARTIAL_READ_MIDDLE_SIZE 5 /* Read 5 bytes from middle */
/* Test constants - Volume Information */
#define EXPECTED_BYTES_PER_SECTOR 512
#define EXPECTED_SECTORS_PER_CLUSTER 8
#define EXPECTED_BYTES_PER_CLUSTER 4096 /* 512 * 8 */
/* Test constants - File Mode Validation */
#define MODE_TEST_DATA "test data\0"
#define MODE_TEST_SIZE 10
#define MODE_WRITE_DATA "writemode\0"
#define MODE_WRITE_SIZE 10
#define MODE_APPEND_DATA "append\0"
#define MODE_APPEND_SIZE 7
#define EXPECTED_MODE_APPEND_POS 17 /* 10 + 7 */
#define MODE_READ_SIZE 9


/* Test constants - Multiple File Operations */
#define MULTI_FILE_COUNT 3
#define MULTI_FILE1_DATA "File One Data"
#define MULTI_FILE1_SIZE 14
#define MULTI_FILE2_DATA "File Two Data"
#define MULTI_FILE2_SIZE 14
#define MULTI_FILE3_DATA "File Three Data"
#define MULTI_FILE3_SIZE 16


/* Test constants - Closed File Operations */
#define CLOSED_FILE_DATA "test data!!\0"
#define CLOSED_FILE_SIZE 12
/* Driver registration function declarations */
extern Return_t RAMDISK0_self_register(void);
extern Return_t BLOCKDEV_self_register(void);
/* Cleanup function declarations */
extern void __FSStateClear__(void);
extern void __BlockDeviceStateClear__(void);
extern void __RAMDiskStateClear__(void);
/* Helper function prototypes */
static void test_driver_registration_and_mount(void);
static void test_basic_file_operations(void);
static void test_directory_operations(void);
static void test_file_management(void);
static void test_null_pointer_and_edge_cases(void);
static void test_large_file_operations(void);
static void test_cluster_boundary_operations(void);
static void test_partial_io_operations(void);
static void test_volume_info_validation(void);
static void test_file_mode_validation(void);
static void test_closed_file_operations(void);
static void test_multiple_file_operations(void);
static void test_file_sync_operations(void);
static void test_file_write_seek_cluster(void);
static void test_volume_info_with_files(void);


void fs_harness(void) {
  unit_print("=== COMPREHENSIVE FILESYSTEM TEST SUITE ===");
  test_driver_registration_and_mount();
  test_basic_file_operations();
  test_directory_operations();
  test_file_management();
  test_null_pointer_and_edge_cases();
  test_large_file_operations();
  test_cluster_boundary_operations();
  test_partial_io_operations();
  test_volume_info_validation();
  test_file_mode_validation();
  test_closed_file_operations();
  test_multiple_file_operations();
  test_file_sync_operations();
  test_file_write_seek_cluster();
  test_volume_info_with_files();
  unit_print("=== FILESYSTEM TEST SUITE COMPLETE ===");


  /* Cleanup */
  __FSStateClear__();
  __BlockDeviceStateClear__();
  __RAMDiskStateClear__();
}


/* ============================================================================
 * SECTION 1: DRIVER REGISTRATION AND FILESYSTEM MOUNT
 * ============================================================================
 */
static void test_driver_registration_and_mount(void) {
  BlockDeviceConfig_t *blockConfig = null;
  Size_t configSize = 0x0u;
  Volume_t *vol = null;
  VolumeInfo_t *volInfo = null;


  unit_print("--- Section 1: Driver Registration and Filesystem Mount ---");


  /* Test 1.1: RAM disk driver registration */
  unit_begin("Register and initialize RAM disk driver");
  unit_assert_ok(xDeviceRegisterDevice(RAMDISK0_self_register));
  unit_assert_ok(xDeviceInitDevice(RAMDISK_UID));
  unit_end();


  /* Test 1.2: Block device driver registration */
  unit_begin("Register and initialize block device driver");
  unit_assert_ok(xDeviceRegisterDevice(BLOCKDEV_self_register));
  unit_assert_ok(xDeviceInitDevice(BLOCKDEV_UID));
  unit_end();


  /* Test 1.3: Block device configuration */
  unit_begin("Configure block device with RAM disk backend");
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &blockConfig, sizeof(BlockDeviceConfig_t)));
  blockConfig->command = BLOCK_CMD_CONFIG;
  blockConfig->ioDriverUID = RAMDISK_UID;
  blockConfig->protocol = BLOCK_PROTOCOL_RAW;
  blockConfig->blockSize = TEST_BLOCK_SIZE;
  blockConfig->totalBlocks = TEST_TOTAL_BLOCKS;
  configSize = sizeof(BlockDeviceConfig_t);
  unit_assert_ok(xDeviceConfigDevice(BLOCKDEV_UID, &configSize, (Addr_t *) blockConfig));
  xMemFree((Addr_t *) blockConfig);
  unit_end();


  /* Test 1.4: Format filesystem */
  unit_begin("Format block device with FAT32 filesystem");
  unit_assert_ok(xFSFormat((const Byte_t *) TEST_VOLUME_LABEL));
  unit_end();


  /* Test 1.5: Mount filesystem */
  unit_begin("Mount filesystem and verify mount state");
  unit_assert_ok(xFSMount(&vol));
  unit_assert_not_null(vol);
  unit_assert_true(vol->mounted);
  unit_assert_equal(vol->blockDeviceUID, BLOCKDEV_UID);
  unit_end();


  /* Test 1.6: Get volume information */
  unit_begin("Retrieve volume information after mount");
  unit_assert_ok(xFSGetVolumeInfo(vol, &volInfo));
  unit_assert_not_null(volInfo);
  unit_assert_ok(xMemFree(volInfo));
  unit_end();


  /* Test 1.7: Unmount filesystem */
  unit_begin("Unmount filesystem successfully");
  unit_assert_ok(xFSUnmount(vol));
  unit_end();
}


/* ============================================================================
 * SECTION 2: BASIC FILE OPERATIONS
 * ============================================================================
 */
static void test_basic_file_operations(void) {
  Volume_t *vol = null;
  File_t *file = null;
  Byte_t *readData = null;
  Word_t position = 0x0u;
  Word_t fileSize = 0x0u;
  Base_t eof = 0x0u;


  unit_print("--- Section 2: Basic File Operations ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping file operations tests");

    return;
  }

  /* Test 2.1: File open with create and write mode */
  unit_begin("File open with create and write mode succeeds");
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/test.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_true(file->isOpen);
  unit_end();


  /* Test 2.2: File write */
  unit_begin("File write operation stores data successfully");
  unit_assert_ok(xFileWrite(file, TEST_FILE_SIZE, (const Byte_t *) TEST_FILE_DATA));
  unit_end();


  /* Test 2.3: File tell */
  unit_begin("File position retrieval returns correct offset");
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(TEST_FILE_SIZE, position);
  unit_end();


  /* Test 2.4: File sync */
  unit_begin("File sync flushes data to storage");
  unit_assert_ok(xFileSync(file));
  unit_end();


  /* Test 2.5: File get size */
  unit_begin("File size retrieval returns correct size");
  unit_assert_ok(xFileGetSize(file, &fileSize));
  unit_assert_equal(TEST_FILE_SIZE, fileSize);
  unit_end();


  /* Test 2.6: Seek to beginning */
  unit_begin("File seek to start positions at beginning");
  unit_assert_ok(xFileSeek(file, 0, FS_SEEK_SET));
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(0, position);
  unit_end();


  /* Test 2.7: File read */
  unit_begin("File read retrieves previously written data");
  unit_assert_ok(xFileRead(file, TEST_FILE_SIZE, &readData));
  unit_assert_not_null(readData);
  unit_assert_equal(0, strncmp(TEST_FILE_DATA, (char *) readData, TEST_FILE_SIZE));
  unit_assert_ok(xMemFree(readData));
  unit_end();


  /* Test 2.8: File seek from beginning */
  unit_begin("File seek from beginning sets absolute position");
  unit_assert_ok(xFileSeek(file, 0, FS_SEEK_SET));
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(0, position);
  unit_end();


  /* Test 2.9: File seek from current */
  unit_begin("File seek from current advances position");
  unit_assert_ok(xFileSeek(file, TEST_SEEK_OFFSET_7, FS_SEEK_CUR));
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(TEST_SEEK_OFFSET_7, position);
  unit_end();


  /* Test 2.10: File seek from end */
  unit_begin("File seek from end positions at file end");
  unit_assert_ok(xFileSeek(file, 0, FS_SEEK_END));
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(TEST_FILE_SIZE, position);
  unit_end();


  /* Test 2.11: EOF detection */
  unit_begin("File EOF detection identifies end of file");
  unit_assert_ok(xFileEOF(file, &eof));
  unit_assert_true(eof);
  unit_end();


  /* Close file */
  xFileClose(file);


  /* Test 2.12: File truncate */
  unit_begin("File truncate reduces file size");
  file = null;
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/test.txt", FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileTruncate(file, TEST_TRUNCATE_SIZE));
  unit_assert_ok(xFileGetSize(file, &fileSize));
  unit_assert_equal(TEST_TRUNCATE_SIZE, fileSize);
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Test 2.13: File append mode */
  unit_begin("File open in append mode adds data at end");
  file = null;
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/test.txt", FS_MODE_APPEND | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, TEST_APPEND_SIZE, (const Byte_t *) TEST_APPEND_DATA));
  unit_assert_ok(xFileGetSize(file, &fileSize));
  unit_assert_equal(EXPECTED_APPEND_TOTAL, fileSize);
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 3: DIRECTORY OPERATIONS
 * ============================================================================
 */
static void test_directory_operations(void) {
  Volume_t *vol = null;
  Dir_t *dir = null;
  DirEntry_t *entry = null;
  Base_t exists = 0x0u;
  File_t *file = null;
  int entryCount = 0x0u;


  unit_print("--- Section 3: Directory Operations ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping directory operations tests");

    return;
  }

  /* Test 3.1: Directory creation */
  unit_begin("Directory creation succeeds");
  unit_assert_ok(xDirMake(vol, (const Byte_t *) "/testdir"));
  unit_end();


  /* Test 3.2: Directory exists */
  unit_begin("Directory existence check confirms creation");
  unit_assert_ok(xFileExists(vol, (const Byte_t *) "/testdir", &exists));
  unit_assert_true(exists);
  unit_end();


  /* Test 3.3: Create files in directory */
  unit_begin("Create test files in directory");
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/testdir/file1.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, TEST_DIR_FILE1_SIZE, (const Byte_t *) TEST_DIR_FILE1_DATA));
  unit_assert_ok(xFileClose(file));
  file = null;
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/testdir/file2.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, TEST_DIR_FILE2_SIZE, (const Byte_t *) TEST_DIR_FILE2_DATA));
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Test 3.4: Directory open */
  unit_begin("Directory open for reading succeeds");
  unit_assert_ok(xDirOpen(&dir, vol, (const Byte_t *) "/testdir"));
  unit_assert_not_null(dir);
  unit_end();


  /* Test 3.5: Directory read */
  unit_begin("Directory read returns all entries");
  entryCount = 0x0u;

  /* Read all directory entries */
  while(OK(xDirRead(dir, &entry))) {
    unit_assert_not_null(entry);
    entryCount++;
    unit_assert_ok(xMemFree(entry));
    entry = null;
  }

  /* Should have at least 2 files (may have . and .. entries too) */
  unit_assert_true(entryCount >= MIN_DIR_ENTRIES);
  unit_end();


  /* Test 3.6: Directory rewind */
  unit_begin("Directory rewind resets read position to start");
  unit_assert_ok(xDirRewind(dir));
  entry = null;
  unit_assert_ok(xDirRead(dir, &entry));
  unit_assert_not_null(entry);
  unit_assert_ok(xMemFree(entry));
  unit_end();


  /* Test 3.7: Directory close */
  unit_begin("Directory close releases resources");
  unit_assert_ok(xDirClose(dir));
  unit_end();


  /* Test 3.8: Root directory open */
  unit_begin("Root directory open succeeds");
  dir = null;
  unit_assert_ok(xDirOpen(&dir, vol, (const Byte_t *) "/"));
  unit_assert_not_null(dir);
  unit_assert_ok(xDirClose(dir));
  unit_end();


  /* Test 3.9: Remove non-empty directory fails */
  unit_begin("Directory removal fails for non-empty directory");
  unit_assert_not_ok(xDirRemove(vol, (const Byte_t *) "/testdir"));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 4: FILE MANAGEMENT OPERATIONS
 * ============================================================================
 */
static void test_file_management(void) {
  Volume_t *vol = null;
  File_t *file = null;
  Base_t exists = 0x0u;
  DirEntry_t *info = null;


  unit_print("--- Section 4: File Management Operations ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping file management tests");

    return;
  }

  /* Test 4.1: Create test file */
  unit_begin("Create test file for management operations");
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/manage.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, TEST_MANAGE_SIZE, (const Byte_t *) TEST_MANAGE_DATA));
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Test 4.2: File exists check */
  unit_begin("File existence check confirms existing file");
  unit_assert_ok(xFileExists(vol, (const Byte_t *) "/manage.txt", &exists));
  unit_assert_true(exists);
  unit_end();


  /* Test 4.3: Non-existing file check */
  unit_begin("File existence check returns false for non-existing file");
  exists = 0x0u;
  unit_assert_ok(xFileExists(vol, (const Byte_t *) "/nonexist.txt", &exists));
  unit_assert_false(exists);
  unit_end();


  /* Test 4.4: File information retrieval */
  unit_begin("File information retrieval returns file details");
  unit_assert_ok(xFileGetInfo(vol, (const Byte_t *) "/manage.txt", &info));
  unit_assert_not_null(info);
  unit_assert_ok(xMemFree(info));
  unit_end();


  /* Test 4.5: File rename */
  unit_begin("File rename updates filename successfully");
  unit_assert_ok(xFileRename(vol, (const Byte_t *) "/manage.txt", (const Byte_t *) "/renamed.txt"));


  /* Verify old name doesn't exist */
  exists = 0x0u;
  unit_assert_ok(xFileExists(vol, (const Byte_t *) "/manage.txt", &exists));
  unit_assert_false(exists);


  /* Verify new name exists */
  exists = 0x0u;
  unit_assert_ok(xFileExists(vol, (const Byte_t *) "/renamed.txt", &exists));
  unit_assert_true(exists);
  unit_end();


  /* Test 4.6: File unlink */
  unit_begin("File unlink removes file from filesystem");
  unit_assert_ok(xFileUnlink(vol, (const Byte_t *) "/renamed.txt"));


  /* Verify file no longer exists */
  exists = 0x0u;
  unit_assert_ok(xFileExists(vol, (const Byte_t *) "/renamed.txt", &exists));
  unit_assert_false(exists);
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 5: NULL POINTER AND EDGE CASES
 * ============================================================================
 */
static void test_null_pointer_and_edge_cases(void) {
  Volume_t *vol = null;
  File_t *file = null;
  Dir_t *dir = null;
  Base_t exists = 0x0u;


  unit_print("--- Section 5: NULL Pointer and Edge Cases ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping edge case tests");

    return;
  }

  /* Test 5.1: NULL pointer handling */
  unit_begin("Edge Case - NULL Pointers");


  /* xFileOpen with NULL file pointer */
  unit_assert_not_ok(xFileOpen(null, vol, (const Byte_t *) "/test.txt", FS_MODE_READ));


  /* xFileOpen with NULL volume */
  unit_assert_not_ok(xFileOpen(&file, null, (const Byte_t *) "/test.txt", FS_MODE_READ));


  /* xFileOpen with NULL path */
  unit_assert_not_ok(xFileOpen(&file, vol, null, FS_MODE_READ));


  /* xFileClose with NULL file */
  unit_assert_not_ok(xFileClose(null));


  /* xDirOpen with NULL directory pointer */
  unit_assert_not_ok(xDirOpen(null, vol, (const Byte_t *) "/"));


  /* xDirOpen with NULL volume */
  unit_assert_not_ok(xDirOpen(&dir, null, (const Byte_t *) "/"));


  /* xDirOpen with NULL path */
  unit_assert_not_ok(xDirOpen(&dir, vol, null));


  /* xDirClose with NULL directory */
  unit_assert_not_ok(xDirClose(null));


  /* xDirMake with NULL volume */
  unit_assert_not_ok(xDirMake(null, (const Byte_t *) "/testdir"));


  /* xDirMake with NULL path */
  unit_assert_not_ok(xDirMake(vol, null));


  /* xDirRead with NULL directory */
  unit_assert_not_ok(xDirRead(null, (DirEntry_t **) &dir));


  /* xDirRewind with NULL directory */
  unit_assert_not_ok(xDirRewind(null));


  /* xFileEOF with NULL file */
  unit_assert_not_ok(xFileEOF(null, &exists));


  /* xFileGetInfo with NULL volume */
  unit_assert_not_ok(xFileGetInfo(null, (const Byte_t *) "/test.txt", (DirEntry_t **) &dir));


  /* xFileGetInfo with NULL path */
  unit_assert_not_ok(xFileGetInfo(vol, null, (DirEntry_t **) &dir));


  /* xFileGetSize with NULL file */
  unit_assert_not_ok(xFileGetSize(null, (Word_t *) &exists));


  /* xFileSeek with NULL file */
  unit_assert_not_ok(xFileSeek(null, 0, FS_SEEK_SET));


  /* xFileSync with NULL file */
  unit_assert_not_ok(xFileSync(null));


  /* xFileTell with NULL file */
  unit_assert_not_ok(xFileTell(null, (Word_t *) &exists));


  /* xFileTruncate with NULL file */
  unit_assert_not_ok(xFileTruncate(null, 0));


  /* xFSFormat with NULL label */
  unit_assert_not_ok(xFSFormat(null));


  /* xFSUnmount with NULL volume */
  unit_assert_not_ok(xFSUnmount(null));
  unit_end();


  /* Test invalid file operations */
  unit_begin("Edge Case - Invalid File Operations");


  /* Try to open non-existent file in read mode (should fail without CREATE) */
  file = null;
  unit_assert_not_ok(xFileOpen(&file, vol, (const Byte_t *) "/nonexist.txt", FS_MODE_READ));
  unit_assert_null(file);


  /* Try to read from a file opened in write-only mode */
  file = null;
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/writeonly.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);


  /* Note: Actual behavior depends on implementation - may or may not allow read
   */
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Test invalid path operations */
  unit_begin("Edge Case - Invalid Paths");


  /* Try to unlink non-existent file */
  unit_assert_not_ok(xFileUnlink(vol, (const Byte_t *) "/doesnotexist.txt"));


  /* Try to rename non-existent file */
  unit_assert_not_ok(xFileRename(vol, (const Byte_t *) "/nosuchfile.txt", (const Byte_t *) "/newname.txt"));


  /* Try to open non-existent directory */
  dir = null;
  unit_assert_not_ok(xDirOpen(&dir, vol, (const Byte_t *) "/nosuchdir"));


  /* Try to remove non-existent directory */
  unit_assert_not_ok(xDirRemove(vol, (const Byte_t *) "/nosuchdir"));
  unit_end();


  /* Test 5.2: Double mount (should fail) */
  unit_begin("Edge Case - Double Mount");
  {
    Volume_t *vol2 = null;


    unit_assert_not_ok(xFSMount(&vol2));
  } unit_end();


  /* Test 5.3: Operations on unmounted volume */
  unit_begin("Edge Case - Operations After Unmount");
  xFSUnmount(vol);


  /* These should fail after unmount */
  file = null;
  unit_assert_not_ok(xFileOpen(&file, vol, (const Byte_t *) "/test.txt", FS_MODE_READ));
  dir = null;
  unit_assert_not_ok(xDirOpen(&dir, vol, (const Byte_t *) "/"));
  exists = 0x0u;
  unit_assert_not_ok(xFileExists(vol, (const Byte_t *) "/test.txt", &exists));
  unit_end();
}


/* ============================================================================
 * SECTION 6: LARGE FILE OPERATIONS
 * ============================================================================
 */
static void test_large_file_operations(void) {
  Volume_t *vol = null;
  File_t *file = null;
  Byte_t *writeData = null;
  Byte_t *readData = null;
  Size_t i = 0x0u;
  Word_t fileSize = 0x0u;
  Word_t position = 0x0u;


  unit_print("--- Section 6: Large File Operations ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping large file operations tests");

    return;
  }

  /* Test 6.1: Multi-cluster write */
  unit_begin("Large File Operations - Multi-cluster Write");


  /* Allocate large buffer with pattern data */
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, LARGE_FILE_SIZE));
  unit_assert_not_null(writeData);

  /* Fill with pattern (repeating 0-255) */
  for(i = 0x0u; i < LARGE_FILE_SIZE; i++) {
    writeData[i] = (Byte_t) (i & 0xFFu);
  }

  /* Create and write large file */
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/largefile.dat", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, LARGE_FILE_SIZE, writeData));


  /* Verify file size */
  unit_assert_ok(xFileGetSize(file, &fileSize));
  unit_assert_equal(LARGE_FILE_SIZE, fileSize);


  /* Verify position */
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(LARGE_FILE_SIZE, position);
  unit_end();


  /* Test 6.2: Multi-cluster read */
  unit_begin("Large File Operations - Multi-cluster Read");


  /* Seek back to start */
  unit_assert_ok(xFileSeek(file, 0, FS_SEEK_SET));


  /* Read entire file */
  unit_assert_ok(xFileRead(file, LARGE_FILE_SIZE, &readData));
  unit_assert_not_null(readData);

  /* Verify data matches */
  for(i = 0x0u; i < LARGE_FILE_SIZE; i++) {
    if(writeData[i] != readData[i]) {
      unit_print("Data mismatch detected in large file read");
      unit_assert_true(false);
      break;
    }
  }

  unit_assert_ok(xMemFree(readData));
  unit_assert_ok(xMemFree(writeData));
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 7: CLUSTER BOUNDARY OPERATIONS
 * ============================================================================
 */
static void test_cluster_boundary_operations(void) {
  Volume_t *vol = null;
  File_t *file = null;
  Byte_t *writeData = null;
  Byte_t *readData = null;
  Size_t testSize = CLUSTER_SIZE - CLUSTER_BOUNDARY_OFFSET;
  Size_t i = 0x0u;
  Word_t position = 0x0u;


  unit_print("--- Section 7: Cluster Boundary Operations ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping cluster boundary tests");

    return;
  }

  /* Test 7.1: Write at cluster boundary */
  unit_begin("Cluster Boundary - Write at Boundary");


  /* Allocate buffer */
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, CLUSTER_SIZE + CLUSTER_SPAN_SIZE));
  unit_assert_not_null(writeData);

  /* Fill with pattern */
  for(i = 0x0u; i < CLUSTER_SIZE + CLUSTER_SPAN_SIZE; i++) {
    writeData[i] = (Byte_t) ((i * 7) & 0xFFu);
  }

  /* Create file and write up to near cluster boundary */
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/boundary.dat", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, testSize, writeData));


  /* Write more data to cross cluster boundary */
  unit_assert_ok(xFileWrite(file, CLUSTER_SPAN_SIZE, writeData + testSize));
  unit_end();


  /* Test 7.2: Read across cluster boundary */
  unit_begin("Cluster Boundary - Read Across Boundary");


  /* Seek to position near cluster boundary */
  unit_assert_ok(xFileSeek(file, CLUSTER_SIZE - CLUSTER_BOUNDARY_READ_OFFSET, FS_SEEK_SET));
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(CLUSTER_SIZE - CLUSTER_BOUNDARY_READ_OFFSET, position);


  /* Read data that spans cluster boundary */
  unit_assert_ok(xFileRead(file, CLUSTER_BOUNDARY_READ_SIZE, &readData));
  unit_assert_not_null(readData);

  /* Verify data */
  for(i = 0x0u; i < CLUSTER_BOUNDARY_READ_SIZE; i++) {
    if(writeData[CLUSTER_SIZE - CLUSTER_BOUNDARY_READ_OFFSET + i] != readData[i]) {
      unit_print("Cluster boundary data mismatch detected");
      unit_assert_true(false);
      break;
    }
  }

  unit_assert_ok(xMemFree(readData));
  unit_assert_ok(xMemFree(writeData));
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 8: PARTIAL I/O OPERATIONS
 * ============================================================================
 */
static void test_partial_io_operations(void) {
  Volume_t *vol = null;
  File_t *file = null;
  Byte_t *readData = null;
  Word_t position = 0x0u;


  unit_print("--- Section 8: Partial I/O Operations ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping partial I/O tests");

    return;
  }

  /* Test 8.1: Write and partial reads */
  unit_begin("Partial I/O - Write and Partial Reads");


  /* Create file with test data */
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/partial.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, PARTIAL_IO_SIZE, (const Byte_t *) PARTIAL_IO_DATA));
  unit_assert_ok(xFileSeek(file, 0, FS_SEEK_SET));
  unit_end();


  /* Test 8.2: Read first 10 bytes */
  unit_begin("Partial I/O - Read First 10 Bytes");
  unit_assert_ok(xFileRead(file, PARTIAL_READ_FIRST, &readData));
  unit_assert_not_null(readData);
  unit_assert_equal(0, strncmp(PARTIAL_IO_DATA, (char *) readData, PARTIAL_READ_FIRST));
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(PARTIAL_READ_FIRST, position);
  unit_assert_ok(xMemFree(readData));
  unit_end();


  /* Test 8.3: Read middle 10 bytes */
  unit_begin("Partial I/O - Read Middle 10 Bytes");
  readData = null;
  unit_assert_ok(xFileRead(file, PARTIAL_READ_SECOND, &readData));
  unit_assert_not_null(readData);
  unit_assert_equal(0, strncmp((PARTIAL_IO_DATA + PARTIAL_READ_FIRST), (char *) readData, PARTIAL_READ_SECOND));
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(PARTIAL_READ_POSITION_20, position);
  unit_assert_ok(xMemFree(readData));
  unit_end();


  /* Test 8.4: Seek and read from middle */
  unit_begin("Partial I/O - Seek and Read from Middle");
  unit_assert_ok(xFileSeek(file, PARTIAL_SEEK_OFFSET_15, FS_SEEK_SET));
  readData = null;
  unit_assert_ok(xFileRead(file, PARTIAL_READ_MIDDLE_SIZE, &readData));
  unit_assert_not_null(readData);
  unit_assert_equal(0, strncmp((PARTIAL_IO_DATA + PARTIAL_SEEK_OFFSET_15), (char *) readData, PARTIAL_READ_MIDDLE_SIZE));
  unit_assert_ok(xMemFree(readData));
  unit_end();


  /* Test 8.5: Read at EOF */
  unit_begin("Partial I/O - Read at EOF");
  unit_assert_ok(xFileSeek(file, 0, FS_SEEK_END));
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(PARTIAL_IO_SIZE, position);


  /* Try to read at EOF - should fail */
  readData = null;
  unit_assert_not_ok(xFileRead(file, PARTIAL_READ_FIRST, &readData));
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 9: VOLUME INFORMATION VALIDATION
 * ============================================================================
 */
static void test_volume_info_validation(void) {
  Volume_t *vol = null;
  VolumeInfo_t *volInfo = null;


  unit_print("--- Section 9: Volume Information Validation ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping volume info tests");

    return;
  }

  /* Test 9.1: Get and validate volume information */
  unit_begin("Volume Info - Get and Validate");


  /* Get volume information */
  unit_assert_ok(xFSGetVolumeInfo(vol, &volInfo));
  unit_assert_not_null(volInfo);


  /* Validate volume parameters */
  unit_assert_equal(EXPECTED_BYTES_PER_SECTOR, volInfo->bytesPerSector);
  unit_assert_equal(EXPECTED_SECTORS_PER_CLUSTER, volInfo->sectorsPerCluster);
  unit_assert_equal(EXPECTED_BYTES_PER_CLUSTER, volInfo->bytesPerCluster);
  unit_assert_ok(xMemFree(volInfo));
  unit_end();


  /* Test 9.2: NULL pointer handling */
  unit_begin("Volume Info - NULL Pointer Handling");


  /* Test NULL pointer for volume */
  unit_assert_not_ok(xFSGetVolumeInfo(null, &volInfo));


  /* Test NULL pointer for info output */
  unit_assert_not_ok(xFSGetVolumeInfo(vol, null));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 10: FILE MODE VALIDATION
 * ============================================================================
 */
static void test_file_mode_validation(void) {
  Volume_t *vol = null;
  File_t *file1 = null;
  File_t *file2 = null;
  Byte_t *readData = null;
  Word_t position = 0x0u;


  unit_print("--- Section 10: File Mode Validation ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping file mode tests");

    return;
  }

  /* Test 10.1: Create without write mode */
  unit_begin("File Mode - Create Without Write Mode");


  /* Try to create file with only CREATE mode */
  unit_assert_ok(xFileOpen(&file1, vol, (const Byte_t *) "/modetest1.txt", FS_MODE_CREATE));
  unit_assert_not_null(file1);


  /* Write should fail without WRITE or APPEND mode */
  unit_assert_not_ok(xFileWrite(file1, MODE_TEST_SIZE, (const Byte_t *) MODE_TEST_DATA));
  unit_assert_ok(xFileClose(file1));
  unit_end();


  /* Test 10.2: Write mode */
  unit_begin("File Mode - Write Mode");
  file1 = null;
  unit_assert_ok(xFileOpen(&file1, vol, (const Byte_t *) "/modetest2.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file1);
  unit_assert_ok(xFileWrite(file1, MODE_WRITE_SIZE, (const Byte_t *) MODE_WRITE_DATA));
  unit_assert_ok(xFileClose(file1));
  unit_end();


  /* Test 10.3: Append mode */
  unit_begin("File Mode - Append Mode");
  file1 = null;
  unit_assert_ok(xFileOpen(&file1, vol, (const Byte_t *) "/modetest2.txt", FS_MODE_APPEND | FS_MODE_WRITE));
  unit_assert_not_null(file1);
  unit_assert_ok(xFileWrite(file1, MODE_APPEND_SIZE, (const Byte_t *) MODE_APPEND_DATA));


  /* Verify position at end after append */
  position = 0x0u;
  unit_assert_ok(xFileTell(file1, &position));
  unit_assert_equal(EXPECTED_MODE_APPEND_POS, position);
  unit_assert_ok(xFileClose(file1));
  unit_end();


  /* Test 10.4: Read mode */
  unit_begin("File Mode - Read Mode");
  file2 = null;
  unit_assert_ok(xFileOpen(&file2, vol, (const Byte_t *) "/modetest2.txt", FS_MODE_READ));
  unit_assert_not_null(file2);


  /* Read should work */
  unit_assert_ok(xFileRead(file2, MODE_READ_SIZE, &readData));
  unit_assert_not_null(readData);
  unit_assert_ok(xMemFree(readData));
  unit_assert_ok(xFileClose(file2));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 11: CLOSED FILE OPERATIONS
 * ============================================================================
 */
static void test_closed_file_operations(void) {
  Volume_t *vol = null;
  File_t *file = null;


  unit_print("--- Section 11: Closed File Operations ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping closed file tests");

    return;
  }

  /* Test 11.1: Create and close */
  unit_begin("Closed File Operations - Create and Close");


  /* Create a file and immediately close it */
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/closedtest.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, CLOSED_FILE_SIZE, (const Byte_t *) CLOSED_FILE_DATA));
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Test 11.2: Operations on closed file */
  unit_begin("Closed File Operations - Operations on Closed File");


  /* Try operations on closed file (file pointer is now invalid/freed) */


  /* Note: These will likely crash or corrupt memory, so we can't test them In a
   * real implementation, file handles would be validated */
  unit_print("Cannot safely test operations on freed file handle");
  unit_end();
  unit_begin("Closed File Operations - Double Close");


  /* Create another file */
  file = null;
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/doubleclose.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileClose(file));


  /* Try to close again - should fail as memory is freed */
  /* Cannot test this safely as file pointer is invalid */
  unit_print("Cannot safely test double close on freed handle");
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 12: MULTIPLE FILE OPERATIONS
 * ============================================================================
 */
static void test_multiple_file_operations(void) {
  Volume_t *vol = null;
  File_t *file1 = null;
  File_t *file2 = null;
  File_t *file3 = null;
  Byte_t *readData1 = null;
  Byte_t *readData2 = null;
  Byte_t *readData3 = null;


  unit_print("--- Section 12: Multiple File Operations ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping multiple file tests");

    return;
  }

  /* Test 12.1: Create three files */
  unit_begin("Multiple Files - Create Three Files");


  /* Create three files simultaneously */
  unit_assert_ok(xFileOpen(&file1, vol, (const Byte_t *) "/multi1.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file1);
  unit_assert_ok(xFileOpen(&file2, vol, (const Byte_t *) "/multi2.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file2);
  unit_assert_ok(xFileOpen(&file3, vol, (const Byte_t *) "/multi3.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file3);
  unit_end();


  /* Test 12.2: Write to all files */
  unit_begin("Multiple Files - Write to All Files");
  unit_assert_ok(xFileWrite(file1, MULTI_FILE1_SIZE, (const Byte_t *) MULTI_FILE1_DATA));
  unit_assert_ok(xFileWrite(file2, MULTI_FILE2_SIZE, (const Byte_t *) MULTI_FILE2_DATA));
  unit_assert_ok(xFileWrite(file3, MULTI_FILE3_SIZE, (const Byte_t *) MULTI_FILE3_DATA));
  unit_end();


  /* Test 12.3: Seek all files */
  unit_begin("Multiple Files - Seek All Files");
  unit_assert_ok(xFileSeek(file1, 0, FS_SEEK_SET));
  unit_assert_ok(xFileSeek(file2, 0, FS_SEEK_SET));
  unit_assert_ok(xFileSeek(file3, 0, FS_SEEK_SET));
  unit_end();


  /* Test 12.4: Read and verify all files */
  unit_begin("Multiple Files - Read and Verify All Files");
  unit_assert_ok(xFileRead(file1, MULTI_FILE1_SIZE, &readData1));
  unit_assert_not_null(readData1);
  unit_assert_equal(0, strncmp(MULTI_FILE1_DATA, (char *) readData1, MULTI_FILE1_SIZE));
  unit_assert_ok(xFileRead(file2, MULTI_FILE2_SIZE, &readData2));
  unit_assert_not_null(readData2);
  unit_assert_equal(0, strncmp(MULTI_FILE2_DATA, (char *) readData2, MULTI_FILE2_SIZE));
  unit_assert_ok(xFileRead(file3, MULTI_FILE3_SIZE, &readData3));
  unit_assert_not_null(readData3);
  unit_assert_equal(0, strncmp(MULTI_FILE3_DATA, (char *) readData3, MULTI_FILE3_SIZE));
  unit_assert_ok(xMemFree(readData1));
  unit_assert_ok(xMemFree(readData2));
  unit_assert_ok(xMemFree(readData3));
  unit_end();


  /* Test 12.5: Close all files */
  unit_begin("Multiple Files - Close All Files");


  /* Close all files */
  unit_assert_ok(xFileClose(file1));
  unit_assert_ok(xFileClose(file2));
  unit_assert_ok(xFileClose(file3));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 13: FILE SYNC OPERATIONS
 * ============================================================================
 */
static void test_file_sync_operations(void) {
  Volume_t *vol = null;
  File_t *file = null;
  Byte_t *readData = null;
  Word_t fileSize = 0x0u;


  unit_print("--- Section 13: File Sync Operations ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping file sync tests");

    return;
  }

  /* Test 13.1: File sync after write */
  unit_begin("File sync after write updates directory entry");


  /* Create and write to a file */
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/synctest.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, 15, (const Byte_t *) "Sync test data!"));


  /* Sync the file */
  unit_assert_ok(xFileSync(file));


  /* Close and reopen to verify data was persisted */
  unit_assert_ok(xFileClose(file));
  file = null;
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/synctest.txt", FS_MODE_READ));
  unit_assert_not_null(file);
  unit_assert_ok(xFileGetSize(file, &fileSize));
  unit_assert_equal(15, fileSize);


  /* Read and verify content */
  unit_assert_ok(xFileRead(file, 15, &readData));
  unit_assert_not_null(readData);
  unit_assert_equal(0, strncmp("Sync test data!", (char *) readData, 15));
  unit_assert_ok(xMemFree(readData));
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Test 13.2: Multiple syncs */
  unit_begin("Multiple file syncs maintain consistency");
  file = null;
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/multisync.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);


  /* Write, sync, write more, sync again */
  unit_assert_ok(xFileWrite(file, 5, (const Byte_t *) "Part1"));
  unit_assert_ok(xFileSync(file));
  unit_assert_ok(xFileWrite(file, 5, (const Byte_t *) "Part2"));
  unit_assert_ok(xFileSync(file));
  unit_assert_ok(xFileGetSize(file, &fileSize));
  unit_assert_equal(10, fileSize);
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 14: FILE WRITE SEEK CLUSTER
 * ============================================================================
 */
static void test_file_write_seek_cluster(void) {
  Volume_t *vol = null;
  File_t *file = null;
  Byte_t *writeData = null;
  Byte_t *readData = null;
  Word_t i = 0x0u;
  Word_t position = 0x0u;
  Word_t clusterSize = 4096; /* 512 bytes/sector * 8 sectors/cluster */


  unit_print("--- Section 14: File Write Seek Cluster ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping write seek cluster tests");

    return;
  }

  /* Test 14.1: Write after seeking to different clusters */
  unit_begin("Write after seeking to different clusters");


  /* Create a file and write initial data */
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/seekcluster.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);


  /* Allocate buffer for multi-cluster data */
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &writeData, clusterSize * 2));
  unit_assert_not_null(writeData);

  /* Fill with pattern */
  for(i = 0x0u; i < clusterSize * 2; i++) {
    writeData[i] = (Byte_t) (i & 0xFFu);
  }

  /* Write data spanning multiple clusters */
  unit_assert_ok(xFileWrite(file, clusterSize * 2, writeData));
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(clusterSize * 2, position);


  /* Seek back to first cluster and overwrite */
  unit_assert_ok(xFileSeek(file, 100, FS_SEEK_SET));
  unit_assert_ok(xFileWrite(file, 10, (const Byte_t *) "OVERWRITE!"));


  /* Seek to second cluster and overwrite */
  unit_assert_ok(xFileSeek(file, clusterSize + 100, FS_SEEK_SET));
  unit_assert_ok(xFileWrite(file, 10, (const Byte_t *) "CLUSTER2!!"));


  /* Read back and verify overwrites */
  unit_assert_ok(xFileSeek(file, 100, FS_SEEK_SET));
  unit_assert_ok(xFileRead(file, 10, &readData));
  unit_assert_not_null(readData);
  unit_assert_equal(0, strncmp("OVERWRITE!", (char *) readData, 10));
  unit_assert_ok(xMemFree(readData));
  unit_assert_ok(xFileSeek(file, clusterSize + 100, FS_SEEK_SET));
  unit_assert_ok(xFileRead(file, 10, &readData));
  unit_assert_not_null(readData);
  unit_assert_equal(0, strncmp("CLUSTER2!!", (char *) readData, 10));
  unit_assert_ok(xMemFree(readData));
  unit_assert_ok(xMemFree(writeData));
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Test 14.2: Write extending file with proper cluster navigation */
  unit_begin("Write extending file with cluster chain navigation");
  file = null;
  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/extend.txt", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);


  /* Write small data first */
  unit_assert_ok(xFileWrite(file, 100, (const Byte_t *)
    "Initial data for extending file test - this will be extended with more clusters as we write more data to test cluster chain"));


  /* Seek past current size and write (should extend) */
  unit_assert_ok(xFileSeek(file, clusterSize + 500, FS_SEEK_SET));
  unit_assert_ok(xFileWrite(file, 20, (const Byte_t *) "Extended data here!!"));


  /* Verify file was extended */
  unit_assert_ok(xFileTell(file, &position));
  unit_assert_equal(clusterSize + 520, position);
  unit_assert_ok(xFileClose(file));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}


/* ============================================================================
 * SECTION 15: VOLUME INFO WITH FILES
 * ============================================================================
 */
static void test_volume_info_with_files(void) {
  Volume_t *vol = null;
  VolumeInfo_t *volInfo1 = null;
  VolumeInfo_t *volInfo2 = null;
  File_t *file = null;
  Word_t initialFreeClusters = 0x0u;
  Word_t afterFreeClusters = 0x0u;
  Byte_t *largeData = null;
  Word_t dataSize = 8192; /* 2 clusters worth */
  Word_t i = 0x0u;


  unit_print("--- Section 15: Volume Info With Files ---");

  /* Mount filesystem first */
  if(!OK(xFSMount(&vol)) || (null == vol)) {
    unit_print("xFSMount() failed - skipping volume info with files tests");

    return;
  }

  /* Test 15.1: Volume info shows cluster usage */
  unit_begin("Volume info reflects cluster allocation");


  /* Get initial volume info */
  unit_assert_ok(xFSGetVolumeInfo(vol, &volInfo1));
  unit_assert_not_null(volInfo1);
  initialFreeClusters = volInfo1->freeClusters;
  unit_assert_ok(xMemFree(volInfo1));


  /* Create a file that uses multiple clusters */
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &largeData, dataSize));
  unit_assert_not_null(largeData);

  /* Fill with data */
  for(i = 0x0u; i < dataSize; i++) {
    largeData[i] = (Byte_t) ((i * 3) & 0xFFu);
  }

  unit_assert_ok(xFileOpen(&file, vol, (const Byte_t *) "/voltest.dat", FS_MODE_CREATE | FS_MODE_WRITE));
  unit_assert_not_null(file);
  unit_assert_ok(xFileWrite(file, dataSize, largeData));
  unit_assert_ok(xFileClose(file));


  /* Get volume info after file creation */
  unit_assert_ok(xFSGetVolumeInfo(vol, &volInfo2));
  unit_assert_not_null(volInfo2);
  afterFreeClusters = volInfo2->freeClusters;


  /* Should have fewer free clusters after creating file */
  unit_assert_true(afterFreeClusters < initialFreeClusters);


  /* Cluster count difference should be at least 2 (for 8KB file with 4KB
   * clusters) */
  unit_assert_true((initialFreeClusters - afterFreeClusters) >= 2);
  unit_assert_ok(xMemFree(volInfo2));
  unit_assert_ok(xMemFree(largeData));
  unit_end();


  /* Test 15.2: Volume info calculations are consistent */
  unit_begin("Volume info calculations are internally consistent");


  /* Get fresh volume info */
  volInfo1 = null;
  unit_assert_ok(xFSGetVolumeInfo(vol, &volInfo1));
  unit_assert_not_null(volInfo1);


  /* Verify calculations */
  unit_assert_equal(volInfo1->bytesPerSector * volInfo1->sectorsPerCluster, volInfo1->bytesPerCluster);
  unit_assert_equal(volInfo1->totalClusters * volInfo1->bytesPerCluster, volInfo1->totalBytes);
  unit_assert_equal(volInfo1->freeClusters * volInfo1->bytesPerCluster, volInfo1->freeBytes);


  /* Free clusters should not exceed total clusters */
  unit_assert_true(volInfo1->freeClusters <= volInfo1->totalClusters);
  unit_assert_ok(xMemFree(volInfo1));
  unit_end();


  /* Unmount filesystem */
  xFSUnmount(vol);
}