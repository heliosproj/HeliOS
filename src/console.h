/*UNCRUSTIFY-OFF*/
/**
 * @file console.h
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
#ifndef CONSOLE_H_
  #define CONSOLE_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"
  #include "fs.h"


  /* Console banner displayed on connect */
  #define CONSOLE_BANNER \
          "\r\n" \
          "   _    _      _ _  ____   _____ \r\n" \
          "  | |  | |    | (_)/ __ \\ / ____|\r\n" \
          "  | |__| | ___| |_| |  | | (___  \r\n" \
          "  |  __  |/ _ \\ | | |  | |\\___ \\ \r\n" \
          "  | |  | |  __/ | | |__| |____) |\r\n" \
          "  |_|  |_|\\___|_|_|\\____/|_____/ \r\n" \
          "\r\n" \
          "  HeliOS Embedded Operating System\r\n" \
          "  Version 0.5.0\r\n" \
          "  Copyright (C) 2020-2026 HeliOS Project\r\n" \
          "  Licensed under GPL-2.0-or-later\r\n" \
          "\r\n"


  /* Console return codes */
  #if defined(CONSOLE_OK)
    #undef CONSOLE_OK
  #endif /* if defined(CONSOLE_OK) */
  #define CONSOLE_OK 0x00u

  #if defined(CONSOLE_ERROR)
    #undef CONSOLE_ERROR
  #endif /* if defined(CONSOLE_ERROR) */
  #define CONSOLE_ERROR 0x01u

  #if defined(CONSOLE_NOT_READY)
    #undef CONSOLE_NOT_READY
  #endif /* if defined(CONSOLE_NOT_READY) */
  #define CONSOLE_NOT_READY 0x02u


  /* Console state */
  typedef struct ConsoleState_s {
    Base_t deviceReady;
    Base_t echoEnabled;
    Byte_t commandBuffer[CONFIG_CONSOLE_MAX_COMMAND_LENGTH];
    HalfWord_t bufferPosition;
    Byte_t currentWorkingDirectory[CONFIG_FS_MAX_PATH_LENGTH];
  } ConsoleState_t;


  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  /* Console Initialization */
  Return_t xConsoleInit(void);


  /* Console Task Entry Point */
  void vConsoleTask(Task_t *task_, TaskParm_t *parm_);

  #if defined(POSIX_ARCH_OTHER)
    void __ConsoleStateClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */


  /* String and path utility functions */
  Size_t __strlen__(const Byte_t *str_);
  Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
  Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t n_);
  Base_t __strcmp__(const Byte_t *s1_, const Byte_t *s2_);
  Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t n_);
  Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
  Byte_t * __strchr__(const Byte_t *str_, const Byte_t ch_);
  Byte_t * __strrchr__(const Byte_t *str_, const Byte_t ch_);
  Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, const Size_t destSize_);
  Return_t __path_normalize__(Byte_t *path_, const Size_t pathSize_);
  Base_t __path_is_absolute__(const Byte_t *path_);
  Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);
  Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef CONSOLE_H_ */