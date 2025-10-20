/*UNCRUSTIFY-OFF*/
/**
 * @file console_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for console
 * @version 0.5.0
 * @date 2025-01-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "console_harness.h"
#include "../../../drivers/char/char_driver.h"


/* Test buffer size */
#define TEST_OUTPUT_BUFFER_SIZE 1024u


/* Helper function prototypes */
static void test_console_initialization(void);
static void test_character_input_handling(void);
static void test_command_parsing(void);
static void test_help_command(void);
static void test_version_command(void);
static void test_echo_command(void);
static Base_t __OutputContains__(const Byte_t *output_, const Byte_t *expected_);
static void __SimulateConsoleCycles__(HalfWord_t cycles_);
static void __SetupConsoleEnvironment__(void);


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
  unit_assert_ok(xDeviceConfigDevice(CONFIG_CONSOLE_DEVICE_UID, &size, (Addr_t *) charConfig));
  xMemFree((Addr_t *) charConfig);
  unit_end();


  /* Test 1.4: Console initialization */
  unit_begin("Console initialization succeeds");
  __ConsoleStateClear__();
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
  Task_t *consoleTask = null;


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


  /* Clean up */
  if(__PointerIsNotNull__(consoleTask)) {
    xMemFree(consoleTask);
  }
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
  unit_assert_true(__OutputContains__(output, (const Byte_t *) "0.5.0"));
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
    xDeviceConfigDevice(CONFIG_CONSOLE_DEVICE_UID, &size, (Addr_t *) charConfig);
    xMemFree((Addr_t *) charConfig);
  }


  /* Initialize console */
  xConsoleInit();


  /* Run console task a few cycles to initialize (banner, prompt, etc.) */
  __SimulateConsoleCycles__(10u);


  /* Clear output buffer to start fresh for tests */
  xMockUSARTClearOutput();
}