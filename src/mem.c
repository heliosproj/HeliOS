/*UNCRUSTIFY-OFF*/
/**
 * @file mem.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for memory management with multiple region support
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
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
        (((value_) & ((alignment_) - 1)) == 0x0u)

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


/* GOOD - DO NOT TOUCH */
static Word_t __checksum__(const BlockHeader_t *header_) {
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

  return (((sum2 << 16) | sum1) ^ 0xB16B00B5u);
}


static Return_t __ValidateBlockHeader__(const BlockHeader_t *header_, const volatile MemoryRegion_t *region_) {
  FUNCTION_ENTER;


  Word_t expectedChecksum = 0x0u;


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
#if !defined(POSIX_ARCH_OTHER)
      __SetFlag__(MEMFAULT);
#endif /* if !defined(POSIX_ARCH_OTHER) */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __MemoryInit__(void) {
  FUNCTION_ENTER;


  ByteOrder_t order;


  if((CONFIG_MEMORY_ALIGNMENT != 0) && ((CONFIG_MEMORY_ALIGNMENT & (CONFIG_MEMORY_ALIGNMENT - 1)) == 0)) {
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
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __MemoryRegionInit__(volatile MemoryRegion_t *region_) {
  FUNCTION_ENTER;


  BlockHeader_t *first = null;


  if(__PointerIsNotNull__(region_)) {
    region_->first = (BlockHeader_t *) region_->mem;
    region_->minAvailableEver = MEMORY_REGION_SIZE;
    region_->allocations = 0x0u;
    region_->frees = 0x0u;

    if(OK(__memset__((volatile Addr_t *) region_->mem, 0x0u, MEMORY_REGION_SIZE))) {
      first = region_->first;
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


  Size_t requested = 0x0u;
  Size_t available = 0x0u;
  BlockHeader_t *cursor = null;
  BlockHeader_t *candidate = null;
  BlockHeader_t *next = null;
  BlockHeader_t *first = null;
  Size_t candidateSize = (Size_t) -1;
  Size_t traversedSize = 0x0u;


  __DisableInterrupts__();
  requested = __AlignUp__(size_, CONFIG_MEMORY_ALIGNMENT);

  if(__FlagIsNotSet__(MEMFAULT) && __PointerIsNotNull__(region_) && __PointerIsNotNull__(addr_) && (0x0u < size_) && (requested >= size_) && ((requested <
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
      if(OK(__ValidateBlockHeader__(cursor, region_))) {
        if(traversedSize > MEMORY_REGION_SIZE - ALIGNED_HEADER_SIZE - cursor->size) {
          candidate = null;
          __AssertOnElse__();
          break;
        }

        traversedSize += ALIGNED_HEADER_SIZE + cursor->size;

        if(__BlockHeaderIsFree__(cursor) && (requested <= cursor->size) && (cursor->size < candidateSize)) {
          candidateSize = cursor->size;
          candidate = cursor;
        }

        if(__BlockHeaderIsFree__(cursor)) {
          available += cursor->size;
        }

        cursor = cursor->next;
      } else {
        __AssertOnElse__();
      }
    }

    if(__PointerIsNotNull__(candidate)) {
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

      if(OK(__memset__(__OffsetBlockHeaderToPointer__(candidate), 0x0u, requested))) {
        *addr_ = __OffsetBlockHeaderToPointer__(candidate);

        if(__IsAligned__((Size_t) *addr_, CONFIG_MEMORY_ALIGNMENT)) {
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
  Size_t traversedSize = 0x0u;
  Size_t mergedSize = 0x0u;
  Base_t headForExit = false;


  if(__PointerIsNotNull__(region_)) {
    cursor = region_->first;
    traversedSize = 0x0u;

    while(__PointerIsNotNull__(cursor) && !headForExit) {
      if(traversedSize <= MEMORY_REGION_SIZE - ALIGNED_HEADER_SIZE - cursor->size) {
        if(OK(__ValidateBlockHeader__(cursor, region_))) {
          traversedSize += ALIGNED_HEADER_SIZE + cursor->size;

          if(__BlockHeaderIsFree__(cursor) && __PointerIsNotNull__(cursor->next)) {
            while(__PointerIsNotNull__(cursor->next) && __BlockHeaderIsFree__(cursor->next) && !headForExit) {
              nextBlock = cursor->next;

              if(OK(__ValidateBlockHeader__(nextBlock, region_))) {
                if(cursor->size <= ((Size_t) -1) - ALIGNED_HEADER_SIZE - nextBlock->size) {
                  mergedSize = ALIGNED_HEADER_SIZE + nextBlock->size;

                  if(traversedSize + mergedSize <= MEMORY_REGION_SIZE) {
                    cursor->size += ALIGNED_HEADER_SIZE + nextBlock->size;
                    cursor->next = nextBlock->next;
                    cursor->checksum = __checksum__(cursor);
                    traversedSize += mergedSize;
                  } else {
                    headForExit = true;
                    __AssertOnElse__();
                  }
                } else {
                  break;
                }
              } else {
                headForExit = true;
                __AssertOnElse__();
              }
            }
          }

          if(!headForExit) {
            cursor = cursor->next;
          }
        } else {
          headForExit = true;
          __AssertOnElse__();
        }
      } else {
        headForExit = true;
        __AssertOnElse__();
      }
    }

    if(!headForExit) {
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
  Size_t used = 0x0u;
  Size_t traversedSize = 0x0u;


  if(__PointerIsNotNull__(size_)) {
    cursor = heap.first;

    while(__PointerIsNotNull__(cursor)) {
      if(traversedSize > MEMORY_REGION_SIZE - ALIGNED_HEADER_SIZE - cursor->size) {
        __AssertOnElse__();
        break;
      }

      traversedSize += ALIGNED_HEADER_SIZE + cursor->size;

      if(__BlockHeaderIsInUse__(cursor)) {
        used += cursor->size + ALIGNED_HEADER_SIZE;
      }

      cursor = cursor->next;
    }

    if(__PointerIsNull__(cursor)) {
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


/* GOOD - DO NOT TOUCH */
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;


  MemoryRegionStats_t *stats = null;
  BlockHeader_t *cursor = null;
  Word_t largestFree = 0x0u;
  Word_t smallestFree = (Word_t) -1;
  Word_t freeBlocks = 0x0u;
  Word_t availableBytes = 0x0u;
  Size_t traversedSize = 0x0u;


  if(__PointerIsNotNull__(region_) && __PointerIsNotNull__(stats_)) {
    if(OK(xMemAlloc((volatile Addr_t **) &stats, sizeof(MemoryRegionStats_t)))) {
      cursor = region_->first;

      while(__PointerIsNotNull__(cursor)) {
        if(traversedSize <= MEMORY_REGION_SIZE - ALIGNED_HEADER_SIZE - cursor->size) {
          traversedSize += ALIGNED_HEADER_SIZE + cursor->size;

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
        } else {
          xMemFree((const volatile Addr_t *) stats);
          __HeapFreeMemory__((const volatile Addr_t *) stats);
          stats = null;
          __AssertOnElse__();
          break;
        }
      }

      if(__PointerIsNotNull__(stats)) {
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


/* GOOD - DO NOT TOUCH */
Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;

  if(OK(__MemGetRegionStats__(&heap, stats_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* GOOD - DO NOT TOUCH */
Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;

  if(OK(__MemGetRegionStats__(&kernel, stats_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* GOOD - DO NOT TOUCH */
Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_) {
  FUNCTION_ENTER;


  Size_t i = 0x0u;
  volatile Byte_t *src = null;
  volatile Byte_t *dest = null;


  if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (0x0u < size_)) {
    src = (Byte_t *) src_;
    dest = (Byte_t *) dest_;

    for(i = 0x0u; i < size_; i++) {
      dest[i] = src[i];
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* GOOD - DO NOT TOUCH */
Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_) {
  FUNCTION_ENTER;


  Size_t i = 0x0u;
  volatile Byte_t *dest = null;


  if(__PointerIsNotNull__(dest_) && (0x0u < size_)) {
    dest = (Byte_t *) dest_;

    for(i = 0x0u; i < size_; i++) {
      dest[i] = (Byte_t) val_;
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* GOOD - DO NOT TOUCH */
Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_) {
  FUNCTION_ENTER;


  Size_t i = 0x0u;
  volatile Byte_t *s1 = null;
  volatile Byte_t *s2 = null;


  if(__PointerIsNotNull__(s1_) && __PointerIsNotNull__(s2_) && (0x0u < size_) && __PointerIsNotNull__(res_)) {
    *res_ = true;
    s1 = (Byte_t *) s1_;
    s2 = (Byte_t *) s2_;

    for(i = 0x0u; i < size_; i++) {
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


/* GOOD - DO NOT TOUCH */
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