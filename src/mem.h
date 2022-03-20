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



void *xMemAlloc(const size_t size_);
void xMemFree(const void *addr_);
size_t xMemGetUsed(void);
size_t xMemGetSize(const void *addr_);
Base_t _MemoryRegionCheck_(const volatile MemoryRegion_t *region_, const void *addr_, const Base_t option_);
Base_t _MemoryRegionCheckAddr_(const volatile MemoryRegion_t *region_, const void *addr_);
void *_calloc_(volatile MemoryRegion_t *region_, const size_t size_);
void _free_(const volatile MemoryRegion_t *region_, const void *addr_);
void *_KernelAllocateMemory_(const size_t size_);
void _KernelFreeMemory_(const void *addr_);
Base_t _MemoryRegionCheckKernel_(const void *addr_, const Base_t option_);
void *_HeapAllocateMemory_(const size_t size_);
void _HeapFreeMemory_(const void *addr_);
Base_t _MemoryRegionCheckHeap_(const void *addr_, const Base_t option_);
void _memcpy_(void *dest_, const void *src_, size_t n_);
void _memset_(volatile void *dest_, uint16_t val_, size_t n_);
uint16_t _memcmp_(const void *s1_, const void *s2_, size_t n_);



/* For debugging only. */
#if defined(MEMDUMP_HEAP) || defined(MEMDUMP_KERNEL)
void _memdump_(void);
#endif

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif