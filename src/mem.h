/**
 * @file mem.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for the management of dynamically allocated memory in HeliOS
 * @version 0.3.0
 * @date 2022-01-31
 * 
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
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
#ifndef MEM_H_
#define MEM_H_

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "defines.h"
#include "types.h"
#include "queue.h"
#include "sched.h"
#include "task.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

void *xMemAlloc(size_t);
void xMemFree(void *);
size_t xMemGetUsed();
size_t xMemGetSize(void *);
void memcpy_(void *, const void *, size_t);
void memset_(void *, int16_t, size_t);
int16_t memcmp_(const void *, const void *, size_t);

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif