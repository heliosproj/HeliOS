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
#include "mem.h"

static volatile MemoryRegion_t heap;

static volatile MemoryRegion_t kernel;

static Return_t __MemoryRegionCheck__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, const Base_t option_, Base_t *res_);
static Return_t __MemoryRegionCheckAddr__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, Base_t *res_);
static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_);
static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_);
static Return_t __DefragMemoryRegion__(const volatile MemoryRegion_t *region_);

/*
  1) Open with RET_DEFINE
  2) Existing SYSASSERT()'s removed
  3) All pointer parameters are checked for null
  4) All IF statements are accompanied by an ELSE with SYSASSERT(false)
  5) RET_SUCCESS is set on the success condition
  6) Closes with RET_RETURN
*/


Return_t __MemoryInit__(void) {

  RET_DEFINE;

  if (ISSUCCESSFUL(__memset__(&heap, 0x0, sizeof(MemoryRegion_t)))) {
    if (ISSUCCESSFUL(__memset__(&kernel, 0x0, sizeof(MemoryRegion_t)))) {

      heap.minAvailableEver = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE;

      kernel.minAvailableEver = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE;

      RET_SUCCESS;


    } else {

      SYSASSERT(false);
    }


  } else {

    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_) {

  RET_DEFINE;

  if (ISNOTNULLPTR(addr_) && (zero < size_)) {
    if (ISSUCCESSFUL(__calloc__(&heap, addr_, size_))) {
      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  }

  RET_RETURN;
}



Return_t xMemFree(const volatile Addr_t *addr_) {

  RET_DEFINE;

  if (ISNOTNULLPTR(addr_)) {
    if (ISSUCCESSFUL(__free__(&heap, addr_))) {
      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  }

  RET_RETURN;
}


Return_t xMemGetUsed(Size_t *size_) {

  RET_DEFINE;

  MemoryEntry_t *cursor = NULL;

  HalfWord_t used = zero;

  Base_t res = zero;

  if (ISNOTNULLPTR(size_) && (false == SYSFLAG_FAULT())) {

    if (ISSUCCESSFUL(__MemoryRegionCheck__(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR, &res))) {

      if (zero != res) {
        cursor = heap.start;

        while (ISNOTNULLPTR(cursor)) {

          if (false == cursor->free) {

            used += cursor->blocks;
          }

          cursor = cursor->next;
        }


        *size_ = used * CONFIG_MEMORY_REGION_BLOCK_SIZE;

        RET_SUCCESS;
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}



Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_) {

  RET_DEFINE;

  MemoryEntry_t *tosize = NULL;

  Base_t res = zero;


  if (ISNOTNULLPTR(addr_) && ISNOTNULLPTR(size_) && (false == SYSFLAG_FAULT())) {


    if (ISSUCCESSFUL(__MemoryRegionCheck__(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR, &res))) {

      if (zero != res) {

        tosize = ADDR2ENTRY(addr_, &heap);

        if (false == tosize->free) {


          *size_ = tosize->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;

          RET_SUCCESS;
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  }


  RET_RETURN;
}


static Return_t __MemoryRegionCheck__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, const Base_t option_, Base_t *res_) {

  RET_DEFINE;

  MemoryEntry_t *cursor = NULL;

  MemoryEntry_t *find = NULL;

  Base_t found = false;

  HalfWord_t blocks = zero;

  Base_t res = zero;


  if ((ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && ISNOTNULLPTR(res_)) || (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && ISNOTNULLPTR(res_))) {


    if (ISNOTNULLPTR(region_->start)) {

      cursor = region_->start;


      if (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) {

        find = ADDR2ENTRY(addr_, region_);
      }

      while (ISNOTNULLPTR(cursor)) {


        if (ISSUCCESSFUL(__MemoryRegionCheckAddr__(region_, cursor, &res))) {

          if (zero != res) {



            blocks += cursor->blocks;


            if ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (cursor == find) && (false == cursor->free)) {

              found = true;
            }

            cursor = cursor->next;
          } else {

            SYSASSERT(false);

            SYSFLAG_FAULT() = true;

            break;
          }

        } else {

          SYSASSERT(false);

          SYSFLAG_FAULT() = true;

          break;
        }
      }



      if (CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks) {


        if (((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == SYSFLAG_FAULT())) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false == SYSFLAG_FAULT()) && (true == found))) {


          RET_SUCCESS;



        } /* Never use an else statement here to mark SYSFLAG_FAULT() = true. Just because an address wasn't
             found does not mean the memory region is corrupt. */



      } else {

        SYSASSERT(false);

        SYSFLAG_FAULT() = true;
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}



static Return_t __MemoryRegionCheckAddr__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, Base_t *res_) {

  RET_DEFINE;


  if (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && ISNOTNULLPTR(res_)) {
    if ((addr_ >= (Addr_t *)(region_->mem)) && (addr_ < (Addr_t *)(region_->mem + MEMORY_REGION_SIZE_IN_BYTES))) {

      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }


  RET_RETURN;
}

static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_) {

  RET_DEFINE;

  Base_t res = zero;

  HalfWord_t requested = zero;

  HalfWord_t free = zero;

  HalfWord_t fewest = -1;

  MemoryEntry_t *cursor = NULL;

  MemoryEntry_t *candidate = NULL;

  MemoryEntry_t *candidateNext = NULL;

  DISABLE_INTERRUPTS();

  if (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (false == SYSFLAG_FAULT()) && (zero < size_)) {

    if (zero == region_->entrySize) {

      region_->entrySize = ((HalfWord_t)(sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE));

      if (zero < ((HalfWord_t)(sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {

        region_->entrySize++;
      }
    }

    if (ISNULLPTR(region_->start)) {

      region_->start = (MemoryEntry_t *)region_->mem;


      if (ISSUCCESSFUL(__memset__(region_->mem, zero, MEMORY_REGION_SIZE_IN_BYTES))) {

        region_->start->free = true;

        region_->start->blocks = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS;

        region_->start->next = NULL;
      } else {
        SYSASSERT(false);
      }
    } else {
      /** DO NOTHING, NORMAL THAT REGION MAY NOT HAVE BEEN INITIALIZED **/
    }


    if (ISSUCCESSFUL(__MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR, &res))) {

      if (zero != res) {

        requested = ((HalfWord_t)(size_ / CONFIG_MEMORY_REGION_BLOCK_SIZE));

        if (zero < ((HalfWord_t)(size_ % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {

          requested++;
        }
        requested += region_->entrySize;

        cursor = region_->start;

        while (ISNOTNULLPTR(cursor)) {

          if ((true == cursor->free) && (requested <= cursor->blocks) && (fewest > cursor->blocks)) {


            fewest = cursor->blocks;

            candidate = cursor;
          }

          if (true == cursor->free) {

            free += cursor->blocks;
          }

          cursor = cursor->next;
        }


        if (ISNOTNULLPTR(candidate)) {

          if ((region_->entrySize + 1) <= (candidate->blocks - requested)) {

            candidateNext = candidate->next;

            candidate->next = (MemoryEntry_t *)((Byte_t *)candidate + (requested * CONFIG_MEMORY_REGION_BLOCK_SIZE));

            candidate->next->next = candidateNext;

            candidate->next->free = true;

            candidate->next->blocks = candidate->blocks - requested;

            candidate->free = false;

            candidate->blocks = requested;

            if (ISSUCCESSFUL(__memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {

              *addr_ = ENTRY2ADDR(candidate, region_);

              RET_SUCCESS;

            } else {
              SYSASSERT(false);
            }

          } else {

            candidate->free = false;

            if (ISSUCCESSFUL(__memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {

              *addr_ = ENTRY2ADDR(candidate, region_);

              RET_SUCCESS;
            } else {
              SYSASSERT(false);
            }
          }

          region_->allocations++;


          free -= requested;


          if ((free * CONFIG_MEMORY_REGION_BLOCK_SIZE) < region_->minAvailableEver) {


            region_->minAvailableEver = (free * CONFIG_MEMORY_REGION_BLOCK_SIZE);
          }
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  ENABLE_INTERRUPTS();

  RET_RETURN;
}


static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {

  RET_DEFINE;

  MemoryEntry_t *free = NULL;

  Base_t res = zero;


  DISABLE_INTERRUPTS();

  if (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (false == SYSFLAG_FAULT())) {


    if (ISSUCCESSFUL(__MemoryRegionCheck__(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR, &res))) {

      if (zero != res) {
        free = ADDR2ENTRY(addr_, region_);


        free->free = true;


        region_->frees++;


        if(ISSUCCESSFUL(__DefragMemoryRegion__(region_)) {
          RET_SUCCESS;
        } else {
          SYSASSERT(false);
        }
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }




  ENABLE_INTERRUPTS();

  RET_RETURN;
}



Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {

  RET_DEFINE;

  if (ISNOTNULLPTR(addr_) && (zero < size_)) {
    if (ISSUCCESSFUL(__calloc__(&kernel, addr_, size_))) {
      if (ISNOTNULLPTR(*addr_)) {
        RET_SUCCESS;

      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t __KernelFreeMemory__(const volatile Addr_t *addr_) {

  RET_DEFINE;

  if (ISNOTNULLPTR(addr_)) {
    if (ISSUCCESSFUL(__free__(&kernel, addr_))) {
      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t __MemoryRegionCheckKernel__(const volatile Addr_t *addr_, const Base_t option_, Base_t *res_) {
  RET_DEFINE;

  if ((ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    if (ISSUCCESSFUL(__MemoryRegionCheck__(&kernel, addr_, option_, res_))) {

      if (zero != res_) {
        RET_SUCCESS;
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {

  RET_DEFINE;

  if (ISNOTNULLPTR(addr_) && (zero < size_)) {
    if (ISSUCCESSFUL(__calloc__(&heap, addr_, size_))) {
      if (ISNOTNULLPTR(*addr_)) {
        RET_SUCCESS;

      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}



/* A wrapper function for __free__() because the memory
   regions cannot be accessed outside the scope of mem.c. */
void __HeapFreeMemory__(const volatile Addr_t *addr_) {

  __free__(&heap, addr_);


  return;
}



Return_t __MemoryRegionCheckHeap__(const volatile Addr_t *addr_, const Base_t option_, Base_t *res_) {
  RET_DEFINE;

  if ((ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    if (ISSUCCESSFUL(__MemoryRegionCheck__(&heap, addr_, option_, res_))) {

      if (zero != res_) {
        RET_SUCCESS;
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}



/* Like the standard libc function, __memcpy__() copies memory from one
   address to another. */
void __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_) {

  Size_t i = zero;

  const volatile Char_t *src = (Char_t *)src_;

  volatile Char_t *dest = (Char_t *)dest_;

  for (i = zero; i < size_; i++) {

    dest[i] = src[i];
  }

  return;
}



/* Like the standard libc function, __memset__() sets the memory
   at the location specified as the address to the defined value. */
void __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_) {

  Size_t i = zero;

  volatile Char_t *dest = (Char_t *)dest_;

  for (i = zero; i < size_; i++) {

    dest[i] = (Char_t)val_;
  }

  return;
}



/* Similar to the standard libc function, __memcmp__() compares the contents
   of two memory locations pointed. */
Base_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_) {

  Size_t i = zero;

  Base_t ret = zero;

  volatile Char_t *s1 = (Char_t *)s1_;

  volatile Char_t *s2 = (Char_t *)s2_;

  for (i = zero; i < size_; i++) {

    if (*s1 != *s2) {

      ret = *s1 - *s2;

      break; /* Typically memcmp() just returns here but we can't do that for MISRA C:2012
                compliance. */
    }

    s1++;

    s2++;
  }


  return ret;
}



/* Return the memory region statistics for the heap. */
MemoryRegionStats_t *xMemGetHeapStats(void) {


  return __MemGetRegionStats__(&heap);
}



/* Return the memory region statistics for the kernel */
MemoryRegionStats_t *xMemGetKernelStats(void) {

  return __MemGetRegionStats__(&kernel);
}



/* Return the memory region statistics for the specified memory region. */
static MemoryRegionStats_t *__MemGetRegionStats__(const volatile MemoryRegion_t *region_) {

  MemoryEntry_t *cursor = NULL;

  MemoryRegionStats_t *ret = NULL;


  /* We can't do anything if the region_ pointer is null so assert if it is. */
  SYSASSERT(ISNOTNULLPTR(region_));


  /* We can't do anything if the region pointer is null so check before we proceed. */
  if (ISNOTNULLPTR(region_)) {


    /* Assert if any memory region is corrupt. */
    SYSASSERT(false == SYSFLAG_FAULT());



    /* Check to make sure no memory regions are
       corrupt before we do anything. */
    if (false == SYSFLAG_FAULT()) {


      /* Assert if the check if the memory region fails. */
      SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR));



      /* Check if the memory region is consistent. */
      if (RETURN_SUCCESS == __MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR)) {

        /* Allocate heap memory for the MemoryRegionStats_t structure. */
        ret = (MemoryRegionStats_t *)__HeapAllocateMemory__(sizeof(MemoryRegionStats_t));


        /* Assert if __HeapAllocateMemory__() failed to allocate the memory. */
        SYSASSERT(ISNOTNULLPTR(ret));


        /* Check to make sure __HeapAllocateMemory__() did its job. */
        if (ISNOTNULLPTR(ret)) {

          cursor = region_->start;

          /*

             MemoryRegionStats_t contains these elements which
             we need to fill-in before returning.

             largestFreeEntryInBytes;
             smallestFreeEntryInBytes;
             numberOfFreeBlocks;
             availableSpaceInBytes;
             successfulAllocations;
             successfulFrees;
             minimumEverFreeBytesRemaining;
           */

          /* Clear the structure to make sure everything is zero. */
          __memset__(ret, zero, sizeof(MemoryRegionStats_t));



          /* Intentionally underflow Word_t to get its max value. */
          ret->smallestFreeEntryInBytes = -1;


          /* Set the number of allocations from the region. */
          ret->successfulAllocations = region_->allocations;


          /* Set the number of frees from the region. */
          ret->successfulFrees = region_->frees;


          /* Set the minimum ever free bytes remaining for the region. */
          ret->minimumEverFreeBytesRemaining = region_->minAvailableEver;


          /* Traverse the memory region as long as there is
             something to traverse. */
          while (ISNOTNULLPTR(cursor)) {

            if (true == cursor->free) {


              if (ret->largestFreeEntryInBytes < (cursor->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE)) {

                ret->largestFreeEntryInBytes = cursor->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
              }


              if (ret->smallestFreeEntryInBytes > (cursor->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE)) {

                ret->smallestFreeEntryInBytes = cursor->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
              }


              ret->numberOfFreeBlocks += cursor->blocks;
            }

            ret->availableSpaceInBytes = ret->numberOfFreeBlocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;

            /* Move on to the next entry. */
            cursor = cursor->next;
          }
        }
      }
    }
  }
  return ret;
}



/* Defrag an entire memory region to reduce memory fragmentation. */
static void __DefragMemoryRegion__(const volatile MemoryRegion_t *region_) {


  MemoryEntry_t *cursor = NULL;



  /* We can't do anything if the region_ pointer is null so assert if it is. */
  SYSASSERT(ISNOTNULLPTR(region_));


  /* We can't do anything if the region pointer is null so check before we proceed. */
  if (ISNOTNULLPTR(region_)) {


    /* Assert if any memory region is corrupt. */
    SYSASSERT(false == SYSFLAG_FAULT());



    /* Check to make sure no memory regions are
       corrupt before we do anything. */
    if (false == SYSFLAG_FAULT()) {


      /* Assert if the check if the memory region fails. */
      SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR));



      /* Check if the memory region is consistent. */
      if (RETURN_SUCCESS == __MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR)) {

        cursor = region_->start;

        /* Traverse the memory region as long as there is
           something to traverse. */
        while (ISNOTNULLPTR(cursor)) {


          /* If the current entry is free AND the subsequent entry is free,
             we can merge the two. */
          if ((ISNOTNULLPTR(cursor)) && (ISNOTNULLPTR(cursor->next)) && (true == cursor->free) && (true == cursor->next->free)) {


            /* Add the blocks from the subsequent entry to the
               current entry. */
            cursor->blocks += cursor->next->blocks;

            /* Just drop the "next" entry just as you would in a linked list. */
            cursor->next = cursor->next->next;


          } else {


            /* Move on to the next entry. */
            cursor = cursor->next;
          }
        }
      }
    }
  }

  return;
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



  for (i = zero; i < (MEMORY_REGION_SIZE_IN_BYTES / CONFIG_MEMORY_REGION_BLOCK_SIZE); i++) {


    printf("%p:", (region_->mem + k));

    for (j = zero; j < CONFIG_MEMORY_REGION_BLOCK_SIZE; j++) {



      if (zero == *(region_->mem + k)) {


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

#endif