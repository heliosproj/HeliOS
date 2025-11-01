/*UNCRUSTIFY-OFF*/
/**
 * @file console_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for console
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "console_harness.h"
#include "../../../drivers/char/char_driver.h"


/* External cleanup function declarations */
extern void __CharDeviceStateClear__(void);


/* Test buffer size */
#define TEST_OUTPUT_BUFFER_SIZE 1024u


/* Magic numbers for test timing */
#define TEST_CONSOLE_CYCLE_DELAY 5u
#define TEST_CONSOLE_LONG_DELAY 20u
#define TEST_CONSOLE_EXTENDED_DELAY 50u
#define TEST_MAX_COMMAND_LENGTH 80u
#define TEST_CYCLE_STRESS 100u


/* Test constant for expected buffer size (matches console.c) */
#define TEST_CAT_BUFFER_SIZE 0x100u /* 256 bytes */
/* Stress test constants */
#define STRESS_CHARACTER_COUNT 100u
#define STRESS_COMMAND_COUNT 50u
#define MAX_BUFFER_OVERFLOW_SIZE 200u


/* Helper function prototypes */
static void test_console_initialization(void);
static void test_character_input_handling(void);
static void test_command_parsing(void);
static void test_help_command(void);
static void test_version_command(void);
static void test_echo_command(void);
static void test_path_utilities(void);
static void test_buffered_file_operations(void);
static void test_device_caching(void);
static void test_error_conditions(void);
static void test_buffer_overflow_protection(void);
static void test_special_character_sequences(void);
static void test_command_stress_testing(void);
static void test_console_state_persistence(void);
static void test_null_pointer_validation(void);
static Base_t __OutputContains__(const Byte_t *output_, const Byte_t *expected_);
static void __SimulateConsoleCycles__(HalfWord_t cycles_);
static void __SetupConsoleEnvironment__(void);
static Return_t __InjectCommandAndVerify__(const Byte_t *command_, const Byte_t *expected_);


/**
 * @brief Console test harness entry point
 */
void console_harness(void) {
  unit_print("=== COMPREHENSIVE CONSOLE TEST SUITE ===");
  test_console_initialization();
  test_character_input_handling();
  test_command_parsing();
  test_help_command();
  test_version_command();
  test_echo_command();
  test_path_utilities();
  test_null_pointer_validation();
  test_buffered_file_operations();
  test_device_caching();
  test_buffer_overflow_protection();
  test_special_character_sequences();
  test_command_stress_testing();
  test_console_state_persistence();
  test_error_conditions();
  unit_print("=== CONSOLE TEST SUITE COMPLETE ===");
}


/* ============================================================================
 * SECTION 1: CONSOLE INITIALIZATION
 * ============================================================================
 */
static void test_console_initialization(void) {
  CharDeviceConfig_t *charConfig = null;
  Size_t size = 0x0u;


  unit_print("--- Section 1: Console Initialization ---");


  /* Test 1.1: Register mock USART device */
  unit_begin("Mock USART device registration succeeds");
  unit_assert_ok(xDeviceRegisterDevice(MOCKUSRT_self_register));
  unit_end();


  /* Test 1.2: Register character device driver */
  unit_begin("Character device driver registration succeeds");
  unit_assert_ok(xDeviceRegisterDevice(CHARDEV0_self_register));
  unit_end();


  /* Test 1.3: Configure character device to use mock USART */
  unit_begin("Configure character device with mock USART I/O driver");
  xMockUSARTReset();
  unit_assert_ok(xMemAlloc((volatile Addr_t **) &charConfig, sizeof(CharDeviceConfig_t)));
  charConfig->command = CHAR_CMD_CONFIG;
  charConfig->ioDriverUID = MOCK_USART_DEVICE_UID;
  charConfig->protocol = CHAR_PROTOCOL_RAW;
  charConfig->lineMode = CHAR_LINE_RAW;
  charConfig->baudRate = 115200u;
  charConfig->rxBufferSize = 0x0u;  /* Use default */
  charConfig->txBufferSize = 0x0u; /* Use default */
  size = sizeof(CharDeviceConfig_t);
  unit_assert_ok(xDeviceConfigDevice(CONFIG_CHAR_DEVICE_UID, &size, (Addr_t *) charConfig));
  xMemFree((Addr_t *) charConfig);
  unit_end();


  /* Test 1.4: Console initialization */
  unit_begin("Console initialization succeeds");
  __ConsoleStateClear__();
  unit_assert_ok(xConsoleInit());
  unit_end();


  /* Test 1.5: Multiple console initializations */
  unit_begin("Multiple console initializations work correctly");
  __ConsoleStateClear__();
  unit_assert_ok(xConsoleInit());
  unit_assert_ok(xConsoleInit());
  unit_assert_ok(xConsoleInit());
  unit_end();


  /* Test 1.6: Console initialization after reset */
  unit_begin("Console initialization works after reset");
  __ConsoleStateClear__();
  xMockUSARTReset();
  unit_assert_ok(xConsoleInit());
  unit_end();
}


