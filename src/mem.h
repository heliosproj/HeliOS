/**
 * @file mem.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for memory management
 * @version 0.3.3
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

#include "config.h"
#include "defines.h"
#include "types.h"
#include "queue.h"
#include "sys.h"
#include "task.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

void *xMemAlloc(size_t size_);
void xMemFree(void *addr_);
size_t xMemGetUsed(void);
size_t xMemGetSize(void *addr_);
Base_t MemoryRegionCheckHeap(const void *addr_, Base_t option_);
Base_t MemoryRegionCheckKernel(const void *addr_, Base_t option_);
Base_t MemoryRegionCheck(const MemoryRegion_t *region_, const void *addr_, Base_t option_);
Base_t MemoryRegionCheckAddr(const MemoryRegion_t *region_, const void *addr_);
void *calloc_(const MemoryRegion_t *region_, size_t size_);
void free_(const MemoryRegion_t *region_, void *addr_);
void memcpy_(void *dest_, const void *src_, size_t n_);
void memset_(void *dest_, uint16_t val_, size_t n_);
uint16_t memcmp_(const void *s1_, const void *s2_, size_t n_);


/* For debugging the heap only. */
#if defined(MEMDUMP_)
void memdump_(void);
#endif

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif