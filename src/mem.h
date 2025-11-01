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
  #if defined(__AlignUp__)
    #undef __AlignUp__
  #endif
  #define __AlignUp__(value_, alignment_) \
          (((value_) + ((alignment_) - 0x1u)) & ~((alignment_) - 0x1u))
  #if defined(__AlignDown__)
    #undef __AlignDown__
  #endif
  #define __AlignDown__(value_, alignment_) \
          ((value_) & ~((alignment_) - 0x1u))
  #if defined(__IsAligned__)
    #undef __IsAligned__
  #endif
  #define __IsAligned__(value_, alignment_) \
          (((value_) & ((alignment_) - 0x1u)) == 0x0u)
  #if defined(ALIGNED_HEADER_SIZE)
    #undef ALIGNED_HEADER_SIZE
  #endif
  #define ALIGNED_HEADER_SIZE \
          (((sizeof(BlockHeader_t)) + (CONFIG_MEMORY_ALIGNMENT - 0x1u)) & ~(CONFIG_MEMORY_ALIGNMENT - 0x1u))
  #if defined(__OffsetPointerToBlockHeader__)
    #undef __OffsetPointerToBlockHeader__
  #endif
  #define __OffsetPointerToBlockHeader__(ptr_) \
          ((BlockHeader_t *) (((Byte_t *) (ptr_)) - ALIGNED_HEADER_SIZE))
  #if defined(__OffsetBlockHeaderToPointer__)
    #undef __OffsetBlockHeaderToPointer__
  #endif
  #define __OffsetBlockHeaderToPointer__(header_) \
          ((Addr_t *) (((Byte_t *) (header_)) + ALIGNED_HEADER_SIZE))
  #if defined(__BlockHeaderIsInUse__)
    #undef __BlockHeaderIsInUse__
  #endif
  #define __BlockHeaderIsInUse__(header_) (INUSE == (header_)->free)
  #if defined(__BlockHeaderIsFree__)
    #undef __BlockHeaderIsFree__
  #endif
  #define __BlockHeaderIsFree__(header_) (FREE == (header_)->free)
  #if defined(INUSE)
    #undef INUSE
  #endif
  #define INUSE 0xAAu
  #if defined(FREE)
    #undef FREE
  #endif
  #define FREE 0x55u
  #if defined(FLETCHER_INIT_VALUE)
    #undef FLETCHER_INIT_VALUE
  #endif
  #define FLETCHER_INIT_VALUE 0xFFFFu
  #if defined(FLETCHER_MASK)
    #undef FLETCHER_MASK
  #endif
  #define FLETCHER_MASK 0xFFFFu
  #if defined(CHECKSUM_WORD_SHIFT)
    #undef CHECKSUM_WORD_SHIFT
  #endif
  #define CHECKSUM_WORD_SHIFT 0x10u
  #if defined(CHECKSUM_DWORD_SHIFT)
    #undef CHECKSUM_DWORD_SHIFT
  #endif
  #define CHECKSUM_DWORD_SHIFT 0x20u
  #if defined(CHECKSUM_XOR_CONSTANT)
    #undef CHECKSUM_XOR_CONSTANT
  #endif
  #define CHECKSUM_XOR_CONSTANT 0xB16B00B5u
  #if defined(MEM_ALIGN_SHIFT_4BIT)
    #undef MEM_ALIGN_SHIFT_4BIT
  #endif
  #define MEM_ALIGN_SHIFT_4BIT 0x4
  #if defined(BYTE_ORDER_TEST_VALUE)
    #undef BYTE_ORDER_TEST_VALUE
  #endif
  #define BYTE_ORDER_TEST_VALUE 0x100
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