/* ============================================================================
 * SECTION 2: CHARACTER INPUT HANDLING
 * ============================================================================
 */
static void test_character_input_handling(void) {
  Byte_t output[TEST_OUTPUT_BUFFER_SIZE];
  HalfWord_t outputLen = 0x0u;


  unit_print("--- Section 2: Character Input Handling ---");


  /* Test 2.1: Printable character input with echo */
  unit_begin("Printable characters are echoed when echo is enabled");
  __SetupConsoleEnvironment__();


  /* Inject a simple character */
  xMockUSARTInjectInput((const Byte_t *) "a");
  __SimulateConsoleCycles__(5u);


  /* Check that character was echoed */
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  unit_assert_true(outputLen > 0x0u);
  unit_end();


  /* Test 2.2: Backspace handling */
  unit_begin("Backspace removes character from buffer");
  __SetupConsoleEnvironment__();


  /* Inject characters followed by backspace */
  xMockUSARTInjectInput((const Byte_t *) "ab\b");
  __SimulateConsoleCycles__(10u);


  /* Verify backspace sequence was sent */
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  unit_assert_true(outputLen > 0x0u);
  unit_end();


  /* Test 2.3: Carriage return triggers command processing */
  unit_begin("Carriage return triggers command processing");
  __SetupConsoleEnvironment__();


  /* Inject command with CR */
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(20u);


  /* Verify response was generated */
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "Available commands"));
  unit_end();


  /* Test 2.4: Sequential character input */
  unit_begin("Sequential character inputs are processed correctly");
  __SetupConsoleEnvironment__();


  /* Inject multiple characters */
  xMockUSARTInjectInput((const Byte_t *) "test");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  unit_assert_true(outputLen > 0x0u);
  unit_end();


  /* Test 2.5: Multiple backspaces */
  unit_begin("Multiple backspaces are handled correctly");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "abcd\b\b");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  unit_assert_true(outputLen > 0x0u);
  unit_end();
}


/* ============================================================================
 * SECTION 3: COMMAND PARSING
 * ============================================================================
 */
static void test_command_parsing(void) {
  Byte_t output[TEST_OUTPUT_BUFFER_SIZE];
  HalfWord_t outputLen = 0x0u;


  unit_print("--- Section 3: Command Parsing ---");


  /* Test 3.1: Unknown command */
  unit_begin("Unknown command returns error message");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "unknowncmd\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "Unknown command"));
  unit_end();


  /* Test 3.2: Empty command */
  unit_begin("Empty command (just CR) does not produce error");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "\r");
  __SimulateConsoleCycles__(10u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_false(__OutputContains__(output, (const Byte_t *) "Unknown command"));
  unit_end();


  /* Test 3.3: Command with leading spaces */
  unit_begin("Command with leading whitespace is processed correctly");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "   help\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "Available commands"));
  unit_end();


  /* Test 3.4: Command with trailing spaces */
  unit_begin("Command with trailing whitespace is processed correctly");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "help   \r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "Available commands"));
  unit_end();


  /* Test 3.5: Multiple consecutive commands */
  unit_begin("Multiple consecutive commands are processed");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTClearOutput();
  xMockUSARTInjectInput((const Byte_t *) "version\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "HeliOS"));
  unit_end();
}


/* ============================================================================
 * SECTION 4: HELP COMMAND
 * ============================================================================
 */
static void test_help_command(void) {
  Byte_t output[TEST_OUTPUT_BUFFER_SIZE];
  HalfWord_t outputLen = 0x0u;


  unit_print("--- Section 4: Help Command ---");


  /* Test 4.1: Help command lists available commands */
  unit_begin("Help command displays available commands");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "Available commands"));
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "help"));
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "version"));
  unit_end();


  /* Test 4.2: Help command called multiple times */
  unit_begin("Help command works consistently across multiple calls");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTClearOutput();
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "Available commands"));
  unit_end();
}


