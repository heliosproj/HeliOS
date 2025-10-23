/*UNCRUSTIFY-OFF*/
/**
 * @file mem.c
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

#include "mem.h"

static volatile MemoryRegion_t heap = {
  0
};
static volatile MemoryRegion_t kernel = {
  0
};

#define __AlignUp__(value_, alignment_) \
        (((value_) + ((alignment_) - 1)) & ~((alignment_) - 1))

#define __AlignDown__(value_, alignment_) \
        ((value_) & ~((alignment_) - 1))

#define __IsAligned__(value_, alignment_) \
        (((value_) & ((alignment_) - 1)) == 0)

#define ALIGNED_HEADER_SIZE \
        (((sizeof(BlockHeader_t)) + (CONFIG_MEMORY_ALIGNMENT - 1)) & ~(CONFIG_MEMORY_ALIGNMENT - 1))

#define __OffsetPointerToBlockHeader__(ptr_) \
        ((BlockHeader_t *) (((Byte_t *) (ptr_)) - ALIGNED_HEADER_SIZE))

#define __OffsetBlockHeaderToPointer__(header_) \
        ((Addr_t *) (((Byte_t *) (header_)) + ALIGNED_HEADER_SIZE))

#define __BlockHeaderIsInUse__(header_) (INUSE == (header_)->free)

#define __BlockHeaderIsFree__(header_) (FREE == (header_)->free)
static Return_t __ValidateBlockHeader__(const BlockHeader_t *header_, const volatile MemoryRegion_t *region_);
static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_);
static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_);
static Return_t __DefragMemoryRegion__(volatile MemoryRegion_t *region_);
static Return_t __MemoryRegionInit__(volatile MemoryRegion_t *region_);
static Return_t __DetectByteOrder__(ByteOrder_t *order_);
static Word_t __checksum__(const BlockHeader_t *header_);


static Word_t __checksum__(const BlockHeader_t *header_) { /* GOOD - DO NOT
                                                            * TOUCH */
  Word_t sum1 = 0xFFFFu;
  Word_t sum2 = 0xFFFFu;
  Word_t temp;


#if UINTPTR_MAX == 0xFF

    sum1 = (sum1 + (Word_t) (uintptr_t) header_->next) & 0xFFFFu;
    sum2 = (sum2 + sum1) & 0xFFFFu;

#elif UINTPTR_MAX == 0xFFFF

    temp = (Word_t) (uintptr_t) header_->next;
    sum1 = (sum1 + temp) & 0xFFFFu;
    sum2 = (sum2 + sum1) & 0xFFFFu;

#elif UINTPTR_MAX == 0xFFFFFFFF

    temp = (Word_t) (uintptr_t) header_->next;
    sum1 = (sum1 + (temp & 0xFFFFu)) & 0xFFFFu;
    sum2 = (sum2 + sum1) & 0xFFFFu;
    sum1 = (sum1 + (temp >> 16)) & 0xFFFFu;
    sum2 = (sum2 + sum1) & 0xFFFFu;

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF

    temp = (Word_t) ((uintptr_t) header_->next & 0xFFFFFFFFu);
    sum1 = (sum1 + (temp & 0xFFFFu)) & 0xFFFFu;
    sum2 = (sum2 + sum1) & 0xFFFFu;
    sum1 = (sum1 + (temp >> 16)) & 0xFFFFu;
    sum2 = (sum2 + sum1) & 0xFFFFu;
    temp = (Word_t) ((uintptr_t) header_->next >> 32);
    sum1 = (sum1 + (temp & 0xFFFFu)) & 0xFFFFu;
    sum2 = (sum2 + sum1) & 0xFFFFu;
    sum1 = (sum1 + (temp >> 16)) & 0xFFFFu;
    sum2 = (sum2 + sum1) & 0xFFFFu;
