/*UNCRUSTIFY-OFF*/
/**
 * @file test.c
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
#include "test.h"


int main(int argc, char **argv) {
  unit_init();
  reset();
  memory_1_harness();
  reset();
  memory_2_harness();
  reset();
  queue_harness();
  reset();
  timer_harness();
  reset();
  task_harness();
  reset();
  stream_harness();
  reset();
  device_harness();
  unit_exit();

  return(0);
}


void reset(void) {
  __MemoryClear__();
  __SysStateClear__();
  __TaskStateClear__();
  __DeviceStateClear__();

  return;
}