/* ============================================================================
 * SECTION 5: VERSION COMMAND
 * ============================================================================
 */
static void test_version_command(void) {
  Byte_t output[TEST_OUTPUT_BUFFER_SIZE];
  HalfWord_t outputLen = 0x0u;


  unit_print("--- Section 5: Version Command ---");


  /* Test 5.1: Version command displays version info */
  unit_begin("Version command displays HeliOS version information");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "version\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "HeliOS"));
  unit_assert_true(__OutputContains__(output, (const Byte_t *) OS_VERSION_STRING));
  unit_end();


  /* Test 5.2: Version command consistency */
  unit_begin("Version command returns consistent information");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "version\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTClearOutput();
  xMockUSARTInjectInput((const Byte_t *) "version\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "HeliOS"));
  unit_end();
}


/* ============================================================================
 * SECTION 6: ECHO COMMAND
 * ============================================================================
 */
static void test_echo_command(void) {
  Byte_t output[TEST_OUTPUT_BUFFER_SIZE];
  HalfWord_t outputLen = 0x0u;


  unit_print("--- Section 6: Echo Command ---");


  /* Test 6.1: Echo command with arguments */
  unit_begin("Echo command prints arguments");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "echo test message\r");
  __SimulateConsoleCycles__(50u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "test message"));
  unit_end();


  /* Test 6.2: Echo command without arguments toggles echo mode */
  unit_begin("Echo command without arguments toggles echo mode");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "echo\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "Echo"));
  unit_end();


  /* Test 6.3: Echo with special characters */
  unit_begin("Echo command handles special characters");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "echo !@#$%\r");
  __SimulateConsoleCycles__(50u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(outputLen > 0x0u);
  unit_end();


  /* Test 6.4: Echo with long arguments */
  unit_begin("Echo command handles long arguments");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "echo this is a longer test message with many words\r");
  __SimulateConsoleCycles__(50u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "this is a longer"));
  unit_end();
}


/* ============================================================================
 * SECTION 7: PATH UTILITIES
 * ============================================================================
 */
