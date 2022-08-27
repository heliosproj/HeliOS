/**
 * @file mem.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for memory management
 * @version 0.3.5
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
#include "port.h"
#include "queue.h"
#include "sys.h"
#include "task.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif



Addr_t *xMemAlloc(const Size_t size_);
void xMemFree(const Addr_t *addr_);
Size_t xMemGetUsed(void);
Size_t xMemGetSize(const Addr_t *addr_);
Base_t _MemoryRegionCheck_(const volatile MemoryRegion_t *region_, const Addr_t *addr_, const Base_t option_);
Base_t _MemoryRegionCheckAddr_(const volatile MemoryRegion_t *region_, const Addr_t *addr_);
Addr_t *_calloc_(volatile MemoryRegion_t *region_, const Size_t size_);
void _free_(volatile MemoryRegion_t *region_, const Addr_t *addr_);
Addr_t *_KernelAllocateMemory_(const Size_t size_);
void _KernelFreeMemory_(const Addr_t *addr_);
Base_t _MemoryRegionCheckKernel_(const Addr_t *addr_, const Base_t option_);
Addr_t *_HeapAllocateMemory_(const Size_t size_);
void _HeapFreeMemory_(const Addr_t *addr_);
Base_t _MemoryRegionCheckHeap_(const Addr_t *addr_, const Base_t option_);
void _memcpy_(Addr_t *dest_, const Addr_t *src_, Size_t n_);
void _memset_(volatile Addr_t *dest_, uint16_t val_, Size_t n_);
uint16_t _memcmp_(const Addr_t *s1_, const Addr_t *s2_, Size_t n_);
MemoryRegionStats_t *xMemGetHeapStats(void);
MemoryRegionStats_t *xMemGetKernelStats(void);
MemoryRegionStats_t *_MemGetRegionStats_(const volatile MemoryRegion_t *region_);


#if defined(POSIX_ARCH_OTHER)
void __MemoryClear__(void);
void __MemoryRegionDumpKernel__(void);
void __MemoryRegionDumpHeap__(void);
void __memdump__(const volatile MemoryRegion_t *region_);
#endif

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif