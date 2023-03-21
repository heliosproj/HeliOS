/*UNCRUSTIFY-OFF*/
/**
 * @file device_harness.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
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
#ifndef DEVICE_HARNESS_H_
  #define DEVICE_HARNESS_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "stream.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"

  #include "unit.h"
  #include "loopback.h"

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  void device_harness(void);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef DEVICE_HARNESS_H_ */