/*UNCRUSTIFY-OFF*/
/**
 * @file console.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for console and shell support
 *
 * @copyright
 * (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "config.h"
#if defined(CONFIG_ENABLE_IO_SUBSYSTEM)
  #include "console.h"
  #include "fs.h"
  #include "../drivers/char/char_driver.h"


/* Character constants for better readability */
  #define CHAR_NULL 0x00u
  #define CHAR_TAB 0x09u
  #define CHAR_LF 0x0Au
  #define CHAR_CR 0x0Du
  #define CHAR_SPACE 0x20u
  #define CHAR_SLASH 0x2Fu
  #define CHAR_DOT 0x2Eu
  #define CHAR_ZERO 0x30u
  #define CHAR_LOWERCASE_X 0x78u
  #define CHAR_BACKSPACE 0x08u
  #define CHAR_DEL 0x7Fu
  #define CHAR_PRINTABLE_MIN 0x20u
  #define CHAR_PRINTABLE_MAX 0x7Eu


/* Console state */
  static ConsoleState_t consoleState;
  static Volume_t *mountedVolume = null;


/* Performance optimization: Cache for device lookup */
  static Device_t *cachedDevice = null;
  static HalfWord_t cachedDeviceUID = 0x0u;


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
  static void __SkipWhitespace__(const Byte_t **str_);
  static void __uitoah__(Word_t value_, Byte_t *buffer_, Word_t bufferSize_);


/* Command table structure */
  typedef struct ConsoleCommand_s {
    const Byte_t *name;



    Return_t (* handler)(const Byte_t *);



    const Byte_t *description;
  } ConsoleCommand_t;



/* Command table */
  static const ConsoleCommand_t commandTable[] = {{
                                                    (const Byte_t *) "help", __ConsoleCmdHelp__, (const Byte_t *) "Display available commands"
                                                  }, {
                                                    (const Byte_t *) "version", __ConsoleCmdVersion__, (const Byte_t *) "Display version information"
                                                  }, {
                                                    (const Byte_t *) "tasks", __ConsoleCmdTasks__, (const Byte_t *) "List running tasks"
                                                  }, {
                                                    (const Byte_t *) "mem", __ConsoleCmdMem__, (const Byte_t *) "Display memory statistics"
                                                  }, {
                                                    (const Byte_t *) "clear", __ConsoleCmdClear__, (const Byte_t *) "Clear the screen"
                                                  }, {
                                                    (const Byte_t *) "echo", __ConsoleCmdEcho__, (const Byte_t *) "Toggle echo mode or print message"
                                                  }, {
                                                    (const Byte_t *) "ls", __ConsoleCmdLs__, (const Byte_t *) "List directory contents"
                                                  }, {
                                                    (const Byte_t *) "cd", __ConsoleCmdCd__, (const Byte_t *) "Change directory"
                                                  }, {
                                                    (const Byte_t *) "pwd", __ConsoleCmdPwd__, (const Byte_t *) "Print working directory"
                                                  }, {
                                                    (const Byte_t *) "cat", __ConsoleCmdCat__, (const Byte_t *) "Display file contents"
                                                  }, {
                                                    (const Byte_t *) "mv", __ConsoleCmdMv__, (const Byte_t *) "Move/rename file"
                                                  }, {
                                                    (const Byte_t *) "rm", __ConsoleCmdRm__, (const Byte_t *) "Remove file"
                                                  }, {
                                                    (const Byte_t *) "mkdir", __ConsoleCmdMkdir__, (const Byte_t *) "Create directory"
                                                  }, {
                                                    null, null, null
                                                  }};


