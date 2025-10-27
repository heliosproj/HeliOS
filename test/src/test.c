/*UNCRUSTIFY-OFF*/
/**
 * @file test.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#include "test.h"
#include <string.h>


/* External cleanup function declarations for filesystem components */
extern void __FSStateClear__(void);
extern void __BlockDeviceStateClear__(void);
extern void __RAMDiskStateClear__(void);
extern void __ConsoleStateClear__(void);
extern void __CharDeviceStateClear__(void);


int main(int argc, char **argv) {
  Base_t json_output = false;
  const char *json_file = null;
  int i;


  /* Parse command line arguments */
  for(i = 1; i < argc; i++) {
    if((strcmp(argv[i], "--json") == 0) || (strcmp(argv[i], "-j") == 0)) {
      json_output = true;

      if((i + 1 < argc) && (argv[i + 1][0] != '-')) {
        json_file = argv[++i];
      }
    }
  }

  unit_init_with_options(json_output, json_file);
  reset();
  sys_harness();
  reset();
  memory_harness();
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
  reset();
  fs_harness();
  reset();
  console_harness();
  unit_exit();

  return(0);
}


void reset(void) {
  __MemoryClear__();
  __SysStateClear__();
  __TaskStateClear__();
  __DeviceStateClear__();
  __FSStateClear__();
  __BlockDeviceStateClear__();
  __RAMDiskStateClear__();
  __ConsoleStateClear__();
  __CharDeviceStateClear__();

  return;
}