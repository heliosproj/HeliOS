/*UNCRUSTIFY-OFF*/
/**
 * @file unit.h
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
#ifndef UNIT_H_
  #define UNIT_H_

  #include "posix.h"
  #include "defines.h"

  #include <stdint.h>
  #include <inttypes.h>
  #include <string.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <unistd.h>

  #if defined(UNIT_NAME_LENGTH)
    #undef UNIT_NAME_LENGTH
  #endif /* if defined(UNIT_NAME_LENGTH) */
  #define UNIT_NAME_LENGTH 0x40 /* 64 */
  typedef struct unit_s {
    char name[UNIT_NAME_LENGTH];
    int32_t begun;
    int32_t failed;
    int32_t pass;
    int32_t fail;
  } unit_t;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  void unit_init(void);
  void unit_begin(const char *name_);
  void unit_try(int expr_);
  void unit_end(void);
  void unit_exit(void);
  void unit_print(const char *msg_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */


#endif /* ifndef UNIT_H_ */