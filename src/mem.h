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


  #define __AlignUp__(value_, alignment_) \
          (((value_) + ((alignment_) - 1)) & ~((alignment_) - 1))


  #define __AlignDown__(value_, alignment_) \
          ((value_) & ~((alignment_) - 1))


  #define __IsAligned__(value_, alignment_) \
          (((value_) & ((alignment_) - 1)) == 0x0u)


  #define ALIGNED_HEADER_SIZE \
          (((sizeof(BlockHeader_t)) + (CONFIG_MEMORY_ALIGNMENT - 1)) & ~(CONFIG_MEMORY_ALIGNMENT - 1))


  #define __OffsetPointerToBlockHeader__(ptr_) \
          ((BlockHeader_t *) (((Byte_t *) (ptr_)) - ALIGNED_HEADER_SIZE))


  #define __OffsetBlockHeaderToPointer__(header_) \
          ((Addr_t *) (((Byte_t *) (header_)) + ALIGNED_HEADER_SIZE))


  #define __BlockHeaderIsInUse__(header_) (INUSE == (header_)->free)


  #define __BlockHeaderIsFree__(header_) (FREE == (header_)->free)


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


    /**
     * @brief Memory block header structure
     * @details Internal structure prepended to each memory block for allocation tracking and integrity checking.
     */
    typedef struct BlockHeader_s {


      struct BlockHeader_s *next; /**< Pointer to next block in free list */
      Word_t checksum; /**< Integrity checksum for block validation */
      Word_t size; /**< Size of the data portion in bytes */
      Byte_t free; /**< Flag indicating if block is free (FREE) or in use (INUSE) */
    } BlockHeader_t;


  #endif /* ifndef BLOCKHEADER_T_ */


  #ifndef MEMORYREGION_T_


    #define MEMORYREGION_T_


    /**
     * @brief Memory region structure
     * @details Manages a contiguous region of memory with block allocation and usage statistics.
     */
    typedef struct MemoryRegion_s {


      volatile Byte_t mem[MEMORY_REGION_SIZE]; /**< Raw memory buffer for allocations */
      BlockHeader_t *first; /**< Pointer to first block in the region */
      HalfWord_t allocations; /**< Count of successful allocations */
      HalfWord_t frees; /**< Count of successful frees */
      Word_t minAvailableEver; /**< Low water mark of available memory */
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
  Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
  Return_t __HeapFreeMemory__(const volatile Addr_t *addr_);
  Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_);
  Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_);
  Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_);
  Return_t __MemoryInit__(void);
  Return_t __ValidateBlockHeader__(const BlockHeader_t *header_, const volatile MemoryRegion_t *region_);
  Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_);
  Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
  Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_);
  Return_t __DefragMemoryRegion__(volatile MemoryRegion_t *region_);
  Return_t __MemoryRegionInit__(volatile MemoryRegion_t *region_);
  Return_t __DetectByteOrder__(ByteOrder_t *order_);
  Word_t __checksum__(const BlockHeader_t *header_);


  #if defined(POSIX_ARCH_OTHER)
    void __MemoryClear__(void);


  #endif /* if defined(POSIX_ARCH_OTHER) */


  #ifdef __cplusplus


    }


  #endif /* ifdef __cplusplus */


#endif /* ifndef MEM_H_ */