/*UNCRUSTIFY-OFF*/
/**
 * @file test.h
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
#ifndef TEST_H_
  #define TEST_H_

  #include "unit.h"
  #include "memory_1_harness.h"
  #include "memory_2_harness.h"
  #include "queue_harness.h"
  #include "task_harness.h"
  #include "timer_harness.h"
  #include "stream_harness.h"
  #include "device_harness.h"

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  int main(int argc, char **argv);
  void reset(void);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef TEST_H_ */