/*UNCRUSTIFY-OFF*/
/**
 * @file mem.c
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


Return_t __MemoryInit__(void) {
  RET_DEFINE;


  /* Initialize the heap and kernel memory regions. */
  if(OK(__MemoryRegionInit__(&heap))) {
    if(OK(__MemoryRegionInit__(&kernel))) {
      FLAG_MEMFAULT = false;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_) {
  RET_DEFINE;

  if(NOTNULLPTR(addr_) && (zero < size_)) {
    /* Simply passthrough the address pointer and size parameters to
     * __calloc__() for the heap memory region since xMemAlloc() can only
     * allocate heap memory. */
    if(OK(__calloc__(&heap, addr_, size_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xMemFree(const volatile Addr_t *addr_) {
  RET_DEFINE;

  if(NOTNULLPTR(addr_)) {
    /* Simply passthrough the address pointer to __free__() for the heap memory
     * region since xMemFree() can only free heap memory. */
    if(OK(__free__(&heap, addr_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xMemGetUsed(Size_t *size_) {
  RET_DEFINE;


  MemoryEntry_t *cursor = null;
  HalfWord_t used = zero;


  if(NOTNULLPTR(size_)) {
    /* Check the consistency of the heap memory region. */
    if(OK(__MemoryRegionCheck__(&heap, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      cursor = heap.start;


      /* Traverse, while the cursor is not null, the memory entries in the heap
       * memory region and add up the in-use blocks as we go. */
      while(NOTNULLPTR(cursor)) {
        /* If the memory entry is *NOT* free, then add the number of blocks it
         * contains to the in-use count. */
        if(INUSE == cursor->free) {
          used += cursor->blocks;
        }

        cursor = cursor->next;
      }


      /* We need to give the user back bytes, not blocks, so multiply the in-use
       * blocks by the block size in bytes. */
      *size_ = used * CONFIG_MEMORY_REGION_BLOCK_SIZE;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_) {
  RET_DEFINE;


  MemoryEntry_t *tosize = null;


  if(NOTNULLPTR(addr_) && NOTNULLPTR(size_)) {
    /* Check the consistency of the heap memory region *AND* check the address
     * pointer to ensure it is pointing to a valid block of heap memory. */
    if(OK(__MemoryRegionCheck__(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      /* ADDR2ENTRY() calculates the location of the memory entry for the
       * allocated memory pointed to by the address pointer. */
      tosize = ADDR2ENTRY(addr_, &heap);


      /* If the memory entry pointed to by tosize is *NOT* free, then give the
       * user back the number of bytes in-use by multiply the blocks contained
       * in the entry by the block size in bytes. */
      if(INUSE == tosize->free) {
        *size_ = tosize->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __MemoryRegionCheck__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, const Base_t option_) {
  RET_DEFINE;


  Base_t found = false;
  HalfWord_t blocks = zero;
  MemoryEntry_t *find = null;
  MemoryEntry_t *cursor = region_->start;


  /* Check to see if we can proceed with checking the memory region without
   * looking for an address.*/
  if(MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) {
    /* Traverse the memory entries in the memory region while cursor is null. */
    while(NOTNULLPTR(cursor)) {
      /* OKADDR() is a C macro that simply checks that the address, in this case
       * "cursor", falls within the bounds of the memory region. */
      if(OKADDR(region_, cursor)) {
        /* OKMAGIC() compares the memory entry's magic value (i.e., the magic
         * member of the memory entry structure) to the magic value calculated
         * by XOR'ing the address of the memory entry with the MAGIC_CONST. This
         * operation helps ensure we are accessing a valid memory entry in the
         * memory region being checked. */
        if(OKMAGIC(cursor)) {
          /* Check to make sure the memory entry's free value is either FREE or
           * INUSE.*/
          if((FREE == cursor->free) || (INUSE == cursor->free)) {
            /* Add up the blocks as we go. We will check to make sure the total
             * number of blocks in the memory region is correct later. */
            blocks += cursor->blocks;
          } else {
            ASSERT;


            /* "Houston, we've had a problem." ~ Jim Lovell
             *
             *
             * Set the memfault flag to true because the address we just checked
             * does *NOT* have the correct value for free. Something is very
             * wrong! */
            FLAG_MEMFAULT = true;
            break;
          }
        } else {
          ASSERT;


          /* "Houston, we've had a problem." ~ Jim Lovell
           *
           *
           * Set the memfault flag to true because the address we just checked
           * does *NOT* have the correct magic value. Something is very wrong!
           */
          FLAG_MEMFAULT = true;
          break;
        }

        cursor = cursor->next;
      } else {
        ASSERT;


        /* "Houston, we've had a problem." ~ Jim Lovell
         *
         *
         * Set the memfault flag to true because the address we just checked is
         * NOT* inside the memory region. Something is very wrong!
         */
        FLAG_MEMFAULT = true;
        break;
      }
    }


    /* Check that the number of blocks we visited matches what we expect to see
     */
    if(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks) {
      RET_OK;
    } else {
      ASSERT;


      /* "Houston, we've had a problem." ~ Jim Lovell
       *
       *
       * Set the memfault flag to true because the number of blocks visited does
       * not match the number of blocks the memory region *SHOULD*
       * have. Something is very wrong!
       */
      FLAG_MEMFAULT = true;
    }


    /* Check to see if we need to look for an address while we check the
     * consistency of the memory region. */
  } else if(MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) {
    /* ADDR2ENTRY() calculates the location of the memory entry for the
     * allocated memory pointed to by the address pointer. This is the memory
     * entry we need to find as we traverse the memory entries in the memory
     * region. */
    find = ADDR2ENTRY(addr_, region_);


    /* Traverse the memory entries in the memory region while cursor is null. */
    while(NOTNULLPTR(cursor)) {
      /* OKADDR() is a C macro that simply checks that the address, in this case
       * "cursor", falls within the bounds of the memory region. */
      if(OKADDR(region_, cursor)) {
        /* OKMAGIC() compares the memory entry's magic value (i.e., the magic
         * member of the memory entry structure) to the magic value calculated
         * by XOR'ing the address of the memory entry with the MAGIC_CONST. This
         * operation helps ensure we are accessing a valid memory entry in the
         * memory region being checked. */
        if(OKMAGIC(cursor)) {
          /* Check to make sure the memory entry's free value is either FREE or
           * INUSE.*/
          if((FREE == cursor->free) || (INUSE == cursor->free)) {
            /* Add up the blocks as we go. We will check to make sure the total
             * number of blocks in the memory region is correct later. */
            blocks += cursor->blocks;


            /* If the cursor points to the memory entry we are looking for *AND*
             * the memory entry is marked as in-use, then set "found" to true
             * because we found the memory entry we are looking for. */
            if((find == cursor) && (INUSE == cursor->free)) {
              found = true;
            }
          } else {
            ASSERT;


            /* "Houston, we've had a problem." ~ Jim Lovell
             *
             *
             * Set the memfault flag to true because the address we just checked
             * does *NOT* have the correct value for free. Something is very
             * wrong! */
            FLAG_MEMFAULT = true;
            break;
          }
        } else {
          ASSERT;


          /* "Houston, we've had a problem." ~ Jim Lovell
           *
           *
           * Set the memfault flag to true because the address we just checked
           * does *NOT* have the correct magic value. Something is very wrong!
           */
          FLAG_MEMFAULT = true;
          break;
        }

        cursor = cursor->next;
      } else {
        ASSERT;


        /* "Houston, we've had a problem." ~ Jim Lovell
         *
         *
         * Set the memfault flag to true because the address we just checked is
         * NOT* inside the memory region. Something is very wrong!
         */
        FLAG_MEMFAULT = true;
        break;
      }
    }


    /* Check that the number of blocks we visited matches what we expect to see
     */
    if(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks) {
      /* Before we can RET_OK, we just need to check to make sure we found the
       * address we were looking for as we traversed the memory region. */
      if(true == found) {
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;


      /* "Houston, we've had a problem." ~ Jim Lovell
       *
       *
       * Set the memfault flag to true because the number of blocks visited does
       * not match the number of blocks the memory region *SHOULD*
       * have. Something is very wrong!
       */
      FLAG_MEMFAULT = true;
    }
  } else {
    /* If we made it here, "option_" did not contain a valid argument. */
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __MemoryRegionInit__(volatile MemoryRegion_t *region_) {
  RET_DEFINE;

  if(NOTNULLPTR(region_)) {
    /* Set the start of the region. */
    region_->start = (MemoryEntry_t *) region_->mem;


    /* Set the starting value of minimum available ever to the size, in bytes,
     * of the memory region. */
    region_->minAvailableEver = MEMORY_REGION_SIZE_IN_BYTES;


    /* Calculate the size of a memory entry in blocks. */
    region_->entrySize = ((HalfWord_t) (sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE));


    /* If there is any remainder from the division, add another block to the
     * memory entry size. */
    if(zero < ((HalfWord_t) (sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
      region_->entrySize++;
    }


    /* Zero out the memory region and create the first memory entry and give it
     * all of the blocks.*/
    if(OK(__memset__(region_->mem, zero, MEMORY_REGION_SIZE_IN_BYTES))) {
      /* CALCMAGIC() calculates the memory entry's magic value (i.e. the magic
       * member of the memory entry structure) by XOR'ing the address of the
       * memory entry with the MAGIC_CONST. The magic value is used by
       * __MemoryRegionCheck__() to check the consistency of the memory region.
       */
      region_->start->magic = CALCMAGIC(region_->start);
      region_->start->free = FREE;
      region_->start->blocks = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS;
      region_->start->next = null;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_) {
  RET_DEFINE;


  HalfWord_t requested = zero;
  HalfWord_t free = zero;


  /* Intentionally underflow the unsigned type so we get the max value of a
   * HalfWord_t. */
  HalfWord_t fewest = -1;
  MemoryEntry_t *cursor = null;
  MemoryEntry_t *candidate = null;
  MemoryEntry_t *next = null;


  /* Because we are modifying memory entries, we need to disable interrupts
   * until __calloc__() is done. */
  DISABLE_INTERRUPTS();

  if(NOTNULLPTR(region_) && NOTNULLPTR(addr_) && (zero < size_)) {
    /* Check the consistency of the memory region before we modify anything. */
    if(OK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      /* Because the user supplied requested memory in bytes, calculate how many
       * blocks have been requested. */
      requested = ((HalfWord_t) (size_ / CONFIG_MEMORY_REGION_BLOCK_SIZE));

      if(zero < ((HalfWord_t) (size_ % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
        requested++;
      }


      /* Add the number of blocks(s) required by the memory entry to the
       * requested blocks. This is the total number of free blocks that will be
       * needed. */
      requested += region_->entrySize;
      cursor = region_->start;

      while(NOTNULLPTR(cursor)) {
        /* See if we have a possible candidate entry to use for the requested
         * blocks. To be a candidate the entry must:
         *  1. Be free.
         *  2. Must contain enough blocks to cover the request.
         *  3. Must be an entry with the fewest blocks (this is to reduce
         * fragmentation). */
        if((FREE == cursor->free) && (requested <= cursor->blocks) && (fewest > cursor->blocks)) {
          fewest = cursor->blocks;
          candidate = cursor;
        }


        /* Keep track of how many free blocks remain as we need to update the
         * statistics for the memory region later. */
        if(FREE == cursor->free) {
          free += cursor->blocks;
        }

        cursor = cursor->next;
      }

      if(NOTNULLPTR(candidate)) {
        /* If the candidate entry contains enough blocks for a memory entry and
         * at least one additional block then we are going to split the memory
         * entry into two. If not, we will just go ahead and use the memory
         * entry as is. */
        if((region_->entrySize + 1) <= (candidate->blocks - requested)) {
          /* This block of code splits the block in two and uses the first of
           * the two blocks for the requested memory. */
          next = candidate->next;
          candidate->next = (MemoryEntry_t *) ((Byte_t *) candidate + (requested * CONFIG_MEMORY_REGION_BLOCK_SIZE));


          /* CALCMAGIC() calculates the memory entry's magic value (i.e. the
           * magic member of the memory entry structure) by XOR'ing the address
           * of the memory entry with the MAGIC_CONST. The magic value is used
           * by __MemoryRegionCheck__() to check the consistency of the memory
           * region. */
          candidate->next->magic = CALCMAGIC(candidate->next);
          candidate->next->free = FREE;
          candidate->next->blocks = candidate->blocks - requested;
          candidate->next->next = next;


          /* We split the unneeded blocks off into a new entry, now let's mark
           * the entry containing the blocks in-use for the requested memory. */
          candidate->magic = CALCMAGIC(candidate);
          candidate->free = INUSE;
          candidate->blocks = requested;


          /* Zero out all of the requested blocks (excluding the memory entry).
           */
          if(OK(__memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
            /* ENTRY2ADDR() does the opposite of ADDR2ENTRY(), it converts the
             * memory entry address to the address of the first block after the
             * memory entry. */
            *addr_ = ENTRY2ADDR(candidate, region_);
            RET_OK;
          } else {
            ASSERT;
          }
        } else {
          /* Because we didn't need to split an entry into two, we just need to
           * mark the entry as in-use and that's it. */
          candidate->free = INUSE;


          /* Zero out all of the requested blocks (excluding the memory
           * entry).*/
          if(OK(__memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
            /* ENTRY2ADDR() does the opposite of ADDR2ENTRY(), it converts the
             * memory entry address to the address of the first block after the
             * memory entry. */
            *addr_ = ENTRY2ADDR(candidate, region_);
            RET_OK;
          } else {
            ASSERT;
          }
        }


        /* Update the statistics for the memory region before we are done. */
        region_->allocations++;
        free -= requested;

        if((free * CONFIG_MEMORY_REGION_BLOCK_SIZE) < region_->minAvailableEver) {
          region_->minAvailableEver = (free * CONFIG_MEMORY_REGION_BLOCK_SIZE);
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }


  /* __calloc__() is done so re-enable interrupts. */
  ENABLE_INTERRUPTS();
  RET_RETURN;
}


static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {
  RET_DEFINE;


  MemoryEntry_t *free = null;


  /* Because we are modifying memory entries, we need to disable interrupts
   * until __free__() is done. */
  DISABLE_INTERRUPTS();

  if(NOTNULLPTR(region_) && NOTNULLPTR(addr_)) {
    /* Check the consistency of the heap memory region *AND* check the address
     * pointer to ensure it is pointing to a valid block of heap memory. */
    if(OK(__MemoryRegionCheck__(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      /* ADDR2ENTRY() calculates the location of the memory entry for the
       * allocated memory pointed to by the address pointer. */
      free = ADDR2ENTRY(addr_, region_);
      free->free = FREE;
      region_->frees++;


      /* After freeing memory, call __DefragMemoryRegion__() to consolidate any
       * adjacent free blocks. */
      if(OK(__DefragMemoryRegion__(region_))) {
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }


  /* __free__() is done so re-enable interrupts. */
  ENABLE_INTERRUPTS();
  RET_RETURN;
}


Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {
  RET_DEFINE;

  if(NOTNULLPTR(addr_) && (zero < size_)) {
    /* Simply passthrough the address pointer to __calloc__() for the kernel
     * memory region and the size of the requested memory. */
    if(OK(__calloc__(&kernel, addr_, size_))) {
      if(NOTNULLPTR(*addr_)) {
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t __KernelFreeMemory__(const volatile Addr_t *addr_) {
  RET_DEFINE;

  if(NOTNULLPTR(addr_)) {
    /* Simply passthrough the address pointer to __free__() for the kernel
     * memory region. */
    if(OK(__free__(&kernel, addr_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t __MemoryRegionCheckKernel__(const volatile Addr_t *addr_, const Base_t option_) {
  RET_DEFINE;

  if((NULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (NOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    /* Simply passthrough the address pointer to __MemoryRegionCheck__() for the
     * kernel memory region and the region check option. */
    if(OK(__MemoryRegionCheck__(&kernel, addr_, option_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {
  RET_DEFINE;

  if(NOTNULLPTR(addr_) && (zero < size_)) {
    /* Simply passthrough the address pointer to __calloc__() for the heap
     * memory region and the size of the requested memory. */
    if(OK(__calloc__(&heap, addr_, size_))) {
      if(NOTNULLPTR(*addr_)) {
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t __HeapFreeMemory__(const volatile Addr_t *addr_) {
  RET_DEFINE;

  if(NOTNULLPTR(addr_)) {
    /* Simply passthrough the address pointer to __free__() for the heap memory
     * region. */
    if(OK(__free__(&heap, addr_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t __MemoryRegionCheckHeap__(const volatile Addr_t *addr_, const Base_t option_) {
  RET_DEFINE;

  if((NULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (NOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    /* Simply passthrough the address pointer to __MemoryRegionCheck__() for the
     * heap memory region and the region check option. */
    if(OK(__MemoryRegionCheck__(&heap, addr_, option_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_) {
  RET_DEFINE;


  Size_t i = zero;
  volatile Byte_t *src = null;
  volatile Byte_t *dest = null;


  if(NOTNULLPTR(dest_) && NOTNULLPTR(src_) && (zero < size_)) {
    src = (Byte_t *) src_;
    dest = (Byte_t *) dest_;

    for(i = zero; i < size_; i++) {
      dest[i] = src[i];
    }

    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_) {
  RET_DEFINE;


  Size_t i = zero;
  volatile Byte_t *dest = null;


  if(NOTNULLPTR(dest_) && (zero < size_)) {
    dest = (Byte_t *) dest_;

    for(i = zero; i < size_; i++) {
      dest[i] = (Byte_t) val_;
    }

    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_) {
  RET_DEFINE;


  Size_t i = zero;
  volatile Byte_t *s1 = null;
  volatile Byte_t *s2 = null;


  if(NOTNULLPTR(s1_) && NOTNULLPTR(s2_) && (zero < size_) && NOTNULLPTR(res_)) {
    /* Set res_ to true by default which indicates the memory is comparable. If
     * we later discover the memory is *NOT* comparable, we will set res_ to
     * false. */
    *res_ = true;
    s1 = (Byte_t *) s1_;
    s2 = (Byte_t *) s2_;

    for(i = zero; i < size_; i++) {
      if(*s1 != *s2) {
        /* The memory is *NOT* comparable so set res_ to false. */
        *res_ = false;
        break;
      }

      s1++;
      s2++;
    }

    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_) {
  RET_DEFINE;

  if(NOTNULLPTR(stats_)) {
    /* Simply passthrough the address pointer to __MemGetRegionStats__() for the
     * heap memory region and a pointer to the stats structure. */
    if(OK(__MemGetRegionStats__(&heap, stats_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_) {
  RET_DEFINE;

  if(NOTNULLPTR(stats_)) {
    /* Simply passthrough the address pointer to __MemGetRegionStats__() for the
     * kernel memory region and a pointer to the stats structure. */
    if(OK(__MemGetRegionStats__(&kernel, stats_))) {
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_) {
  RET_DEFINE;


  MemoryEntry_t *cursor = null;


  if(NOTNULLPTR(region_) && NOTNULLPTR(stats_)) {
    /* Check the memory region consistency before we calculate the statistics
     * for the memory region. */
    if(OK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      /* Allocate some heap memory to hold the memory region statistics
       * structure. */
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) stats_, sizeof(MemoryRegionStats_t)))) {
        cursor = region_->start;

        if(OK(__memset__(*stats_, zero, sizeof(MemoryRegionStats_t)))) {
          /* We intentionally underflow a word (an unsigned type) to get its
           * maximum value. */
          (*stats_)->smallestFreeEntryInBytes = -1;


          /* Copy in the statistics we already have from the memory region. */
          (*stats_)->successfulAllocations = region_->allocations;
          (*stats_)->successfulFrees = region_->frees;
          (*stats_)->minimumEverFreeBytesRemaining = region_->minAvailableEver;


          /* Traverse the memory region to calculate the remaining statistics.
           */
          while(NOTNULLPTR(cursor)) {
            if(FREE == cursor->free) {
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

          RET_OK;
        } else {
          ASSERT;


          /* Free the heap memory because the call to __memset__() failed. */
          __HeapFreeMemory__(stats_);
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __DefragMemoryRegion__(const volatile MemoryRegion_t *region_) {
  RET_DEFINE;


  MemoryEntry_t *cursor = null;
  MemoryEntry_t *merge = null;


  if(NOTNULLPTR(region_)) {
    /* Check the memory region consistency before we attempt to defrag the
     * memory region. */
    if(OK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      cursor = region_->start;

      while(NOTNULLPTR(cursor)) {
        /* We will merge the blocks from two adjacent memory entries if:
         *  1. The cursor is pointing to an entry.
         *  2. "next" points to an entry.
         *  3. The entry pointed to be the cursor is free.
         *  4. The entry pointed to be "next" is free. */
        if(NOTNULLPTR(cursor) && NOTNULLPTR(cursor->next) && (FREE == cursor->free) && (FREE == cursor->next->free)) {
          merge = cursor->next;


          /* CALCMAGIC() calculates the memory entry's magic value (i.e. the
           * magic member of the memory entry structure) by XOR'ing the address
           * of the memory entry with the MAGIC_CONST. The magic value is used
           * by __MemoryRegionCheck__() to check the consistency of the memory
           * region. */
          cursor->magic = CALCMAGIC(cursor);
          cursor->free = FREE;
          cursor->blocks += merge->blocks;
          cursor->next = merge->next;


          /* Zero out the block formerly occupied by the memory entry that was
           * merged. */
          if(OK(__memset__(merge, zero, sizeof(MemoryEntry_t)))) {
            /* Do nothing - literally. */
          } else {
            ASSERT;
            break;
          }
        } else {
          cursor = cursor->next;
        }
      }

      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


#if defined(POSIX_ARCH_OTHER)


  /* For unit testing only! */
  void __MemoryClear__(void) {
    __MemoryRegionInit__(&heap);
    __MemoryRegionInit__(&kernel);

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */