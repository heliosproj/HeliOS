/*UNCRUSTIFY-OFF*/
/**
 * @file mem.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for memory management
 * @version 0.5.0
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
#include "mem.h"


/*UNCRUSTIFY-OFF*/
/*
 * WARNING: THIS DIAGRAM IS CONCEPTUAL ONLY AS IT DOES *NOT* CORRECTLY DEPICT
 *          MEMORY-ACCESS OPTIMIZATION BY ALIGNMENT CARRIED OUT BY THE COMPILER!
 * 
 *  BYTE   STRUCTURE OF A MEMORY ENTRY AND BLOCKS
 * +----+ +--------------------------------------+
 * | 01 | |                                      | <-- START OF MEMORY ENTRY
 * +----+ |                                      |
 * | 02 | |                                      |
 * +----+ |  MAGIC (4 BYTES)                     |
 * | 03 | |                                      |
 * +----+ |                                      |
 * | 04 | |                                      |
 * +----+ +--------------------------------------+
 * | 05 | |  FREE (1 BYTE)                       |
 * +----+ +--------------------------------------+
 * | 06 | |                                      |
 * +----+ |  BLOCKS (2 BYTES)                    |
 * | 07 | |                                      |
 * +----+ +--------------------------------------+
 * | 08 | |                                      |
 * +----+ |                                      |
 * | 09 | |                                      |
 * +----+ |                                      |
 * | 10 | |                                      |
 * +----+ |                                      |
 * | 11 | |                                      |
 * +----+ |  NEXT (4 - 8 BYTES)                  |
 * | 12 | |                                      |
 * +----+ |                                      |
 * | 13 | |                                      |
 * +----+ |                                      |
 * | 14 | |                                      |
 * +----+ |                                      |
 * | 15 | |                                      |
 * +----+ +--------------------------------------+
 * | 16 | |                                      |
 * +----+ |                                      |
 * | 17 | |                                      |
 * +----+ |                                      |
 * | 18 | |                                      |
 * +----+ |                                      |
 * | 19 | |                                      |
 * +----+ |                                      |
 * | 20 | |                                      |
 * +----+ |                                      |
 * | 21 | |                                      |
 * +----+ |                                      |
 * | 22 | |                                      |
 * +----+ |                                      |
 * | 23 | |                                      |
 * +----+ |  UNUSED (17 - 21 BYTES)              |
 * | 24 | |                                      |
 * +----+ |                                      |
 * | 25 | |                                      |
 * +----+ |                                      |
 * | 26 | |                                      |
 * +----+ |                                      |
 * | 27 | |                                      |
 * +----+ |                                      |
 * | 28 | |                                      |
 * +----+ |                                      |
 * | 29 | |                                      |
 * +----+ |                                      |
 * | 30 | |                                      |
 * +----+ |                                      |
 * | 31 | |                                      |
 * +----+ |                                      |
 * | 32 | |                                      | <-- END OF MEMORY ENTRY
 * +----+ +--------------------------------------+
 * | 33 | |                                      | <-- START OF ALLOCATED MEMORY BLOCK
 * +----+ |                                      |     (ADDRESS GIVEN TO CALLER BY __calloc__())
 * | 34 | |                                      |
 * +----+ |                                      |
 * | 35 | |                                      |
 * +----+ |                                      |
 * | 36 | |                                      |
 * +----+ |                                      |
 * | 37 | |                                      |
 * +----+ |                                      |
 * | 38 | |                                      |
 * +----+ |                                      |
 * | 39 | |                                      |
 * +----+ |                                      |
 * | 40 | |                                      |
 * +----+ |                                      |
 * | 41 | |                                      |
 * +----+ |                                      |
 * | 42 | |                                      |
 * +----+ |                                      |
 * | 43 | |                                      |
 * +----+ |                                      |
 * | 44 | |                                      |
 * +----+ |                                      |
 * | 45 | |                                      |
 * +----+ |                                      |
 * | 46 | |                                      |
 * +----+ |                                      |
 * | 47 | |                                      |
 * +----+ |                                      |
 * | 48 | |                                      |
 * +----+ |  ALLOCATED MEMORY BLOCK (32 BYTES)   |
 * | 49 | |                                      |
 * +----+ |                                      |
 * | 50 | |                                      |
 * +----+ |                                      |
 * | 51 | |                                      |
 * +----+ |                                      |
 * | 52 | |                                      |
 * +----+ |                                      |
 * | 53 | |                                      |
 * +----+ |                                      |
 * | 54 | |                                      |
 * +----+ |                                      |
 * | 55 | |                                      |
 * +----+ |                                      |
 * | 56 | |                                      |
 * +----+ |                                      |
 * | 57 | |                                      |
 * +----+ |                                      |
 * | 58 | |                                      |
 * +----+ |                                      |
 * | 59 | |                                      |
 * +----+ |                                      |
 * | 50 | |                                      |
 * +----+ |                                      |
 * | 61 | |                                      |
 * +----+ |                                      |
 * | 62 | |                                      |
 * +----+ |                                      |
 * | 63 | |                                      |
 * +----+ |                                      |
 * | 64 | |                                      | <-- END OF ALLOCATED MEMORY BLOCK
 * +----+ +--------------------------------------+
 */