/* ============================================================================
 * String and Path Utility Functions (used only by console)
 * ============================================================================
 */
  Size_t __strlen__(const Byte_t *str_) {
    Size_t len = 0x0u;


    if(__PointerIsNotNull__(str_)) {
      while(CHAR_NULL != str_[len]) {
        len++;
      }
    }

    return (len);
  }


  Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_) {
    FUNCTION_ENTER;


    Size_t i = 0x0u;


    if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (0x0u < destSize_)) {
      while((CHAR_NULL != src_[i]) && (i < (destSize_ - 0x1u))) {
        dest_[i] = src_[i];
        i++;
      }

      dest_[i] = CHAR_NULL;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


  Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t n_) {
    FUNCTION_ENTER;


    Size_t i = 0x0u;


    if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (0x0u < n_)) {
      for(i = 0x0u; (i < n_) && (CHAR_NULL != src_[i]); i++) {
        dest_[i] = src_[i];
      }

      for(; i < n_; i++) {
        dest_[i] = CHAR_NULL;
      }

      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


  Base_t __strcmp__(const Byte_t *s1_, const Byte_t *s2_) {
    Size_t i = 0x0u;


    if(__PointerIsNull__(s1_) || __PointerIsNull__(s2_)) {
      return (0x0u);
    }

    while((CHAR_NULL != s1_[i]) && (CHAR_NULL != s2_[i])) {
      if(s1_[i] != s2_[i]) {
        return ((s1_[i] < s2_[i]) ? -0x1 : 0x1);
      }

      i++;
    }

    if(s1_[i] == s2_[i]) {
      return (0x0u);
    }

    return ((s1_[i] < s2_[i]) ? -0x1 : 0x1);
  }


  Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t n_) {
    Size_t i = 0x0u;


    if(__PointerIsNull__(s1_) || __PointerIsNull__(s2_) || (0x0u == n_)) {
      return (0x0u);
    }

    for(i = 0x0u; i < n_; i++) {
      if((CHAR_NULL == s1_[i]) || (s1_[i] != s2_[i])) {
        return ((s1_[i] < s2_[i]) ? (Base_t) -0x1 : ((s1_[i] > s2_[i]) ? (Base_t) 0x1 : (Base_t) 0x0u));
      }
    }

    return (0x0u);
  }


  Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_) {
    FUNCTION_ENTER;


    Size_t destLen = 0x0u;
    Size_t i = 0x0u;


    if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (0x0u < destSize_)) {
      destLen = __strlen__(dest_);

      if(destLen < destSize_) {
        while((CHAR_NULL != src_[i]) && ((destLen + i) < (destSize_ - 0x1u))) {
          dest_[destLen + i] = src_[i];
          i++;
        }

        dest_[destLen + i] = CHAR_NULL;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


  Byte_t * __strchr__(const Byte_t *str_, const Byte_t ch_) {
    Size_t i = 0x0u;


    if(__PointerIsNull__(str_)) {
      return (null);
    }

    while(CHAR_NULL != str_[i]) {
      if(str_[i] == ch_) {
        return ((Byte_t *) &str_[i]);
      }

      i++;
    }

    if(CHAR_NULL == ch_) {
      return ((Byte_t *) &str_[i]);
    }

    return (null);
  }


  Byte_t * __strrchr__(const Byte_t *str_, const Byte_t ch_) {
    Size_t len = 0x0u;
    Size_t i = 0x0u;


    if(__PointerIsNull__(str_)) {
      return (null);
    }

    len = __strlen__(str_);

    for(i = len; i > 0x0u; i--) {
      if(str_[i - 0x1u] == ch_) {
        return ((Byte_t *) &str_[i - 0x1u]);
      }
    }

    if((CHAR_NULL == ch_) && (len > 0x0u)) {
      return ((Byte_t *) &str_[len]);
    }

    return (null);
  }


  Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, const Size_t destSize_) {
    FUNCTION_ENTER;


    Size_t baseLen = 0x0u;
    Size_t pathLen = 0x0u;
    Base_t needSlash = false;


    if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(base_) && __PointerIsNotNull__(path_) && (0x0u != destSize_)) {
      baseLen = __strlen__(base_);
      pathLen = __strlen__(path_);

      if((0x0u != baseLen) && (0x0u != pathLen)) {
        if(CHAR_SLASH == path_[0x0u]) {
          if(pathLen < destSize_) {
            if(OK(__strcpy__(dest_, path_, destSize_))) {
              __ReturnOk__();
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          needSlash = (CHAR_SLASH != base_[baseLen - 0x1u]) && (CHAR_SLASH != path_[0x0u]);

          if((baseLen + pathLen + (needSlash ? 0x1u : 0x0u)) < destSize_) {
            if(OK(__strcpy__(dest_, base_, destSize_))) {
              if(needSlash) {
                dest_[baseLen] = CHAR_SLASH;
                dest_[baseLen + 0x1u] = CHAR_NULL;
              }

              if(OK(__strcat__(dest_, path_, destSize_))) {
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
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


  Return_t __path_normalize__(Byte_t *path_, const Size_t pathSize_) {
    FUNCTION_ENTER;


    Size_t i = 0x0u;
    Size_t j = 0x0u;
    Size_t len = 0x0u;
    Byte_t temp[CONFIG_FS_MAX_PATH_LENGTH];
    Byte_t segments[CONFIG_FS_MAX_PATH_LENGTH / 2][CONFIG_FS_MAX_PATH_LENGTH];
    Size_t segmentCount = 0x0u;
    Size_t k = 0x0u;
    Size_t segLen = 0x0u;
    Size_t segIdx = 0x0u;


    if(__PointerIsNotNull__(path_) && (0x0u != pathSize_)) {
      len = __strlen__(path_);

      if((0x0u != len) && (len < CONFIG_FS_MAX_PATH_LENGTH)) {
        for(i = 0x0u; i <= len; i++) {
          temp[i] = path_[i];
        }

        i = 0x0u;

        if(CHAR_SLASH == temp[0x0u]) {
          i = 0x1u;
        }

        segIdx = 0x0u;

        for(; i <= len; i++) {
          if((CHAR_SLASH == temp[i]) || (CHAR_NULL == temp[i])) {
            if(segIdx > 0x0u) {
              segments[segmentCount][segIdx] = CHAR_NULL;

              if((segments[segmentCount][0x0u] == '.') && (segments[segmentCount][0x1u] == '.') && (segments[segmentCount][0x2u] == CHAR_NULL)) {
                if(segmentCount > 0x0u) {
                  segmentCount--;
                }
              } else if(!((segments[segmentCount][0x0u] == '.') && (segments[segmentCount][0x1u] == CHAR_NULL))) {
                segmentCount++;
              }

              segIdx = 0x0u;
            }
          } else {
            segments[segmentCount][segIdx++] = temp[i];
          }
        }

        j = 0x0u;

        if(CHAR_SLASH == path_[0x0u]) {
          path_[j++] = CHAR_SLASH;
        }

        for(k = 0x0u; k < segmentCount; k++) {
          Size_t m;


          segLen = __strlen__(segments[k]);

          if(k > 0x0u) {
            path_[j++] = CHAR_SLASH;
          }

          for(m = 0x0u; m < segLen; m++) {
            path_[j++] = segments[k][m];
          }
        }

        if((0x0u == j) || ((0x1u == j) && (CHAR_SLASH == path_[0x0u]))) {
          path_[0x0u] = CHAR_SLASH;
          j = 0x1u;
        }

        path_[j] = CHAR_NULL;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


  Base_t __path_is_absolute__(const Byte_t *path_) {
    if(__PointerIsNull__(path_)) {
      return (false);
    }

    return ((CHAR_SLASH == path_[0x0u]) ? true : false);
  }


  Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_) {
    FUNCTION_ENTER;


    Size_t len = 0x0u;
    Size_t i = 0x0u;


    if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(path_) && (0x0u != destSize_)) {
      len = __strlen__(path_);

      if(0x0u == len) {
        if(OK(__strcpy__(dest_, (const Byte_t *) ".", destSize_))) {
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        for(i = len; i > 0x0u; i--) {
          if(CHAR_SLASH == path_[i - 0x1u]) {
            break;
          }
        }

        if(0x0u == i) {
          if(OK(__strcpy__(dest_, (const Byte_t *) ".", destSize_))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {
          if(i <= destSize_) {
            if(OK(__strncpy__(dest_, path_, i - 0x1u))) {
              dest_[i - 0x1u] = CHAR_NULL;
              __ReturnOk__();
            } else {
              __AssertOnElse__();
            }
          } else {
            __AssertOnElse__();
          }
        }
      }
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


  Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_) {
    FUNCTION_ENTER;


    Size_t len = 0x0u;
    Size_t i = 0x0u;
    Size_t start = 0x0u;


    if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(path_) && (0x0u != destSize_)) {
      len = __strlen__(path_);

      if(0x0u == len) {
        if(OK(__strcpy__(dest_, (const Byte_t *) ".", destSize_))) {
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        for(i = len; i > 0x0u; i--) {
          if(CHAR_SLASH == path_[i - 0x1u]) {
            start = i;
            break;
          }
        }

        if((len - start) < destSize_) {
          if(OK(__strcpy__(dest_, &path_[start], destSize_))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      }
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


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
    consoleState.bufferPosition = 0x0u;
    __memset__(consoleState.commandBuffer, CHAR_NULL, CONFIG_CONSOLE_MAX_COMMAND_LENGTH);
    __strcpy__(consoleState.currentWorkingDirectory, (const Byte_t *) "/", CONFIG_FS_MAX_PATH_LENGTH);
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


    (void) task_; /* Unused parameter */
    (void) parm_; /* Unused parameter */

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
        consoleState.bufferPosition = 0x0u;


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
      if((CHAR_BACKSPACE == ch) || (CHAR_DEL == ch)) {
        /* Backspace or DEL */
        __ConsoleHandleBackspace__();
      } else if((CHAR_CR == ch) || (CHAR_LF == ch)) {
        /* Carriage return or line feed */
        __ConsoleWriteString__((const Byte_t *) "\r\n");


        /* Process command if buffer is not empty */
        if(consoleState.bufferPosition > 0x0u) {
          consoleState.commandBuffer[consoleState.bufferPosition] = CHAR_NULL;
          __ConsoleProcessCommand__();
          consoleState.bufferPosition = 0x0u;
        }

        __ConsolePrintPrompt__();
      } else if((ch >= CHAR_PRINTABLE_MIN) && (ch <= CHAR_PRINTABLE_MAX)) {
        /* Printable character */
        if(consoleState.bufferPosition < (CONFIG_CONSOLE_MAX_COMMAND_LENGTH - 0x1u)) {
          consoleState.commandBuffer[consoleState.bufferPosition++] = ch;


          /* Echo character if enabled */
          if(consoleState.echoEnabled) {
            Byte_t echoChar[0x2];


            echoChar[0x0u] = ch;
            echoChar[0x1] = CHAR_NULL;
            __ConsoleWriteString__(echoChar);
          }
        }
      }
    }
  }


/**
 * @brief Check if character device is ready with caching
 * @return Return_t OK if ready, error otherwise
 */
  static Return_t __ConsoleCheckDevice__(void) {
    FUNCTION_ENTER;


    Base_t needLookup = true;


    /* Performance optimization: Use cached device if UID matches */
    if(__PointerIsNotNull__(cachedDevice) && (CONFIG_CHAR_DEVICE_UID == cachedDeviceUID)) {
      if(DeviceStateRunning == cachedDevice->state) {
        needLookup = false;
      } else {
        /* Device state changed, invalidate cache */
        cachedDevice = null;
        cachedDeviceUID = 0x0u;
      }
    }

    if(needLookup) {
      /* Cache miss or invalid - look up device */
      if(OK(__DeviceListFind__(CONFIG_CHAR_DEVICE_UID, &cachedDevice))) {
        if(__PointerIsNotNull__(cachedDevice) && (DeviceStateRunning == cachedDevice->state)) {
          /* Update cache */
          cachedDeviceUID = CONFIG_CHAR_DEVICE_UID;
        } else {
          cachedDevice = null;
          cachedDeviceUID = 0x0u;
          __AssertOnElse__();
        }
      } else {
        cachedDevice = null;
        cachedDeviceUID = 0x0u;
        __AssertOnElse__();
      }
    }

    if(!needLookup || (__PointerIsNotNull__(cachedDevice) && (DeviceStateRunning == cachedDevice->state))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


/**
 * @brief Write string to console with cached device
 * @param  str_ String to write
 * @return      Return_t OK or error
 */
  static Return_t __ConsoleWriteString__(const Byte_t *str_) {
    FUNCTION_ENTER;


    Word_t len = 0x0u;
    Size_t size = 0x0u;
    Device_t *device = null;
    CharDeviceCommand_t cmd;
    Base_t success = false;


    if(__PointerIsNotNull__(str_)) {
      len = __strlen__(str_);

      if(0x0u < len) {
        /* Performance optimization: Try cached device first */
        if(__PointerIsNotNull__(cachedDevice) && (CONFIG_CHAR_DEVICE_UID == cachedDeviceUID)) {
          device = cachedDevice;
        } else {
          /* Cache miss - look up device */
          if(OK(__DeviceListFind__(CONFIG_CHAR_DEVICE_UID, &device))) {
            /* Update cache */
            cachedDevice = device;
            cachedDeviceUID = CONFIG_CHAR_DEVICE_UID;
          } else {
            device = null;
          }
        }

        if(__PointerIsNotNull__(device)) {
          /* Step 1: Configure byte count for write operation */
          cmd.command = CHAR_CMD_SET_PARAMS;
          cmd.byteCount = (HalfWord_t) len;


          /* Performance: Use interrupt mode if available, else blocking */
          cmd.transferMode = CHAR_IO_MODE_INTERRUPT;
          size = sizeof(CharDeviceCommand_t);

          if(OK((*device->config)(device, &size, (Addr_t *) &cmd))) {
            /* Step 2: Perform the write operation */
            size = len;

            if(OK((*device->write)(device, &size, (Addr_t *) str_))) {
              success = true;
            } else {
              /* Fall back to blocking mode if interrupt fails */
              cmd.transferMode = CHAR_IO_MODE_BLOCKING;
              size = sizeof(CharDeviceCommand_t);

              if(OK((*device->config)(device, &size, (Addr_t *) &cmd))) {
                size = len;

                if(OK((*device->write)(device, &size, (Addr_t *) str_))) {
                  success = true;
                } else {
                  __AssertOnElse__();
                }
              } else {
                __AssertOnElse__();
              }
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

    if(success) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


/**
 * @brief Read single character from console with cached device
 * @param  ch_ Pointer to store character
 * @return     Return_t OK or error
 */
  static Return_t __ConsoleReadChar__(Byte_t *ch_) {
    FUNCTION_ENTER;


    Size_t size = 0x1u;
    Addr_t *readData = null;
    Device_t *device = null;
    CharDeviceCommand_t cmd;
    Base_t success = false;


    if(__PointerIsNotNull__(ch_)) {
      /* Performance optimization: Try cached device first */
      if(__PointerIsNotNull__(cachedDevice) && (CONFIG_CHAR_DEVICE_UID == cachedDeviceUID)) {
        device = cachedDevice;
      } else {
        /* Cache miss - look up device */
        if(OK(__DeviceListFind__(CONFIG_CHAR_DEVICE_UID, &device))) {
          /* Update cache */
          cachedDevice = device;
          cachedDeviceUID = CONFIG_CHAR_DEVICE_UID;
        } else {
          device = null;
        }
      }

      if(__PointerIsNotNull__(device)) {
        /* Step 1: Configure byte count for read operation */
        cmd.command = CHAR_CMD_SET_PARAMS;
        cmd.byteCount = 0x1u;


        /* Use blocking mode for reliable operation */
        cmd.transferMode = CHAR_IO_MODE_BLOCKING;
        size = sizeof(CharDeviceCommand_t);

        if(OK((*device->config)(device, &size, (Addr_t *) &cmd))) {
          /* Step 2: Perform the read operation */
          size = 0x1u;

          if(OK((*device->read)(device, &size, &readData))) {
            if(__PointerIsNotNull__(readData) && (0x0u < size)) {
              *ch_ = *((Byte_t *) readData);
              __KernelFreeMemory__(readData);
              success = true;
            } else {
              if(__PointerIsNotNull__(readData)) {
                __KernelFreeMemory__(readData);
              }


              /* No data available - normal condition, not an error */
            }
          } else {
            /* Read failed - could be no data or device issue */
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

    if(success) {
      __ReturnOk__();
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
      /* Return error by default */
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
    Base_t commandFound = false;
    Base_t success = false;


    /* Skip leading whitespace */
    __SkipWhitespace__((const Byte_t **) &cmdName);


    /* Find space to separate command from arguments */
    for(i = 0x0u; cmdName[i] != CHAR_NULL; i++) {
      if(CHAR_SPACE == cmdName[i]) {
        cmdName[i] = CHAR_NULL;
        cmdArgs = &cmdName[i + 0x1u];
        __SkipWhitespace__((const Byte_t **) &cmdArgs);
        break;
      }
    }


    /* Empty command - return success */
    if(CHAR_NULL == cmdName[0x0u]) {
      success = true;
    } else {
      /* Search command table */
      for(i = 0x0u; __PointerIsNotNull__(commandTable[i].name) && !commandFound; i++) {
        if(0 == __strcmp__(cmdName, commandTable[i].name)) {
          if(__PointerIsNotNull__(commandTable[i].handler)) {
            if(OK(commandTable[i].handler((const Byte_t *) cmdArgs))) {
              success = true;
            } else {
              __AssertOnElse__();
            }

            commandFound = true;
          }
        }
      }

      if(!commandFound) {
        /* Unknown command */
        __ConsoleWriteString__((const Byte_t *) "Unknown command: ");
        __ConsoleWriteString__(cmdName);
        __ConsoleWriteString__((const Byte_t *) "\r\nType 'help' for available commands.\r\n");
        __AssertOnElse__();
      }
    }

    if(success) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

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
    __ConsoleWriteString__((const Byte_t *) "Version: " OS_VERSION_STRING "\r\n");
    __ConsoleWriteString__((const Byte_t *) "(C) 2020-2026 Manny Peterson <manny@heliosproj.org>\r\n");
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
        case TaskStateSuspended: __ConsoleWriteString__((const Byte_t *) "Suspended  ");
          break;
        case TaskStateRunning: __ConsoleWriteString__((const Byte_t *) "Running    ");
          break;
        case TaskStateWaiting: __ConsoleWriteString__((const Byte_t *) "Waiting    ");
          break;
        default: __ConsoleWriteString__((const Byte_t *) "Unknown    ");
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


      /* Return error by default */
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


      /* Return error by default */
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
 * @param  args_ Command arguments
 * @return       Return_t OK
 */
  static Return_t __ConsoleCmdEcho__(const Byte_t *args_) {
    FUNCTION_ENTER;

    if(__PointerIsNotNull__(args_) && (CHAR_NULL != args_[0x0u])) {
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
 * @param  args_ Command arguments (directory path, optional)
 * @return       Return_t OK or error
 */
  static Return_t __ConsoleCmdLs__(const Byte_t *args_) {
    FUNCTION_ENTER;


    Dir_t *dir = null;
    DirEntry_t *entry = null;
    Byte_t path[CONFIG_FS_MAX_PATH_LENGTH];
    Byte_t numBuf[0x10];
    Base_t success = false;


    if(__PointerIsNotNull__(mountedVolume)) {
      /* Determine path */
      if(__PointerIsNotNull__(args_) && (CHAR_NULL != args_[0x0u])) {
        __strcpy__(path, args_, CONFIG_FS_MAX_PATH_LENGTH);
      } else {
        __strcpy__(path, consoleState.currentWorkingDirectory, CONFIG_FS_MAX_PATH_LENGTH);
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
        success = true;
      } else {
        __ConsoleWriteString__((const Byte_t *) "Error: Unable to open directory.\r\n");
        __AssertOnElse__();
      }
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
      __AssertOnElse__();
    }

    if(success) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


/**
 * @brief Command: cd - Change directory
 * @param  args_ Command arguments (directory path)
 * @return       Return_t OK or error
 */
  static Return_t __ConsoleCmdCd__(const Byte_t *args_) {
    FUNCTION_ENTER;


    Base_t exists = false;
    Byte_t newPath[CONFIG_FS_MAX_PATH_LENGTH];
    Base_t success = false;
    Base_t pathBuilt = false;


    if(__PointerIsNotNull__(mountedVolume)) {
      if(!__PointerIsNotNull__(args_) || (CHAR_NULL == args_[0x0u])) {
        /* No argument - go to root */
        __strcpy__(newPath, (const Byte_t *) "/", CONFIG_FS_MAX_PATH_LENGTH);
        pathBuilt = true;
      } else if(0 == __strcmp__(args_, (const Byte_t *) "..")) {
        /* Go up one directory - use path utility */
        if(OK(__path_dirname__(newPath, consoleState.currentWorkingDirectory, CONFIG_FS_MAX_PATH_LENGTH))) {
          pathBuilt = true;
        } else {
          __strcpy__(newPath, (const Byte_t *) "/", CONFIG_FS_MAX_PATH_LENGTH);
          pathBuilt = true;
        }
      } else if(__path_is_absolute__(args_)) {
        /* Absolute path */
        __strcpy__(newPath, args_, CONFIG_FS_MAX_PATH_LENGTH);
        pathBuilt = true;
      } else {
        /* Relative path - use path join */
        if(OK(__path_join__(newPath, consoleState.currentWorkingDirectory, args_, CONFIG_FS_MAX_PATH_LENGTH))) {
          pathBuilt = true;
        } else {
          __ConsoleWriteString__((const Byte_t *) "Error: Path too long.\r\n");
          __AssertOnElse__();
        }
      }

      if(pathBuilt) {
        /* Normalize the path to remove . and .. references */
        if(OK(__path_normalize__(newPath, CONFIG_FS_MAX_PATH_LENGTH))) {
          /* Verify directory exists */
          if(OK(xFileExists(mountedVolume, newPath, &exists)) && exists) {
            __strcpy__(consoleState.currentWorkingDirectory, newPath, CONFIG_FS_MAX_PATH_LENGTH);
            success = true;
          } else {
            __ConsoleWriteString__((const Byte_t *) "Error: Directory not found.\r\n");
            __AssertOnElse__();
          }
        } else {
          __ConsoleWriteString__((const Byte_t *) "Error: Invalid path.\r\n");
          __AssertOnElse__();
        }
      }
    } else {
      __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
      __AssertOnElse__();
    }

    if(success) {
      __ReturnOk__();
    } else {
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


/* Magic number for cat command buffer size */
  #define CAT_BUFFER_SIZE 0x100u /* 256 bytes */


/**
 * @brief Command: cat - Display file contents with buffered reading
 * @param  args_ Command arguments (file path)
 * @return       Return_t OK or error
 */
  static Return_t __ConsoleCmdCat__(const Byte_t *args_) {
    FUNCTION_ENTER;


    File_t *file = null;
    Byte_t *buffer = null;
    Word_t bytesToRead = 0x0u;
    Word_t fileSize = 0x0u;
    Word_t totalRead = 0x0u;
    Byte_t path[CONFIG_FS_MAX_PATH_LENGTH];
    Base_t success = false;
    Base_t pathBuilt = false;
    Base_t fileOpened = false;


    if(__PointerIsNotNull__(mountedVolume) && __PointerIsNotNull__(args_) && (0x00u != args_[0x0u])) {
      /* Use path utility to build full path */
      if(__path_is_absolute__(args_)) {
        __strcpy__(path, args_, CONFIG_FS_MAX_PATH_LENGTH);
        pathBuilt = true;
      } else {
        if(OK(__path_join__(path, consoleState.currentWorkingDirectory, args_, CONFIG_FS_MAX_PATH_LENGTH))) {
          pathBuilt = true;
        } else {
          __ConsoleWriteString__((const Byte_t *) "Error: Path too long.\r\n");
          __AssertOnElse__();
        }
      }

      if(pathBuilt) {
        /* Open file for reading */
        if(OK(xFileOpen(&file, mountedVolume, path, FS_MODE_READ))) {
          fileOpened = true;

          /* Get file size for progress tracking */
          if(OK(xFileGetSize(file, &fileSize))) {
            if(0x0u < fileSize) {
              /* Allocate buffer for chunked reading */
              if(OK(xMemAlloc((volatile Addr_t **) &buffer, CAT_BUFFER_SIZE))) {
                Base_t readError = false;


                /* Read file in chunks */
                while((totalRead < fileSize) && !readError) {
                  /* Calculate bytes to read in this chunk */
                  bytesToRead = (fileSize - totalRead) > CAT_BUFFER_SIZE ? CAT_BUFFER_SIZE : (fileSize - totalRead);


                  /* Read chunk from file */
                  if(OK(xFileRead(file, bytesToRead, &buffer))) {
                    /* Display chunk contents */
                    Word_t i = 0x0u;
                    Byte_t ch[0x2] = {
                      0x00u, 0x00u
                    };


                    for(i = 0x0u; i < bytesToRead; i++) {
                      ch[0x0u] = buffer[i];


                      /* Convert LF to CRLF for terminal */
                      if(CHAR_LF == ch[0x0u]) {
                        __ConsoleWriteString__((const Byte_t *) "\r\n");
                      } else {
                        __ConsoleWriteString__(ch);
                      }
                    }

                    totalRead += bytesToRead;
                  } else {
                    __ConsoleWriteString__((const Byte_t *) "Error: Failed to read file.\r\n");
                    readError = true;
                    __AssertOnElse__();
                  }
                }

                if(!readError) {
                  /* Ensure final newline */
                  if(buffer[bytesToRead - 0x1u] != CHAR_LF) {
                    __ConsoleWriteString__((const Byte_t *) "\r\n");
                  }

                  success = true;
                }


                /* Free buffer */
                xMemFree(buffer);
              } else {
                __ConsoleWriteString__((const Byte_t *) "Error: Unable to allocate buffer.\r\n");
                __AssertOnElse__();
              }
            } else {
              __ConsoleWriteString__((const Byte_t *) "(empty file)\r\n");
              success = true;
            }
          } else {
            __ConsoleWriteString__((const Byte_t *) "Error: Unable to get file size.\r\n");
            __AssertOnElse__();
          }
        } else {
          __ConsoleWriteString__((const Byte_t *) "Error: Unable to open file.\r\n");
          __AssertOnElse__();
        }
      }
    } else {
      if(!__PointerIsNotNull__(mountedVolume)) {
        __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
      } else {
        __ConsoleWriteString__((const Byte_t *) "Error: No file specified.\r\n");
      }

      __AssertOnElse__();
    }

    if(fileOpened) {
      xFileClose(file);
    }

    if(success) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


/**
 * @brief Command: mv - Move/rename file
 * @param  args_ Command arguments (source and destination paths)
 * @return       Return_t OK or error
 */
  static Return_t __ConsoleCmdMv__(const Byte_t *args_) {
    FUNCTION_ENTER;


    Byte_t oldPath[CONFIG_FS_MAX_PATH_LENGTH];
    Byte_t newPath[CONFIG_FS_MAX_PATH_LENGTH];
    const Byte_t *src = args_;
    const Byte_t *dst = null;
    Word_t i = 0x0u;
    Base_t success = false;


    if(__PointerIsNotNull__(mountedVolume) && __PointerIsNotNull__(args_) && (CHAR_NULL != args_[0x0u])) {
      /* Find space separating source and destination */
      for(i = 0x0u; args_[i] != CHAR_NULL; i++) {
        if(CHAR_SPACE == args_[i]) {
          dst = &args_[i + 0x1u];
          __SkipWhitespace__(&dst);
          break;
        }
      }

      if(__PointerIsNotNull__(dst) && (CHAR_NULL != dst[0x0u])) {
        /* Build source path */
        __memcpy__(oldPath, src, i);
        oldPath[i] = CHAR_NULL;


        /* Build destination path */
        __strcpy__(newPath, dst, CONFIG_FS_MAX_PATH_LENGTH);


        /* Rename file */
        if(OK(xFileRename(mountedVolume, oldPath, newPath))) {
          success = true;
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

    if(success) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


/**
 * @brief Command: rm - Remove file
 * @param  args_ Command arguments (file path)
 * @return       Return_t OK or error
 */
  static Return_t __ConsoleCmdRm__(const Byte_t *args_) {
    FUNCTION_ENTER;


    Byte_t path[CONFIG_FS_MAX_PATH_LENGTH];
    Base_t success = false;
    Base_t pathBuilt = false;


    if(__PointerIsNotNull__(mountedVolume) && __PointerIsNotNull__(args_) && (CHAR_NULL != args_[0x0u])) {
      /* Build full path using path utilities */
      if(__path_is_absolute__(args_)) {
        __strcpy__(path, args_, CONFIG_FS_MAX_PATH_LENGTH);
        pathBuilt = true;
      } else {
        if(OK(__path_join__(path, consoleState.currentWorkingDirectory, args_, CONFIG_FS_MAX_PATH_LENGTH))) {
          pathBuilt = true;
        } else {
          __ConsoleWriteString__((const Byte_t *) "Error: Path too long.\r\n");
          __AssertOnElse__();
        }
      }

      if(pathBuilt) {
        /* Remove file */
        if(OK(xFileUnlink(mountedVolume, path))) {
          success = true;
        } else {
          __ConsoleWriteString__((const Byte_t *) "Error: Unable to remove file.\r\n");
          __AssertOnElse__();
        }
      }
    } else {
      if(!__PointerIsNotNull__(mountedVolume)) {
        __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
      } else {
        __ConsoleWriteString__((const Byte_t *) "Error: No file specified.\r\n");
      }

      __AssertOnElse__();
    }

    if(success) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


/**
 * @brief Command: mkdir - Create directory
 * @param  args_ Command arguments (directory path)
 * @return       Return_t OK or error
 */
  static Return_t __ConsoleCmdMkdir__(const Byte_t *args_) {
    FUNCTION_ENTER;


    Byte_t path[CONFIG_FS_MAX_PATH_LENGTH];
    Base_t success = false;
    Base_t pathBuilt = false;


    if(__PointerIsNotNull__(mountedVolume) && __PointerIsNotNull__(args_) && (CHAR_NULL != args_[0x0u])) {
      /* Build full path using path utilities */
      if(__path_is_absolute__(args_)) {
        __strcpy__(path, args_, CONFIG_FS_MAX_PATH_LENGTH);
        pathBuilt = true;
      } else {
        if(OK(__path_join__(path, consoleState.currentWorkingDirectory, args_, CONFIG_FS_MAX_PATH_LENGTH))) {
          pathBuilt = true;
        } else {
          __ConsoleWriteString__((const Byte_t *) "Error: Path too long.\r\n");
          __AssertOnElse__();
        }
      }

      if(pathBuilt) {
        /* Create directory */
        if(OK(xDirMake(mountedVolume, path))) {
          success = true;
        } else {
          __ConsoleWriteString__((const Byte_t *) "Error: Unable to create directory.\r\n");
          __AssertOnElse__();
        }
      }
    } else {
      if(!__PointerIsNotNull__(mountedVolume)) {
        __ConsoleWriteString__((const Byte_t *) "Error: No filesystem mounted.\r\n");
      } else {
        __ConsoleWriteString__((const Byte_t *) "Error: No directory specified.\r\n");
      }

      __AssertOnElse__();
    }

    if(success) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }


/**
 * @brief Skip whitespace in string
 * @param str_ Pointer to string pointer (will be updated)
 */
  static void __SkipWhitespace__(const Byte_t **str_) {
    if(__PointerIsNotNull__(str_) && __PointerIsNotNull__(*str_)) {
      while(CHAR_SPACE == **str_ || CHAR_TAB == **str_) {
        (*str_)++;
      }
    }
  }


/**
 * @brief Convert unsigned integer to hexadecimal string
 * @param value_      Value to convert
 * @param buffer_     Buffer to store result
 * @param bufferSize_ Size of buffer
 */
  static void __uitoah__(Word_t value_, Byte_t *buffer_, Word_t bufferSize_) {
    const Byte_t *hexDigits = (const Byte_t *) "0123456789ABCDEF";
    Word_t i = 0x0u;
    Word_t temp = value_;


    if(!__PointerIsNotNull__(buffer_) || (bufferSize_ < 0x3u)) {
      return;
    }


    /* Add "0x" prefix */
    buffer_[i++] = CHAR_ZERO; /* '0' */
    buffer_[i++] = CHAR_LOWERCASE_X; /* 'x' */

    /* Handle zero specially */
    if(0x0u == value_) {
      if(i < bufferSize_ - 0x1u) {
        buffer_[i++] = CHAR_ZERO; /* '0' */
      }

      buffer_[i] = CHAR_NULL;

      return;
    }


    /* Convert to hex digits (will be in reverse order initially) */
    {
      Word_t start = i;
      Word_t end = 0x0u;
      Byte_t tmpChar = CHAR_NULL;


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
    } buffer_[i] = CHAR_NULL;
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
      __memset__(consoleState.commandBuffer, CHAR_NULL, CONFIG_CONSOLE_MAX_COMMAND_LENGTH);
      __strcpy__(consoleState.currentWorkingDirectory, (const Byte_t *) "/", CONFIG_FS_MAX_PATH_LENGTH);
      mountedVolume = null;
    }


  #endif /* if defined(POSIX_ARCH_OTHER) */
#endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */