/**
 * @file unit.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief A simple unit testing framework for HeliOS
 * @version 0.3.6
 * @date 2022-08-23
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef UNIT_H_
#define UNIT_H_

#include "posix.h"

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#if !defined(UNIT_NAME_LENGTH)
#define UNIT_NAME_LENGTH 0x40 /* 64 */
#endif

#if !defined(true)
#define true 0x1 /* 1 */
#endif

#if !defined(false)
#define false 0x0 /* 0 */
#endif

#if !defined(NULL)
#if !defined(__cplusplus)
#define NULL ((void *)0x0) /* 0 */
#else
#define NULL 0x0 /* 0 */
#endif
#endif

typedef struct unit_s {
  char name[UNIT_NAME_LENGTH];
  int32_t begun;
  int32_t failed;
  int32_t pass;
  int32_t fail;
} unit_t;

#ifdef __cplusplus
extern "C" {
#endif

void unit_init(void);
void unit_begin(const char *name_);
void unit_try(int expr_);
void unit_end(void);
void unit_exit(void);

#ifdef __cplusplus
}
#endif


#endif