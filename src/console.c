/*UNCRUSTIFY-OFF*/
/**
 * @file console.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for console and shell support
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
#include "console.h"
#include "fs.h"
#include "../drivers/char/char_driver.h"


/* Console state */
static ConsoleState_t consoleState;
static Volume_t *mountedVolume = null;


/* Forward declarations for command handlers */
static Return_t __ConsoleCmdHelp__(const Byte_t *args_);
static Return_t __ConsoleCmdVersion__(const Byte_t *args_);
static Return_t __ConsoleCmdTasks__(const Byte_t *args_);
static Return_t __ConsoleCmdMem__(const Byte_t *args_);
static Return_t __ConsoleCmdClear__(const Byte_t *args_);
static Return_t __ConsoleCmdEcho__(const Byte_t *args_);
static Return_t __ConsoleCmdLs__(const Byte_t *args_);
static Return_t __ConsoleCmdCd__(const Byte_t *args_);
static Return_t __ConsoleCmdPwd__(const Byte_t *args_);
static Return_t __ConsoleCmdCat__(const Byte_t *args_);
static Return_t __ConsoleCmdMv__(const Byte_t *args_);
static Return_t __ConsoleCmdRm__(const Byte_t *args_);
static Return_t __ConsoleCmdMkdir__(const Byte_t *args_);


/* Forward declarations for helper functions */
static Return_t __ConsoleWriteString__(const Byte_t *str_);
static Return_t __ConsoleReadChar__(Byte_t *ch_);
static Return_t __ConsoleCheckDevice__(void);
static Return_t __ConsoleProcessCommand__(void);
static void __ConsolePrintPrompt__(void);
static Return_t __ConsoleHandleBackspace__(void);
static void __StringCopy__(Byte_t *dest_, const Byte_t *src_);
static Word_t __StringLength__(const Byte_t *str_);
static Base_t __StringCompare__(const Byte_t *s1_, const Byte_t *s2_);
static void __SkipWhitespace__(const Byte_t **str_);
static void __uitoah__(Word_t value_, Byte_t *buffer_, Word_t bufferSize_);


/* Command table structure */
typedef struct ConsoleCommand_s {
  const Byte_t *name;
  Return_t (* handler)(const Byte_t *);
  const Byte_t *description;
} ConsoleCommand_t;


/* Command table */
static const ConsoleCommand_t commandTable[] = {
  { (const Byte_t *) "help", __ConsoleCmdHelp__, (const Byte_t *) "Display available commands" },
  { (const Byte_t *) "version", __ConsoleCmdVersion__, (const Byte_t *) "Display version information" },
  { (const Byte_t *) "tasks", __ConsoleCmdTasks__, (const Byte_t *) "List running tasks" },
  { (const Byte_t *) "mem", __ConsoleCmdMem__, (const Byte_t *) "Display memory statistics" },
  { (const Byte_t *) "clear", __ConsoleCmdClear__, (const Byte_t *) "Clear the screen" },
  { (const Byte_t *) "echo", __ConsoleCmdEcho__, (const Byte_t *) "Toggle echo mode or print message" },
  { (const Byte_t *) "ls", __ConsoleCmdLs__, (const Byte_t *) "List directory contents" },
  { (const Byte_t *) "cd", __ConsoleCmdCd__, (const Byte_t *) "Change directory" },
  { (const Byte_t *) "pwd", __ConsoleCmdPwd__, (const Byte_t *) "Print working directory" },
  { (const Byte_t *) "cat", __ConsoleCmdCat__, (const Byte_t *) "Display file contents" },
  { (const Byte_t *) "mv", __ConsoleCmdMv__, (const Byte_t *) "Move/rename file" },
  { (const Byte_t *) "rm", __ConsoleCmdRm__, (const Byte_t *) "Remove file" },
  { (const Byte_t *) "mkdir", __ConsoleCmdMkdir__, (const Byte_t *) "Create directory" },
  { null, null, null }
};


/**
 * @brief Initialize console subsystem
 * @return Return_t OK or error
 */
