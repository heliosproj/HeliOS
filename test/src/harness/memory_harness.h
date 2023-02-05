/*UNCRUSTIFY-OFF*/
/**
 * @file memory_harness.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.4.0
 * @date 2022-08-27
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
/*UNCRUSTIFY-ON*/
#ifndef MEMORY_HARNESS_H_
#define MEMORY_HARNESS_H_

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
#endif
void memory_harness(void);
void memory_harness_task(Task_t *task_, TaskParm_t *parm_);

#ifdef __cplusplus
  }
#endif
#endif