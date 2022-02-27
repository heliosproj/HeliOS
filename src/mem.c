/**
 * @file mem.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for memory management
 * @version 0.3.2
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



static Heap_t heap = {
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


  HeapEntry_t *entryCursor = NULL;


  HeapEntry_t *entryCandidate = NULL;


  /* Assert if the end-user tries to allocate zero bytes of heap memory. */
  SYSASSERT(zero < size_);


  /* We can't allocate zero bytes of heap memory so just head toward the
  return statement. */
  if (zero < size_) {



    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE I: Determine how many blocks a heap entry requires. One block is generally
    sufficient but we shouldn't assume. This only needs to be done once.
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* Figure out how many blocks are needed to store a heap entry by performing some
    division. */
    if (zero == heap.entrySizeInBlocks) {


      /* Calculate the quotient portion of the blocks needed to store a heap entry. */
      heap.entrySizeInBlocks = ((Word_t)(sizeof(HeapEntry_t) / CONFIG_HEAP_BLOCK_SIZE));



      /* Calculate the remainder portion of the blocks needed to store a heap entry.
      If there is a remainder, then add one block to cover it. */
      if (zero < ((Word_t)(sizeof(HeapEntry_t) % CONFIG_HEAP_BLOCK_SIZE))) {


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
      heap.startEntry = (HeapEntry_t *)heap.heap;

      /* Zero out the entire heap. */
      memset_(heap.startEntry, zero, HEAP_RAW_SIZE);


      /* The first heap entry is free at this point so mark it as such. */
      heap.startEntry->free = true;


      /* The first heap entry is UN-protected at this point so mark it as such. For an entry to be marked protected,
      the system must be in privileged mode by calling ENTER_PRIVILEGED(). Only heap memory allocated to the kernel
      can be protected. Heap memory allocated by the end-user cannot be protected. */
      heap.startEntry->protected = false;



      /* The first entry will contain all of the blocks in the heap at this point. */
      heap.startEntry->blocks = CONFIG_HEAP_SIZE_IN_BLOCKS;


      /* There is only one heap entry at this point so set the pointer to the next
      entry to null. */
      heap.startEntry->next = NULL;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE III: Check the health of the heap by calling HeapCheck().
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


    /* Assert if the heap is NOT healthy (i.e., contains consistency errors). */
    SYSASSERT(RETURN_SUCCESS == HeapCheck(HEAP_CHECK_HEALTH_ONLY, NULL));


    /* If the heap is healthy, then proceed to phase IV. */
    if (RETURN_SUCCESS == HeapCheck(HEAP_CHECK_HEALTH_ONLY, NULL)) {


      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      PHASE IV: Calculate how many blocks are needed for the requested size in bytes.
      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


      /* Calculate the quotient portion of the requested blocks by performing some division. */
      requestedBlocks = ((Word_t)(size_ / CONFIG_HEAP_BLOCK_SIZE));

      /* Calculate the remainder portion of the requested blocks by performing some division. If
      there is a remainder then add just one more block. */
      if (zero < ((Word_t)(size_ % CONFIG_HEAP_SIZE_IN_BLOCKS))) {


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
        PHASE VI: Found a good candidate so either reuse a free entry OR split the last
        entry in the heap. Oh, and we need to clear the memory at the same time.
        * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


        /* Check if the candidate entry is the last entry in the heap, if it is we will
        need to split the entry into two. */
        if (ISNULLPTR(entryCandidate->next)) {


          /* Assert if we can't split the block because if there is only one block (not including
          the heap entry) then you can't split it. */
          SYSASSERT(0x1u < ((Word_t)(entryCandidate->blocks - heap.entrySizeInBlocks)));


          /* If there is only one block left, we can't split it so skip splitting the block. */
          if (0x1u < ((Word_t)(entryCandidate->blocks - heap.entrySizeInBlocks))) {

            /* Let's update our candidate entry to point to the next entry which will contain
            the remain blocks after we perform the split. */
            entryCandidate->next = (HeapEntry_t *)((Byte_t *)entryCandidate + (requestedBlocks * CONFIG_HEAP_BLOCK_SIZE));

            /* Our next entry is free so mark it as such. */
            entryCandidate->next->free = true;

            /* Our next entry is also UN-protected so mark it as such. */
            entryCandidate->next->protected = false;

            /* Perform the split by calculating how many blocks the next entry will contain
            after we take what we need. */
            entryCandidate->next->blocks = entryCandidate->blocks - requestedBlocks;

            /* Our next entry doesn't have a entry after it so set its "next" to null. */
            entryCandidate->next->next = NULL;
          }


          /* Since we will be using the candidate entry, mark it as no longer free. */
          entryCandidate->free = false;

          /* If we are in privileged mode, then mark the candidate entry as protected.
          Otherwise, mark it as UN-protected. */
          if (true == SYSFLAG_PRIVILEGED()) {

            entryCandidate->protected = true;

          } else {

            entryCandidate->protected = false;
          }

          /* Update the candidate entry with how many blocks it contains including
          the blocks required for the heap entry. */
          entryCandidate->blocks = requestedBlocks;

          /* Clear the memory. */
          memset_((void *)((Byte_t *)entryCandidate + (heap.entrySizeInBlocks * CONFIG_HEAP_BLOCK_SIZE)), zero, (requestedBlocks - heap.entrySizeInBlocks) * CONFIG_HEAP_BLOCK_SIZE);



          /* Since the heap entry sits in the block prior to the blocks allocated for the und-user,
          we want to return a pointer to the start of the allocated space and NOT the heap entry
          itself. */
          ret = (void *)((Byte_t *)entryCandidate + (heap.entrySizeInBlocks * CONFIG_HEAP_BLOCK_SIZE));

        } else {


          /* Looks like the candidate entry is not at the end so we just need to mark it
          as taken. */
          entryCandidate->free = false;



          /* If we are in privileged mode, then mark the candidate entry as protected.
          Otherwise, mark it as UN-protected. */
          if (true == SYSFLAG_PRIVILEGED()) {

            entryCandidate->protected = true;

          } else {

            entryCandidate->protected = false;
          }


          /* Clear the memory. */
          memset_((void *)((Byte_t *)entryCandidate + (heap.entrySizeInBlocks * CONFIG_HEAP_BLOCK_SIZE)), zero, (requestedBlocks - heap.entrySizeInBlocks) * CONFIG_HEAP_BLOCK_SIZE);


          /* Since the heap entry sits in the block prior to the blocks allocated for the und user,
          we want to return a pointer to the start of the allocated space and NOT the heap entry
          itself. */
          ret = (void *)((Byte_t *)entryCandidate + (heap.entrySizeInBlocks * CONFIG_HEAP_BLOCK_SIZE));
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
void xMemFree(void *ptr_) {


  /* Disable interrupts because we can't be interrupted while modifying the heap. */
  DISABLE_INTERRUPTS();


  HeapEntry_t *entryToFree = NULL;



  /* Assert if the heap doesn't pass its health check OR if the pointer the end-user
  passed to us isn't a good one. */
  SYSASSERT(RETURN_SUCCESS == HeapCheck(HEAP_CHECK_HEALTH_AND_POINTER, ptr_));



  /* Check if the heap is healthy and the pointer the end-user passed to us
  is a good one. If everything checks out, proceed with freeing the memory. Otherwise,
  head toward the exit. */
  if (RETURN_SUCCESS == HeapCheck(HEAP_CHECK_HEALTH_AND_POINTER, ptr_)) {



    /* End-user gave us a pointer to the start of their allocated space in the heap, we
    need to move back one block to get to the heap entry. */
    entryToFree = (HeapEntry_t *)((Byte_t *)ptr_ - (heap.entrySizeInBlocks * CONFIG_HEAP_BLOCK_SIZE));


    /* Assert if the heap entry is protected and we are not in privileged mode. */
    SYSASSERT((false == entryToFree->protected) || ((true == entryToFree->protected) && (true == SYSFLAG_PRIVILEGED())));

    /* Check if we are in privileged mode if the heap entry is protected. If it is not protected, that
    is fine too. */
    if ((false == entryToFree->protected) || ((true == entryToFree->protected) && (true == SYSFLAG_PRIVILEGED()))) {


      /* Mark the entry as free. */
      entryToFree->free = true;

      /* Mark the entry as UN-protected. */
      entryToFree->protected = false;


      /* Never change the entry's blocks!! */
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

  HeapEntry_t *entryCursor = NULL;

  Word_t usedBlocks = zero;


  /* Assert if the heap does not pass its health check. */
  SYSASSERT(RETURN_SUCCESS == HeapCheck(HEAP_CHECK_HEALTH_ONLY, NULL));


  /* If the heap is healthy, we can proceed with calculating heap
  memory in use. Otherwise, just head toward the exit. */
  if (RETURN_SUCCESS == HeapCheck(HEAP_CHECK_HEALTH_ONLY, NULL)) {

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
    ret = usedBlocks * CONFIG_HEAP_BLOCK_SIZE;
  }


  return ret;
}




/* The xMemGetSize() system call returns the amount of memory in bytes that
is currently allocated to a specific pointer. */
size_t xMemGetSize(void *ptr_) {


  size_t ret = zero;


  HeapEntry_t *entryToSize = NULL;




  /* Assert if the heap failed its health check OR if the end-user scammed
  us on the pointer. */
  SYSASSERT(RETURN_SUCCESS == HeapCheck(HEAP_CHECK_HEALTH_AND_POINTER, ptr_));



  /* If the heap passes its health check and the pointer the end-user passed
  us is valid, then continue. */
  if (RETURN_SUCCESS == HeapCheck(HEAP_CHECK_HEALTH_AND_POINTER, ptr_)) {



    /* The end-user's pointer points to the start of their allocated space, we
    need to move back one block to read the entry. */
    entryToSize = (HeapEntry_t *)((Byte_t *)ptr_ - (heap.entrySizeInBlocks * CONFIG_HEAP_BLOCK_SIZE));


    /* The entry should not be free, also check if it is protected because if it is
    then we must be in privileged mode. */
    SYSASSERT((false == entryToSize->free) && ((false == entryToSize->protected) || ((false == entryToSize->free) && (true == entryToSize->protected) && (true == SYSFLAG_PRIVILEGED()))));


    /* The entry should not be free, also check if it is protected because if it is
    then we must be in privileged mode. */
    if ((false == entryToSize->free) && ((false == entryToSize->protected) || ((false == entryToSize->free) && (true == entryToSize->protected) && (true == SYSFLAG_PRIVILEGED())))) {



      /* The end-user is expecting us to return the number of bytes in used. So
      perform some advanced multiplication. */
      ret = entryToSize->blocks * CONFIG_HEAP_BLOCK_SIZE;
    }
  }


  EXIT_PRIVILEGED();

  return ret;
}




/* The CheckHeapHealth() function checks the health of the heap and optionally
will check that a pointer is valid at the same time. CheckHeapHealth() does
not respect the entry protected flag because it isn't changing anything. */
Base_t HeapCheck(const Base_t option_, const void *ptr_) {


  HeapEntry_t *entryCursor = NULL;

  HeapEntry_t *entryToCheck = NULL;

  Base_t ptrFound = false;

  Word_t blocks = zero;


  Base_t ret = RETURN_FAILURE;


  /* Assert if there is an invalid combination of arguments
  passed to function. */
  SYSASSERT(((HEAP_CHECK_HEALTH_ONLY == option_) && (ISNULLPTR(ptr_))) || ((HEAP_CHECK_HEALTH_AND_POINTER == option_) && (ISNOTNULLPTR(ptr_))));


  /* Check if there is an invalid combination of arguments passed
  to function before proceeding with checks. */
  if (((HEAP_CHECK_HEALTH_ONLY == option_) && (ISNULLPTR(ptr_))) || ((HEAP_CHECK_HEALTH_AND_POINTER == option_) && (ISNOTNULLPTR(ptr_)))) {


    /* Assert if the heap has not been initialized. */
    SYSASSERT(ISNOTNULLPTR(heap.startEntry));


    /* Check if the heap has been initialized, if it has then
    proceed with the checks. */
    if (ISNOTNULLPTR(heap.startEntry)) {

      entryCursor = heap.startEntry;




      /* If we need to also check that the end-user's pointer is valid at the same time,
      then we must calculate where its heap entry would be. */
      if (HEAP_CHECK_HEALTH_AND_POINTER == option_) {

        entryToCheck = (HeapEntry_t *)((Byte_t *)ptr_ - (heap.entrySizeInBlocks * CONFIG_HEAP_BLOCK_SIZE));
      }


      /* Traverse the heap and sum the blocks from
      each entry. */
      while (ISNOTNULLPTR(entryCursor)) {

        blocks += entryCursor->blocks;



        /* At the same time if we are checking for a pointer, let's find it. If
        found then set the pointer found variable to true. */
        if ((HEAP_CHECK_HEALTH_AND_POINTER == option_) && (entryCursor == entryToCheck) && (false == entryCursor->free)) {

          ptrFound = true;
        }

        /* Move on to the next heap entry. */
        entryCursor = entryCursor->next;
      }


      /* Assert if the blocks we summed while traversing the heap
      do not match what is expected. */
      SYSASSERT(CONFIG_HEAP_SIZE_IN_BLOCKS == blocks);

      /* Check if the blocks we summed while traversing the heap
      matches what we expected, if so proceed. */
      if (CONFIG_HEAP_SIZE_IN_BLOCKS == blocks) {


        /* Assert if the pointer was not found if we
        were looking for it. */
        SYSASSERT((HEAP_CHECK_HEALTH_ONLY == option_) || ((HEAP_CHECK_HEALTH_AND_POINTER == option_) && (true == ptrFound)));



        /* If we only have to check the heap health then set the
        return value to success OR if we are also checking that
        the pointer was valid then set the return value to success
        if the pointer's heap entry was found. */
        if ((HEAP_CHECK_HEALTH_ONLY == option_) || ((HEAP_CHECK_HEALTH_AND_POINTER == option_) && (true == ptrFound))) {


          ret = RETURN_SUCCESS;
        }
      }
    }
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

  for (Word_t i = zero; i < (HEAP_RAW_SIZE / MEMDUMP_ROW_WIDTH); i++) {


    printf("%p:", (heap.heap + k));

    for (Word_t j = zero; j < MEMDUMP_ROW_WIDTH; j++) {



      if (zero == *(heap.heap + k)) {


        printf(" --");


      } else {


        printf(" %02X", *(heap.heap + k));
      }

      k++;
    }



    printf("\n");
  }



  return;
}

#endif