Return_t xConsoleInit(void) {
  FUNCTION_ENTER;

  /* Initialize console state */
  consoleState.deviceReady = false;
  #if defined(CONFIG_CONSOLE_ECHO_ENABLED)
    consoleState.echoEnabled = true;
  #else /* if defined(CONFIG_CONSOLE_ECHO_ENABLED) */
    consoleState.echoEnabled = false;
  #endif /* if defined(CONFIG_CONSOLE_ECHO_ENABLED) */
  consoleState.bufferPosition = 0;
  __memset__(consoleState.commandBuffer, 0x00u, CONFIG_CONSOLE_MAX_COMMAND_LENGTH);
  __StringCopy__(consoleState.currentWorkingDirectory, (const Byte_t *) "/");
  mountedVolume = null;

  __ReturnOk__();
  FUNCTION_EXIT;
}


/**
 * @brief Console task entry point
 * @param task_ Task handle
 * @param parm_ Task parameter (unused)
 */
void vConsoleTask(Task_t *task_, TaskParm_t *parm_) {
  Byte_t ch = 0x00u;


  (void) task_;   /* Unused parameter */
  (void) parm_;   /* Unused parameter */


  /* Check device status */
  if(OK(__ConsoleCheckDevice__())) {
    /* Device became ready - display banner */
    if(!consoleState.deviceReady) {
      consoleState.deviceReady = true;
      __ConsoleWriteString__((const Byte_t *) CONSOLE_BANNER);

      /* Try to mount filesystem */
      if(OK(xFSMount(&mountedVolume))) {
        __ConsoleWriteString__((const Byte_t *) "Filesystem mounted successfully.\r\n");
      } else {
        __ConsoleWriteString__((const Byte_t *) "Warning: Filesystem not available.\r\n");
      }

      __ConsolePrintPrompt__();
    }
  } else {
    /* Device not ready - reset state */
    if(consoleState.deviceReady) {
      consoleState.deviceReady = false;
      consoleState.bufferPosition = 0;

      /* Unmount filesystem if it was mounted */
      if(__PointerIsNotNull__(mountedVolume)) {
        xFSUnmount(mountedVolume);
        mountedVolume = null;
      }
    }

    return;
  }

  /* Read character from device */
  if(OK(__ConsoleReadChar__(&ch))) {
    /* Handle special characters */
    if(0x08u == ch || 0x7Fu == ch) {
      /* Backspace or DEL */
      __ConsoleHandleBackspace__();
    } else if(0x0Du == ch || 0x0Au == ch) {
      /* Carriage return or line feed */
      __ConsoleWriteString__((const Byte_t *) "\r\n");

      /* Process command if buffer is not empty */
      if(consoleState.bufferPosition > 0x0u) {
        consoleState.commandBuffer[consoleState.bufferPosition] = 0x00u;
        __ConsoleProcessCommand__();
        consoleState.bufferPosition = 0x0u;
      }

      __ConsolePrintPrompt__();
    } else if(ch >= 0x20u && ch <= 0x7Eu) {
      /* Printable character */
      if(consoleState.bufferPosition < (CONFIG_CONSOLE_MAX_COMMAND_LENGTH - 0x1u)) {
        consoleState.commandBuffer[consoleState.bufferPosition++] = ch;

        /* Echo character if enabled */
        if(consoleState.echoEnabled) {
          Byte_t echoChar[0x2];

          echoChar[0x0] = ch;
          echoChar[0x1] = 0x00u;
          __ConsoleWriteString__(echoChar);
        }
      }
    }
  }
}


/**
 * @brief Check if character device is ready
 * @return Return_t OK if ready, error otherwise
 */
