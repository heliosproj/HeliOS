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




void *xMemAlloc(size_t size_) {


  return calloc_(&heap, size_);

}



/* The xMemFree() system call will free heap memory pointed to by the pointer parameter. */
void xMemFree(void *addr_) {


  /* Disable interrupts because we can't be interrupted while modifying the heap. */
  DISABLE_INTERRUPTS();


  MemoryEntry_t *entryToFree = NULL;

  /* Assert if the heap is corrupted. */
  SYSASSERT(false == SYSFLAG_CORRUPT());


  /* Check to make sure the heap is not corrupted before we do anything with
  the heap. */
  if (false == SYSFLAG_CORRUPT()) {

    /* Assert if the heap doesn't pass its health check OR if the pointer the end-user
    passed to us isn't a good one. */
    SYSASSERT(RETURN_SUCCESS == MemoryRegionCheck(&heap, addr_, MEMORY_CHECK_REGION_OPTION_W_ADDR));



    /* Check if the heap is healthy and the pointer the end-user passed to us
    is a good one. If everything checks out, proceed with freeing the memory. Otherwise,
    head toward the exit. */
    if (RETURN_SUCCESS == MemoryRegionCheck(&heap, addr_, MEMORY_CHECK_REGION_OPTION_W_ADDR)) {




      /* End-user gave us a pointer to the start of their allocated space in the heap, we
      need to move back one block to get to the heap entry. */
      entryToFree = ADDR2ENTRY(addr_, &heap);


      /* Assert if the heap entry is protected and we are not in privileged mode. */
      SYSASSERT((false == entryToFree->protected) || ((true == entryToFree->protected) && (true == SYSFLAG_PRIVILEGED())));

      /* Check if we are in privileged mode if the heap entry is protected. If it is not protected, that
      is fine too. */
      if ((false == entryToFree->protected) || ((true == entryToFree->protected) && (true == SYSFLAG_PRIVILEGED()))) {




        /* Mark the entry as free. */
        entryToFree->free = true;

        /* Mark the entry as UN-protected. */
        entryToFree->protected = false;


        /* Let's check to see if the entry we just freed can be consolidated with the next entry
        to minimize fragmentation. To start we just need to see if there IS a next entry and
        make sure it is free. */
        if ((ISNOTNULLPTR(entryToFree->next)) && (true == entryToFree->next->free)) {

          /* It looks like the entry we just freed can be consolidated with the next entry
          so add the next entry's blocks to the entry we just freed. */
          entryToFree->blocks += entryToFree->next->blocks;


          /* Now we just need to drop the next entry from the heap memory list by setting
          the entry we just freed's next to the next next entry if that makes sense. :) */
          entryToFree->next = entryToFree->next->next;

          /* Done! */
        }
      }
    }
  }

  /* Exit protect and enable interrupts before returning. */
  EXIT_PRIVILEGED();

  ENABLE_INTERRUPTS();

  return;
}

/* The xMemGetUsed() system call returns the amount of memory in bytes
that is currently allocated. */
size_t xMemGetUsed(void) {


  size_t ret = zero;

  MemoryEntry_t *entryCursor = NULL;

  Word_t usedBlocks = zero;

  /* Assert if the heap is corrupted. */
  SYSASSERT(false == SYSFLAG_CORRUPT());


  /* Check to make sure the heap is not corrupted before we do anything with
  the heap. */
  if (false == SYSFLAG_CORRUPT()) {

    /* Assert if the heap does not pass its health check. */
    SYSASSERT(RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_CHECK_REGION_OPTION_WO_ADDR));


    /* If the heap is healthy, we can proceed with calculating heap
    memory in use. Otherwise, just head toward the exit. */
    if (RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_CHECK_REGION_OPTION_WO_ADDR)) {

      entryCursor = heap.start;

      /* While we have a heap entry to read, keep traversing the heap. */
      while (ISNOTNULLPTR(entryCursor)) {

        /* If the heap entry we come across is free then let's add its blocks
        to the used blocks. */
        if (entryCursor->free == false) {

          /* Sum the number of used blocks for each heap entry in use. */
          usedBlocks += entryCursor->blocks;
        }

        /* Move on to the next heap entry. */
        entryCursor = entryCursor->next;
      }

      /* End-user is expecting bytes, so calculate it based on the
      block size. */
      ret = usedBlocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
    }
  }

  return ret;
}