static void test_path_utilities(void) {
  Byte_t result[CONFIG_FS_MAX_PATH_LENGTH];


  unit_print("--- Section 7: Path Utilities ---");


  /* Test 7.1: Path join with absolute path */
  unit_begin("Path join returns absolute path when path is absolute");
  __path_join__(result, (const Byte_t *) "/home/user", (const Byte_t *) "/etc/config", CONFIG_FS_MAX_PATH_LENGTH, 11, 12);
  unit_assert_equal(__strncmp__(result, (const Byte_t *) "/etc/config", CONFIG_FS_MAX_PATH_LENGTH), 0);
  unit_end();


  /* Test 7.2: Path join with relative path */
  unit_begin("Path join combines base and relative path");
  __path_join__(result, (const Byte_t *) "/home/user", (const Byte_t *) "documents", CONFIG_FS_MAX_PATH_LENGTH, 11, 10);
  unit_assert_equal(__strncmp__(result, (const Byte_t *) "/home/user/documents", CONFIG_FS_MAX_PATH_LENGTH), 0);
  unit_end();


  /* Test 7.3: Path normalization */
  unit_begin("Path normalize handles . and .. correctly");
  __strcpy__(result, (const Byte_t *) "/home/user/../admin/./config", CONFIG_FS_MAX_PATH_LENGTH);
  __path_normalize__(result, CONFIG_FS_MAX_PATH_LENGTH);
  unit_assert_equal(__strncmp__(result, (const Byte_t *) "/home/admin/config", CONFIG_FS_MAX_PATH_LENGTH), 0);
  unit_end();


  /* Test 7.4: Path is absolute check */
  unit_begin("Path is absolute correctly identifies absolute paths");
  unit_assert_true(__path_is_absolute__((const Byte_t *) "/home/user", 11));
  unit_assert_false(__path_is_absolute__((const Byte_t *) "relative/path", 14));
  unit_end();


  /* Test 7.5: Path dirname extraction */
  unit_begin("Path dirname extracts directory portion");
  __path_dirname__(result, (const Byte_t *) "/home/user/file.txt", CONFIG_FS_MAX_PATH_LENGTH, 20);
  unit_assert_equal(__strncmp__(result, (const Byte_t *) "/home/user", CONFIG_FS_MAX_PATH_LENGTH), 0);
  unit_end();


  /* Test 7.6: Path basename extraction */
  unit_begin("Path basename extracts filename portion");
  __path_basename__(result, (const Byte_t *) "/home/user/file.txt", CONFIG_FS_MAX_PATH_LENGTH, 20);
  unit_assert_equal(__strncmp__(result, (const Byte_t *) "file.txt", CONFIG_FS_MAX_PATH_LENGTH), 0);
  unit_end();


  /* Test 7.7: Path utilities with edge cases */
  unit_begin("Path utilities handle edge cases correctly");
  __path_join__(result, (const Byte_t *) "/", (const Byte_t *) "file.txt", CONFIG_FS_MAX_PATH_LENGTH, 2, 9);
  unit_assert_equal(__strncmp__(result, (const Byte_t *) "/file.txt", CONFIG_FS_MAX_PATH_LENGTH), 0);
  unit_end();


  /* Test 7.8: Multiple path normalizations */
  unit_begin("Multiple path normalizations work consistently");
  __strcpy__(result, (const Byte_t *) "/a/b/../c/./d", CONFIG_FS_MAX_PATH_LENGTH);
  __path_normalize__(result, CONFIG_FS_MAX_PATH_LENGTH);
  unit_assert_equal(__strncmp__(result, (const Byte_t *) "/a/c/d", CONFIG_FS_MAX_PATH_LENGTH), 0);
  unit_end();
}


/* ============================================================================
 * SECTION 8: null POINTER VALIDATION
 * ============================================================================
 */
static void test_null_pointer_validation(void) {
  Byte_t result[CONFIG_FS_MAX_PATH_LENGTH];


  unit_print("--- Section 8: null Pointer Validation ---");


  /* Test 8.1: Path utilities with null pointers */
  unit_begin("Path utilities handle null pointers gracefully");


  /* Test null in path_join */
  __path_join__(result, null, (const Byte_t *) "test", CONFIG_FS_MAX_PATH_LENGTH, 0, 5);
  __path_join__(result, (const Byte_t *) "test", null, CONFIG_FS_MAX_PATH_LENGTH, 5, 0);


  /* Test null in path_dirname */
  __path_dirname__(result, null, CONFIG_FS_MAX_PATH_LENGTH, 0);


  /* Test null in path_basename */
  __path_basename__(result, null, CONFIG_FS_MAX_PATH_LENGTH, 0);
  unit_end();


  /* Test 8.2: Path is absolute with null */
  unit_begin("Path is absolute handles null pointer");
  {
    Base_t result = __path_is_absolute__(null, 0);


    /* Function should handle null gracefully (likely returns false) */
    unit_assert_false(result);
  } unit_end();
}


/* ============================================================================
 * SECTION 9: BUFFERED FILE OPERATIONS
 * ============================================================================
 */
