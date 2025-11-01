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
  #endif
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
  #endif
  #define INUSE 0xAAu
  #if defined(FREE)
    #undef FREE
  #endif
  #define FREE 0x55u
  #ifndef BLOCKHEADER_T_
    #define BLOCKHEADER_T_
    typedef struct BlockHeader_s {
      struct BlockHeader_s *next;
      Word_t checksum;
      Word_t size;
      Byte_t free;
    } BlockHeader_t;
  #endif
  #ifndef MEMORYREGION_T_
    #define MEMORYREGION_T_
    typedef struct MemoryRegion_s {
      volatile Byte_t mem[MEMORY_REGION_SIZE];
      BlockHeader_t *first;
      HalfWord_t allocations;
      HalfWord_t frees;
      Word_t minAvailableEver;
    } MemoryRegion_t;
  #endif
  #ifdef __cplusplus
    extern "C" {
  #endif
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
  #endif
  #ifdef __cplusplus
    }
  #endif
#endif