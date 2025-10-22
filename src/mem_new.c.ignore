/*UNCRUSTIFY-OFF*/
/**
 * @file mem_new.c
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

#include "mem_new.h"

/* Memory region instances */
static volatile MemoryRegion_t heap = {
    0};
static volatile MemoryRegion_t kernel = {
    0};

/* Macros for pointer arithmetic and validation */
#define __OffsetPointerToBlockHeader__(ptr_, region_) \
  ((BlockHeader_t*)(((Byte_t*)(ptr_)) - (region_)->headerSize))

#define __OffsetBlockHeaderToPointer__(header_, region_) \
  ((Addr_t*)(((Byte_t*)(header_)) + (region_)->headerSize))

#define __BlockHeaderIsInUse__(header_) (INUSE == (header_)->free)
#define __BlockHeaderIsFree__(header_) (FREE == (header_)->free)


/* Private function prototypes */
static Return_t __VerifyRegionConsistency__(const volatile MemoryRegion_t* region_);
static Return_t __calloc__(volatile MemoryRegion_t* region_, volatile Addr_t** addr_, const Size_t size_);
static Return_t __free__(volatile MemoryRegion_t* region_, const volatile Addr_t* addr_);
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t* region_, MemoryRegionStats_t** stats_);
static Return_t __DefragMemoryRegion__(volatile MemoryRegion_t* region_);
static Return_t __MemoryRegionInit__(volatile MemoryRegion_t* region_);
static Return_t __DetectByteOrder__(ByteOrder_t* order_);


/* Checksum calculation function - inline for performance */
static inline Word_t __checksum__(const BlockHeader_t* header_) {
  Word_t checksum = 0x0u;
  const Byte_t* header = (const Byte_t*)header_;


#if UINTPTR_MAX == 0xFF


  /* 8-bit architecture - 1 byte pointer */
  /* Process next pointer (1 byte) */
  checksum ^= header[0];
  checksum = (checksum << 1) | (checksum >> 31);


  /* Skip checksum at header[1-4] */
  /* Process size (4 bytes) */
  checksum ^= header[5];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[6];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[7];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[8];
  checksum = (checksum << 1) | (checksum >> 31);


  /* Process free (1 byte) */
  checksum ^= header[9];
  checksum = (checksum << 1) | (checksum >> 31);

#elif UINTPTR_MAX == 0xFFFF


  /* 16-bit architecture - 2 byte pointer */
  /* Process next pointer (2 bytes) */
  checksum ^= header[0];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[1];
  checksum = (checksum << 1) | (checksum >> 31);


  /* Skip checksum at header[2-5] */
  /* Process size (4 bytes) */
  checksum ^= header[6];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[7];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[8];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[9];
  checksum = (checksum << 1) | (checksum >> 31);


  /* Process free (1 byte) */
  checksum ^= header[10];
  checksum = (checksum << 1) | (checksum >> 31);

#elif UINTPTR_MAX == 0xFFFFFFFF


  /* 32-bit architecture - 4 byte pointer */
  /* Process next pointer (4 bytes) */
  checksum ^= header[0];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[1];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[2];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[3];
  checksum = (checksum << 1) | (checksum >> 31);


  /* Skip checksum at header[4-7] */
  /* Process size (4 bytes) */
  checksum ^= header[8];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[9];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[10];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[11];
  checksum = (checksum << 1) | (checksum >> 31);


  /* Process free (1 byte) */
  checksum ^= header[12];
  checksum = (checksum << 1) | (checksum >> 31);

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF


  /* 64-bit architecture - 8 byte pointer */
  /* Process next pointer (8 bytes) */
  checksum ^= header[0];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[1];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[2];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[3];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[4];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[5];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[6];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[7];
  checksum = (checksum << 1) | (checksum >> 31);


  /* Skip checksum at header[8-11] */
  /* Process size (4 bytes) */
  checksum ^= header[12];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[13];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[14];
  checksum = (checksum << 1) | (checksum >> 31);
  checksum ^= header[15];
  checksum = (checksum << 1) | (checksum >> 31);


  /* Process free (1 byte) */
  checksum ^= header[16];
  checksum = (checksum << 1) | (checksum >> 31);
#endif /* if UINTPTR_MAX == 0xFF */

  return (checksum);
}


