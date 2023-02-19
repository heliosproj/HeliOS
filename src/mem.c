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

  if(ISOK(__memset__(&heap, 0x0, sizeof(MemoryRegion_t)))) {
    if(ISOK(__memset__(&kernel, 0x0, sizeof(MemoryRegion_t)))) {
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

  if(ISNOTNULLPTR(addr_) && (zero < size_)) {
    if(ISOK(__calloc__(&heap, addr_, size_))) {
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

  if(ISNOTNULLPTR(addr_)) {
    if(ISOK(__free__(&heap, addr_))) {
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


  if(ISNOTNULLPTR(size_) && (false == SYSFLAG_FAULT())) {
    if(ISOK(__MemoryRegionCheck__(&heap, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      cursor = heap.start;

      while(ISNOTNULLPTR(cursor)) {
        /* If the memory entry is *NOT* free, then add the number of blocks it
         * contains to the in-use count. */
        if(false == cursor->free) {
          used += cursor->blocks;
        }

        cursor = cursor->next;
      }

      /* We need to give the user back bytes, not blocks so multiply the in-use
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


  if(ISNOTNULLPTR(addr_) && ISNOTNULLPTR(size_) && (false == SYSFLAG_FAULT())) {
    if(ISOK(__MemoryRegionCheck__(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      tosize = ADDR2ENTRY(addr_, &heap);

      /* If the memory entry pointed to by tosize is *NOT* free, then give the
       * user back the number of bytes in-use by multiply the blocks contained
       * in the entry by the block size in bytes. */
      if(false == tosize->free) {
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


  if((ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) &&
    (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    if(ISNOTNULLPTR(region_->start)) {
      cursor = region_->start;

      /* Check option to see if we also need to check an address in the memory
       * region, if so then set find to the memory entry address. */
      if(MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) {
        find = ADDR2ENTRY(addr_, region_);
      }

      while(ISNOTNULLPTR(cursor)) {
        if(ISOK(__MemoryRegionCheckAddr__(region_, cursor))) {
          if(ISGOODMAGIC(cursor)) {
            blocks += cursor->blocks;

            /* If we are checking for an address in the memory region, then see
             * if the cursor matches it and check to make sure the memory entry
             * is NOT* free. Otherwise ignore this step. */
            if((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (cursor == find) && (false == cursor->free)) {
              found = true;
            }

            cursor = cursor->next;
          } else {
            ASSERT;


            /* Set the memfault sysflag to true because the address we just
             * checked does *NOT* have the correct magic value. Something is
             * wrong! */
            SYSFLAG_FAULT() = true;
            break;
          }
        } else {
          ASSERT;


          /* Set the memfault sysflag to true because the address we just
           * checked is *NOT* inside the memory region. Something is wrong! */
          SYSFLAG_FAULT() = true;
          break;
        }
      }

      if(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks) {
        if(((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == SYSFLAG_FAULT())) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false ==
          SYSFLAG_FAULT()) && (true == found))) {
          RET_OK;
        } else {
          /* Never use an else statement here to mark SYSFLAG_FAULT() = true.
           * Just because an address wasn't found does not mean the memory
           * region is corrupt. */
          ASSERT;
        }
      } else {
        ASSERT;


        /* Set the memfault sysflag to true because the number of blocks visited
         * does not match the number of blocks the memory region *SHOULD* have.
         */
        SYSFLAG_FAULT() = true;
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

  if(ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_)) {
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
  MemoryEntry_t *candidateNext = null;


  DISABLE_INTERRUPTS();

  if(ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (false == SYSFLAG_FAULT()) && (zero < size_)) {
    /* If we haven't already, calculate how many blocks in size a memory entry
     * is. Typically this is one (1), but that may not always be the case. */
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
     * we need to zero out the memory and set the first block. */
    if(ISNULLPTR(region_->start)) {
      region_->start = (MemoryEntry_t *) region_->mem;

      if(ISOK(__memset__(region_->mem, zero, MEMORY_REGION_SIZE_IN_BYTES))) {
        region_->start->magic = CALCMAGIC(region_->start);
        region_->start->free = true;
        region_->start->blocks = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS;
        region_->start->next = null;
      } else {
        ASSERT;
      }
    }

    if(ISOK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      /* Because the user supplied requested memory in bytes, calculate how many
       * blocks have been requested. */
      requested = ((HalfWord_t) (size_ / CONFIG_MEMORY_REGION_BLOCK_SIZE));

      if(zero < ((HalfWord_t) (size_ % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
        requested++;
      }

      requested += region_->entrySize;
      cursor = region_->start;

      while(ISNOTNULLPTR(cursor)) {
        /* See if we have a possible candidate entry to use for the requested
         * blocks. To be a candidate the entry must:
         *  1. Be free.
         *  2. Must contain enough blocks to cover the request.
         *  3. Must be an entry with the fewest blocks (this is to reduce
         * fragmentation). */
        if((true == cursor->free) && (requested <= cursor->blocks) && (fewest > cursor->blocks)) {
          fewest = cursor->blocks;
          candidate = cursor;
        }

        /* Keep track of how many free blocks remain as we need to update the
         * statistics for the memory region later. */
        if(true == cursor->free) {
          free += cursor->blocks;
        }

        cursor = cursor->next;
      }

      if(ISNOTNULLPTR(candidate)) {
        /* If the candidate entry contains enough blocks for a memory entry and
         * at least one additional block then we are going to split the memory
         * entry into two. If not, we will just go ahead and use the memory
         * entry as is. */
        if((region_->entrySize + 1) <= (candidate->blocks - requested)) {
          /* This block of code splits the block in two and uses the first of
           * the two blocks for the requested memory. */
          candidateNext = candidate->next;
          candidate->next = (MemoryEntry_t *) ((Byte_t *) candidate + (requested * CONFIG_MEMORY_REGION_BLOCK_SIZE));
          candidate->next->magic = CALCMAGIC(candidate->next);
          candidate->next->free = true;
          candidate->next->blocks = candidate->blocks - requested;
          candidate->next->next = candidateNext;


          /* We split the unneeded blocks off into a new entry, now let's mark
           * the entry containing the blocks in-use for the requested memory. */
          candidate->magic = CALCMAGIC(candidate);
          candidate->free = false;
          candidate->blocks = requested;

          if(ISOK(__memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
            *addr_ = ENTRY2ADDR(candidate, region_);
            RET_OK;
          } else {
            ASSERT;
          }
        } else {
          /* Because we didn't need to split an entry into two, we just need to
           * mark the entry as in-use and that's it. */
          candidate->free = false;

          if(ISOK(__memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {
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

  ENABLE_INTERRUPTS();
  RET_RETURN;
}


static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {
  RET_DEFINE;


  MemoryEntry_t *free = null;


  DISABLE_INTERRUPTS();

  if(ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (false == SYSFLAG_FAULT())) {
    if(ISOK(__MemoryRegionCheck__(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      free = ADDR2ENTRY(addr_, region_);
      free->free = true;
      region_->frees++;

      if(ISOK(__DefragMemoryRegion__(region_))) {
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

  ENABLE_INTERRUPTS();
  RET_RETURN;
}


Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(addr_) && (zero < size_)) {
    if(ISOK(__calloc__(&kernel, addr_, size_))) {
      if(ISNOTNULLPTR(*addr_)) {
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

  if(ISNOTNULLPTR(addr_)) {
    if(ISOK(__free__(&kernel, addr_))) {
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

  if((ISNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    if(ISOK(__MemoryRegionCheck__(&kernel, addr_, option_))) {
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

  if(ISNOTNULLPTR(addr_) && (zero < size_)) {
    if(ISOK(__calloc__(&heap, addr_, size_))) {
      if(ISNOTNULLPTR(*addr_)) {
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

  if(ISNOTNULLPTR(addr_)) {
    if(ISOK(__free__(&heap, addr_))) {
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

  if((ISNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    if(ISOK(__MemoryRegionCheck__(&heap, addr_, option_))) {
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


  if(ISNOTNULLPTR(dest_) && ISNOTNULLPTR(src_) && (zero < size_)) {
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


  if(ISNOTNULLPTR(dest_) && (zero < size_)) {
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


  if(ISNOTNULLPTR(s1_) && ISNOTNULLPTR(s2_) && (zero < size_) && ISNOTNULLPTR(res_)) {
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

  if(ISNOTNULLPTR(stats_)) {
    if(ISOK(__MemGetRegionStats__(&heap, stats_))) {
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

  if(ISNOTNULLPTR(stats_)) {
    if(ISOK(__MemGetRegionStats__(&kernel, stats_))) {
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


  if(ISNOTNULLPTR(region_) && ISNOTNULLPTR(stats_) && (false == SYSFLAG_FAULT())) {
    if(ISOK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      if(ISOK(__HeapAllocateMemory__((volatile Addr_t **) stats_, sizeof(MemoryRegionStats_t)))) {
        cursor = region_->start;

        if(ISOK(__memset__(*stats_, zero, sizeof(MemoryRegionStats_t)))) {
          /* We intentionally underflow a word (an unsigned type) to get its
           * maximum value. */
          (*stats_)->smallestFreeEntryInBytes = -1;


          /* Copy in the statistics we already have from the memory region. */
          (*stats_)->successfulAllocations = region_->allocations;
          (*stats_)->successfulFrees = region_->frees;
          (*stats_)->minimumEverFreeBytesRemaining = region_->minAvailableEver;

          /* Traverse the memory region to calculate the remaining statistics.
           */
          while(ISNOTNULLPTR(cursor)) {
            if(true == cursor->free) {
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


  if(ISNOTNULLPTR(region_) && (false == SYSFLAG_FAULT())) {
    if(ISOK(__MemoryRegionCheck__(region_, null, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      cursor = region_->start;

      while(ISNOTNULLPTR(cursor)) {
        /* We will merge the blocks from two adjacent memory entries if:
         *  1. The cursor is pointing to an entry.
         *  2. "next" points to an entry.
         *  3. The entry pointed to be the cursor is free.
         *  4. The entry pointed to be "next" is free. */
        if(ISNOTNULLPTR(cursor) && ISNOTNULLPTR(cursor->next) && (true == cursor->free) && (true == cursor->next->free)) {
          merge = cursor->next;
          cursor->magic = CALCMAGIC(cursor);
          cursor->free = true;
          cursor->blocks += merge->blocks;
          cursor->next = merge->next;

          if(ISOK(__memset__(merge, zero, sizeof(MemoryEntry_t)))) {
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