/*UNCRUSTIFY-OFF*/
/**
 * @file mem.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Memory allocator API header
 * @details
 * Defines memory region structures, allocation functions, and interfaces for heap and kernel memory management with statistics.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef MEM_H_

  #define MEM_H_

  #include "config.h"

  #include "defines.h"

  #include "types.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "console.h"

    #include "device.h"

    #include "fat.h"

    #include "fs.h"

  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

  #include "port.h"

  #include "posix.h"

  #include "queue.h"

  #include "streams.h"

  #include "sys.h"

  #include "task.h"

  #include "timer.h"

  #include <stdint.h>

  #include <stddef.h>

  #if defined(INUSE)

    #undef INUSE

  #endif /* if defined(INUSE) */

  #define INUSE 0xAAu

  #if defined(FREE)

    #undef FREE

  #endif /* if defined(FREE) */

  #define FREE 0x55u

  #ifndef BLOCKHEADER_T_

    #define BLOCKHEADER_T_

    typedef struct BlockHeader_s {

      struct BlockHeader_s *next;

      Word_t checksum;

      Word_t size;

      Byte_t free;

    } BlockHeader_t;

  #endif /* ifndef BLOCKHEADER_T_ */

  #ifndef MEMORYREGION_T_

    #define MEMORYREGION_T_

    typedef struct MemoryRegion_s {

      volatile Byte_t mem[MEMORY_REGION_SIZE];

      BlockHeader_t *first;

      HalfWord_t allocations;

      HalfWord_t frees;

      Word_t minAvailableEver;

    } MemoryRegion_t;

  #endif /* ifndef MEMORYREGION_T_ */

  #ifdef __cplusplus

    extern "C" {

  #endif /* ifdef __cplusplus */
  Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);
  Return_t xMemFree(const volatile Addr_t *addr_);
  Return_t xMemFreeAll(void);
  Return_t xMemGetUsed(Size_t *size_);
  Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_);
  Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_);
  Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_);
  Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
  Return_t __KernelFreeMemory__(const volatile Addr_t *addr_);
  Return_t __MemoryRegionCheckKernel__(const volatile Addr_t *addr_, const Base_t option_);
  Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
  Return_t __HeapFreeMemory__(const volatile Addr_t *addr_);
  Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_);
  Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_);
  Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_);
  Return_t __MemoryInit__(void);

  #if defined(POSIX_ARCH_OTHER)
    void __MemoryClear__(void);

  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus

    }

  #endif /* ifdef __cplusplus */

#endif /* ifndef MEM_H_ */