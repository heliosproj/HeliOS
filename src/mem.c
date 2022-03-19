/**
 * @file mem.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for memory management
 * @version 0.3.3
 * @date 2022-01-31
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
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



static MemoryRegion_t heap = {
    .entrySize = zero,
    .start = NULL,
};




static MemoryRegion_t kernel = {
    .entrySize = zero,
    .start = NULL,
};




void *xMemAlloc(const size_t size_) {


  return calloc_(&heap, size_);
}




void xMemFree(const void *addr_) {

  free_(&heap, addr_);

  return;
}




size_t xMemGetUsed(void) {


  size_t ret = zero;


  MemoryEntry_t *cursor = NULL;


  Word_t used = zero;




  SYSASSERT(false == SYSFLAG_CORRUPT());




  if (false == SYSFLAG_CORRUPT()) {



    SYSASSERT(RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR));




    if (RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR)) {

      cursor = heap.start;



      while (ISNOTNULLPTR(cursor)) {



        if (cursor->free == false) {



          used += cursor->blocks;
        }



        cursor = cursor->next;
      }



      ret = used * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE;
    }
  }

  return ret;
}




/* The xMemGetSize() system call returns the amount of memory in bytes that
is currently allocated to a specific pointer. */
size_t xMemGetSize(const void *addr_) {


  size_t ret = zero;


  MemoryEntry_t *tosize = NULL;




  SYSASSERT(false == SYSFLAG_CORRUPT());




  if (false == SYSFLAG_CORRUPT()) {




    SYSASSERT(RETURN_SUCCESS == MemoryRegionCheck(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR));




    if (RETURN_SUCCESS == MemoryRegionCheck(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {




      tosize = ADDR2ENTRY(addr_, &heap);




      SYSASSERT(false == tosize->free);




      if (false == tosize->free) {




        ret = tosize->blocks * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE;
      }
    }
  }

  

  return ret;
}


Base_t MemoryRegionCheckHeap(const void *addr_, const Base_t option_) {

  return MemoryRegionCheck(&heap, addr_, option_);
}




Base_t MemoryRegionCheckKernel(const void *addr_, const Base_t option_) {

  return MemoryRegionCheck(&kernel, addr_, option_);
}



Base_t MemoryRegionCheck(const MemoryRegion_t *region_, const void *addr_, const Base_t option_) {


  MemoryEntry_t *cursor = NULL;

  MemoryEntry_t *find = NULL;

  Base_t found = false;

  Word_t blocks = zero;


  Base_t ret = RETURN_FAILURE;


  SYSASSERT((ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_)));



  if ((ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {


    SYSASSERT(ISNOTNULLPTR(region_->start));


    if (ISNOTNULLPTR(region_->start)) {

      cursor = region_->start;


      if (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) {

        find = ADDR2ENTRY(addr_, region_);
      }


      while (ISNOTNULLPTR(cursor)) {



        SYSASSERT(RETURN_SUCCESS == MemoryRegionCheckAddr(region_, cursor));




        if (RETURN_SUCCESS == MemoryRegionCheckAddr(region_, cursor)) {

          blocks += cursor->blocks;




          if ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (cursor == find) && (false == cursor->free)) {

            found = true;
          }


          cursor = cursor->next;

        } else {


          SYSFLAG_CORRUPT() = true;


          break;
        }
      }

      SYSASSERT(CONFIG_ALL_MEMORY_REGIONS_SIZE_IN_BLOCKS == blocks);


      if (CONFIG_ALL_MEMORY_REGIONS_SIZE_IN_BLOCKS == blocks) {



        SYSASSERT(((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == SYSFLAG_CORRUPT())) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false == SYSFLAG_CORRUPT()) && (true == found)));



        if (((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == SYSFLAG_CORRUPT())) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false == SYSFLAG_CORRUPT()) && (true == found))) {


          ret = RETURN_SUCCESS;
        }



      } else {


        SYSFLAG_CORRUPT() = true;
      }
    }
  }

  return ret;
}



Base_t MemoryRegionCheckAddr(const MemoryRegion_t *region_, const void *addr_) {




  Base_t ret = RETURN_FAILURE;



  SYSASSERT((addr_ >= (void *)(region_->mem)) && (addr_ < (void *)(region_->mem + ALL_MEMORY_REGIONS_SIZE_IN_BYTES)));




  if ((addr_ >= (void *)(region_->mem)) && (addr_ < (void *)(region_->mem + ALL_MEMORY_REGIONS_SIZE_IN_BYTES))) {


    ret = RETURN_SUCCESS;




  } else {




    SYSFLAG_CORRUPT() = true;
  }


  return ret;
}