#endif /* if UINTPTR_MAX == 0xFF */

  sum1 = (sum1 + (header_->size & 0xFFFFu)) & 0xFFFFu;
  sum2 = (sum2 + sum1) & 0xFFFFu;
  sum1 = (sum1 + (header_->size >> 16)) & 0xFFFFu;
  sum2 = (sum2 + sum1) & 0xFFFFu;
  sum1 = (sum1 + header_->free) & 0xFFFFu;
  sum2 = (sum2 + sum1) & 0xFFFFu;

  return(((sum2 << 16) | sum1) ^ 0xB16B00B5u);
}


static Return_t __ValidateBlockHeader__(const BlockHeader_t *header_, const volatile MemoryRegion_t *region_) {  /* GOOD - DO NOT TOUCH!! */
  FUNCTION_ENTER;


  Word_t expectedChecksum = nil;


  if(((const Byte_t *) header_ >= (const Byte_t *) region_->mem) && ((const Byte_t *) header_ < ((const Byte_t *) region_->mem + MEMORY_REGION_SIZE -
    ALIGNED_HEADER_SIZE))) {
    expectedChecksum = __checksum__(header_);

    if(header_->checksum == expectedChecksum) {
      if((header_->free == FREE) || (header_->free == INUSE)) {
        __ReturnOk__();
      } else {

#if !defined(POSIX_ARCH_OTHER)
          __SetFlag__(MEMFAULT);
#endif /* if !defined(POSIX_ARCH_OTHER) */
        __AssertOnElse__();
      }
    } else {

#if !defined(POSIX_ARCH_OTHER)
        __SetFlag__(MEMFAULT);
#endif /* if !defined(POSIX_ARCH_OTHER) */
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __MemoryInit__(void) {  /* GOOD - DO NOT TOUCH!! */
  FUNCTION_ENTER;


  ByteOrder_t order;


  if(OK(__MemoryRegionInit__(&heap))) {
    if(OK(__MemoryRegionInit__(&kernel))) {
      if(OK(__DetectByteOrder__(&order))) {
        if(ByteOrderLittleEndian == order) {
          __SetFlag__(LITTLEEND);
        } else {
          __UnsetFlag__(LITTLEEND);
        }

        __UnsetFlag__(MEMFAULT);
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __MemoryRegionInit__(volatile MemoryRegion_t *region_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(region_)) {
    region_->first = (BlockHeader_t *) region_->mem;
    region_->minAvailableEver = MEMORY_REGION_SIZE;
    region_->allocations = 0;
    region_->frees = 0;

    if(OK(__memset__((volatile Addr_t *) region_->mem, nil, MEMORY_REGION_SIZE))) {
      BlockHeader_t *first = region_->first;


      first->next = null;
      first->size = MEMORY_REGION_SIZE - ALIGNED_HEADER_SIZE;
      first->free = FREE;
      first->checksum = __checksum__(first);
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;


  Size_t requested;
  Size_t available = 0;
  BlockHeader_t *cursor = null;
  BlockHeader_t *candidate = null;
  BlockHeader_t *next = null;
  BlockHeader_t *first = null;
  Size_t candidateSize = (Size_t) -1;
  Size_t traversedSize = 0;
  Base_t cycleDetected = false;


  requested = __AlignUp__(size_, CONFIG_MEMORY_ALIGNMENT);
  __DisableInterrupts__();

  if(__FlagIsNotSet__(MEMFAULT) && __PointerIsNotNull__(region_) && __PointerIsNotNull__(addr_) && (nil < size_) && (requested >= size_) && ((requested <
    MEMORY_REGION_SIZE) && ((requested + ALIGNED_HEADER_SIZE) <= MEMORY_REGION_SIZE))) {
    if(__PointerIsNull__(region_->first)) {
      region_->first = (BlockHeader_t *) region_->mem;
      first = region_->first;
      first->next = null;
      first->size = MEMORY_REGION_SIZE - ALIGNED_HEADER_SIZE;
      first->free = FREE;
      first->checksum = __checksum__(first);
    }

    cursor = region_->first;

    while(__PointerIsNotNull__(cursor)) {
      if(!OK(__ValidateBlockHeader__(cursor, region_))) {
        candidate = null;
        __SetFlag__(MEMFAULT);
        __AssertOnElse__();
        break;
      }

      traversedSize += ALIGNED_HEADER_SIZE + cursor->size;

      if(traversedSize > MEMORY_REGION_SIZE) {
        cycleDetected = true;
        candidate = null;
        __SetFlag__(MEMFAULT);
        __AssertOnElse__();
        break;
      }

      if(__BlockHeaderIsFree__(cursor) && (requested <= cursor->size) && (cursor->size < candidateSize)) {
        candidateSize = cursor->size;
        candidate = cursor;
      }

      if(__BlockHeaderIsFree__(cursor)) {
        available += cursor->size;
      }

      cursor = cursor->next;
    }

    if((cycleDetected == false) && __PointerIsNotNull__(candidate)) {
      if((candidate->size >= requested) && ((candidate->size - requested) >= (ALIGNED_HEADER_SIZE + CONFIG_MEMORY_MINIMUM_BLOCK_SIZE))) {
        next = candidate->next;
        candidate->next = (BlockHeader_t *) (((Byte_t *) candidate) + ALIGNED_HEADER_SIZE + requested);
        candidate->next->next = next;
        candidate->next->size = candidate->size - requested - ALIGNED_HEADER_SIZE;
        candidate->next->free = FREE;
        candidate->next->checksum = __checksum__(candidate->next);
        candidate->size = requested;
      }

      candidate->free = INUSE;
      candidate->checksum = __checksum__(candidate);

      if(OK(__memset__(__OffsetBlockHeaderToPointer__(candidate), nil, requested))) {
        *addr_ = __OffsetBlockHeaderToPointer__(candidate);

        if(!__IsAligned__((Size_t) *addr_, CONFIG_MEMORY_ALIGNMENT)) {
          __SetFlag__(MEMFAULT);
          __AssertOnElse__();
        }

        region_->allocations++;
        available -= requested;

        if(available < region_->minAvailableEver) {
          region_->minAvailableEver = available;
        }

        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  __EnableInterrupts__();
  FUNCTION_EXIT;
}


static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {
  FUNCTION_ENTER;


  BlockHeader_t *header = null;


  __DisableInterrupts__();

  if(__PointerIsNotNull__(addr_)) {
    if(__FlagIsNotSet__(MEMFAULT) && __PointerIsNotNull__(region_)) {
      header = __OffsetPointerToBlockHeader__(addr_);

      if(OK(__ValidateBlockHeader__(header, region_))) {
        if(__BlockHeaderIsInUse__(header)) {
          header->free = FREE;
          header->checksum = __checksum__(header);
          region_->frees++;

          if(OK(__DefragMemoryRegion__(region_))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {

#if !defined(POSIX_ARCH_OTHER)
            __SetFlag__(MEMFAULT);
#endif /* if !defined(POSIX_ARCH_OTHER) */
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __ReturnOk__();
    __AssertOnElse__();
  }

  __EnableInterrupts__();
  FUNCTION_EXIT;
}


static Return_t __DefragMemoryRegion__(volatile MemoryRegion_t *region_) {
  FUNCTION_ENTER;


  BlockHeader_t *cursor = null;
  BlockHeader_t *nextBlock = null;
  Base_t merged = true;
  Base_t cycleDetected = false;
  Size_t traversedSize = 0;


  if(__PointerIsNotNull__(region_)) {
    while(merged && !cycleDetected) {
      merged = false;
      cursor = region_->first;
      traversedSize = 0;

      while(__PointerIsNotNull__(cursor) && __PointerIsNotNull__(cursor->next) && !cycleDetected) {
        traversedSize += ALIGNED_HEADER_SIZE + cursor->size;

        if(traversedSize > MEMORY_REGION_SIZE) {
          __SetFlag__(MEMFAULT);
          cycleDetected = true;
        } else {
          if(__BlockHeaderIsFree__(cursor) && __BlockHeaderIsFree__(cursor->next)) {
            nextBlock = cursor->next;

            if(cursor->size > ((Size_t) -1) - ALIGNED_HEADER_SIZE - nextBlock->size) {
              cursor = cursor->next;
              continue;
            }

            cursor->size += ALIGNED_HEADER_SIZE + nextBlock->size;
            cursor->next = nextBlock->next;
            cursor->checksum = __checksum__(cursor);
            merged = true;
          } else {
            cursor = cursor->next;
          }
        }
      }
    }

    if(cycleDetected) {
      __AssertOnElse__();
    } else {
      __ReturnOk__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if(OK(__calloc__(&heap, addr_, size_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemFree(const volatile Addr_t *addr_) {
  FUNCTION_ENTER;

  if(OK(__free__(&heap, addr_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemFreeAll(void) {
  FUNCTION_ENTER;

  if(OK(__MemoryRegionInit__(&heap))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetUsed(Size_t *size_) {
  FUNCTION_ENTER;


  BlockHeader_t *cursor = null;
  Size_t used = 0;
  Size_t traversedSize = 0;
  Base_t cycleDetected = false;


  if(__PointerIsNotNull__(size_)) {
    cursor = heap.first;

    while(__PointerIsNotNull__(cursor) && !cycleDetected) {
      traversedSize += ALIGNED_HEADER_SIZE + cursor->size;

      if(traversedSize > MEMORY_REGION_SIZE) {
        __SetFlag__(MEMFAULT);
        cycleDetected = true;
      } else {
        if(__BlockHeaderIsInUse__(cursor)) {
          used += cursor->size + ALIGNED_HEADER_SIZE;
        }

        cursor = cursor->next;
      }
    }

    if(cycleDetected) {
      __AssertOnElse__();
    } else {
      *size_ = used;
      __ReturnOk__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_) {
  FUNCTION_ENTER;


  BlockHeader_t *header = null;


  if(__PointerIsNotNull__(addr_) && __PointerIsNotNull__(size_)) {
    header = __OffsetPointerToBlockHeader__(addr_);

    if(OK(__ValidateBlockHeader__(header, &heap))) {
      if(__BlockHeaderIsInUse__(header)) {
        *size_ = header->size;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if(OK(__calloc__(&kernel, addr_, size_))) {
    if(__PointerIsNotNull__(*addr_)) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __KernelFreeMemory__(const volatile Addr_t *addr_) {
  FUNCTION_ENTER;

  if(OK(__free__(&kernel, addr_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if(OK(__calloc__(&heap, addr_, size_))) {
    if(__PointerIsNotNull__(*addr_)) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __HeapFreeMemory__(const volatile Addr_t *addr_) {
  FUNCTION_ENTER;

  if(OK(__free__(&heap, addr_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;


  MemoryRegionStats_t *stats = null;
  BlockHeader_t *cursor = null;
  Word_t largestFree = 0;
  Word_t smallestFree = (Word_t) -1;
  Word_t freeBlocks = 0;
  Word_t availableBytes = 0;
  Size_t traversedSize = 0;
  Base_t cycleDetected = false;


  if(__PointerIsNotNull__(region_) && __PointerIsNotNull__(stats_)) {
    if(OK(xMemAlloc((volatile Addr_t **) &stats, sizeof(MemoryRegionStats_t)))) {
      cursor = region_->first;

      while(__PointerIsNotNull__(cursor) && !cycleDetected) {
        traversedSize += ALIGNED_HEADER_SIZE + cursor->size;

        if(traversedSize > MEMORY_REGION_SIZE) {
          __SetFlag__(MEMFAULT);
          xMemFree((const volatile Addr_t *) stats);
          cycleDetected = true;
        } else {
          if(__BlockHeaderIsFree__(cursor)) {
            freeBlocks++;
            availableBytes += cursor->size;

            if(cursor->size > largestFree) {
              largestFree = cursor->size;
            }

            if(cursor->size < smallestFree) {
              smallestFree = cursor->size;
            }
          }

          cursor = cursor->next;
        }
      }

      if(cycleDetected) {
        __AssertOnElse__();
      } else {
        stats->largestFreeEntryInBytes = largestFree;
        stats->smallestFreeEntryInBytes = (smallestFree == (Word_t) -1) ? 0 : smallestFree;
        stats->numberOfFreeBlocks = freeBlocks;
        stats->availableSpaceInBytes = availableBytes;
        stats->successfulAllocations = region_->allocations;
        stats->successfulFrees = region_->frees;
        stats->minimumEverFreeBytesRemaining = region_->minAvailableEver;
        *stats_ = stats;
        __ReturnOk__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;

  if(OK(__MemGetRegionStats__(&heap, stats_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;

  if(OK(__MemGetRegionStats__(&kernel, stats_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_) {
  FUNCTION_ENTER;


  Size_t i = nil;
  volatile Byte_t *src = null;
  volatile Byte_t *dest = null;


  if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (nil < size_)) {
    src = (Byte_t *) src_;
    dest = (Byte_t *) dest_;

    for(i = nil; i < size_; i++) {
      dest[i] = src[i];
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_) {
  FUNCTION_ENTER;


  Size_t i = nil;
  volatile Byte_t *dest = null;


  if(__PointerIsNotNull__(dest_) && (nil < size_)) {
    dest = (Byte_t *) dest_;

    for(i = nil; i < size_; i++) {
      dest[i] = (Byte_t) val_;
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_) {
  FUNCTION_ENTER;


  Size_t i = nil;
  volatile Byte_t *s1 = null;
  volatile Byte_t *s2 = null;


  if(__PointerIsNotNull__(s1_) && __PointerIsNotNull__(s2_) && (nil < size_) && __PointerIsNotNull__(res_)) {
    *res_ = true;
    s1 = (Byte_t *) s1_;
    s2 = (Byte_t *) s2_;

    for(i = nil; i < size_; i++) {
      if(*s1 != *s2) {
        *res_ = false;
        break;
      }

      s1++;
      s2++;
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __DetectByteOrder__(ByteOrder_t *order_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(order_)) {
    if((*(uint16_t *) "\xFF\x00") < 0x100) {
      *order_ = ByteOrderLittleEndian;
      __ReturnOk__();
    } else {
      *order_ = ByteOrderBigEndian;
      __ReturnOk__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


#if defined(POSIX_ARCH_OTHER)


  void __MemoryClear__(void) {
    __MemoryRegionInit__(&heap);
    __MemoryRegionInit__(&kernel);
  }


#endif /* if defined(POSIX_ARCH_OTHER) */

#define CHAR_NULL 0x00u

#define CHAR_SLASH 0x2Fu

#define CHAR_DOT 0x2Eu

#if !defined(CONFIG_FS_MAX_PATH_LENGTH)

  #define CONFIG_FS_MAX_PATH_LENGTH 256u
#endif /* if !defined(CONFIG_FS_MAX_PATH_LENGTH) */


Size_t __strlen__(const Byte_t *str_) {
  Size_t len = nil;


  if(__PointerIsNotNull__(str_)) {
    while(CHAR_NULL != str_[len]) {
      len++;
    }
  }

  return (len);
}


Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t i = nil;


  if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (nil < destSize_)) {
    while((CHAR_NULL != src_[i]) && (i < (destSize_ - 0x1u))) {
      dest_[i] = src_[i];
      i++;
    }

    dest_[i] = CHAR_NULL;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t n_) {
  FUNCTION_ENTER;


  Size_t i = nil;


  if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (nil < n_)) {
    for(i = nil; (i < n_) && (CHAR_NULL != src_[i]); i++) {
      dest_[i] = src_[i];
    }

    for(; i < n_; i++) {
      dest_[i] = CHAR_NULL;
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Base_t __strcmp__(const Byte_t *s1_, const Byte_t *s2_) {
  Size_t i = nil;


  if(__PointerIsNull__(s1_) || __PointerIsNull__(s2_)) {
    return (nil);
  }

  while((CHAR_NULL != s1_[i]) && (CHAR_NULL != s2_[i])) {
    if(s1_[i] != s2_[i]) {
      return ((s1_[i] < s2_[i]) ? -0x1 : 0x1);
    }

    i++;
  }

  if(s1_[i] == s2_[i]) {
    return (nil);
  }

  return ((s1_[i] < s2_[i]) ? -0x1 : 0x1);
}


Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t n_) {
  Size_t i = nil;


  if(__PointerIsNull__(s1_) || __PointerIsNull__(s2_) || (nil == n_)) {
    return (nil);
  }

  for(i = nil; i < n_; i++) {
    if((CHAR_NULL == s1_[i]) || (s1_[i] != s2_[i])) {
      return ((s1_[i] < s2_[i]) ? (Base_t) -0x1 : ((s1_[i] > s2_[i]) ? (Base_t) 0x1 : (Base_t) 0x0));
    }
  }

  return (nil);
}


Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t destLen = nil;
  Size_t i = nil;


  if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (nil < destSize_)) {
    destLen = __strlen__(dest_);

    if(destLen < destSize_) {
      while((CHAR_NULL != src_[i]) && ((destLen + i) < (destSize_ - 0x1u))) {
        dest_[destLen + i] = src_[i];
        i++;
      }

      dest_[destLen + i] = CHAR_NULL;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Byte_t * __strchr__(const Byte_t *str_, const Byte_t ch_) {
  Size_t i = nil;


  if(__PointerIsNull__(str_)) {
    return (null);
  }

  while(CHAR_NULL != str_[i]) {
    if(str_[i] == ch_) {
      return ((Byte_t *) &str_[i]);
    }

    i++;
  }

  if(CHAR_NULL == ch_) {
    return ((Byte_t *) &str_[i]);
  }

  return (null);
}


Byte_t * __strrchr__(const Byte_t *str_, const Byte_t ch_) {
  Size_t len = nil;
  Size_t i = nil;


  if(__PointerIsNull__(str_)) {
    return (null);
  }

  len = __strlen__(str_);

  for(i = len; i > nil; i--) {
    if(str_[i - 0x1u] == ch_) {
      return ((Byte_t *) &str_[i - 0x1u]);
    }
  }

  if((CHAR_NULL == ch_) && (len > nil)) {
    return ((Byte_t *) &str_[len]);
  }

  return (null);
}


Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t baseLen = nil;
  Size_t pathLen = nil;
  Base_t needSlash = false;


  if(__PointerIsNull__(dest_) || __PointerIsNull__(base_) || __PointerIsNull__(path_) || (nil == destSize_)) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  baseLen = __strlen__(base_);
  pathLen = __strlen__(path_);

  if((nil == baseLen) || (nil == pathLen)) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  if(CHAR_SLASH == path_[nil]) {
    if(pathLen >= destSize_) {
      __AssertOnElse__();
      __AssertOnElse__();
      FUNCTION_EXIT;
    }

    if(OK(__strcpy__(dest_, path_, destSize_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }

  needSlash = (CHAR_SLASH != base_[baseLen - 0x1u]) && (CHAR_SLASH != path_[nil]);

  if((baseLen + pathLen + (needSlash ? 0x1u : nil)) >= destSize_) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  if(OK(__strcpy__(dest_, base_, destSize_))) {
    if(needSlash) {
      dest_[baseLen] = CHAR_SLASH;
      dest_[baseLen + 0x1u] = CHAR_NULL;
    }

    if(OK(__strcat__(dest_, path_, destSize_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __path_normalize__(Byte_t *path_, const Size_t pathSize_) {
  FUNCTION_ENTER;


  Size_t i = nil;
  Size_t j = nil;
  Size_t len = nil;
  Byte_t temp[CONFIG_FS_MAX_PATH_LENGTH];
  Byte_t segments[CONFIG_FS_MAX_PATH_LENGTH / 2][CONFIG_FS_MAX_PATH_LENGTH];
  Size_t segmentCount = nil;
  Size_t k = nil;
  Size_t segLen = nil;
  Size_t segIdx = nil;


  if(__PointerIsNull__(path_) || (nil == pathSize_)) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  len = __strlen__(path_);

  if((nil == len) || (len >= CONFIG_FS_MAX_PATH_LENGTH)) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  for(i = nil; i <= len; i++) {
    temp[i] = path_[i];
  }

  i = nil;

  if(CHAR_SLASH == temp[nil]) {
    i = 0x1u;
  }

  segIdx = nil;

  for(; i <= len; i++) {
    if((CHAR_SLASH == temp[i]) || (CHAR_NULL == temp[i])) {
      if(segIdx > nil) {
        segments[segmentCount][segIdx] = CHAR_NULL;

        if((segments[segmentCount][nil] == '.') && (segments[segmentCount][0x1u] == '.') && (segments[segmentCount][0x2u] == CHAR_NULL)) {
          if(segmentCount > nil) {
            segmentCount--;
          }
        } else if(!((segments[segmentCount][nil] == '.') && (segments[segmentCount][0x1u] == CHAR_NULL))) {
          segmentCount++;
        }

        segIdx = nil;
      }
    } else {
      segments[segmentCount][segIdx++] = temp[i];
    }
  }

  j = nil;

  if(CHAR_SLASH == path_[nil]) {
    path_[j++] = CHAR_SLASH;
  }

  for(k = nil; k < segmentCount; k++) {
    Size_t m;


    segLen = __strlen__(segments[k]);

    if(k > nil) {
      path_[j++] = CHAR_SLASH;
    }

    for(m = nil; m < segLen; m++) {
      path_[j++] = segments[k][m];
    }
  }

  if((nil == j) || ((0x1u == j) && (CHAR_SLASH == path_[nil]))) {
    path_[nil] = CHAR_SLASH;
    j = 0x1u;
  }

  path_[j] = CHAR_NULL;
  __ReturnOk__();
  FUNCTION_EXIT;
}


Base_t __path_is_absolute__(const Byte_t *path_) {
  if(__PointerIsNull__(path_)) {
    return (false);
  }

  return ((CHAR_SLASH == path_[nil]) ? true : false);
}


Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t len = nil;
  Size_t i = nil;


  if(__PointerIsNull__(dest_) || __PointerIsNull__(path_) || (nil == destSize_)) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  len = __strlen__(path_);

  if(nil == len) {
    if(OK(__strcpy__(dest_, (const Byte_t *) ".", destSize_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }

  for(i = len; i > nil; i--) {
    if(CHAR_SLASH == path_[i - 0x1u]) {
      break;
    }
  }

  if(nil == i) {
    if(OK(__strcpy__(dest_, (const Byte_t *) ".", destSize_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    if(i > destSize_) {
      __AssertOnElse__();
      __AssertOnElse__();
      FUNCTION_EXIT;
    }

    if(OK(__strncpy__(dest_, path_, i - 0x1u))) {
      dest_[i - 0x1u] = CHAR_NULL;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  }

  FUNCTION_EXIT;
}


Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t len = nil;
  Size_t i = nil;
  Size_t start = nil;


  if(__PointerIsNull__(dest_) || __PointerIsNull__(path_) || (nil == destSize_)) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  len = __strlen__(path_);

  if(nil == len) {
    if(OK(__strcpy__(dest_, (const Byte_t *) ".", destSize_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }

  for(i = len; i > nil; i--) {
    if(CHAR_SLASH == path_[i - 0x1u]) {
      start = i;
      break;
    }
  }

  if((len - start) >= destSize_) {
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  if(OK(__strcpy__(dest_, &path_[start], destSize_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}