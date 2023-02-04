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
static Return_t __MemoryRegionCheck__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, const Base_t option_);
static Return_t __MemoryRegionCheckAddr__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_);
static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_);
static Return_t __DefragMemoryRegion__(const volatile MemoryRegion_t *region_);


Return_t __MemoryInit__(void) {

  RET_DEFINE;

  if(ISSUCCESSFUL(__memset__(&heap, 0x0, sizeof(MemoryRegion_t)))) {
    if(ISSUCCESSFUL(__memset__(&kernel, 0x0, sizeof(MemoryRegion_t)))) {

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

  if(ISNOTNULLPTR(addr_) && (zero < size_)) {
    if(ISSUCCESSFUL(__calloc__(&heap, addr_, size_))) {
      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  }

  RET_RETURN;
}
Return_t xMemFree(const volatile Addr_t *addr_) {

  RET_DEFINE;

  if(ISNOTNULLPTR(addr_)) {
    if(ISSUCCESSFUL(__free__(&heap, addr_))) {
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

  if(ISNOTNULLPTR(size_) && (false == SYSFLAG_FAULT())) {

    if(ISSUCCESSFUL(__MemoryRegionCheck__(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {


      cursor = heap.start;

      while(ISNOTNULLPTR(cursor)) {

        if(false == cursor->free) {

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

  RET_RETURN;
}
Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_) {

  RET_DEFINE;

  MemoryEntry_t *tosize = NULL;

  if(ISNOTNULLPTR(addr_) && ISNOTNULLPTR(size_) && (false == SYSFLAG_FAULT())) {

    if(ISSUCCESSFUL(__MemoryRegionCheck__(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {


      tosize = ADDR2ENTRY(addr_, &heap);

      if(false == tosize->free) {


        *size_ = tosize->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;

        RET_SUCCESS;
      } else {
        SYSASSERT(false);
      }
    } else {
      SYSASSERT(false);
    }
  }

  RET_RETURN;
}
static Return_t __MemoryRegionCheck__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, const Base_t option_) {

  RET_DEFINE;

  MemoryEntry_t *cursor = NULL;

  MemoryEntry_t *find = NULL;

  Base_t found = false;

  HalfWord_t blocks = zero;

  if((ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {

    if(ISNOTNULLPTR(region_->start)) {

      cursor = region_->start;

      if(MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) {

        find = ADDR2ENTRY(addr_, region_);
      }

      while(ISNOTNULLPTR(cursor)) {

        if(ISSUCCESSFUL(__MemoryRegionCheckAddr__(region_, cursor))) {


          blocks += cursor->blocks;

          if((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (cursor == find) && (false == cursor->free)) {

            found = true;
          }

          cursor = cursor->next;


        } else {

          SYSASSERT(false);

          SYSFLAG_FAULT() = true;

          break;
        }
      }

      if(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks) {

        if(((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == SYSFLAG_FAULT())) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false == SYSFLAG_FAULT()) && (true == found))) {


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
static Return_t __MemoryRegionCheckAddr__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {

  RET_DEFINE;

  if(ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_)) {
    if((addr_ >= (Addr_t *) (region_->mem)) && (addr_ < (Addr_t *) (region_->mem + MEMORY_REGION_SIZE_IN_BYTES))) {

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

  HalfWord_t requested = zero;

  HalfWord_t free = zero;

  HalfWord_t fewest = -1;

  MemoryEntry_t *cursor = NULL;

  MemoryEntry_t *candidate = NULL;

  MemoryEntry_t *candidateNext = NULL;

  DISABLE_INTERRUPTS();

  if(ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (false == SYSFLAG_FAULT()) && (zero < size_)) {

    if(zero == region_->entrySize) {

      region_->entrySize = ((HalfWord_t) (sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE));

      if(zero < ((HalfWord_t) (sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {

        region_->entrySize++;
      }
    }

    if(ISNULLPTR(region_->start)) {

      region_->start = (MemoryEntry_t *) region_->mem;

      if(ISSUCCESSFUL(__memset__(region_->mem, zero, MEMORY_REGION_SIZE_IN_BYTES))) {

        region_->start->free = true;

        region_->start->blocks = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS;

        region_->start->next = NULL;
      } else {
        SYSASSERT(false);
      }
    } else {
      /** DO NOTHING, NORMAL THAT REGION MAY NOT HAVE BEEN INITIALIZED **/
    }

    if(ISSUCCESSFUL(__MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {


      requested = ((HalfWord_t) (size_ / CONFIG_MEMORY_REGION_BLOCK_SIZE));

      if(zero < ((HalfWord_t) (size_ % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {

        requested++;
      }

      requested += region_->entrySize;

      cursor = region_->start;

      while(ISNOTNULLPTR(cursor)) {

        if((true == cursor->free) && (requested <= cursor->blocks) && (fewest > cursor->blocks)) {


          fewest = cursor->blocks;

          candidate = cursor;
        }

        if(true == cursor->free) {

          free += cursor->blocks;
        }

        cursor = cursor->next;
      }

      if(ISNOTNULLPTR(candidate)) {

        if((region_->entrySize + 1) <= (candidate->blocks - requested)) {

          candidateNext = candidate->next;

          candidate->next = (MemoryEntry_t *) ((Byte_t *) candidate + (requested * CONFIG_MEMORY_REGION_BLOCK_SIZE));

          candidate->next->next = candidateNext;

          candidate->next->free = true;

          candidate->next->blocks = candidate->blocks - requested;

          candidate->free = false;

          candidate->blocks = requested;

          if(ISSUCCESSFUL(__memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {

            *addr_ = ENTRY2ADDR(candidate, region_);

            RET_SUCCESS;

          } else {
            SYSASSERT(false);
          }
        } else {

          candidate->free = false;

          if(ISSUCCESSFUL(__memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE))) {

            *addr_ = ENTRY2ADDR(candidate, region_);

            RET_SUCCESS;
          } else {
            SYSASSERT(false);
          }
        }

        region_->allocations++;


        free -= requested;

        if((free * CONFIG_MEMORY_REGION_BLOCK_SIZE) < region_->minAvailableEver) {


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

  ENABLE_INTERRUPTS();

  RET_RETURN;
}
static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {

  RET_DEFINE;

  MemoryEntry_t *free = NULL;


  DISABLE_INTERRUPTS();

  if(ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (false == SYSFLAG_FAULT())) {

    if(ISSUCCESSFUL(__MemoryRegionCheck__(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {


      free = ADDR2ENTRY(addr_, region_);


      free->free = true;


      region_->frees++;

      if(ISSUCCESSFUL(__DefragMemoryRegion__(region_))) {
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

  ENABLE_INTERRUPTS();

  RET_RETURN;
}
Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {

  RET_DEFINE;

  if(ISNOTNULLPTR(addr_) && (zero < size_)) {
    if(ISSUCCESSFUL(__calloc__(&kernel, addr_, size_))) {
      if(ISNOTNULLPTR(*addr_)) {
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

  if(ISNOTNULLPTR(addr_)) {
    if(ISSUCCESSFUL(__free__(&kernel, addr_))) {
      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}
Return_t __MemoryRegionCheckKernel__(const volatile Addr_t *addr_, const Base_t option_) {
  RET_DEFINE;

  if((ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    if(ISSUCCESSFUL(__MemoryRegionCheck__(&kernel, addr_, option_))) {


      RET_SUCCESS;

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

  if(ISNOTNULLPTR(addr_) && (zero < size_)) {
    if(ISSUCCESSFUL(__calloc__(&heap, addr_, size_))) {
      if(ISNOTNULLPTR(*addr_)) {
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
Return_t __HeapFreeMemory__(const volatile Addr_t *addr_) {

  RET_DEFINE;

  if(ISNOTNULLPTR(addr_)) {
    if(ISSUCCESSFUL(__free__(&heap, addr_))) {
      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}
Return_t __MemoryRegionCheckHeap__(const volatile Addr_t *addr_, const Base_t option_) {
  RET_DEFINE;

  if((ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {
    if(ISSUCCESSFUL(__MemoryRegionCheck__(&heap, addr_, option_))) {


      RET_SUCCESS;

    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
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

    RET_SUCCESS;
  } else {

    SYSASSERT(false);
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

    RET_SUCCESS;
  } else {
    SYSASSERT(false);
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

    RET_SUCCESS;
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}
Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_) {

  RET_DEFINE;

  if(ISNOTNULLPTR(stats_)) {
    if(ISSUCCESSFUL(__MemGetRegionStats__(&heap, stats_))) {
      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}
Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_) {

  RET_DEFINE;

  if(ISNOTNULLPTR(stats_)) {
    if(ISSUCCESSFUL(__MemGetRegionStats__(&kernel, stats_))) {
      RET_SUCCESS;
    } else {
      SYSASSERT(false);
    }
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_) {

  RET_DEFINE;

  MemoryEntry_t *cursor = NULL;

  if(ISNOTNULLPTR(region_) && ISNOTNULLPTR(stats_) && (false == SYSFLAG_FAULT())) {

    if(ISSUCCESSFUL(__MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {

      if(ISSUCCESSFUL(__HeapAllocateMemory__((volatile Addr_t **) stats_, sizeof(MemoryRegionStats_t)))) {

        cursor = region_->start;

        if(ISSUCCESSFUL(__memset__(*stats_, zero, sizeof(MemoryRegionStats_t)))) {


          (*stats_)->smallestFreeEntryInBytes = -1;


          (*stats_)->successfulAllocations = region_->allocations;


          (*stats_)->successfulFrees = region_->frees;


          (*stats_)->minimumEverFreeBytesRemaining = region_->minAvailableEver;

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

  RET_RETURN;
}
static Return_t __DefragMemoryRegion__(const volatile MemoryRegion_t *region_) {


  RET_DEFINE;

  MemoryEntry_t *cursor = NULL;

  if(ISNOTNULLPTR(region_) && (false == SYSFLAG_FAULT())) {

    if(ISSUCCESSFUL(__MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR))) {
      cursor = region_->start;

      while(ISNOTNULLPTR(cursor)) {

        if(ISNOTNULLPTR(cursor) && ISNOTNULLPTR(cursor->next) && (true == cursor->free) && (true == cursor->next->free)) {


          cursor->blocks += cursor->next->blocks;


          cursor->next = cursor->next->next;


        } else {

          cursor = cursor->next;
        }
      }

      RET_SUCCESS;
    }
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
#endif