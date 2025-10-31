/*UNCRUSTIFY-OFF*/
/**
 * @file sys.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief System initialization and configuration API header
 * @details
 * Defines system flags, version information, and function prototypes for kernel initialization and feature detection.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef SYS_H_

  #define SYS_H_

  #include "config.h"

  #include "defines.h"

  #include "types.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "console.h"

    #include "device.h"

    #include "fat.h"

    #include "fs.h"

  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

  #include "mem.h"

  #include "port.h"

  #include "posix.h"

  #include "queue.h"

  #include "streams.h"

  #include "task.h"

  #include "timer.h"

  #if defined(OS_PRODUCT_NAME)

    #undef OS_PRODUCT_NAME

  #endif /* if defined(OS_PRODUCT_NAME) */

  #define OS_PRODUCT_NAME "HeliOS"

  #if defined(RUNNING)

    #undef RUNNING

  #endif /* if defined(RUNNING) */

  #define RUNNING flag.running

  #if defined(OVERFLOW)

    #undef OVERFLOW

  #endif /* if defined(OVERFLOW) */

  #define OVERFLOW flag.overflow

  #if defined(MEMFAULT)

    #undef MEMFAULT

  #endif /* if defined(MEMFAULT) */

  #define MEMFAULT flag.memfault

  #if defined(LITTLEEND)

    #undef LITTLEEND

  #endif /* if defined(LITTLEEND) */

  #define LITTLEEND flag.littleend

  #if defined(__SetFlag__)

    #undef __SetFlag__

  #endif /* if defined(__SetFlag__) */

  #define __SetFlag__(flag_) flag_ = 0xFFu

  #if defined(__UnsetFlag__)

    #undef __UnsetFlag__

  #endif /* if defined(__UnsetFlag__) */

  #define __UnsetFlag__(flag_) flag_ = 0x00u

  #if defined(__FlagIsSet__)

    #undef __FlagIsSet__

  #endif /* if defined(__FlagIsSet__) */

  #define __FlagIsSet__(flag_) (0xFFu == (flag_))

  #if defined(__FlagIsNotSet__)

    #undef __FlagIsNotSet__

  #endif /* if defined(__FlagIsNotSet__) */

  #define __FlagIsNotSet__(flag_) (0x0u == (flag_))

  #ifdef __cplusplus

    extern "C" {

  #endif /* ifdef __cplusplus */

  extern Flags_t flag;
  Return_t xSystemAssert(const char *file_, const int line_);
  Return_t xSystemInit(void);
  Return_t xSystemHalt(void);
  Return_t xSystemGetSystemInfo(SystemInfo_t **info_);

  #if defined(CONFIG_ENABLE_ARDUINO_CPP_INTERFACE)
    void __ArduinoAssert__(const char *file_, int line_);

  #endif /* if defined(CONFIG_ENABLE_ARDUINO_CPP_INTERFACE) */

  #if defined(POSIX_ARCH_OTHER)
    void __SysStateClear__(void);

  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus

    }

  #endif /* ifdef __cplusplus */

#endif /* ifndef SYS_H_ */