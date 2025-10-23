/*UNCRUSTIFY-OFF*/
/**
 * @file mem.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for memory management with multiple region support
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
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
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"

  #include <stdint.h>
  #include <stddef.h>


  /* Memory management constants */
  #if defined(INUSE)
    #undef INUSE
  #endif /* if defined(INUSE) */
  #define INUSE 0xAAu /* 170 - Block is in use */

  #if defined(FREE)
    #undef FREE
  #endif /* if defined(FREE) */
  #define FREE 0xD5u /* 213 - Block is free */

  #if defined(MEMORY_REGION_CHECK_OPTION_WO_ADDR)
    #undef MEMORY_REGION_CHECK_OPTION_WO_ADDR
  #endif /* if defined(MEMORY_REGION_CHECK_OPTION_WO_ADDR) */
  #define MEMORY_REGION_CHECK_OPTION_WO_ADDR 0x1u /* Check without address */

  #if defined(MEMORY_REGION_CHECK_OPTION_W_ADDR)
    #undef MEMORY_REGION_CHECK_OPTION_W_ADDR
  #endif /* if defined(MEMORY_REGION_CHECK_OPTION_W_ADDR) */
  #define MEMORY_REGION_CHECK_OPTION_W_ADDR 0x2u /* Check with address */


  /* The BlockHeader_t and updated MemoryRegion_t structures are now defined in
   * types.h */

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  /* Public API Functions - Maintain compatibility */
  Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);
  Return_t xMemFree(const volatile Addr_t *addr_);
  Return_t xMemFreeAll(void);
  Return_t xMemGetUsed(Size_t *size_);
  Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_);
  Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_);
  Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_);


  /* Kernel API Functions */
  Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
  Return_t __KernelFreeMemory__(const volatile Addr_t *addr_);
  Return_t __MemoryRegionCheckKernel__(const volatile Addr_t *addr_, const Base_t option_);
  Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
  Return_t __HeapFreeMemory__(const volatile Addr_t *addr_);


  /* Memory utility functions */
  Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_);
  Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_);
  Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_);
  Return_t __MemoryInit__(void);


  /* String utility functions following libc naming conventions */
  Size_t __strlen__(const Byte_t *str_);
  Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
  Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t n_);
  Base_t __strcmp__(const Byte_t *s1_, const Byte_t *s2_);
  Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t n_);
  Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
  Byte_t * __strchr__(const Byte_t *str_, const Byte_t ch_);
  Byte_t * __strrchr__(const Byte_t *str_, const Byte_t ch_);


  /* Path utility functions */
  Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, const Size_t destSize_);
  Return_t __path_normalize__(Byte_t *path_, const Size_t pathSize_);
  Base_t __path_is_absolute__(const Byte_t *path_);
  Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);
  Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);

  #if defined(POSIX_ARCH_OTHER)
    void __MemoryClear__(void);
  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef MEM_H_ */