static Return_t __ConsoleCheckDevice__(void) {
  FUNCTION_ENTER;


  Device_t *device = null;

  /* Use internal device API to check if device exists and is running */
  if(OK(__DeviceListFind__(CONFIG_CONSOLE_DEVICE_UID, &device))) {
    if(__PointerIsNotNull__(device) && (DeviceStateRunning == device->state)) {
      __ReturnOk__();
    } else {
      __ReturnError__();
    }
  } else {
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Write string to console
 * @param str_ String to write
 * @return Return_t OK or error
 */
static Return_t __ConsoleWriteString__(const Byte_t *str_) {
  FUNCTION_ENTER;

  Word_t len = 0x0u;
  Size_t size = 0x0u;
  Device_t *device = null;
  CharDeviceCommand_t cmd;

  if(__PointerIsNotNull__(str_)) {
    len = __StringLength__(str_);

    if(0x0u < len) {
      /* Use internal device API instead of public API */
      if(OK(__DeviceListFind__(CONFIG_CONSOLE_DEVICE_UID, &device))) {
        if(__PointerIsNotNull__(device)) {
          /* Step 1: Configure byte count for write operation */
          cmd.command = CHAR_CMD_SET_PARAMS;
          cmd.byteCount = (HalfWord_t)len;
          cmd.transferMode = CHAR_IO_MODE_BLOCKING;
          size = sizeof(CharDeviceCommand_t);

          if(OK((*device->config)(device, &size, (Addr_t *) &cmd))) {
            /* Step 2: Perform the write operation */
            size = len;

            if(OK((*device->write)(device, &size, (Addr_t *) str_))) {
              __ReturnOk__();
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Read single character from console
 * @param ch_ Pointer to store character
 * @return Return_t OK or error
 */
static Return_t __ConsoleReadChar__(Byte_t *ch_) {
  FUNCTION_ENTER;

  Size_t size = 0x1u;
  Addr_t *readData = null;
  Device_t *device = null;
  CharDeviceCommand_t cmd;

  if(__PointerIsNotNull__(ch_)) {
    /* Use internal device API instead of public API */
    if(OK(__DeviceListFind__(CONFIG_CONSOLE_DEVICE_UID, &device))) {
      if(__PointerIsNotNull__(device)) {
        /* Step 1: Configure byte count for read operation */
        cmd.command = CHAR_CMD_SET_PARAMS;
        cmd.byteCount = 0x1u;
        cmd.transferMode = CHAR_IO_MODE_BLOCKING;
        size = sizeof(CharDeviceCommand_t);

        if(OK((*device->config)(device, &size, (Addr_t *) &cmd))) {
          /* Step 2: Perform the read operation */
          size = 0x1u;

          if(OK((*device->read)(device, &size, &readData))) {
            if(__PointerIsNotNull__(readData) && (0x0u < size)) {
              *ch_ = *((Byte_t *) readData);
              __KernelFreeMemory__(readData);
              __ReturnOk__();
            } else {
              if(__PointerIsNotNull__(readData)) {
                __KernelFreeMemory__(readData);
              }

              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Print command prompt
 */
static void __ConsolePrintPrompt__(void) {
  __ConsoleWriteString__((const Byte_t *) CONFIG_CONSOLE_PROMPT);
}


/**
 * @brief Handle backspace character
 * @return Return_t OK or error
 */
static Return_t __ConsoleHandleBackspace__(void) {
  FUNCTION_ENTER;

  if(consoleState.bufferPosition > 0x0u) {
    consoleState.bufferPosition--;
    consoleState.commandBuffer[consoleState.bufferPosition] = 0x00u;

    /* Echo backspace sequence if enabled */
    if(consoleState.echoEnabled) {
      __ConsoleWriteString__((const Byte_t *) "\b \b");
    }

    __ReturnOk__();
  } else {
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Process command in buffer
 * @return Return_t OK or error
 */
static Return_t __ConsoleProcessCommand__(void) {
  FUNCTION_ENTER;


  Byte_t *cmdName = consoleState.commandBuffer;
  Byte_t *cmdArgs = null;
  Word_t i = 0x0u;


  /* Skip leading whitespace */
  __SkipWhitespace__((const Byte_t **) &cmdName);

  /* Find space to separate command from arguments */
  for(i = 0x0u; cmdName[i] != 0x00u; i++) {
    if(0x20u == cmdName[i]) {
      cmdName[i] = 0x00u;
      cmdArgs = &cmdName[i + 0x1u];
      __SkipWhitespace__((const Byte_t **) &cmdArgs);
      break;
    }
  }

  /* Empty command */
  if(0x00u == cmdName[0x0]) {
    __ReturnOk__();
    FUNCTION_EXIT;
  }

  /* Search command table */
  for(i = 0x0u; __PointerIsNotNull__(commandTable[i].name); i++) {
    if(__StringCompare__(cmdName, commandTable[i].name)) {
      if(__PointerIsNotNull__(commandTable[i].handler)) {
        if(OK(commandTable[i].handler((const Byte_t *) cmdArgs))) {
          __ReturnOk__();
        } else {
          __ReturnError__();
        }

        FUNCTION_EXIT;
      }
    }
  }

  /* Unknown command */
  __ConsoleWriteString__((const Byte_t *) "Unknown command: ");
  __ConsoleWriteString__(cmdName);
  __ConsoleWriteString__((const Byte_t *) "\r\nType 'help' for available commands.\r\n");

  __ReturnError__();
  FUNCTION_EXIT;
}


/**
 * @brief Command: help - Display available commands
 * @return Return_t OK
 */
static Return_t __ConsoleCmdHelp__(const Byte_t *args_) {
  FUNCTION_ENTER;

  Word_t i = 0x0u;

  (void) args_;


  __ConsoleWriteString__((const Byte_t *) "Available commands:\r\n");

  for(i = 0x0u; __PointerIsNotNull__(commandTable[i].name); i++) {
    __ConsoleWriteString__((const Byte_t *) "  ");
    __ConsoleWriteString__(commandTable[i].name);
    __ConsoleWriteString__((const Byte_t *) " - ");
    __ConsoleWriteString__(commandTable[i].description);
    __ConsoleWriteString__((const Byte_t *) "\r\n");
  }

  __ReturnOk__();
  FUNCTION_EXIT;
}


/**
 * @brief Command: version - Display version information
 * @return Return_t OK
 */
static Return_t __ConsoleCmdVersion__(const Byte_t *args_) {
  FUNCTION_ENTER;

  (void) args_;
  __ConsoleWriteString__((const Byte_t *) "HeliOS Embedded Operating System\r\n");
  __ConsoleWriteString__((const Byte_t *) "Version: 0.5.0\r\n");
  __ConsoleWriteString__((const Byte_t *) "Copyright (C) 2020-2026 HeliOS Project\r\n");
  __ConsoleWriteString__((const Byte_t *) "License: GPL-2.0-or-later\r\n");

  __ReturnOk__();
  FUNCTION_EXIT;
}


/**
 * @brief Command: tasks - List running tasks
 * @return Return_t OK or error
 */
static Return_t __ConsoleCmdTasks__(const Byte_t *args_) {
  FUNCTION_ENTER;

  TaskInfo_t *taskList = null;
  Base_t taskCount = 0x0u;
  Base_t i = 0x0u;
  Byte_t numBuf[0x10];

  (void) args_;


  __ConsoleWriteString__((const Byte_t *) "Task List:\r\n");
  __ConsoleWriteString__((const Byte_t *) "  ID   State      Runtime\r\n");
  __ConsoleWriteString__((const Byte_t *) "  ---- ---------- --------\r\n");

  if(OK(xTaskGetAllTaskInfo(&taskList, &taskCount))) {
    for(i = 0x0u; i < taskCount; i++) {
      /* Print task ID */
      __ConsoleWriteString__((const Byte_t *) "  ");
      __uitoah__((Word_t) taskList[i].id, numBuf, sizeof(numBuf));
      __ConsoleWriteString__(numBuf);
      __ConsoleWriteString__((const Byte_t *) "   ");

      /* Print task state */
      switch(taskList[i].state) {
      case TaskStateSuspended:
        __ConsoleWriteString__((const Byte_t *) "Suspended  ");
        break;

      case TaskStateRunning:
        __ConsoleWriteString__((const Byte_t *) "Running    ");
        break;

      case TaskStateWaiting:
        __ConsoleWriteString__((const Byte_t *) "Waiting    ");
        break;

      default:
        __ConsoleWriteString__((const Byte_t *) "Unknown    ");
        break;
      }

      /* Print runtime */
      __uitoah__((Word_t) taskList[i].totalRunTime, numBuf, sizeof(numBuf));
      __ConsoleWriteString__(numBuf);
      __ConsoleWriteString__((const Byte_t *) "\r\n");
    }

    /* Free task list */
    xMemFree(taskList);
    __ReturnOk__();
  } else {
    __ConsoleWriteString__((const Byte_t *) "Error: Unable to retrieve task information.\r\n");
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Command: mem - Display memory statistics
 * @return Return_t OK or error
 */
static Return_t __ConsoleCmdMem__(const Byte_t *args_) {
  FUNCTION_ENTER;

  MemoryRegionStats_t *memState = null;
  Byte_t numBuf[0x10];

  (void) args_;


  __ConsoleWriteString__((const Byte_t *) "Memory Statistics:\r\n");

  if(OK(xMemGetHeapStats(&memState))) {
    __ConsoleWriteString__((const Byte_t *) "  Available Space:  ");
    __uitoah__((Word_t) memState->availableSpaceInBytes, numBuf, sizeof(numBuf));
    __ConsoleWriteString__(numBuf);
    __ConsoleWriteString__((const Byte_t *) " bytes\r\n");

    __ConsoleWriteString__((const Byte_t *) "  Free Blocks:      ");
    __uitoah__((Word_t) memState->numberOfFreeBlocks, numBuf, sizeof(numBuf));
    __ConsoleWriteString__(numBuf);
    __ConsoleWriteString__((const Byte_t *) "\r\n");

    __ConsoleWriteString__((const Byte_t *) "  Largest Free:     ");
    __uitoah__((Word_t) memState->largestFreeEntryInBytes, numBuf, sizeof(numBuf));
    __ConsoleWriteString__(numBuf);
    __ConsoleWriteString__((const Byte_t *) " bytes\r\n");

    __ConsoleWriteString__((const Byte_t *) "  Smallest Free:    ");
    __uitoah__((Word_t) memState->smallestFreeEntryInBytes, numBuf, sizeof(numBuf));
    __ConsoleWriteString__(numBuf);
    __ConsoleWriteString__((const Byte_t *) " bytes\r\n");

    /* Free memory state */
    xMemFree(memState);
    __ReturnOk__();
  } else {
    __ConsoleWriteString__((const Byte_t *) "Error: Unable to retrieve memory information.\r\n");
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Command: clear - Clear the screen
 * @return Return_t OK
 */
static Return_t __ConsoleCmdClear__(const Byte_t *args_) {
  FUNCTION_ENTER;

  (void) args_;
  /* ANSI escape sequence to clear screen and move cursor to home */
  __ConsoleWriteString__((const Byte_t *) "\x1b[2J\x1b[H");

  __ReturnOk__();
  FUNCTION_EXIT;
}


/**
 * @brief Command: echo - Toggle echo mode or print message
 * @param args_ Command arguments
 * @return Return_t OK
 */
static Return_t __ConsoleCmdEcho__(const Byte_t *args_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(args_) && 0x00u != args_[0x0]) {
    /* Print the arguments */
    __ConsoleWriteString__(args_);
    __ConsoleWriteString__((const Byte_t *) "\r\n");
  } else {
    /* Toggle echo mode */
    consoleState.echoEnabled = !consoleState.echoEnabled;

    if(consoleState.echoEnabled) {
      __ConsoleWriteString__((const Byte_t *) "Echo enabled.\r\n");
    } else {
      __ConsoleWriteString__((const Byte_t *) "Echo disabled.\r\n");
    }
  }

  __ReturnOk__();
  FUNCTION_EXIT;
}


/**
 * @brief Command: ls - List directory contents
 * @param args_ Command arguments (directory path, optional)
 * @return Return_t OK or error
 */
static Return_t __ConsoleCmdLs__(const Byte_t *args_) {
  FUNCTION_ENTER;


  Dir_t *dir = null;
  DirEntry_t *entry = null;
  Byte_t path[CONFIG_FS_MAX_PATH_LENGTH];
  Byte_t numBuf[0x10];


  if(!__PointerIsNotNull__(mountedVolume)) {
    __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
    __ReturnError__();
    FUNCTION_EXIT;
  }

  /* Determine path */
  if(__PointerIsNotNull__(args_) && 0x00u != args_[0x0]) {
    __StringCopy__(path, args_);
  } else {
    __StringCopy__(path, consoleState.currentWorkingDirectory);
  }

  /* Open directory */
  if(OK(xDirOpen(&dir, mountedVolume, path))) {
    __ConsoleWriteString__((const Byte_t *) "Directory listing for: ");
    __ConsoleWriteString__(path);
    __ConsoleWriteString__((const Byte_t *) "\r\n");

    /* Read directory entries */
    while(OK(xDirRead(dir, &entry))) {
      if(__PointerIsNotNull__(entry)) {
        /* Print entry name */
        __ConsoleWriteString__((const Byte_t *) "  ");

        if(entry->isDirectory) {
          __ConsoleWriteString__((const Byte_t *) "[DIR]  ");
        } else {
          __ConsoleWriteString__((const Byte_t *) "[FILE] ");
        }

        __ConsoleWriteString__(entry->name);

        /* Print file size for files */
        if(!entry->isDirectory) {
          __ConsoleWriteString__((const Byte_t *) " (");
          __uitoah__((Word_t) entry->size, numBuf, sizeof(numBuf));
          __ConsoleWriteString__(numBuf);
          __ConsoleWriteString__((const Byte_t *) " bytes)");
        }

        __ConsoleWriteString__((const Byte_t *) "\r\n");

        /* Free entry */
        xMemFree(entry);
      }
    }

    /* Close directory */
    xDirClose(dir);
    __ReturnOk__();
  } else {
    __ConsoleWriteString__((const Byte_t *) "Error: Unable to open directory.\r\n");
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Command: cd - Change directory
 * @param args_ Command arguments (directory path)
 * @return Return_t OK or error
 */
static Return_t __ConsoleCmdCd__(const Byte_t *args_) {
  FUNCTION_ENTER;

  Base_t exists = false;
  Byte_t newPath[CONFIG_FS_MAX_PATH_LENGTH];

  if(__PointerIsNotNull__(mountedVolume)) {
    if(!__PointerIsNotNull__(args_) || 0x00u == args_[0x0]) {
      /* No argument - go to root */
      __StringCopy__(newPath, (const Byte_t *) "/");
    } else if(__StringCompare__(args_, (const Byte_t *) "..")) {
      /* Go up one directory */
      Word_t len = __StringLength__(consoleState.currentWorkingDirectory);
      Word_t i = len;

      /* Find last slash */
      while(i > 0x0u && 0x2Fu != consoleState.currentWorkingDirectory[i]) {
        i--;
      }

      if(0x0u == i) {
        __StringCopy__(newPath, (const Byte_t *) "/");
      } else {
        __memcpy__(newPath, consoleState.currentWorkingDirectory, i);
        newPath[i] = 0x00u;
      }
    } else if(0x2Fu == args_[0x0]) {
      /* Absolute path */
      __StringCopy__(newPath, args_);
    } else {
      /* Relative path */
      __StringCopy__(newPath, consoleState.currentWorkingDirectory);

      if(0x2Fu != newPath[__StringLength__(newPath) - 0x1u]) {
        Word_t len = __StringLength__(newPath);

        newPath[len] = 0x2Fu;
        newPath[len + 0x1u] = 0x00u;
      }

      __StringCopy__(newPath + __StringLength__(newPath), args_);
    }

    /* Verify directory exists */
    if(OK(xFileExists(mountedVolume, newPath, &exists)) && exists) {
      __StringCopy__(consoleState.currentWorkingDirectory, newPath);
      __ReturnOk__();
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: Directory not found.\r\n");
      __AssertOnElse__();
    }
  } else {
    __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Command: pwd - Print working directory
 * @return Return_t OK
 */
static Return_t __ConsoleCmdPwd__(const Byte_t *args_) {
  FUNCTION_ENTER;

  (void) args_;
  __ConsoleWriteString__(consoleState.currentWorkingDirectory);
  __ConsoleWriteString__((const Byte_t *) "\r\n");
  __ReturnOk__();

  FUNCTION_EXIT;
}


/**
 * @brief Command: cat - Display file contents
 * @param args_ Command arguments (file path)
 * @return Return_t OK or error
 */
static Return_t __ConsoleCmdCat__(const Byte_t *args_) {
  FUNCTION_ENTER;

  File_t *file = null;
  Byte_t *data = null;
  Word_t fileSize = 0x0u;
  Byte_t path[CONFIG_FS_MAX_PATH_LENGTH];

  if(__PointerIsNotNull__(mountedVolume) && __PointerIsNotNull__(args_) && (0x00u != args_[0x0])) {
    /* Build full path */
    if(0x2Fu == args_[0x0]) {
      __StringCopy__(path, args_);
    } else {
      __StringCopy__(path, consoleState.currentWorkingDirectory);

      if(0x2Fu != path[__StringLength__(path) - 0x1u]) {
        Word_t len = __StringLength__(path);

        path[len] = 0x2Fu;
        path[len + 0x1u] = 0x00u;
      }

      __StringCopy__(path + __StringLength__(path), args_);
    }

    /* Open file for reading */
    if(OK(xFileOpen(&file, mountedVolume, path, FS_MODE_READ))) {
      /* Get file size */
      if(OK(xFileGetSize(file, &fileSize))) {
        if(0x0u < fileSize) {
          /* Read entire file */
          if(OK(xFileRead(file, fileSize, &data))) {
            /* Display contents */
            Word_t i = 0x0u;
            Byte_t ch[0x2] = { 0x00u, 0x00u };

            for(i = 0x0u; i < fileSize; i++) {
              ch[0x0] = data[i];

              /* Convert LF to CRLF for terminal */
              if(0x0Au == ch[0x0]) {
                __ConsoleWriteString__((const Byte_t *) "\r\n");
              } else {
                __ConsoleWriteString__(ch);
              }
            }

            __ConsoleWriteString__((const Byte_t *) "\r\n");

            /* Free data buffer */
            xMemFree(data);
          }
        } else {
          __ConsoleWriteString__((const Byte_t *) "(empty file)\r\n");
        }
      }

      /* Close file */
      xFileClose(file);
      __ReturnOk__();
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: Unable to open file.\r\n");
      __AssertOnElse__();
    }
  } else {
    if(!__PointerIsNotNull__(mountedVolume)) {
      __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: No file specified.\r\n");
    }

    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Command: mv - Move/rename file
 * @param args_ Command arguments (source and destination paths)
 * @return Return_t OK or error
 */
static Return_t __ConsoleCmdMv__(const Byte_t *args_) {
  FUNCTION_ENTER;

  Byte_t oldPath[CONFIG_FS_MAX_PATH_LENGTH];
  Byte_t newPath[CONFIG_FS_MAX_PATH_LENGTH];
  const Byte_t *src = args_;
  const Byte_t *dst = null;
  Word_t i = 0x0u;

  if(__PointerIsNotNull__(mountedVolume) && __PointerIsNotNull__(args_) && (0x00u != args_[0x0])) {
    /* Find space separating source and destination */
    for(i = 0x0u; args_[i] != 0x00u; i++) {
      if(0x20u == args_[i]) {
        dst = &args_[i + 0x1u];
        __SkipWhitespace__(&dst);
        break;
      }
    }

    if(__PointerIsNotNull__(dst) && (0x00u != dst[0x0])) {
      /* Build source path */
      __memcpy__(oldPath, src, i);
      oldPath[i] = 0x00u;

      /* Build destination path */
      __StringCopy__(newPath, dst);

      /* Rename file */
      if(OK(xFileRename(mountedVolume, oldPath, newPath))) {
        __ReturnOk__();
      } else {
        __ConsoleWriteString__((const Byte_t *) "Error: Unable to rename/move file.\r\n");
        __AssertOnElse__();
      }
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: Usage: mv <source> <destination>\r\n");
      __AssertOnElse__();
    }
  } else {
    if(!__PointerIsNotNull__(mountedVolume)) {
      __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: Usage: mv <source> <destination>\r\n");
    }

    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Command: rm - Remove file
 * @param args_ Command arguments (file path)
 * @return Return_t OK or error
 */
static Return_t __ConsoleCmdRm__(const Byte_t *args_) {
  FUNCTION_ENTER;

  Byte_t path[CONFIG_FS_MAX_PATH_LENGTH];

  if(__PointerIsNotNull__(mountedVolume) && __PointerIsNotNull__(args_) && (0x00u != args_[0x0])) {
    /* Build full path */
    if(0x2Fu == args_[0x0]) {
      __StringCopy__(path, args_);
    } else {
      __StringCopy__(path, consoleState.currentWorkingDirectory);

      if(0x2Fu != path[__StringLength__(path) - 0x1u]) {
        Word_t len = __StringLength__(path);

        path[len] = 0x2Fu;
        path[len + 0x1u] = 0x00u;
      }

      __StringCopy__(path + __StringLength__(path), args_);
    }

    /* Remove file */
    if(OK(xFileUnlink(mountedVolume, path))) {
      __ReturnOk__();
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: Unable to remove file.\r\n");
      __AssertOnElse__();
    }
  } else {
    if(!__PointerIsNotNull__(mountedVolume)) {
      __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: No file specified.\r\n");
    }

    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Command: mkdir - Create directory
 * @param args_ Command arguments (directory path)
 * @return Return_t OK or error
 */
static Return_t __ConsoleCmdMkdir__(const Byte_t *args_) {
  FUNCTION_ENTER;

  Byte_t path[CONFIG_FS_MAX_PATH_LENGTH];

  if(__PointerIsNotNull__(mountedVolume) && __PointerIsNotNull__(args_) && (0x00u != args_[0x0])) {
    /* Build full path */
    if(0x2Fu == args_[0x0]) {
      __StringCopy__(path, args_);
    } else {
      __StringCopy__(path, consoleState.currentWorkingDirectory);

      if(0x2Fu != path[__StringLength__(path) - 0x1u]) {
        Word_t len = __StringLength__(path);

        path[len] = 0x2Fu;
        path[len + 0x1u] = 0x00u;
      }

      __StringCopy__(path + __StringLength__(path), args_);
    }

    /* Create directory */
    if(OK(xDirMake(mountedVolume, path))) {
      __ReturnOk__();
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: Unable to create directory.\r\n");
      __AssertOnElse__();
    }
  } else {
    if(!__PointerIsNotNull__(mountedVolume)) {
      __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: No directory specified.\r\n");
    }

    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Copy string from source to destination
 * @param dest_ Destination buffer
 * @param src_ Source string
 */
static void __StringCopy__(Byte_t *dest_, const Byte_t *src_) {
  Word_t i = 0x0u;


  while(0x00u != src_[i]) {
    dest_[i] = src_[i];
    i++;
  }

  dest_[i] = 0x00u;
}


/**
 * @brief Get length of string
 * @param str_ String
 * @return Word_t Length
 */
static Word_t __StringLength__(const Byte_t *str_) {
  Word_t len = 0x0u;


  if(__PointerIsNotNull__(str_)) {
    while(0x00u != str_[len]) {
      len++;
    }
  }

  return(len);
}


/**
 * @brief Compare two strings
 * @param s1_ First string
 * @param s2_ Second string
 * @return Base_t true if equal, false otherwise
 */
static Base_t __StringCompare__(const Byte_t *s1_, const Byte_t *s2_) {
  Word_t i = 0x0u;


  if(!__PointerIsNotNull__(s1_) || !__PointerIsNotNull__(s2_)) {
    return(false);
  }

  while(s1_[i] != 0x00u && s2_[i] != 0x00u) {
    if(s1_[i] != s2_[i]) {
      return(false);
    }

    i++;
  }

  return(s1_[i] == s2_[i]);
}


/**
 * @brief Skip whitespace in string
 * @param str_ Pointer to string pointer (will be updated)
 */
static void __SkipWhitespace__(const Byte_t **str_) {
  if(__PointerIsNotNull__(str_) && __PointerIsNotNull__(*str_)) {
    while(0x20u == **str_ || 0x09u == **str_) {
      (*str_)++;
    }
  }
}


/**
 * @brief Convert unsigned integer to hexadecimal string
 * @param value_ Value to convert
 * @param buffer_ Buffer to store result
 * @param bufferSize_ Size of buffer
 */
static void __uitoah__(Word_t value_, Byte_t *buffer_, Word_t bufferSize_) {
  const Byte_t *hexDigits = (const Byte_t *) "0123456789ABCDEF";
  Word_t i = 0x0u;
  Word_t temp = value_;


  if(!__PointerIsNotNull__(buffer_) || bufferSize_ < 0x3u) {
    return;
  }

  /* Add "0x" prefix */
  buffer_[i++] = 0x30u;  /* '0' */
  buffer_[i++] = 0x78u;  /* 'x' */

  /* Handle zero specially */
  if(0x0u == value_) {
    if(i < bufferSize_ - 0x1u) {
      buffer_[i++] = 0x30u;  /* '0' */
    }

    buffer_[i] = 0x00u;
    return;
  }

  /* Convert to hex digits (will be in reverse order initially) */
  {
    Word_t start = i;
    Word_t end = 0x0u;
    Byte_t tmpChar = 0x00u;


    while(temp > 0x0u && i < bufferSize_ - 0x1u) {
      buffer_[i++] = hexDigits[temp & 0xFu];
      temp >>= 0x4;
    }

    /* Reverse the hex digits */
    end = i - 0x1u;

    while(start < end) {
      tmpChar = buffer_[start];
      buffer_[start] = buffer_[end];
      buffer_[end] = tmpChar;
      start++;
      end--;
    }
  }

  buffer_[i] = 0x00u;
}


#if defined(POSIX_ARCH_OTHER)


/**
 * @brief Clear console state for testing
 */
void __ConsoleStateClear__(void) {
  consoleState.deviceReady = false;
  #if defined(CONFIG_CONSOLE_ECHO_ENABLED)
    consoleState.echoEnabled = true;
  #else /* if defined(CONFIG_CONSOLE_ECHO_ENABLED) */
    consoleState.echoEnabled = false;
  #endif /* if defined(CONFIG_CONSOLE_ECHO_ENABLED) */
  consoleState.bufferPosition = 0x0u;
  __memset__(consoleState.commandBuffer, 0x00u, CONFIG_CONSOLE_MAX_COMMAND_LENGTH);
  __StringCopy__(consoleState.currentWorkingDirectory, (const Byte_t *) "/");
  mountedVolume = null;
}


#endif /* if defined(POSIX_ARCH_OTHER) */