/* The xMemGetSize() system call returns the amount of memory in bytes that
is currently allocated to a specific pointer. */
size_t xMemGetSize(void *addr_) {


  size_t ret = zero;


  MemoryEntry_t *entryToSize = NULL;



  /* Assert if the heap is corrupted. */
  SYSASSERT(false == SYSFLAG_CORRUPT());


  /* Check to make sure the heap is not corrupted before we do anything with
  the heap. */
  if (false == SYSFLAG_CORRUPT()) {


    /* Assert if the heap failed its health check OR if the end-user scammed
    us on the pointer. */
    SYSASSERT(RETURN_SUCCESS == MemoryRegionCheck(&heap, addr_, MEMORY_CHECK_REGION_OPTION_W_ADDR));



    /* If the heap passes its health check and the pointer the end-user passed
    us is valid, then continue. */
    if (RETURN_SUCCESS == MemoryRegionCheck(&heap, addr_, MEMORY_CHECK_REGION_OPTION_W_ADDR)) {



      /* The end-user's pointer points to the start of their allocated space, we
      need to move back one block to read the entry. */
      entryToSize = ADDR2ENTRY(addr_, &heap);


      /* The entry should not be free, also check if it is protected because if it is
      then we must be in privileged mode. */
      SYSASSERT((false == entryToSize->free) && ((false == entryToSize->protected) || ((false == entryToSize->free) && (true == entryToSize->protected) && (true == SYSFLAG_PRIVILEGED()))));


      /* The entry should not be free, also check if it is protected because if it is
      then we must be in privileged mode. */
      if ((false == entryToSize->free) && ((false == entryToSize->protected) || ((false == entryToSize->free) && (true == entryToSize->protected) && (true == SYSFLAG_PRIVILEGED())))) {



        /* The end-user is expecting us to return the number of bytes in used. So
        perform some advanced multiplication. */
        ret = entryToSize->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
      }
    }
  }

  EXIT_PRIVILEGED();

  return ret;
}


Base_t MemoryRegionCheckHeap(const void *addr_, Base_t option_) {

  return MemoryRegionCheck(&heap, addr_, option_);
}




Base_t MemoryRegionCheckKernel(const void *addr_, Base_t option_) {

  return MemoryRegionCheck(&kernel, addr_, option_);
}



Base_t MemoryRegionCheck(const MemoryRegion_t *region_, const void *addr_, Base_t option_) {


  MemoryEntry_t *cursor = NULL;

  MemoryEntry_t *find = NULL;

  Base_t found = false;

  Word_t blocks = zero;


  Base_t ret = RETURN_FAILURE;


  SYSASSERT((ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_CHECK_REGION_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (MEMORY_CHECK_REGION_OPTION_W_ADDR == option_)));

  /*
    condition #1

    (ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_CHECK_REGION_OPTION_WO_ADDR == option_))

    region_ = is not null

    addr_ = is null

    option_ = MEMORY_CHECK_REGION_OPTION_WO_ADDR


    condition #2

    (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (MEMORY_CHECK_REGION_OPTION_W_ADDR == option_))

    region_ is not null

    addr_ is not null

    option_ MEMORY_CHECK_REGION_OPTION_W_ADDR

  */

  if ((ISNOTNULLPTR(region_) && ISNULLPTR(addr_) && (MEMORY_CHECK_REGION_OPTION_WO_ADDR == option_)) || (ISNOTNULLPTR(region_) && ISNOTNULLPTR(addr_) && (MEMORY_CHECK_REGION_OPTION_W_ADDR == option_))) {


    SYSASSERT(ISNOTNULLPTR(region_->start));


    if (ISNOTNULLPTR(region_->start)) {

      cursor = region_->start;


      if (MEMORY_CHECK_REGION_OPTION_W_ADDR == option_) {

        find = ADDR2ENTRY(addr_, region_);
      }


      while (ISNOTNULLPTR(cursor)) {



        SYSASSERT(RETURN_SUCCESS == MemoryRegionCheckAddr(region_, cursor));




        if (RETURN_SUCCESS == MemoryRegionCheckAddr(region_, cursor)) {

          blocks += cursor->blocks;




          if ((MEMORY_CHECK_REGION_OPTION_W_ADDR == option_) && (cursor == find) && (false == cursor->free)) {

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


        /*
        condition #1

        ((MEMORY_CHECK_REGION_OPTION_WO_ADDR == option_) && (false == SYSFLAG_CORRUPT()))

        option_ = MEMORY_CHECK_REGION_OPTION_WO_ADDR

        SYSFLAG_CORRUPT() = false

        found = N/A


        ((MEMORY_CHECK_REGION_OPTION_W_ADDR == option_) && (false == SYSFLAG_CORRUPT()) && (true == found))

        condition #2

        option_ = MEMORY_CHECK_REGION_OPTION_W_ADDR

        SYSFLAG_CORRUPT() = false

        found = true


        */


        SYSASSERT(((MEMORY_CHECK_REGION_OPTION_WO_ADDR == option_) && (false == SYSFLAG_CORRUPT())) || ((MEMORY_CHECK_REGION_OPTION_W_ADDR == option_) && (false == SYSFLAG_CORRUPT()) && (true == found)));



        if (((MEMORY_CHECK_REGION_OPTION_WO_ADDR == option_) && (false == SYSFLAG_CORRUPT())) || ((MEMORY_CHECK_REGION_OPTION_W_ADDR == option_) && (false == SYSFLAG_CORRUPT()) && (true == found))) {


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



void *calloc_(MemoryRegion_t *region_, size_t size_) {




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



        region_->entrySize = ((Word_t)(sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE));




        if (zero < ((Word_t)(sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {



          region_->entrySize++;
        }
      }


      if (ISNULLPTR(region_->start)) {



        region_->start = (MemoryEntry_t *)region_->mem;


        memset_(region_->mem, zero, ALL_MEMORY_REGIONS_SIZE_IN_BYTES);


        region_->start->free = true;



        region_->start->protected = false;




        region_->start->blocks = CONFIG_ALL_MEMORY_REGIONS_SIZE_IN_BLOCKS;




        region_->start->next = NULL;
      }




      SYSASSERT(RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_CHECK_REGION_OPTION_WO_ADDR));



      if (RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_CHECK_REGION_OPTION_WO_ADDR)) {



        requested = ((Word_t)(size_ / CONFIG_MEMORY_REGION_BLOCK_SIZE));



        if (zero < ((Word_t)(size_ % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {



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




            candidate->next = (MemoryEntry_t *)((Byte_t *)candidate + (requested * CONFIG_MEMORY_REGION_BLOCK_SIZE));




            candidate->next->next = candidateNext;




            candidate->next->free = true;




            candidate->next->protected = false;




            candidate->next->blocks = candidate->blocks - requested;




            candidate->free = false;



            if (true == SYSFLAG_PRIVILEGED()) {

              candidate->protected = true;

            } else {

              candidate->protected = false;
            }




            candidate->blocks = requested;




            memset_(ENTRY2ADDR(candidate, &heap), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE);




            ret = ENTRY2ADDR(candidate, &heap);



          } else {



            candidate->free = false;



            if (true == SYSFLAG_PRIVILEGED()) {


              candidate->protected = true;


            } else {

              candidate->protected = false;
            }




            memset_(ENTRY2ADDR(candidate, &heap), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE);




            ret = ENTRY2ADDR(candidate, &heap);
          }
        }
      }
    }
  }



  EXIT_PRIVILEGED();

  ENABLE_INTERRUPTS();

  return ret;
}



void free_(MemoryRegion_t *region_, void *addr_) {
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



#if defined(MEMDUMP_)

void memdump_(void) {

  Word_t k = zero;

  for (Word_t i = zero; i < (ALL_MEMORY_REGIONS_SIZE_IN_BYTES / MEMDUMP_ROW_WIDTH); i++) {


    printf("%p:", (heap.mem + k));

    for (Word_t j = zero; j < MEMDUMP_ROW_WIDTH; j++) {



      if (zero == *(heap.mem + k)) {


        printf(" --");


      } else {


        printf(" %02X", *(heap.mem + k));
      }

      k++;
    }



    printf("\n");
  }



  return;
}

#endif