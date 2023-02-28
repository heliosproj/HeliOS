/*UNCRUSTIFY-OFF*/
/**
 * @file mem.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for memory management
 * @version 0.4.0
 * @date 2022-01-31
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
/*UNCRUSTIFY-ON*/
#include "mem.h"


/*UNCRUSTIFY-OFF*/
/*
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
static Return_t __MemoryRegionCheckAddr__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_);
static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_);
static Return_t __DefragMemoryRegion__(const volatile MemoryRegion_t *region_);


Return_t __MemoryInit__(void) {
  RET_DEFINE;

  /* Call __memset__() on the heap and kernel memory regions to clear (i.e.,
   * zero out) them then set the starting memory statistics for both regions.*/
  if(OK(__memset__(&heap, 0x0, sizeof(MemoryRegion_t)))) {
    if(OK(__memset__(&kernel, 0x0, sizeof(MemoryRegion_t)))) {
      heap.minAvailableEver = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE;
      kernel.minAvailableEver = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE;
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


  MemoryEntry_t *cursor = null;
  MemoryEntry_t *find = null;
  Base_t found = false;
  HalfWord_t blocks = zero;


  if((NOTNULLPTR(region_) && NULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (NOTNULLPTR(region_) && NOTNULLPTR(addr_) &&
    (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    /* Check to make sure a previous call to __MemoryRegionCheck__() did not
     * detect a memory fault. If it did, just give up and go home. */
    if(false == FLAG_MEMFAULT) {
      /* Check to make sure the memory region has been initialized. If it
       * hasn't, then the region structure start member will be null. */
      if(NOTNULLPTR(region_->start)) {
        cursor = region_->start;

        /* Check option to see if we also need to check an address in the memory
         * region, if so then set find to the memory entry address. */
        if(MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) {
          /* ADDR2ENTRY() calculates the location of the memory entry for the
           * allocated memory pointed to by the address pointer. */
          find = ADDR2ENTRY(addr_, region_);
        }

        /* Start traversing the memory entries in the memory region while the
         * cursor is null. */
        while(NOTNULLPTR(cursor)) {
          if(OK(__MemoryRegionCheckAddr__(region_, cursor))) {
            /* OKMAGIC() compares the memory entry's magic value (i.e., the
             * magic member of the memory entry structure) to the magic value
             * calculated by XOR'ing the address of the memory entry with the
             * MAGIC_CONST. This operation helps ensure we are accessing a valid
             * memory entry in the memory region being checked. */
            if(OKMAGIC(cursor)) {
              blocks += cursor->blocks;

              /* If we are checking for an address in the memory region, then
               * see if the cursor matches it and check to make sure the memory
               * entry is *NOT* free. If the cursor matches and the entry is
               * NOT*
               * free then we set "found" to true because we found the entry
               * being searched for. Otherwise ignore this step. */
              if((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (cursor == find) && (INUSE == cursor->free)) {
                found = true;
              }

              cursor = cursor->next;
            } else {
              ASSERT;


              /* "Houston, we've had a problem." ~ Jim Lovell
               *
               *
               * Set the memfault flag to true because the address we just
               * checked does *NOT* have the correct magic value. Something is
               * very wrong! */
              FLAG_MEMFAULT = true;
              break;
            }
          } else {
            ASSERT;


            /* "Houston, we've had a problem." ~ Jim Lovell
             *
             *
             * Set the memfault flag to true because the address we just checked
             * is *NOT* inside the memory region. Something is very wrong!
             */
            FLAG_MEMFAULT = true;
            break;
          }
        }

        /* Check to make sure the number of blocks we visited equals the number
         * of blocks that *SHOULD* be in the memory region which is defined by
         * CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS. */
        if(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks) {
          if(((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == FLAG_MEMFAULT)) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false ==
            FLAG_MEMFAULT) && (true == found))) {
            RET_OK;
          } else {
            /* Never set FLAG_MEMFAULT = true here! Just because an address
             * wasn't found does not mean the memory region is corrupt. */
            ASSERT;
          }
        } else {
          ASSERT;


          /* "Houston, we've had a problem." ~ Jim Lovell
           *
           *
           * Set the memfault flag to true because the number of blocks visited
           * does not match the number of blocks the memory region *SHOULD*
           * have. Something is very wrong!
           */
          FLAG_MEMFAULT = true;
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


static Return_t __MemoryRegionCheckAddr__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {
  RET_DEFINE;

  if(NOTNULLPTR(region_) && NOTNULLPTR(addr_)) {
    /* Check to make sure the address falls within the bounds of the memory
     * region. */
    if((addr_ >= (Addr_t *) (region_->mem)) && (addr_ < (Addr_t *) (region_->mem + MEMORY_REGION_SIZE_IN_BYTES))) {
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
    /* If we haven't already, calculate how many blocks in size a memory entry
     * is. Typically this is one, but that may not always be the case. */
    if(zero == region_->entrySize) {
      /* Divide the size of the memory entry type by the block size to determine
       * how many blocks one entry requires. */
      region_->entrySize = ((HalfWord_t) (sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE));

      /* Calculate the remainder in case part of the memory entry spills over
       * into another block. */
      if(zero < ((HalfWord_t) (sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
        region_->entrySize++;
      }
    }

    /* Check to see if the memory region has been initialized yet, if it hasn't
     * we need to set the start member of the memory region structure and zero
     * out the memory and set the first block. */
    if(NULLPTR(region_->start)) {
      /* Setting the start member indicates the memory region has been
       * initialized. */
      region_->start = (MemoryEntry_t *) region_->mem;

      if(OK(__memset__(region_->mem, zero, MEMORY_REGION_SIZE_IN_BYTES))) {
        /* CALCMAGIC() calculates the memory entry's magic value (i.e. the magic
         * member of the memory entry structure) by XOR'ing the address of the
         * memory entry with the MAGIC_CONST. The magic value is used by
         * __MemoryRegionCheck__() to check the consistency of the memory
         * region. */
        region_->start->magic = CALCMAGIC(region_->start);
        region_->start->free = FREE;
        region_->start->blocks = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS;
        region_->start->next = null;
      } else {
        ASSERT;
      }
    }

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
    *res_ = true;
    s1 = (Byte_t *) s1_;
    s2 = (Byte_t *) s2_;

    for(i = zero; i < size_; i++) {
      if(*s1 != *s2) {
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
    if(OK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
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
          cursor->magic = CALCMAGIC(cursor);
          cursor->free = FREE;
          cursor->blocks += merge->blocks;
          cursor->next = merge->next;

          if(OK(__memset__(merge, zero, sizeof(MemoryEntry_t)))) {
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


  void __MemoryClear__(void) {
    __memset__(&heap, 0x0, sizeof(MemoryRegion_t));
    heap.minAvailableEver = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE;
    __memset__(&kernel, 0x0, sizeof(MemoryRegion_t));
    kernel.minAvailableEver = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE;

    return;
  }


/* Just a debugging function to dump the contents of kernel memory. */
  void __MemoryRegionDumpKernel__(void) {
    __memdump__(&kernel);

    return;
  }


/* Just a debugging function to dump the contents of heap memory. */
  void __MemoryRegionDumpHeap__(void) {
    __memdump__(&heap);

    return;
  }


/* Function to dump the memory of the specified memory region. */
  void __memdump__(const volatile MemoryRegion_t *region_) {
    Size_t i = zero;
    Size_t j = zero;
    Size_t k = zero;


    for(i = zero; i < (MEMORY_REGION_SIZE_IN_BYTES / CONFIG_MEMORY_REGION_BLOCK_SIZE); i++) {
      printf("%p:", (region_->mem + k));

      for(j = zero; j < CONFIG_MEMORY_REGION_BLOCK_SIZE; j++) {
        if(zero == *(region_->mem + k)) {
          printf(" --");
        } else {
          printf(" %02X", *(region_->mem + k));
        }

        k++;
      }

      printf("\n");
    }

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */