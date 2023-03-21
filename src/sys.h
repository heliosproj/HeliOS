/*UNCRUSTIFY-OFF*/
/**
 * @file sys.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for system control
 * @version 0.4.1
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#ifndef SYS_H_
  #define SYS_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "stream.h"
  #include "task.h"
  #include "timer.h"

  extern Flags_t flags;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
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