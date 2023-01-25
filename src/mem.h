/**
 * @file mem.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for memory management
 * @version 0.4.0
 * @date 2022-01-31
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
#ifndef MEM_H_
#define MEM_H_

#include "config.h"
#include "defines.h"
#include "types.h"
#include "port.h"
#include "device.h"
#include "queue.h"
#include "stream.h"
#include "sys.h"
#include "task.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

Return_t xMemAlloc(Addr_t **addr_, const Size_t size_);
Return_t xMemFree(const volatile Addr_t *addr_);
Return_t xMemGetUsed(Size_t *size_);
Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_);
Return_t __KernelAllocateMemory__(Addr_t **addr_, const Size_t size_);
Return_t __KernelFreeMemory__(const volatile Addr_t *addr_);
Return_t __MemoryRegionCheckKernel__(const volatile Addr_t *addr_, const Base_t option_, Base_t *res_);
Return_t __HeapAllocateMemory__(Addr_t **addr_, const Size_t size_);
Return_t __HeapFreeMemory__(const volatile Addr_t *addr_);
Return_t __MemoryRegionCheckHeap__(const volatile Addr_t *addr_, const Base_t option_, Base_t *res_);
Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_);
Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_);
Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_);
Return_t __MemoryInit__(void);
Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_);
Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_);

#if defined(POSIX_ARCH_OTHER)
void __MemoryClear__(void);
void __MemoryRegionDumpKernel__(void);
void __MemoryRegionDumpHeap__(void);
void __memdump__(const volatile MemoryRegion_t *region_);
#endif

#ifdef __cplusplus
}
#endif
#endif