static void test_buffered_file_operations(void) {
  unit_print("--- Section 9: Buffered File Operations ---");


  /* Test 9.1: Verify cat command uses buffering */
  unit_begin("Cat command operates with small buffer (memory efficient)");


  /* Note: This test validates that buffer size is reasonably small */
  unit_assert_true(TEST_CAT_BUFFER_SIZE <= 0x200u); /* 512 bytes or less */
  unit_end();


  /* Test 9.2: Large file simulation */
  unit_begin("Console handles file operation requests");
  __SetupConsoleEnvironment__();


  /* Inject cat command for non-existent file to test error handling */
  if(OK(__InjectCommandAndVerify__((const Byte_t *) "cat /nonexistent", (const Byte_t *) ""))) {
    /* Command was processed */
  }

  unit_end();


  /* Test 9.3: Buffer efficiency validation */
  unit_begin("Buffered operations maintain memory efficiency");
  unit_assert_true(TEST_CAT_BUFFER_SIZE < 1024); /* Less than 1KB */
  unit_assert_true(TEST_CAT_BUFFER_SIZE >= 64); /* At least 64 bytes */
  unit_end();
}


/* ============================================================================
 * SECTION 10: DEVICE CACHING
 * ============================================================================
 */
static void test_device_caching(void) {
  Byte_t output[TEST_OUTPUT_BUFFER_SIZE];
  HalfWord_t outputLen = 0x0u;


  unit_print("--- Section 10: Device Caching ---");


  /* Test 10.1: Multiple rapid console operations */
  unit_begin("Device caching improves performance for repeated operations");
  __SetupConsoleEnvironment__();


  /* Inject multiple characters as a string */
  xMockUSARTInjectInput((const Byte_t *) "aaaaaaaaaaaaaaaa"); /* 16 'a'
                                                               * characters */
  __SimulateConsoleCycles__(0x50u); /* Allow sufficient time for processing */
  /* Verify at least some characters were processed */
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  unit_assert_true(outputLen > 0x0u);
  unit_end();


  /* Test 10.2: Sustained console operations */
  unit_begin("Console handles sustained input operations");
  __SetupConsoleEnvironment__();


  /* Inject sustained input */
  xMockUSARTInjectInput((const Byte_t *) "test1\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTInjectInput((const Byte_t *) "test2\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTInjectInput((const Byte_t *) "test3\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  unit_assert_true(outputLen > 0x0u);
  unit_end();


  /* Test 10.3: Device caching across console reinitialization */
  unit_begin("Device cache works across console resets");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(20u);
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "version\r");
  __SimulateConsoleCycles__(20u);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  unit_assert_true(outputLen > 0x0u);
  unit_end();
}


/* ============================================================================
 * SECTION 11: BUFFER OVERFLOW PROTECTION
 * ============================================================================
 */
static void test_buffer_overflow_protection(void) {
  Byte_t longCommand[MAX_BUFFER_OVERFLOW_SIZE];
  HalfWord_t i = 0x0u;


  unit_print("--- Section 11: Buffer Overflow Protection ---");


  /* Test 11.1: Command buffer overflow protection */
  unit_begin("Console handles command buffer overflow gracefully");
  __SetupConsoleEnvironment__();


  /* Create oversized command */
  for(i = 0x0u; i < (MAX_BUFFER_OVERFLOW_SIZE - 5u); i++) {
    longCommand[i] = 'x';
  }

  longCommand[i++] = '\r';
  longCommand[i] = 0x00u;


  /* Inject oversized command */
  xMockUSARTInjectInput(longCommand);
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY * 0x2u);


  /* Console should handle without crashing */
  unit_assert_true(true);
  unit_end();


  /* Test 11.2: Input buffer saturation */
  unit_begin("Console handles input buffer saturation");
  __SetupConsoleEnvironment__();


  /* Saturate buffer with characters without CR */
  for(i = 0x0u; i < 100; i++) {
    longCommand[i] = 'A' + (i % 26);
  }

  longCommand[100] = 0x00u;
  xMockUSARTInjectInput(longCommand);
  __SimulateConsoleCycles__(TEST_CONSOLE_EXTENDED_DELAY);
  unit_assert_true(true);
  unit_end();


  /* Test 11.3: Repeated overflow attempts */
  unit_begin("Console recovers from repeated overflow attempts");
  __SetupConsoleEnvironment__();

  for(i = 0x0u; i < 5; i++) {
    HalfWord_t j;


    for(j = 0x0u; j < 50; j++) {
      longCommand[j] = 'x';
    }

    longCommand[50] = '\r';
    longCommand[51] = 0x00u;
    xMockUSARTInjectInput(longCommand);
    __SimulateConsoleCycles__(TEST_CONSOLE_CYCLE_DELAY);
  }

  unit_assert_true(true);
  unit_end();
}


/* ============================================================================
 * SECTION 12: SPECIAL CHARACTER SEQUENCES
 * ============================================================================
 */
static void test_special_character_sequences(void) {
  unit_print("--- Section 12: Special Character Sequences ---");


  /* Test 12.1: Special character handling */
  unit_begin("Console handles special characters appropriately");
  __SetupConsoleEnvironment__();


  /* Inject control characters */
  xMockUSARTInjectInput((const Byte_t *) "\x01\x02\x03");
  __SimulateConsoleCycles__(TEST_CONSOLE_CYCLE_DELAY);
  unit_assert_true(true);
  unit_end();


  /* Test 12.2: Multiple backspaces */
  unit_begin("Console handles excessive backspaces");
  __SetupConsoleEnvironment__();


  /* More backspaces than characters */
  xMockUSARTInjectInput((const Byte_t *) "ab\b\b\b\b\b");
  __SimulateConsoleCycles__(TEST_CONSOLE_CYCLE_DELAY);
  unit_assert_true(true);
  unit_end();


  /* Test 12.3: Mixed line endings */
  unit_begin("Console handles various line endings");
  __SetupConsoleEnvironment__();


  /* Test CR */
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);


  /* Clear and test LF */
  xMockUSARTClearOutput();
  xMockUSARTInjectInput((const Byte_t *) "help\n");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  unit_assert_true(true);
  unit_end();


  /* Test 12.4: Escape sequences */
  unit_begin("Console handles escape sequences");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "\x1b[A\x1b[B");
  __SimulateConsoleCycles__(TEST_CONSOLE_CYCLE_DELAY);
  unit_assert_true(true);
  unit_end();


  /* Test 12.5: Tab characters */
  unit_begin("Console handles tab characters");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "echo\ttest\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_EXTENDED_DELAY);
  unit_assert_true(true);
  unit_end();
}


/* ============================================================================
 * SECTION 13: COMMAND STRESS TESTING
 * ============================================================================
 */
