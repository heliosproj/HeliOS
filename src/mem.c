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



static volatile MemoryRegion_t heap = {
    .entrySize = zero,
    .start = NULL,
};




static volatile MemoryRegion_t kernel = {
    .entrySize = zero,
    .start = NULL,
};



/* System call used by end-user tasks to allocate memory
from the heap memory region. */
Addr_t *xMemAlloc(const Size_t size_) {

  /* Just call _calloc_() with the heap memory region
  to free from the heap. */
  return _calloc_(&heap, size_);
}




void xMemFree(const Addr_t *addr_) {


  /* Just call _free_() with the heap memory region to free
  from the heap. */
  _free_(&heap, addr_);

  return;
}



/* System call to find out how much memory is allocated
in the heap memory region. */
Size_t xMemGetUsed(void) {


  Size_t ret = zero;


  MemoryEntry_t *cursor = NULL;


  Word_t used = zero;



  /* Assert if any memory region is corrupt. */
  SYSASSERT(false == SYSFLAG_CORRUPT());



  /* Check to make sure no memory regions are
  corrupt before we do anything. */
  if (false == SYSFLAG_CORRUPT()) {


    /* Assert if the check of the heap memory region fails. */
    SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheck_(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR));



    /* Check if the heap memory region is consistent. */
    if (RETURN_SUCCESS == _MemoryRegionCheck_(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR)) {

      cursor = heap.start;


      /* Traverse the memory region as long as there is
      something to traverse. */
      while (ISNOTNULLPTR(cursor)) {



        /* If the entry pointed to by the cursor is not
        free, then add its blocks to the used block count. */
        if (cursor->free == false) {



          used += cursor->blocks;
        }


        /* Move on to the next entry. */
        cursor = cursor->next;
      }



      /* The end-user is expecting bytes, not blocks so multiply
      the block size by the number of used blocks. */
      ret = used * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE;
    }
  }

  return ret;
}




/* A system call used by end-user tasks to return the amount
of memory in bytes assigned to an address in the heap memory
region. */
Size_t xMemGetSize(const Addr_t *addr_) {


  Size_t ret = zero;


  MemoryEntry_t *tosize = NULL;



  /* Assert if any memory region is corrupt. */
  SYSASSERT(false == SYSFLAG_CORRUPT());



  /* Check to make sure no memory regions are
  corrupt before we do anything. */
  if (false == SYSFLAG_CORRUPT()) {



    /* I LEFT OFF HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheck_(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR));




    if (RETURN_SUCCESS == _MemoryRegionCheck_(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {




      tosize = ADDR2ENTRY(addr_, &heap);




      SYSASSERT(false == tosize->free);




      if (false == tosize->free) {




        ret = tosize->blocks * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE;
      }
    }
  }



  return ret;
}




Base_t _MemoryRegionCheck_(const volatile MemoryRegion_t *region_, const Addr_t *addr_, const Base_t option_) {


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



        SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckAddr_(region_, cursor));




        if (RETURN_SUCCESS == _MemoryRegionCheckAddr_(region_, cursor)) {

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
        } /* Never us an else statement here to mark SYSFLAG_CORRUPT() = true. */



      } else {


        SYSFLAG_CORRUPT() = true;
      }
    }
  }

  return ret;
}



Base_t _MemoryRegionCheckAddr_(const volatile MemoryRegion_t *region_, const Addr_t *addr_) {




  Base_t ret = RETURN_FAILURE;



  SYSASSERT((addr_ >= (Addr_t *)(region_->mem)) && (addr_ < (Addr_t *)(region_->mem + ALL_MEMORY_REGIONS_SIZE_IN_BYTES)));




  if ((addr_ >= (Addr_t *)(region_->mem)) && (addr_ < (Addr_t *)(region_->mem + ALL_MEMORY_REGIONS_SIZE_IN_BYTES))) {


    ret = RETURN_SUCCESS;




  } else {




    SYSFLAG_CORRUPT() = true;
  }


  return ret;
}



Addr_t *_calloc_(volatile MemoryRegion_t *region_, const Size_t size_) {




  DISABLE_INTERRUPTS();

  Addr_t *ret = NULL;

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


        _memset_(region_->mem, zero, ALL_MEMORY_REGIONS_SIZE_IN_BYTES);


        region_->start->free = true;




        region_->start->blocks = CONFIG_ALL_MEMORY_REGIONS_SIZE_IN_BLOCKS;




        region_->start->next = NULL;
      }




      SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheck_(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR));



      if (RETURN_SUCCESS == _MemoryRegionCheck_(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR)) {



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




            _memset_(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE);




            ret = ENTRY2ADDR(candidate, region_);



          } else {



            candidate->free = false;




            _memset_(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE);




            ret = ENTRY2ADDR(candidate, region_);
          }
        }
      }
    }
  }




  ENABLE_INTERRUPTS();

  return ret;
}



void _free_(const volatile MemoryRegion_t *region_, const Addr_t *addr_) {



  DISABLE_INTERRUPTS();


  MemoryEntry_t *free = NULL;




  SYSASSERT(false == SYSFLAG_CORRUPT());



  if (false == SYSFLAG_CORRUPT()) {




    SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheck_(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR));




    if (RETURN_SUCCESS == _MemoryRegionCheck_(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {




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




Addr_t *_KernelAllocateMemory_(const Size_t size_) {


  return _calloc_(&kernel, size_);
}




void _KernelFreeMemory_(const Addr_t *addr_) {

  _free_(&kernel, addr_);


  return;
}


Base_t _MemoryRegionCheckKernel_(const Addr_t *addr_, const Base_t option_) {

  return _MemoryRegionCheck_(&kernel, addr_, option_);
}


Addr_t *_HeapAllocateMemory_(const Size_t size_) {


  return _calloc_(&heap, size_);
}




void _HeapFreeMemory_(const Addr_t *addr_) {

  _free_(&heap, addr_);


  return;
}



Base_t _MemoryRegionCheckHeap_(const Addr_t *addr_, const Base_t option_) {

  return _MemoryRegionCheck_(&heap, addr_, option_);
}




void _memcpy_(Addr_t *dest_, const Addr_t *src_, Size_t n_) {

  char *src = (char *)src_;

  char *dest = (char *)dest_;

  for (Size_t i = zero; i < n_; i++) {

    dest[i] = src[i];
  }

  return;
}




void _memset_(volatile Addr_t *dest_, uint16_t val_, Size_t n_) {

  char *dest = (char *)dest_;

  for (Size_t i = zero; i < n_; i++) {

    dest[i] = (char)val_;
  }

  return;
}




uint16_t _memcmp_(const Addr_t *s1_, const Addr_t *s2_, Size_t n_) {

  uint16_t ret = zero;

  char *s1 = (char *)s1_;

  char *s2 = (char *)s2_;

  for (Size_t i = zero; i < n_; i++) {

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

void _memdump_(void) {

  Word_t k = zero;

  volatile MemoryRegion_t *region = NULL;

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