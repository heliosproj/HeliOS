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
  to allocate memory from the heap. */
  return _calloc_(&heap, size_);
}



/* System call used by end-user tasks to free memory from
the heap memory region. */
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



    /* Assert if the heap memory region fails its consistency check OR if the memory address is not
    valid for the heap memory region. */
    SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheck_(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR));



    /* Check if the heap memory region is consistent AND if the memory address is valid for the heap
    memory region. */
    if (RETURN_SUCCESS == _MemoryRegionCheck_(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {



      /* Get the memory region entry for the address to get the size of. */
      tosize = ADDR2ENTRY(addr_, &heap);



      /* Assert if it is free. */
      SYSASSERT(false == tosize->free);



      /* Check to make sure the entry is not free. */
      if (false == tosize->free) {



        /* The end-user is expecting the size in bytes so multiple the block
        size by the number of blocks the entry contains. */
        ret = tosize->blocks * CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE;
      }
    }
  }



  return ret;
}



/* The _MemoryRegionCheck_() function checks the consistency of a memory region. If specified, it will also check
that an address points to valid memory that was previously allocated by _calloc_() for the respective memory
region. */
Base_t _MemoryRegionCheck_(const volatile MemoryRegion_t *region_, const Addr_t *addr_, const Base_t option_) {


  MemoryEntry_t *cursor = NULL;

  MemoryEntry_t *find = NULL;

  Base_t found = false;

  Word_t blocks = zero;


  Base_t ret = RETURN_FAILURE;


  /* Assert if there is an invalid combination of arguments. */
  SYSASSERT((ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_)));


  /* Check if the combination of arguments is valid. */
  if ((ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_))) {


    /* Assert if the starting entry of the memory region has not been set. */
    SYSASSERT(ISNOTNULLPTR(region_->start));


    /* Check if the starting entry of the memory region has been set. if it hasn't then
    the memory region hasn't been initialized. */
    if (ISNOTNULLPTR(region_->start)) {

      cursor = region_->start;


      /* Check if we will also be checking if an address is a valid
      address that points to allocated memory in the memory region. */
      if (MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) {

        find = ADDR2ENTRY(addr_, region_);
      }


      /* Traverse the memory region's entries if there is
      an entry. */
      while (ISNOTNULLPTR(cursor)) {



        /* Assert if the memory address of the cursor is outside of the scope
        of the memory region. */
        SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckAddr_(region_, cursor));



        /* Check if the memory address of the cursor is inside the scope of the
        memory region, this is important in case an entry is corrupt we don't
        want to inadvertently access some other area of memory. */
        if (RETURN_SUCCESS == _MemoryRegionCheckAddr_(region_, cursor)) {

          blocks += cursor->blocks;



          /* If we are also checking if an address is valid, let's see if the entry for the address matches
          the entry we are currently on and that entry is NOT free. If that is the case, let's flag that
          we found what we are looking for. */
          if ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (cursor == find) && (false == cursor->free)) {

            found = true;
          }



          /* Move on to the next entry. */
          cursor = cursor->next;

        } else {


          /* The address of the memory entry was outside of the scope of the memory region
          so the memory region is corrupt, so mark the corrupt system flag. */
          SYSFLAG_CORRUPT() = true;


          /* No point in continuing to traverse the memory entries in the memory region
          so break out of the loop. */
          break;
        }
      }


      /* Assert if the memory region blocks does not match the setting because this would
      indicate a serious issued. */
      SYSASSERT(CONFIG_ALL_MEMORY_REGIONS_SIZE_IN_BLOCKS == blocks);


      /* Check if the number of blocks in the memory region matches the setting before
      we give the memory region a clean bill of health. */
      if (CONFIG_ALL_MEMORY_REGIONS_SIZE_IN_BLOCKS == blocks) {


        /* Assert if the memory region is flagged corrupt or if the address we were looking
        for was not found. */
        SYSASSERT(((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == SYSFLAG_CORRUPT())) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false == SYSFLAG_CORRUPT()) && (true == found)));


        /* Check that the memory region is not flagged corrupt and we found the memory address
        we were looking before. */
        if (((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == SYSFLAG_CORRUPT())) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false == SYSFLAG_CORRUPT()) && (true == found))) {


          /* Things look good so set the return value to success. */
          ret = RETURN_SUCCESS;



        } /* Never use an else statement here to mark SYSFLAG_CORRUPT() = true. Just because an address wasn't
          found does not mean the memory region is corrupt. */



      } else {


        /* The number of blocks counted in the memory region does not match the setting
        so something is seriously wrong, so set the corrupt system flag. */
        SYSFLAG_CORRUPT() = true;
      }
    }
  }

  return ret;
}



/* Function to check if an address falls within the scope of a memory region. This
function is used exclusively by _MemoryRegionCheck_(). */
Base_t _MemoryRegionCheckAddr_(const volatile MemoryRegion_t *region_, const Addr_t *addr_) {




  Base_t ret = RETURN_FAILURE;


  /* Assert if the address is outside of the scope of the memory region. */
  SYSASSERT((addr_ >= (Addr_t *)(region_->mem)) && (addr_ < (Addr_t *)(region_->mem + ALL_MEMORY_REGIONS_SIZE_IN_BYTES)));



  /* Check if the address is inside the scope of the memory region, if it is
  then return success. */
  if ((addr_ >= (Addr_t *)(region_->mem)) && (addr_ < (Addr_t *)(region_->mem + ALL_MEMORY_REGIONS_SIZE_IN_BYTES))) {


    ret = RETURN_SUCCESS;




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



#if defined(DEBUG_ON)

void _MemoryRegionDumpKernel_(void) {

  _memdump_(&kernel);


  return;
}



void _MemoryRegionDumpHeap_(void) {

  _memdump_(&heap);



  return;
}




void _memdump_(const volatile MemoryRegion_t *region_) {

  Size_t k = zero;


  for (Size_t i = zero; i < (ALL_MEMORY_REGIONS_SIZE_IN_BYTES / CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE); i++) {


    printf("%p:", (region_->mem + k));

    for (Size_t j = zero; j < CONFIG_ALL_MEMORY_REGIONS_BLOCK_SIZE; j++) {



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