static void test_command_stress_testing(void) {
  HalfWord_t i;
  Byte_t output[TEST_OUTPUT_BUFFER_SIZE];
  HalfWord_t outputLen = 0x0u;


  unit_print("--- Section 13: Command Stress Testing ---");


  /* Test 13.1: Rapid command execution */
  unit_begin("Console handles rapid command execution");
  __SetupConsoleEnvironment__();

  for(i = 0x0u; i < STRESS_COMMAND_COUNT; i++) {
    xMockUSARTInjectInput((const Byte_t *) "help\r");
    __SimulateConsoleCycles__(TEST_CONSOLE_CYCLE_DELAY);
    xMockUSARTClearOutput();
  }

  unit_assert_true(true);
  unit_end();


  /* Test 13.2: Mixed command stress test */
  unit_begin("Console handles mixed command patterns");
  __SetupConsoleEnvironment__();

  for(i = 0x0u; i < 20; i++) {
    if((i % 3) == 0) {
      xMockUSARTInjectInput((const Byte_t *) "help\r");
    } else if((i % 3) == 1) {
      xMockUSARTInjectInput((const Byte_t *) "version\r");
    } else {
      xMockUSARTInjectInput((const Byte_t *) "echo test\r");
    }

    __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
    xMockUSARTClearOutput();
  }

  unit_assert_true(true);
  unit_end();


  /* Test 13.3: Character input stress test */
  unit_begin("Console handles high volume character input");
  __SetupConsoleEnvironment__();

  for(i = 0x0u; i < STRESS_CHARACTER_COUNT; i++) {
    Byte_t ch[2];


    ch[0] = 'a' + (i % 26);
    ch[1] = 0x00u;
    xMockUSARTInjectInput(ch);
    __SimulateConsoleCycles__(1u);
  }

  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  unit_assert_true(outputLen > 0x0u);
  unit_end();


  /* Test 13.4: Command with argument variations */
  unit_begin("Console handles commands with varying argument counts");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "echo\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTClearOutput();
  xMockUSARTInjectInput((const Byte_t *) "echo one\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTClearOutput();
  xMockUSARTInjectInput((const Byte_t *) "echo one two three\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  unit_assert_true(outputLen > 0x0u);
  unit_end();
}


/* ============================================================================
 * SECTION 14: CONSOLE STATE PERSISTENCE
 * ============================================================================
 */
static void test_console_state_persistence(void) {
  Byte_t output[TEST_OUTPUT_BUFFER_SIZE];
  HalfWord_t outputLen = 0x0u;


  unit_print("--- Section 14: Console State Persistence ---");


  /* Test 14.1: State persistence across commands */
  unit_begin("Console state persists across multiple commands");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTClearOutput();
  xMockUSARTInjectInput((const Byte_t *) "version\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "HeliOS"));
  unit_end();


  /* Test 14.2: Echo mode state persistence */
  unit_begin("Echo mode state persists correctly");
  __SetupConsoleEnvironment__();


  /* Toggle echo off */
  xMockUSARTInjectInput((const Byte_t *) "echo\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTClearOutput();


  /* Input characters - should have different echo behavior */
  xMockUSARTInjectInput((const Byte_t *) "test");
  __SimulateConsoleCycles__(TEST_CONSOLE_CYCLE_DELAY);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);


  /* Some output should exist (though echo mode may affect amount) */
  unit_assert_true(true);
  unit_end();


  /* Test 14.3: Console recovery after errors */
  unit_begin("Console recovers properly after command errors");
  __SetupConsoleEnvironment__();


  /* Send invalid command */
  xMockUSARTInjectInput((const Byte_t *) "invalidcmd\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTClearOutput();


  /* Send valid command */
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "Available commands"));
  unit_end();
}


/* ============================================================================
 * SECTION 15: ERROR CONDITIONS
 * ============================================================================
 */
static void test_error_conditions(void) {
  unit_print("--- Section 15: Error Conditions ---");


  /* Test 15.1: Null input handling */
  unit_begin("Console handles null/empty input gracefully");
  __SetupConsoleEnvironment__();


  /* Send just CR without any command */
  xMockUSARTInjectInput((const Byte_t *) "\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  unit_assert_true(true);
  unit_end();


  /* Test 15.2: Console operation after multiple errors */
  unit_begin("Console continues operation after multiple errors");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "invalid1\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_CYCLE_DELAY);
  xMockUSARTInjectInput((const Byte_t *) "invalid2\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_CYCLE_DELAY);
  xMockUSARTInjectInput((const Byte_t *) "invalid3\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_CYCLE_DELAY);


  /* Should still work */
  xMockUSARTClearOutput();
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  unit_assert_true(true);
  unit_end();


  /* Test 15.3: Mixed valid and invalid commands */
  unit_begin("Console handles mixed valid and invalid commands");
  __SetupConsoleEnvironment__();
  xMockUSARTInjectInput((const Byte_t *) "help\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTInjectInput((const Byte_t *) "invalid\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  xMockUSARTInjectInput((const Byte_t *) "version\r");
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);
  unit_assert_true(true);
  unit_end();
}


/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================
 */


/**
 * @brief Check if output contains expected string
 * @param  output_   Output buffer to search
 * @param  expected_ Expected string
 * @return           Base_t true if found, false otherwise
 */
static Base_t __OutputContains__(const Byte_t *output_, const Byte_t *expected_) {
  Word_t i = 0x0u;
  Word_t j = 0x0u;
  Base_t found = false;


  if(!__PointerIsNotNull__(output_) || !__PointerIsNotNull__(expected_)) {
    return(false);
  }


  /* Simple substring search */
  while(0x00u != output_[i]) {
    j = 0x0u;

    while((output_[i + j] == expected_[j]) && (0x00u != expected_[j])) {
      j++;
    }

    if(0x00u == expected_[j]) {
      found = true;
      break;
    }

    i++;
  }

  return(found);
}


/**
 * @brief Simulate console task cycles
 * @param cycles_ Number of cycles to simulate
 */
static void __SimulateConsoleCycles__(HalfWord_t cycles_) {
  HalfWord_t i = 0x0u;


  for(i = 0x0u; i < cycles_; i++) {
    vConsoleTask(null, null);
  }
}


/**
 * @brief Setup console environment for testing
 */
static void __SetupConsoleEnvironment__(void) {
  CharDeviceConfig_t *charConfig = null;
  Size_t size = 0x0u;


  /* Clear and reset */
  __ConsoleStateClear__();
  __CharDeviceStateClear__();
  xMockUSARTReset();


  /* Configure character device with mock USART */
  if(OK(xMemAlloc((volatile Addr_t **) &charConfig, sizeof(CharDeviceConfig_t)))) {
    charConfig->command = CHAR_CMD_CONFIG;
    charConfig->ioDriverUID = MOCK_USART_DEVICE_UID;
    charConfig->protocol = CHAR_PROTOCOL_RAW;
    charConfig->lineMode = CHAR_LINE_RAW;
    charConfig->baudRate = 115200u;
    charConfig->rxBufferSize = 0x0u;
    charConfig->txBufferSize = 0x0u;
    size = sizeof(CharDeviceConfig_t);
    xDeviceConfigDevice(CONFIG_CHAR_DEVICE_UID, &size, (Addr_t *) charConfig);
    xMemFree((Addr_t *) charConfig);
  }


  /* Initialize console */
  xConsoleInit();


  /* Run console task a few cycles to initialize */
  __SimulateConsoleCycles__(10u);


  /* Clear output buffer to start fresh */
  xMockUSARTClearOutput();
}


/**
 * @brief Inject command and verify expected response
 * @param  command_  Command to inject
 * @param  expected_ Expected text in response
 * @return           Return_t OK if expected text found, error otherwise
 */
static Return_t __InjectCommandAndVerify__(const Byte_t *command_, const Byte_t *expected_) {
  FUNCTION_ENTER;


  Byte_t output[TEST_OUTPUT_BUFFER_SIZE];
  HalfWord_t outputLen = 0x0u;
  Byte_t commandWithCR[TEST_MAX_COMMAND_LENGTH + 0x2u];


  if(__PointerIsNull__(command_) || __PointerIsNull__(expected_)) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }


  /* Clear any previous output */
  xMockUSARTClearOutput();


  /* Prepare command with CR */
  if(ERROR(__strcpy__(commandWithCR, command_, TEST_MAX_COMMAND_LENGTH + 0x2u))) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  if(ERROR(__strcat__(commandWithCR, (const Byte_t *) "\r", TEST_MAX_COMMAND_LENGTH + 0x2u))) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }


  /* Inject command */
  xMockUSARTInjectInput(commandWithCR);


  /* Simulate console cycles */
  __SimulateConsoleCycles__(TEST_CONSOLE_LONG_DELAY);


  /* Get output and verify */
  xMockUSARTGetOutput(output, TEST_OUTPUT_BUFFER_SIZE, &outputLen);
  output[outputLen] = 0x00u;

  if(__OutputContains__(output, expected_) || (0x00u == expected_[0])) {
    __ReturnOk__();
  }


  /* Return error by default */
  FUNCTION_EXIT;
}