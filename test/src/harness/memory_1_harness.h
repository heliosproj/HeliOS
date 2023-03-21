/*UNCRUSTIFY-OFF*/
/**
 * @file memory_1_harness.h
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
#ifndef MEMORY_1_HARNESS_H_
  #define MEMORY_1_HARNESS_H_

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

  typedef struct MemoryTest_s {
    Size_t size;
    Size_t blocks;
    void *ptr;
  } MemoryTest_t;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  void memory_1_harness(void);
  void memory_1_harness_task(Task_t *task_, TaskParm_t *parm_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef MEMORY_1_HARNESS_H_ */