void *calloc_(MemoryRegion_t *region_, const size_t size_) {




  DISABLE_INTERRUPTS();

  void *ret = NULL;

  Word_t requested = zero;



  Word_t fewest = -1;


  MemoryEntry_t *cursor = NULL;


  MemoryEntry_t *candidate = NULL;


  MemoryEntry_t *candidateNext = NULL;


  SYSASSERT(zero < size_);



  if (zero < size_) {



    SYSASSERT(false == SYSFLAG_CORRUPT());



    if (false == SYSFLAG_CORRUPT()) {



      if (zero == region_->entrySize) {



        region_->entrySize = ((Word_t)(sizeof(MemoryEntry_t) / CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE));




        if (zero < ((Word_t)(sizeof(MemoryEntry_t) % CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE))) {



          region_->entrySize++;
        }
      }


      if (ISNULLPTR(region_->start)) {



        region_->start = (MemoryEntry_t *)region_->mem;


        memset_(region_->mem, zero, ALL_MEMORY_REGIONS_SIZE_IN_BYTES);


        region_->start->free = true;






        region_->start->blocks = CONFIG_ALL_MEMORY_REGIONS_SIZE_IN_BLOCKS;




        region_->start->next = NULL;
      }




      SYSASSERT(RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR));



      if (RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR)) {



        requested = ((Word_t)(size_ / CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE));



        if (zero < ((Word_t)(size_ % CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE))) {



          requested++;
        }



        requested += region_->entrySize;




        cursor = region_->start;




        while (ISNOTNULLPTR(cursor)) {




          if ((true == cursor->free) && (requested <= cursor->blocks) && (fewest > cursor->blocks)) {




            fewest = cursor->blocks;



            candidate = cursor;
          }




          cursor = cursor->next;
        }



        SYSASSERT(ISNOTNULLPTR(candidate));




        if (ISNOTNULLPTR(candidate)) {




          if ((region_->entrySize + 1) <= (candidate->blocks - requested)) {




            candidateNext = candidate->next;




            candidate->next = (MemoryEntry_t *)((Byte_t *)candidate + (requested * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE));




            candidate->next->next = candidateNext;




            candidate->next->free = true;






            candidate->next->blocks = candidate->blocks - requested;




            candidate->free = false;






            candidate->blocks = requested;




            memset_(ENTRY2ADDR(candidate, &heap), zero, (requested - region_->entrySize) * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE);




            ret = ENTRY2ADDR(candidate, &heap);



          } else {



            candidate->free = false;






            memset_(ENTRY2ADDR(candidate, &heap), zero, (requested - region_->entrySize) * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE);




            ret = ENTRY2ADDR(candidate, &heap);
          }
        }
      }
    }
  }




  ENABLE_INTERRUPTS();

  return ret;
}



void free_(const MemoryRegion_t *region_, const void *addr_) {



  DISABLE_INTERRUPTS();


  MemoryEntry_t *free = NULL;




  SYSASSERT(false == SYSFLAG_CORRUPT());



  if (false == SYSFLAG_CORRUPT()) {




    SYSASSERT(RETURN_SUCCESS == MemoryRegionCheck(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR));




    if (RETURN_SUCCESS == MemoryRegionCheck(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {




      free = ADDR2ENTRY(addr_, region_);




        free->free = true;




        if ((ISNOTNULLPTR(free->next)) && (true == free->next->free)) {



          free->blocks += free->next->blocks;




          free->next = free->next->next;
        }
      
    }
  }




  ENABLE_INTERRUPTS();

  return;
}




void *KernelAllocateMemory(const size_t size_) {


  return calloc_(&kernel, size_);
}




void KernelFreeMemory(const void *addr_) {

  free_(&kernel, addr_);


  return;
}



void *HeapAllocateMemory(const size_t size_) {


  return calloc_(&heap, size_);
}




void HeapFreeMemory(const void *addr_) {

  free_(&heap, addr_);


  return;
}



void memcpy_(void *dest_, const void *src_, size_t n_) {

  char *src = (char *)src_;

  char *dest = (char *)dest_;

  for (size_t i = zero; i < n_; i++) {

    dest[i] = src[i];
  }

  return;
}




void memset_(void *dest_, uint16_t val_, size_t n_) {

  char *dest = (char *)dest_;

  for (size_t i = zero; i < n_; i++) {

    dest[i] = (char)val_;
  }

  return;
}




uint16_t memcmp_(const void *s1_, const void *s2_, size_t n_) {

  uint16_t ret = zero;

  char *s1 = (char *)s1_;

  char *s2 = (char *)s2_;

  for (size_t i = zero; i < n_; i++) {

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



#if defined(MEMDUMP_HEAP) || defined(MEMDUMP_KERNEL)

void memdump_(void) {

  Word_t k = zero;

  MemoryRegion_t *region = NULL;

  #if defined(MEMDUMP_HEAP)
  region = &heap;
  #elif defined(MEMDUMP_KERNEL)
  region = &kernel;
  #endif

  for (Word_t i = zero; i < (ALL_MEMORY_REGIONS_SIZE_IN_BYTES / MEMDUMP_ROW_WIDTH); i++) {


    printf("%p:", (region->mem + k));

    for (Word_t j = zero; j < MEMDUMP_ROW_WIDTH; j++) {



      if (zero == *(region->mem + k)) {


        printf(" --");


      } else {


        printf(" %02X", *(region->mem + k));
      }

      k++;
    }



    printf("\n");
  }



  return;
}

#endif