/*UNCRUSTIFY-ON*/
static volatile MemoryRegion_t heap;
static volatile MemoryRegion_t kernel;
static Return_t __MemoryRegionCheck__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, const Base_t option_);
static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_);
static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_);
static Return_t __DefragMemoryRegion__(const volatile MemoryRegion_t *region_);
static Return_t __MemoryRegionInit__(volatile MemoryRegion_t *region_);
static Return_t __DetectByteOrder__(ByteOrder_t *order_);


#define __OffsetPointerToMemEntry__(ptr_, region_) ((MemoryEntry_t *) (((Byte_t *) (ptr_)) - ((region_)->entrySize * CONFIG_MEMORY_REGION_BLOCK_SIZE)))


#define __OffsetMemEntryToPointer__(ptr_, region_) ((Addr_t *) (((Byte_t *) (ptr_)) + ((region_)->entrySize * CONFIG_MEMORY_REGION_BLOCK_SIZE)))


#define __CalculateMemEntryMagic__(ptr_) (((Word_t) (ptr_)) ^ MAGIC_CONST)


#define __MemEntryMagicOk__(ptr_) (__CalculateMemEntryMagic__(ptr_) == (ptr_)->magic)


#define __PointerInRegionBounds__(region_, ptr_) (((const volatile Addr_t *) (ptr_) >= (Addr_t *) ((region_)->mem)) && ((const volatile Addr_t *) (ptr_) < \
        (Addr_t *) ((region_)->mem + MEMORY_REGION_SIZE_IN_BYTES)))


#define __MemEntryIsInUse__(ptr_) (INUSE == (ptr_)->free)


#define __MemEntryIsFree__(ptr_) (FREE == (ptr_)->free)


