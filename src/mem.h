/*UNCRUSTIFY-OFF*/
/**
 * @file mem.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for memory management
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
  #endif /* ifdef __cplusplus */
  Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);
  Return_t xMemFree(const volatile Addr_t *addr_);
  Return_t xMemGetUsed(Size_t *size_);
  Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_);
  Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
  Return_t __KernelFreeMemory__(const volatile Addr_t *addr_);
  Return_t __MemoryRegionCheckKernel__(const volatile Addr_t *addr_, const Base_t option_);
  Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
  Return_t __HeapFreeMemory__(const volatile Addr_t *addr_);
  Return_t __MemoryRegionCheckHeap__(const volatile Addr_t *addr_, const Base_t option_);
  Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_);
  Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_);
  Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_);
  Return_t __MemoryInit__(void);
  Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_);
  Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_);

  #if defined(POSIX_ARCH_OTHER)
    void __MemoryClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef MEM_H_ */