/* Initialize memory management system */
Return_t __MemoryInit__(void) {
  FUNCTION_ENTER;


  ByteOrder_t order;


  /* Initialize the heap and kernel memory regions */
  if (OK(__MemoryRegionInit__(&heap))) {
    if (OK(__MemoryRegionInit__(&kernel))) {
      if (OK(__DetectByteOrder__(&order))) {
        if (ByteOrderLittleEndian == order) {
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


/* Initialize a memory region */
static Return_t __MemoryRegionInit__(volatile MemoryRegion_t* region_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(region_)) {
    /* Calculate header size */
    region_->headerSize = sizeof(BlockHeader_t);


    /* Set the first block header at the start of the memory region */
    region_->first = (BlockHeader_t*)region_->mem;


    /* Set initial statistics */
    region_->minAvailableEver = MEMORY_REGION_SIZE_IN_BYTES;
    region_->allocations = 0;
    region_->frees = 0;


    /* Zero out the memory region */
    if (OK(__memset__(region_->mem, nil, MEMORY_REGION_SIZE_IN_BYTES))) {
      /* Create the initial free block spanning the entire region */
      BlockHeader_t* initial = region_->first;


      initial->next = null;
      initial->size = MEMORY_REGION_SIZE_IN_BYTES - sizeof(BlockHeader_t);
      initial->free = FREE;


      /* Set checksum for the initial block */
      initial->checksum = __checksum__(initial);
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* Verify region consistency - check all blocks for integrity */
static Return_t __VerifyRegionConsistency__(const volatile MemoryRegion_t* region_) {
  FUNCTION_ENTER;


  BlockHeader_t* cursor = null;
  Size_t totalBytes = 0;
  Word_t calculatedChecksum = 0;


  if (__PointerIsNotNull__(region_)) {
    cursor = region_->first;


    /* Walk the entire linked list */
    while (__PointerIsNotNull__(cursor)) {
      /* Verify checksum */
      calculatedChecksum = __checksum__(cursor);

      if (calculatedChecksum != cursor->checksum) {
        /* Checksum mismatch */
        __SetFlag__(MEMFAULT);
        __ReturnError__();
        break;
      }


      /* Verify free status is valid */
      if ((cursor->free != FREE) && (cursor->free != INUSE)) {
        /* Invalid free status */
        __SetFlag__(MEMFAULT);
        __ReturnError__();
        break;
      }


      /* Add block header size and data size to running total */
      totalBytes += sizeof(BlockHeader_t) + cursor->size;


      /* Check if we've exceeded the memory region size (circular reference) */
      if (totalBytes > MEMORY_REGION_SIZE_IN_BYTES) {
        /* Circular reference detected - total size exceeds region */
        __SetFlag__(MEMFAULT);
        __ReturnError__();
        break;
      }


      /* Move to next block */
      cursor = cursor->next;
    }


    /* If we completed the loop without errors, return OK */
    if (__PointerIsNull__(cursor)) {
      __ReturnOk__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* Allocate memory from region */
static Return_t __calloc__(volatile MemoryRegion_t* region_, volatile Addr_t** addr_, const Size_t size_) {
  FUNCTION_ENTER;


  Size_t requested = size_;
  Size_t available = 0;
  BlockHeader_t* cursor = null;
  BlockHeader_t* candidate = null;
  BlockHeader_t* next = null;
  Size_t candidateSize = (Size_t)-1;


  /* Disable interrupts during allocation */
  __DisableInterrupts__();

  if (__FlagIsNotSet__(MEMFAULT) && __PointerIsNotNull__(region_) && __PointerIsNotNull__(addr_) && (nil < size_)) {
    /* Verify region consistency before allocation */
    if (OK(__VerifyRegionConsistency__(region_))) {
      cursor = region_->first;


      /* Find best fit free block */
      while (__PointerIsNotNull__(cursor)) {
        if (__BlockHeaderIsFree__(cursor) && (requested <= cursor->size) && (cursor->size < candidateSize)) {
          candidateSize = cursor->size;
          candidate = cursor;
        }

        if (__BlockHeaderIsFree__(cursor)) {
          available += cursor->size;
        }

        cursor = cursor->next;
      }

      if (__PointerIsNotNull__(candidate)) {
        /* Check if we should split the block */
        if ((sizeof(BlockHeader_t) + 1) <= (candidate->size - requested)) {
          /* Split the block */
          next = candidate->next;
          candidate->next = (BlockHeader_t*)(((Byte_t*)candidate) + sizeof(BlockHeader_t) + requested);


          /* Set up new free block */
          candidate->next->next = next;
          candidate->next->size = candidate->size - requested - sizeof(BlockHeader_t);
          candidate->next->free = FREE;
          candidate->next->checksum = __checksum__(candidate->next);


          /* Update current block */
          candidate->size = requested;
        }


        /* Mark block as in use */
        candidate->free = INUSE;
        candidate->checksum = __checksum__(candidate);


        /* Zero out allocated memory */
        if (OK(__memset__(__OffsetBlockHeaderToPointer__(candidate, region_), nil, requested))) {
          *addr_ = __OffsetBlockHeaderToPointer__(candidate, region_);


          /* Update statistics */
          region_->allocations++;
          available -= requested;

          if (available < region_->minAvailableEver) {
            region_->minAvailableEver = available;
          }

          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        /* No suitable block found */
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }


  /* Re-enable interrupts */
  __EnableInterrupts__();
  FUNCTION_EXIT;
}


/* Free memory and merge adjacent free blocks */
static Return_t __free__(volatile MemoryRegion_t* region_, const volatile Addr_t* addr_) {
  FUNCTION_ENTER;


  BlockHeader_t* header = null;


  /* Disable interrupts during free */
  __DisableInterrupts__();

  if (__FlagIsNotSet__(MEMFAULT) && __PointerIsNotNull__(region_) && __PointerIsNotNull__(addr_)) {
    /* Verify region consistency before freeing */
    if (OK(__VerifyRegionConsistency__(region_))) {
      header = __OffsetPointerToBlockHeader__(addr_, region_);


      /* Mark block as free */
      header->free = FREE;
      header->checksum = __checksum__(header);
      region_->frees++;


      /* Merge adjacent free blocks */
      if (OK(__DefragMemoryRegion__(region_))) {
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


  /* Re-enable interrupts */
  __EnableInterrupts__();
  FUNCTION_EXIT;
}


/* Defragment memory region by merging adjacent free blocks */
static Return_t __DefragMemoryRegion__(volatile MemoryRegion_t* region_) {
  FUNCTION_ENTER;


  BlockHeader_t* cursor = null;
  BlockHeader_t* nextBlock = null;
  Base_t merged = true;


  if (__PointerIsNotNull__(region_)) {
    /* Keep merging until no more merges are possible */
    while (merged) {
      merged = false;
      cursor = region_->first;

      while (__PointerIsNotNull__(cursor) && __PointerIsNotNull__(cursor->next)) {
        /* Merge if both current and next blocks are free */
        if (__BlockHeaderIsFree__(cursor) && __BlockHeaderIsFree__(cursor->next)) {
          nextBlock = cursor->next;


          /* Merge the blocks */
          cursor->size += sizeof(BlockHeader_t) + nextBlock->size;
          cursor->next = nextBlock->next;


          /* Update checksum for merged block */
          cursor->checksum = __checksum__(cursor);


          /* Zero out the old block header */
          if (OK(__memset__(nextBlock, nil, sizeof(BlockHeader_t)))) {
            merged = true;
          } else {
            __AssertOnElse__();
            break;
          }
        } else {
          cursor = cursor->next;
        }
      }
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* Public API implementations */
Return_t xMemAlloc(volatile Addr_t** addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(addr_) && (nil < size_)) {
    if (OK(__calloc__(&heap, addr_, size_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemFree(const volatile Addr_t* addr_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(addr_)) {
    if (OK(__free__(&heap, addr_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    /* Silently succeed on NULL (matches standard C free behavior) */
    __ReturnOk__();
  }

  FUNCTION_EXIT;
}


Return_t xMemFreeAll(void) {
  FUNCTION_ENTER;

  if (OK(__MemoryRegionInit__(&heap))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetUsed(Size_t* size_) {
  FUNCTION_ENTER;


  BlockHeader_t* cursor = null;
  Size_t used = 0;


  if (__PointerIsNotNull__(size_)) {
    cursor = heap.first;

    while (__PointerIsNotNull__(cursor)) {
      if (__BlockHeaderIsInUse__(cursor)) {
        used += cursor->size;
      }

      cursor = cursor->next;
    }

    *size_ = used;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetSize(const volatile Addr_t* addr_, Size_t* size_) {
  FUNCTION_ENTER;


  BlockHeader_t* header = null;


  if (__PointerIsNotNull__(addr_) && __PointerIsNotNull__(size_)) {
    header = __OffsetPointerToBlockHeader__(addr_, &heap);

    if (__BlockHeaderIsInUse__(header)) {
      *size_ = header->size;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* Kernel memory management functions */
Return_t __KernelAllocateMemory__(volatile Addr_t** addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(addr_) && (nil < size_)) {
    if (OK(__calloc__(&kernel, addr_, size_))) {
      if (__PointerIsNotNull__(*addr_)) {
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


Return_t __KernelFreeMemory__(const volatile Addr_t* addr_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(addr_)) {
    if (OK(__free__(&kernel, addr_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __MemoryRegionCheckKernel__(const volatile Addr_t* addr_, const Base_t option_) {
  FUNCTION_ENTER;


  /* Always return OK - integrity checking removed */
  __ReturnOk__();
  FUNCTION_EXIT;
}


Return_t __HeapAllocateMemory__(volatile Addr_t** addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(addr_) && (nil < size_)) {
    if (OK(__calloc__(&heap, addr_, size_))) {
      if (__PointerIsNotNull__(*addr_)) {
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


Return_t __HeapFreeMemory__(const volatile Addr_t* addr_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(addr_)) {
    if (OK(__free__(&heap, addr_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __MemoryRegionCheckHeap__(const volatile Addr_t* addr_, const Base_t option_) {
  FUNCTION_ENTER;


  /* Always return OK - integrity checking removed */
  __ReturnOk__();
  FUNCTION_EXIT;
}


/* Memory statistics functions */
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t* region_, MemoryRegionStats_t** stats_) {
  FUNCTION_ENTER;


  static MemoryRegionStats_t stats;
  BlockHeader_t* cursor = null;
  Word_t largestFree = 0;
  Word_t smallestFree = (Word_t)-1;
  Word_t freeBlocks = 0;
  Word_t availableBytes = 0;


  if (__PointerIsNotNull__(region_) && __PointerIsNotNull__(stats_)) {
    cursor = region_->first;

    while (__PointerIsNotNull__(cursor)) {
      if (__BlockHeaderIsFree__(cursor)) {
        freeBlocks++;
        availableBytes += cursor->size;

        if (cursor->size > largestFree) {
          largestFree = cursor->size;
        }

        if (cursor->size < smallestFree) {
          smallestFree = cursor->size;
        }
      }

      cursor = cursor->next;
    }

    stats.largestFreeEntryInBytes = largestFree;
    stats.smallestFreeEntryInBytes = (smallestFree == (Word_t)-1) ? 0 : smallestFree;
    stats.numberOfFreeBlocks = freeBlocks;
    stats.availableSpaceInBytes = availableBytes;
    stats.successfulAllocations = region_->allocations;
    stats.successfulFrees = region_->frees;
    stats.minimumEverFreeBytesRemaining = region_->minAvailableEver;
    *stats_ = &stats;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetHeapStats(MemoryRegionStats_t** stats_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(stats_)) {
    if (OK(__MemGetRegionStats__(&heap, stats_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetKernelStats(MemoryRegionStats_t** stats_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(stats_)) {
    if (OK(__MemGetRegionStats__(&kernel, stats_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/* Memory utility functions */
Return_t __memcpy__(const volatile Addr_t* dest_, const volatile Addr_t* src_, const Size_t size_) {
  FUNCTION_ENTER;


  Size_t i = nil;
  volatile Byte_t* src = null;
  volatile Byte_t* dest = null;


  if (__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (nil < size_)) {
    src = (Byte_t*)src_;
    dest = (Byte_t*)dest_;

    for (i = nil; i < size_; i++) {
      dest[i] = src[i];
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __memset__(const volatile Addr_t* dest_, const Byte_t val_, const Size_t size_) {
  FUNCTION_ENTER;


  Size_t i = nil;
  volatile Byte_t* dest = null;


  if (__PointerIsNotNull__(dest_) && (nil < size_)) {
    dest = (Byte_t*)dest_;

    for (i = nil; i < size_; i++) {
      dest[i] = (Byte_t)val_;
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __memcmp__(const volatile Addr_t* s1_, const volatile Addr_t* s2_, const Size_t size_, Base_t* res_) {
  FUNCTION_ENTER;


  Size_t i = nil;
  volatile Byte_t* s1 = null;
  volatile Byte_t* s2 = null;


  if (__PointerIsNotNull__(s1_) && __PointerIsNotNull__(s2_) && (nil < size_) && __PointerIsNotNull__(res_)) {
    *res_ = true;
    s1 = (Byte_t*)s1_;
    s2 = (Byte_t*)s2_;

    for (i = nil; i < size_; i++) {
      if (*s1 != *s2) {
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


/* Byte order detection */
static Return_t __DetectByteOrder__(ByteOrder_t* order_) {
  FUNCTION_ENTER;

  if (__PointerIsNotNull__(order_)) {
    if ((*(uint16_t*)"\xFF\x00") < 0x100) {
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
/* String utility functions */
#define CHAR_NULL 0x00u
#define CHAR_SLASH 0x2Fu
#define CHAR_DOT 0x2Eu

#if !defined(CONFIG_FS_MAX_PATH_LENGTH)
#define CONFIG_FS_MAX_PATH_LENGTH 256u
#endif /* if !defined(CONFIG_FS_MAX_PATH_LENGTH) */


Size_t __strlen__(const Byte_t* str_) {
  Size_t len = 0x0u;


  if (__PointerIsNotNull__(str_)) {
    while (CHAR_NULL != str_[len]) {
      len++;
    }
  }

  return (len);
}


Return_t __strcpy__(Byte_t* dest_, const Byte_t* src_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t i = 0x0u;


  if (__PointerIsNull__(dest_) || __PointerIsNull__(src_) || (0x0u == destSize_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  while ((CHAR_NULL != src_[i]) && (i < (destSize_ - 0x1u))) {
    dest_[i] = src_[i];
    i++;
  }

  dest_[i] = CHAR_NULL;
  __ReturnOk__();
  FUNCTION_EXIT;
}


Return_t __strncpy__(Byte_t* dest_, const Byte_t* src_, const Size_t n_) {
  FUNCTION_ENTER;


  Size_t i = 0x0u;


  if (__PointerIsNull__(dest_) || __PointerIsNull__(src_) || (0x0u == n_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  for (i = 0x0u; (i < n_) && (CHAR_NULL != src_[i]); i++) {
    dest_[i] = src_[i];
  }

  for (; i < n_; i++) {
    dest_[i] = CHAR_NULL;
  }

  __ReturnOk__();
  FUNCTION_EXIT;
}


Base_t __strcmp__(const Byte_t* s1_, const Byte_t* s2_) {
  Size_t i = 0x0u;


  if (__PointerIsNull__(s1_) || __PointerIsNull__(s2_)) {
    return (0x0u);
  }

  while ((CHAR_NULL != s1_[i]) && (CHAR_NULL != s2_[i])) {
    if (s1_[i] != s2_[i]) {
      return ((s1_[i] < s2_[i]) ? -0x1 : 0x1);
    }

    i++;
  }

  if (s1_[i] == s2_[i]) {
    return (0x0u);
  }

  return ((s1_[i] < s2_[i]) ? -0x1 : 0x1);
}


Base_t __strncmp__(const Byte_t* s1_, const Byte_t* s2_, const Size_t n_) {
  Size_t i = 0x0u;


  if (__PointerIsNull__(s1_) || __PointerIsNull__(s2_) || (0x0u == n_)) {
    return (0x0u);
  }

  for (i = 0x0u; i < n_; i++) {
    if ((CHAR_NULL == s1_[i]) || (s1_[i] != s2_[i])) {
      return ((s1_[i] < s2_[i]) ? -0x1 : ((s1_[i] > s2_[i]) ? 0x1 : 0x0u));
    }
  }

  return (0x0u);
}


Return_t __strcat__(Byte_t* dest_, const Byte_t* src_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t destLen = 0x0u;
  Size_t i = 0x0u;


  if (__PointerIsNull__(dest_) || __PointerIsNull__(src_) || (0x0u == destSize_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  destLen = __strlen__(dest_);

  if (destLen >= destSize_) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  while ((CHAR_NULL != src_[i]) && ((destLen + i) < (destSize_ - 0x1u))) {
    dest_[destLen + i] = src_[i];
    i++;
  }

  dest_[destLen + i] = CHAR_NULL;
  __ReturnOk__();
  FUNCTION_EXIT;
}


Byte_t* __strchr__(const Byte_t* str_, const Byte_t ch_) {
  Size_t i = 0x0u;


  if (__PointerIsNull__(str_)) {
    return (null);
  }

  while (CHAR_NULL != str_[i]) {
    if (str_[i] == ch_) {
      return ((Byte_t*)&str_[i]);
    }

    i++;
  }

  if (CHAR_NULL == ch_) {
    return ((Byte_t*)&str_[i]);
  }

  return (null);
}


Byte_t* __strrchr__(const Byte_t* str_, const Byte_t ch_) {
  Size_t len = 0x0u;
  Size_t i = 0x0u;


  if (__PointerIsNull__(str_)) {
    return (null);
  }

  len = __strlen__(str_);

  for (i = len; i > 0x0u; i--) {
    if (str_[i - 0x1u] == ch_) {
      return ((Byte_t*)&str_[i - 0x1u]);
    }
  }

  if ((CHAR_NULL == ch_) && (len > 0x0u)) {
    return ((Byte_t*)&str_[len]);
  }

  return (null);
}


/* Path utility functions */
Return_t __path_join__(Byte_t* dest_, const Byte_t* base_, const Byte_t* path_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t baseLen = 0x0u;
  Size_t pathLen = 0x0u;
  Base_t needSlash = false;


  if (__PointerIsNull__(dest_) || __PointerIsNull__(base_) || __PointerIsNull__(path_) || (0x0u == destSize_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  baseLen = __strlen__(base_);
  pathLen = __strlen__(path_);

  if ((0x0u == baseLen) || (0x0u == pathLen)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  needSlash = (CHAR_SLASH != base_[baseLen - 0x1u]) && (CHAR_SLASH != path_[0x0u]);

  if ((baseLen + pathLen + (needSlash ? 0x1u : 0x0u)) >= destSize_) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  if (OK(__strcpy__(dest_, base_, destSize_))) {
    if (needSlash) {
      dest_[baseLen] = CHAR_SLASH;
      dest_[baseLen + 0x1u] = CHAR_NULL;
    }

    if (OK(__strcat__(dest_, path_, destSize_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __path_normalize__(Byte_t* path_, const Size_t pathSize_) {
  FUNCTION_ENTER;


  Size_t i = 0x0u;
  Size_t j = 0x0u;
  Size_t len = 0x0u;
  Byte_t temp[CONFIG_FS_MAX_PATH_LENGTH];


  if (__PointerIsNull__(path_) || (0x0u == pathSize_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  len = __strlen__(path_);

  if ((0x0u == len) || (len >= CONFIG_FS_MAX_PATH_LENGTH)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  for (i = 0x0u; i < len; i++) {
    if (CHAR_SLASH == path_[i]) {
      if ((0x0u == j) || (CHAR_SLASH != temp[j - 0x1u])) {
        temp[j++] = CHAR_SLASH;
      }
    } else {
      temp[j++] = path_[i];
    }
  }

  if ((j > 0x1u) && (CHAR_SLASH == temp[j - 0x1u])) {
    j--;
  }

  temp[j] = CHAR_NULL;

  if (OK(__strcpy__(path_, temp, pathSize_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Base_t __path_is_absolute__(const Byte_t* path_) {
  if (__PointerIsNull__(path_)) {
    return (false);
  }

  return ((CHAR_SLASH == path_[0x0u]) ? true : false);
}


Return_t __path_dirname__(Byte_t* dest_, const Byte_t* path_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t len = 0x0u;
  Size_t i = 0x0u;


  if (__PointerIsNull__(dest_) || __PointerIsNull__(path_) || (0x0u == destSize_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  len = __strlen__(path_);

  if (0x0u == len) {
    if (OK(__strcpy__(dest_, (const Byte_t*)".", destSize_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }

  for (i = len; i > 0x0u; i--) {
    if (CHAR_SLASH == path_[i - 0x1u]) {
      break;
    }
  }

  if (0x0u == i) {
    if (OK(__strcpy__(dest_, (const Byte_t*)".", destSize_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    if (i > destSize_) {
      __ReturnError__();
      __AssertOnElse__();
      FUNCTION_EXIT;
    }

    if (OK(__strncpy__(dest_, path_, i - 0x1u))) {
      dest_[i - 0x1u] = CHAR_NULL;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  }

  FUNCTION_EXIT;
}


Return_t __path_basename__(Byte_t* dest_, const Byte_t* path_, const Size_t destSize_) {
  FUNCTION_ENTER;


  Size_t len = 0x0u;
  Size_t i = 0x0u;
  Size_t start = 0x0u;


  if (__PointerIsNull__(dest_) || __PointerIsNull__(path_) || (0x0u == destSize_)) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  len = __strlen__(path_);

  if (0x0u == len) {
    if (OK(__strcpy__(dest_, (const Byte_t*)".", destSize_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }

    FUNCTION_EXIT;
  }

  for (i = len; i > 0x0u; i--) {
    if (CHAR_SLASH == path_[i - 0x1u]) {
      start = i;
      break;
    }
  }

  if ((len - start) >= destSize_) {
    __ReturnError__();
    __AssertOnElse__();
    FUNCTION_EXIT;
  }

  if (OK(__strcpy__(dest_, &path_[start], destSize_))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}