Return_t __MemoryInit__(void) {
  FUNCTION_ENTER;


  ByteOrder_t order;


  /* Initialize the heap and kernel memory regions. */
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


Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(addr_) && (nil < size_)) {
    /* Simply passthrough the address pointer and size parameters to
     * __calloc__() for the heap memory region since xMemAlloc() can only
     * allocate heap memory. */
    if(OK(__calloc__(&heap, addr_, size_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemFree(const volatile Addr_t *addr_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(addr_)) {
    /* Simply passthrough the address pointer to __free__() for the heap memory
     * region since xMemFree() can only free heap memory. */
    if(OK(__free__(&heap, addr_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
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


  MemoryEntry_t *cursor = null;
  HalfWord_t used = nil;


  if(__PointerIsNotNull__(size_)) {
    /* Check the consistency of the heap memory region. */
    if(OK(__MemoryRegionCheck__(&heap, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      cursor = heap.start;

      /* Traverse, while the cursor is not null, the memory entries in the heap
       * memory region and add up the in-use blocks as we go. */
      while(__PointerIsNotNull__(cursor)) {
        /* If the memory entry is *NOT* free, then add the number of blocks it
         * contains to the in-use count. */
        if(__MemEntryIsInUse__(cursor)) {
          used += cursor->blocks;
        }

        cursor = cursor->next;
      }

      /* We need to give the user back bytes, not blocks, so multiply the in-use
       * blocks by the block size in bytes. */
      *size_ = used * CONFIG_MEMORY_REGION_BLOCK_SIZE;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_) {
  FUNCTION_ENTER;


  MemoryEntry_t *entry = null;


  if(__PointerIsNotNull__(addr_) && __PointerIsNotNull__(size_)) {
    /* Check the consistency of the heap memory region *AND* check the address
     * pointer to ensure it is pointing to a valid block of heap memory. */
    if(OK(__MemoryRegionCheck__(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      /* __OffsetPointerToMemEntry__() calculates the location of the memory
      * entry for the allocated memory pointed to by the address pointer. */
      entry = __OffsetPointerToMemEntry__(addr_, &heap);

      /* If the memory entry pointed to by tosize is *NOT* free, then give the
       * user back the number of bytes in-use by multiply the blocks contained
       * in the entry by the block size in bytes. */
      if(__MemEntryIsInUse__(entry)) {
        *size_ = entry->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
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


static Return_t __MemoryRegionCheck__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, const Base_t option_) {
  FUNCTION_ENTER;


  Base_t found = false;
  HalfWord_t blocks = nil;
  MemoryEntry_t *entry = null;
  MemoryEntry_t *cursor = region_->start;


  /* Check to see if we can proceed with checking the memory region without
   * looking for an address.*/
  if(MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) {
    /* Traverse the memory entries in the memory region while cursor is null. */
    while(__PointerIsNotNull__(cursor)) {
      /* __PointerInRegionBounds__() is a C macro that simply checks that the
       * address, in this case
       * "cursor", falls within the bounds of the memory region. */
      if(__PointerInRegionBounds__(region_, cursor)) {
        /* __MemEntryMagicOk__() compares the memory entry's magic value (i.e.,
         * the magic member of the memory entry structure) to the magic value
         * calculated by XOR'ing the address of the memory entry with the
         * MAGIC_CONST. This operation helps ensure we are accessing a valid
         * memory entry in the memory region being checked. */
        if(__MemEntryMagicOk__(cursor)) {
          /* Check to make sure the memory entry's free value is either FREE or
           * INUSE.*/
          if(__MemEntryIsFree__(cursor) || __MemEntryIsInUse__(cursor)) {
            /* Add up the blocks as we go. We will check to make sure the total
             * number of blocks in the memory region is correct later. */
            blocks += cursor->blocks;
          } else {
            __AssertOnElse__();


            /* "Houston, we've had a problem." ~ Jim Lovell
             *
             *
             * Set the memfault flag to true because the address we just checked
             * does *NOT* have the correct value for free. Something is very
             * wrong! */
            __SetFlag__(MEMFAULT);
            break;
          }
        } else {
          __AssertOnElse__();


          /* "Houston, we've had a problem." ~ Jim Lovell
           *
           *
           * Set the memfault flag to true because the address we just checked
           * does *NOT* have the correct magic value. Something is very wrong!
           */
          __SetFlag__(MEMFAULT);
          break;
        }

        cursor = cursor->next;
      } else {
        __AssertOnElse__();


        /* "Houston, we've had a problem." ~ Jim Lovell
         *
         *
         * Set the memfault flag to true because the address we just checked is
         * NOT* inside the memory region. Something is very wrong!
         */
        __SetFlag__(MEMFAULT);
        break;
      }
    }

    /* Check that the number of blocks we visited matches what we expect to see
     */
    if(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();


      /* "Houston, we've had a problem." ~ Jim Lovell
       *
       *
       * Set the memfault flag to true because the number of blocks visited does
       * not match the number of blocks the memory region *SHOULD*
       * have. Something is very wrong!
       */
      __SetFlag__(MEMFAULT);
    }

    /* Check to see if we need to look for an address while we check the
     * consistency of the memory region. */
  } else if(MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) {
    /* __OffsetPointerToMemEntry__() calculates the location of the memory entry
     * for the allocated memory pointed to by the address pointer. This is the
     * memory entry we need to find as we traverse the memory entries in the
     * memory region. */
    entry = __OffsetPointerToMemEntry__(addr_, region_);

    /* Traverse the memory entries in the memory region while cursor is null. */
    while(__PointerIsNotNull__(cursor)) {
      /* __PointerInRegionBounds__() is a C macro that simply checks that the
       * address, in this case
       * "cursor", falls within the bounds of the memory region. */
      if(__PointerInRegionBounds__(region_, cursor)) {
        /* __MemEntryMagicOk__() compares the memory entry's magic value (i.e.,
         * the magic member of the memory entry structure) to the magic value
         * calculated by XOR'ing the address of the memory entry with the
         * MAGIC_CONST. This operation helps ensure we are accessing a valid
         * memory entry in the memory region being checked. */
        if(__MemEntryMagicOk__(cursor)) {
          /* Check to make sure the memory entry's free value is either FREE or
           * INUSE.*/
          if(__MemEntryIsFree__(cursor) || __MemEntryIsInUse__(cursor)) {
            /* Add up the blocks as we go. We will check to make sure the total
             * number of blocks in the memory region is correct later. */
            blocks += cursor->blocks;

            /* If the cursor points to the memory entry we are looking for *AND*
             * the memory entry is marked as in-use, then set "found" to true
             * because we found the memory entry we are looking for. */
            if((entry == cursor) && __MemEntryIsInUse__(cursor)) {
              found = true;
            }
          } else {
            __AssertOnElse__();


            /* "Houston, we've had a problem." ~ Jim Lovell
             *
             *
             * Set the memfault flag to true because the address we just checked
             * does *NOT* have the correct value for free. Something is very
             * wrong! */
            __SetFlag__(MEMFAULT);
            break;
          }
        } else {
          __AssertOnElse__();


          /* "Houston, we've had a problem." ~ Jim Lovell
           *
           *
           * Set the memfault flag to true because the address we just checked
           * does *NOT* have the correct magic value. Something is very wrong!
           */
          __SetFlag__(MEMFAULT);
          break;
        }

        cursor = cursor->next;
      } else {
        __AssertOnElse__();


        /* "Houston, we've had a problem." ~ Jim Lovell
         *
         *
         * Set the memfault flag to true because the address we just checked is
         * NOT* inside the memory region. Something is very wrong!
         */
        __SetFlag__(MEMFAULT);
        break;
      }
    }

    /* Check that the number of blocks we visited matches what we expect to see
     */
    if(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks) {
      /* Before we can __ReturnOk__(), we just need to check to make sure we
       * found the address we were looking for as we traversed the memory
       * region. */
      if(true == found) {
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();


      /* "Houston, we've had a problem." ~ Jim Lovell
       *
       *
       * Set the memfault flag to true because the number of blocks visited does
       * not match the number of blocks the memory region *SHOULD*
       * have. Something is very wrong!
       */
      __SetFlag__(MEMFAULT);
    }
  } else {
    /* If we made it here, "option_" did not contain a valid argument. */
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __MemoryRegionInit__(volatile MemoryRegion_t *region_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(region_)) {
    /* Set the start of the region. */
    region_->start = (MemoryEntry_t *) region_->mem;


    /* Set the starting value of minimum available ever to the size, in bytes,
     * of the memory region. */
    region_->minAvailableEver = MEMORY_REGION_SIZE_IN_BYTES;


    /* Calculate the size of a memory entry in blocks. */
    region_->entrySize = ((HalfWord_t) (sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE));

    /* If there is any remainder from the division, add another block to the
     * memory entry size. */
    if(nil < ((HalfWord_t) (sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
      region_->entrySize++;
    }

    /* Zero out the memory region and create the first memory entry and give it
     * all of the blocks.*/
    if(OK(__memset__(region_->mem, nil, MEMORY_REGION_SIZE_IN_BYTES))) {
      /* __CalculateMemEntryMagic__() calculates the memory entry's magic value
       * (i.e. the magic member of the memory entry structure) by XOR'ing the
       * address of the memory entry with the MAGIC_CONST. The magic value is
       * used by __MemoryRegionCheck__() to check the consistency of the memory
       * region.
       */
      region_->start->magic = __CalculateMemEntryMagic__(region_->start);
      region_->start->free = FREE;
      region_->start->blocks = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS;
      region_->start->next = null;
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


  HalfWord_t requested = nil;
  HalfWord_t free = nil;


  /* Intentionally underflow the unsigned type so we get the max value of a
   * HalfWord_t. */
  HalfWord_t fewest = -1;
  MemoryEntry_t *cursor = null;
  MemoryEntry_t *candidate = null;
  MemoryEntry_t *next = null;


  /* Because we are modifying memory entries, we need to disable interrupts
   * until __calloc__() is done. */
  __DisableInterrupts__();

  if(__PointerIsNotNull__(region_) && __PointerIsNotNull__(addr_) && (nil < size_)) {
    /* Check the consistency of the memory region before we modify anything. */
    if(OK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      /* Because the user supplied requested memory in bytes, calculate how many
       * blocks have been requested. */
      requested = ((HalfWord_t) (size_ / CONFIG_MEMORY_REGION_BLOCK_SIZE));

      if(nil < ((HalfWord_t) (size_ % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
        requested++;
      }

      /* Add the number of blocks(s) required by the memory entry to the
       * requested blocks. This is the total number of free blocks that will be
       * needed. */
      requested += region_->entrySize;
      cursor = region_->start;

      while(__PointerIsNotNull__(cursor)) {
        /* See if we have a possible candidate entry to use for the requested
         * blocks. To be a candidate the entry must:
         *  1. Be free.
         *  2. Must contain enough blocks to cover the request.
         *  3. Must be an entry with the fewest blocks (this is to reduce
         * fragmentation). */
        if(__MemEntryIsFree__(cursor) && (requested <= cursor->blocks) && (fewest > cursor->blocks)) {
          fewest = cursor->blocks;
          candidate = cursor;
        }

        /* Keep track of how many free blocks remain as we need to update the
         * statistics for the memory region later. */
        if(__MemEntryIsFree__(cursor)) {
          free += cursor->blocks;
        }

        cursor = cursor->next;
      }

      if(__PointerIsNotNull__(candidate)) {
        /* If the candidate entry contains enough blocks for a memory entry and
         * at least one additional block then we are going to split the memory
         * entry into two. If not, we will just go ahead and use the memory
         * entry as is. */
        if((region_->entrySize + 1) <= (candidate->blocks - requested)) {
          /* This block of code splits the block in two and uses the first of
           * the two blocks for the requested memory. */
          next = candidate->next;
          candidate->next = (MemoryEntry_t *) ((Byte_t *) candidate + (requested * CONFIG_MEMORY_REGION_BLOCK_SIZE));


          /* __CalculateMemEntryMagic__() calculates the memory entry's magic
           * value (i.e. the magic member of the memory entry structure) by
           * XOR'ing the address of the memory entry with the MAGIC_CONST. The
           * magic value is used by __MemoryRegionCheck__() to check the
           * consistency of the memory region. */
          candidate->next->magic = __CalculateMemEntryMagic__(candidate->next);
          candidate->next->free = FREE;
          candidate->next->blocks = candidate->blocks - requested;
          candidate->next->next = next;


          /* We split the unneeded blocks off into a new entry, now let's mark
           * the entry containing the blocks in-use for the requested memory. */
          candidate->magic = __CalculateMemEntryMagic__(candidate);
          candidate->free = INUSE;
          candidate->blocks = requested;

          /* Zero out all of the requested blocks (excluding the memory entry).
           */
          if(OK(__memset__(__OffsetMemEntryToPointer__(candidate, region_), nil, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
            /* __OffsetMemEntryToPointer__() does the opposite of
             * __OffsetPointerToMemEntry__(), it converts the memory entry
             * address to the address of the first block after the memory entry.
             */
            *addr_ = __OffsetMemEntryToPointer__(candidate, region_);
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {
          /* Because we didn't need to split an entry into two, we just need to
           * mark the entry as in-use and that's it. */
          candidate->free = INUSE;

          /* Zero out all of the requested blocks (excluding the memory
           * entry).*/
          if(OK(__memset__(__OffsetMemEntryToPointer__(candidate, region_), nil, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
            /* __OffsetMemEntryToPointer__() does the opposite of
             * __OffsetPointerToMemEntry__(), it converts the memory entry
             * address to the address of the first block after the memory entry.
             */
            *addr_ = __OffsetMemEntryToPointer__(candidate, region_);
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        }

        /* Update the statistics for the memory region before we are done. */
        region_->allocations++;
        free -= requested;

        if((free * CONFIG_MEMORY_REGION_BLOCK_SIZE) < region_->minAvailableEver) {
          region_->minAvailableEver = (free * CONFIG_MEMORY_REGION_BLOCK_SIZE);
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

  /* __calloc__() is done so re-enable interrupts. */
  __EnableInterrupts__();
  FUNCTION_EXIT;
}


static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {
  FUNCTION_ENTER;


  MemoryEntry_t *entry = null;


  /* Because we are modifying memory entries, we need to disable interrupts
   * until __free__() is done. */
  __DisableInterrupts__();

  if(__PointerIsNotNull__(region_) && __PointerIsNotNull__(addr_)) {
    /* Check the consistency of the heap memory region *AND* check the address
     * pointer to ensure it is pointing to a valid block of heap memory. */
    if(OK(__MemoryRegionCheck__(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      /* __OffsetPointerToMemEntry__() calculates the location of the memory
      * entry for the allocated memory pointed to by the address pointer. */
      entry = __OffsetPointerToMemEntry__(addr_, region_);
      entry->free = FREE;
      region_->frees++;

      /* After freeing memory, call __DefragMemoryRegion__() to consolidate any
       * adjacent free blocks. */
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

  /* __free__() is done so re-enable interrupts. */
  __EnableInterrupts__();
  FUNCTION_EXIT;
}


Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(addr_) && (nil < size_)) {
    /* Simply passthrough the address pointer to __calloc__() for the kernel
     * memory region and the size of the requested memory. */
    if(OK(__calloc__(&kernel, addr_, size_))) {
      if(__PointerIsNotNull__(*addr_)) {
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


Return_t __KernelFreeMemory__(const volatile Addr_t *addr_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(addr_)) {
    /* Simply passthrough the address pointer to __free__() for the kernel
     * memory region. */
    if(OK(__free__(&kernel, addr_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __MemoryRegionCheckKernel__(const volatile Addr_t *addr_, const Base_t option_) {
  FUNCTION_ENTER;

  if((__PointerIsNull__(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (__PointerIsNotNull__(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR ==
    option_))) {
    /* Simply passthrough the address pointer to __MemoryRegionCheck__() for the
     * kernel memory region and the region check option. */
    if(OK(__MemoryRegionCheck__(&kernel, addr_, option_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(addr_) && (nil < size_)) {
    /* Simply passthrough the address pointer to __calloc__() for the heap
     * memory region and the size of the requested memory. */
    if(OK(__calloc__(&heap, addr_, size_))) {
      if(__PointerIsNotNull__(*addr_)) {
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


Return_t __HeapFreeMemory__(const volatile Addr_t *addr_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(addr_)) {
    /* Simply passthrough the address pointer to __free__() for the heap memory
     * region. */
    if(OK(__free__(&heap, addr_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t __MemoryRegionCheckHeap__(const volatile Addr_t *addr_, const Base_t option_) {
  FUNCTION_ENTER;

  if((__PointerIsNull__(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (__PointerIsNotNull__(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR ==
    option_))) {
    /* Simply passthrough the address pointer to __MemoryRegionCheck__() for the
     * heap memory region and the region check option. */
    if(OK(__MemoryRegionCheck__(&heap, addr_, option_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
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
    /* Set res_ to true by default which indicates the memory is comparable. If
     * we later discover the memory is *NOT* comparable, we will set res_ to
     * false. */
    *res_ = true;
    s1 = (Byte_t *) s1_;
    s2 = (Byte_t *) s2_;

    for(i = nil; i < size_; i++) {
      if(*s1 != *s2) {
        /* The memory is *NOT* comparable so set res_ to false. */
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


Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stats_)) {
    /* Simply passthrough the address pointer to __MemGetRegionStats__() for the
     * heap memory region and a pointer to the stats structure. */
    if(OK(__MemGetRegionStats__(&heap, stats_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(stats_)) {
    /* Simply passthrough the address pointer to __MemGetRegionStats__() for the
     * kernel memory region and a pointer to the stats structure. */
    if(OK(__MemGetRegionStats__(&kernel, stats_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;


  MemoryEntry_t *cursor = null;


  if(__PointerIsNotNull__(region_) && __PointerIsNotNull__(stats_)) {
    /* Check the memory region consistency before we calculate the statistics
     * for the memory region. */
    if(OK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      /* Allocate some heap memory to hold the memory region statistics
       * structure. */
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) stats_, sizeof(MemoryRegionStats_t)))) {
        cursor = region_->start;

        if(OK(__memset__(*stats_, nil, sizeof(MemoryRegionStats_t)))) {
          /* We intentionally underflow a word (an unsigned type) to get its
           * maximum value. */
          (*stats_)->smallestFreeEntryInBytes = -1;


          /* Copy in the statistics we already have from the memory region. */
          (*stats_)->successfulAllocations = region_->allocations;
          (*stats_)->successfulFrees = region_->frees;
          (*stats_)->minimumEverFreeBytesRemaining = region_->minAvailableEver;

          /* Traverse the memory region to calculate the remaining statistics.
           */
          while(__PointerIsNotNull__(cursor)) {
            if(__MemEntryIsFree__(cursor)) {
              if((*stats_)->largestFreeEntryInBytes < (cursor->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE)) {
                (*stats_)->largestFreeEntryInBytes = cursor->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
              }

              if((*stats_)->smallestFreeEntryInBytes > (cursor->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE)) {
                (*stats_)->smallestFreeEntryInBytes = cursor->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
              }

              (*stats_)->numberOfFreeBlocks += cursor->blocks;
            }

            (*stats_)->availableSpaceInBytes = (*stats_)->numberOfFreeBlocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
            cursor = cursor->next;
          }

          __ReturnOk__();
        } else {
          __AssertOnElse__();


          /* Free the heap memory because the call to __memset__() failed. */
          __HeapFreeMemory__(stats_);
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


static Return_t __DefragMemoryRegion__(const volatile MemoryRegion_t *region_) {
  FUNCTION_ENTER;


  MemoryEntry_t *cursor = null;
  MemoryEntry_t *entry = null;


  if(__PointerIsNotNull__(region_)) {
    /* Check the memory region consistency before we attempt to defrag the
     * memory region. */
    if(OK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      cursor = region_->start;

      while(__PointerIsNotNull__(cursor)) {
        /* We will merge the blocks from two adjacent memory entries if:
         *  1. The cursor is pointing to an entry.
         *  2. "next" points to an entry.
         *  3. The entry pointed to be the cursor is free.
         *  4. The entry pointed to be "next" is free. */
        if(__PointerIsNotNull__(cursor) && __PointerIsNotNull__(cursor->next) && __MemEntryIsFree__(cursor) && __MemEntryIsFree__(cursor->next)) {
          entry = cursor->next;


          /* __CalculateMemEntryMagic__() calculates the memory entry's magic
           * value (i.e. the magic member of the memory entry structure) by
           * XOR'ing the address of the memory entry with the MAGIC_CONST. The
           * magic value is used by __MemoryRegionCheck__() to check the
           * consistency of the memory region. */
          cursor->magic = __CalculateMemEntryMagic__(cursor);
          cursor->free = FREE;
          cursor->blocks += entry->blocks;
          cursor->next = entry->next;

          /* Zero out the block formerly occupied by the memory entry that was
           * merged. */
          if(OK(__memset__(entry, nil, sizeof(MemoryEntry_t)))) {
            /* Do nothing - literally. */
          } else {
            __AssertOnElse__();
            break;
          }
        } else {
          cursor = cursor->next;
        }
      }

      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __DetectByteOrder__(ByteOrder_t *order_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(order_)) {
    if(0x100 > (*(uint16_t *) "\xFF\x00")) {
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


  /* For unit testing only! */
  void __MemoryClear__(void) {
    __MemoryInit__();

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */