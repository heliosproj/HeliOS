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
    .entrySizeInBlocks = zero,
    .startEntry = NULL,
};




static MemoryRegion_t kernel = {
    .entrySizeInBlocks = zero,
    .startEntry = NULL,
};


/* The xMemAlloc() system call will allocate heap memory and return a pointer
to the newly allocated memory. */
void *xMemAlloc(size_t size_) {


  /* Disable interrupts because we can't be interrupted while modifying the heap. */
  DISABLE_INTERRUPTS();

  void *ret = NULL;

  Word_t requestedBlocks = zero;


  /* To get the maximum value of Word_t, we underflow the unsigned type. */
  Word_t leastBlocks = -1;


  MemoryEntry_t *entryCursor = NULL;


  MemoryEntry_t *entryCandidate = NULL;


  MemoryEntry_t *entryCandidateNext = NULL;


  /* Assert if the end-user tries to allocate zero bytes of heap memory. */
  SYSASSERT(zero < size_);


  /* We can't allocate zero bytes of heap memory so just head toward the
  return statement. */
  if (zero < size_) {


    /* Assert if the heap is corrupted. */
    SYSASSERT(false == SYSFLAG_CORRUPT());


    /* Check to make sure the heap is not corrupted before we do anything with
    the heap. */
    if (false == SYSFLAG_CORRUPT()) {

      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      PHASE I: Determine how many blocks a heap entry requires. One block is generally
      sufficient but we shouldn't assume. This only needs to be done once.
      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

      /* Figure out how many blocks are needed to store a heap entry by performing some
      division. */
      if (zero == heap.entrySizeInBlocks) {


        /* Calculate the quotient portion of the blocks needed to store a heap entry. */
        heap.entrySizeInBlocks = ((Word_t)(sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE));



        /* Calculate the remainder portion of the blocks needed to store a heap entry.
        If there is a remainder, then add one block to cover it. */
        if (zero < ((Word_t)(sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {


          /* Add just one more block to cover the remainder. */
          heap.entrySizeInBlocks++;
        }
      }

      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      PHASE II: Determine if the first heap entry has been created. This effectively
      initializes the heap. This also only needs to be done once.
      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

      /* If the starting entry of the heap is null, the heap has not been initialized
      so let's do that now. */
      if (ISNULLPTR(heap.startEntry)) {


        /* Set the starting heap entry to the start of the heap. We do this so
        we don't have to cast every time we want to reference the start of the
        heap. */
        heap.startEntry = (MemoryEntry_t *)heap.mem;

        /* Zero out the entire heap. */
        memset_(heap.mem, zero, ALL_MEMORY_REGIONS_SIZE_IN_BYTES);


        /* The first heap entry is free at this point so mark it as such. */
        heap.startEntry->free = true;


        /* The first heap entry is UN-protected at this point so mark it as such. For an entry to be marked protected,
        the system must be in privileged mode by calling ENTER_PRIVILEGED(). Only heap memory allocated to the kernel
        can be protected. Heap memory allocated by the end-user cannot be protected. */
        heap.startEntry->protected = false;



        /* The first entry will contain all of the blocks in the heap at this point. */
        heap.startEntry->blocks = CONFIG_ALL_MEMORY_REGIONS_SIZE_IN_BLOCKS;


        /* There is only one heap entry at this point so set the pointer to the next
        entry to null. */
        heap.startEntry->next = NULL;
      }

      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      PHASE III: Check the health of the heap by calling MemoryRegionCheck().
      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


      /* Assert if the heap is NOT healthy (i.e., contains consistency errors). */
      SYSASSERT(RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_CHECK_REGION_OPTION_WO_ADDR));


      /* If the heap is healthy, then proceed to phase IV. */
      if (RETURN_SUCCESS == MemoryRegionCheck(&heap, NULL, MEMORY_CHECK_REGION_OPTION_WO_ADDR)) {


        /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
        PHASE IV: Calculate how many blocks are needed for the requested size in bytes.
        * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


        /* Calculate the quotient portion of the requested blocks by performing some division. */
        requestedBlocks = ((Word_t)(size_ / CONFIG_MEMORY_REGION_BLOCK_SIZE));

        /* Calculate the remainder portion of the requested blocks by performing some division. If
        there is a remainder then add just one more block. */
        if (zero < ((Word_t)(size_ % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {


          /* There was a remainder for the requested blocks so add one more block. */
          requestedBlocks++;
        }


        /* We need to include how many blocks we need for the heap entry. */
        requestedBlocks += heap.entrySizeInBlocks;


        /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
        PHASE V: Scan the heap entries to find a heap entry that would be a good candidate
        for the requested blocks. This may be the last entry in the heap OR an entry that
        was recently freed by xMemFree().
        * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

        /* Start off at the start of the heap. */
        entryCursor = heap.startEntry;

        /* While there is a heap entry, continue to traverse the heap. */
        while (ISNOTNULLPTR(entryCursor)) {


          /* See if there is a candidate heap entry for the requested blocks by checking:
              1) The entry at the cursor is free.
              2) The entry has enough blocks to cover the requested blocks.
              3) The entry has the fewest possible number of blocks based on our need (i.e., we don't
              want to use 12 free blocks if we just need 3 and there is 4 available somewhere else
              in the heap). */
          if ((true == entryCursor->free) && (requestedBlocks <= entryCursor->blocks) && (leastBlocks > entryCursor->blocks)) {


            /* Seems like a good candidate so update the least blocks in case there is
            an even BETTER candidate. */
            leastBlocks = entryCursor->blocks;


            /* Keep a copy of the entry cursor as the best entry candidate in case we find out
            that the candidate is the winner. */
            entryCandidate = entryCursor;
          }

          /* Keep on move'n on. */
          entryCursor = entryCursor->next;
        }


        /* Assert if a good candidate was never found. */
        SYSASSERT(ISNOTNULLPTR(entryCandidate));

        /* If we found a candidate, then let's claim in for France. Otherwise just head
        toward the exit. */
        if (ISNOTNULLPTR(entryCandidate)) {


          /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
          PHASE VI: Found a good candidate so either reuse the entry OR split the entry in
          two if it is larger than we need. Oh, and we need to clear the memory too.
          * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


          /* Check if there is any value in splitting the entry candidate in two. */
          if ((heap.entrySizeInBlocks + 1) <= (entryCandidate->blocks - requestedBlocks)) {


            /* Preserve the next entry after the entry candidate so it can be linked to
            after we split the candidate entry in two. */
            entryCandidateNext = entryCandidate->next;


            /* Calculate the location of the new entry that will containing the remaining
            blocks not used in the candidate entry. */
            entryCandidate->next = (MemoryEntry_t *)((Byte_t *)entryCandidate + (requestedBlocks * CONFIG_MEMORY_REGION_BLOCK_SIZE));


            /* Set the split entry's next entry to the entry we preserved earlier - if that makes
            any sense at all. */
            entryCandidate->next->next = entryCandidateNext;

            /* Mark the split entry as free. */
            entryCandidate->next->free = true;


            /* Mark the split entry as not protected. */
            entryCandidate->next->protected = false;


            /* Give the split entry the remaining blocks. */
            entryCandidate->next->blocks = entryCandidate->blocks - requestedBlocks;


            /* Mark the candidate entry as no longer free. */
            entryCandidate->free = false;


            /* Set the protection on the candidate entry if in privileged mode. */
            if (true == SYSFLAG_PRIVILEGED()) {

              entryCandidate->protected = true;

            } else {

              entryCandidate->protected = false;
            }


            /* Set the candidate entry's blocks to the requested blocks. */
            entryCandidate->blocks = requestedBlocks;



            /* Clear the memory. */
            memset_(ENTRY2ADDR(entryCandidate, &heap), zero, (requestedBlocks - heap.entrySizeInBlocks) * CONFIG_MEMORY_REGION_BLOCK_SIZE);



            /* Set the return value to the address of the newly allocated heap memory. */
            ret = ENTRY2ADDR(entryCandidate, &heap);



          } else {


            /* Nothing to split so just mark the candidate entry as not free. */
            entryCandidate->free = false;

            /* Set the protection on the candidate entry if in privileged mode. */
            if (true == SYSFLAG_PRIVILEGED()) {


              entryCandidate->protected = true;


            } else {

              entryCandidate->protected = false;
            }



            /* Clear the memory. */
            memset_(ENTRY2ADDR(entryCandidate, &heap), zero, (requestedBlocks - heap.entrySizeInBlocks) * CONFIG_MEMORY_REGION_BLOCK_SIZE);


            /* Set the return value to the address of the newly allocated heap memory. */
            ret = ENTRY2ADDR(entryCandidate, &heap);
          }
        }
      }
    }
  }

  /* Exit privileged mode and enable interrupts before returning. */

  EXIT_PRIVILEGED();

  ENABLE_INTERRUPTS();

  return ret;
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

      entryCursor = heap.startEntry;

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


Base_t MemoryRegionCheck(const MemoryRegion_t *region_, const void *addr_, Base_t option_) {


  MemoryEntry_t *entryCursor = NULL;

  MemoryEntry_t *entryToFind = NULL;

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


    SYSASSERT(ISNOTNULLPTR(region_->startEntry));


    if (ISNOTNULLPTR(region_->startEntry)) {

      entryCursor = region_->startEntry;


      if (MEMORY_CHECK_REGION_OPTION_W_ADDR == option_) {

        entryToFind = ADDR2ENTRY(addr_, region_);


      }


      while (ISNOTNULLPTR(entryCursor)) {



        SYSASSERT(RETURN_SUCCESS == MemoryRegionCheckAddr(region_, entryCursor));




        if (RETURN_SUCCESS == MemoryRegionCheckAddr(region_, entryCursor)) {

          blocks += entryCursor->blocks;




          if ((MEMORY_CHECK_REGION_OPTION_W_ADDR == option_) && (entryCursor == entryToFind) && (false == entryCursor->free)) {

            found = true;
          }


          entryCursor = entryCursor->next;

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


        SYSASSERT(((MEMORY_CHECK_REGION_OPTION_WO_ADDR == option_) && (false == SYSFLAG_CORRUPT())) || ((MEMORY_CHECK_REGION_OPTION_W_ADDR == option_) && (false == SYSFLAG_CORRUPT()) && (true == found)  ));



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




/* A memory utility to copy memory between the source and destination pointers. */
void memcpy_(void *dest_, const void *src_, size_t n_) {

  char *src = (char *)src_;

  char *dest = (char *)dest_;

  for (size_t i = zero; i < n_; i++) {

    dest[i] = src[i];
  }

  return;
}




/* A memory utility to set the memory pointed to by the destination pointer
to the specified value. */
void memset_(void *dest_, uint16_t val_, size_t n_) {

  char *dest = (char *)dest_;

  for (size_t i = zero; i < n_; i++) {

    dest[i] = (char)val_;
  }

  return;
}




/* A memory utility to compare the contents of memory at two locations pointed to by
the pointers s1 and s2. */
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


/* For debugging the heap only. */
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