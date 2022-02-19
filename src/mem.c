/**
 * @file mem.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for memory management
 * @version 0.3.0
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

extern SysFlags_t sysFlags;

static Byte_t heap[HEAP_RAW_SIZE];

static HeapEntry_t *start = (HeapEntry_t *)heap;

static Word_t entryBlocksNeeded = zero;

/* The xMemAlloc() system call will allocate heap memory and return a pointer
to the newly allocated memory. */
void *xMemAlloc(size_t size_) {
  /* Disable interrupts because we can't be interrupted while modifying the heap. */
  DISABLE_INTERRUPTS();

  void *ret = NULL;

  Word_t blocks = zero;

  Word_t requestedBlocks = zero;

  /* Requested blocks with overhead is the requested blocks + the number of blocks
  required for the heap entry. */
  Word_t requestedBlocksWithOverhead = zero;

  /* To get the maximum value of Word_t, we underflow the unsigned type. */
  Word_t leastBlocks = -1;

  HeapEntry_t *entryCursor = NULL;

  HeapEntry_t *entryCandidate = NULL;

  SYSASSERT(size_ > zero);

  /* Confirm the requested size in bytes is greater than zero. If not, just head toward
  return and return null. */
  if (size_ > zero) {
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE I: Determine how many blocks a heap entry requires. One block is generally
    sufficient but we shouldn't assume. This only needs to be done once.
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* If we haven't calculated how many blocks a heap entry requires, calculate
    it now. */
    if (entryBlocksNeeded == zero) {
      /* Calculate the quotient portion of the blocks needed for the heap entry. */
      entryBlocksNeeded = (Word_t)sizeof(HeapEntry_t) / CONFIG_HEAP_BLOCK_SIZE;

      /* Calculate the remainder portion of the blocks needed for the heap entry. If there is
      a remainder, add one more block to the blocks needed. */
      if ((sizeof(HeapEntry_t) % CONFIG_HEAP_BLOCK_SIZE) > zero) {
        /* Add one to the blocks needed since there is a remainder for the blocks
        needed. */
        entryBlocksNeeded++;
      }
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE II: Determine if the first heap entry has been created. This effectively
    initializes the heap. This also only needs to be done once.
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* If the heap entry at the start of the heap has zero blocks then it hasn't
    been initialized yet, so do that now. If it has then just move on.*/
    if (start->blocks == zero) {
      /* Zero out the entire heap. HEAP_RAW_SIZE equates to HEAP_SIZE_IN_BLOCKS * HEAP_BLOCK_SIZE. */
      memset_(heap, zero, HEAP_RAW_SIZE);

      /* Set the heap entry to free because, it is free. */
      start->free = true;

      /* Mark the entry unprotected by setting protected to false. An entry is protected if the macro ENTER_PROTECT()
      is called before invoking xMemAlloc(). A protected entry cannot be freed by xMemFree() unless ENTER_PROTECT()
      is called beforehand calling xMemFree().

      NOTE: Protected heap memory is ONLY for system calls, not for use by the end-user.*/
      start->protected = false;

      /* Set the number of blocks in the first entry to the total number of blocks
      in the heap heap minus one block which is occupied by the first heap entry. */
      start->blocks = CONFIG_HEAP_SIZE_IN_BLOCKS - entryBlocksNeeded;

      /* There is only one heap entry at this point so set the next to null. */
      start->next = NULL;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE III: Check the health of the heap by scanning through all of the heap entries
    counting how many blocks are in each entry then comparing that against the
    HEAP_SIZE_IN_BLOCKS setting. If the two do not match there is a problem!!
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* To scan the heap, set the heap entry cursor to the start of the heap. */
    entryCursor = start;

    /* While the heap entry cursor is not null, keep scanning. */
    while (ISNOTNULLPTR(entryCursor)) {
      /* Continue to sum the blocks while keeping in mind that the heap entries
      consume blocks too. */
      blocks += entryCursor->blocks + entryBlocksNeeded;

      /* Move on to the next heap entry. */
      entryCursor = entryCursor->next;
    }

    SYSASSERT(blocks == CONFIG_HEAP_SIZE_IN_BLOCKS);

    /* If the block count does not match HEAP_SIZE_IN_BLOCKS then we need
    to return because the heap is corrupt. Otherwise continue to phase IV. */
    if (blocks == CONFIG_HEAP_SIZE_IN_BLOCKS) {
      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
       PHASE IV: Calculate how many blocks are needed for the requested size in bytes.
       * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

      /* Calculate the quotient portion of the requested blocks by dividing the requested size
      paramater by the heap block size also in bytes. */
      requestedBlocks = (Word_t)size_ / CONFIG_HEAP_BLOCK_SIZE;

      /* Calculate the remainder portion of the requested blocks. If there is a remainder we
      need to add one more block. */
      if ((size_ % CONFIG_HEAP_SIZE_IN_BLOCKS) > zero) {
        /* There was a remainder for the requested blocks so add one more block. */
        requestedBlocks++;
      }

      /* Because the requested blocks also requires an additional heap entry (if not the first),
      calculate how many blocks are needed inclusive of the heap entry (i.e., the overhead). */
      requestedBlocksWithOverhead = requestedBlocks + entryBlocksNeeded;

      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      PHASE V: Scan the heap entries to find a heap entry that would be a good candidate
      for the requested blocks. This may be the last entry in the heap OR an entry that
      was recently freed by xMemFree().
      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

      /* To scan the heap, need to set the heap entry cursor to the start of the heap. */
      entryCursor = start;

      /* While the heap entry cursor is not null, keep scanning. */
      while (ISNOTNULLPTR(entryCursor)) {
        /* See if there is a candidate heap entry for the requested blocks by checking:
            1) The entry at the cursor is free.
            2) The entry has enough blocks to cover the requested blocks with overhead.
            3) The entry has the fewest possible number of blocks based on our need.*/
        if ((entryCursor->free == true) && (entryCursor->blocks >= requestedBlocksWithOverhead) && (entryCursor->blocks < leastBlocks)) {
          /* Seems like a good candidate so update the least blocks in case
          there is an entry with fewer blocks that is free yet will fit
          the requested blocks with overhead. */
          leastBlocks = entryCursor->blocks;

          /* Keep a copy of the entry cursor as the best entry candidate in case we find out
          that the candidate is the winner. */
          entryCandidate = entryCursor;
        }

        /* Move on to the next entry. */
        entryCursor = entryCursor->next;
      }

      /* If the entry candidate is null, well.... we can't fulfill the request so
      move on and return null. */
      if (ISNOTNULLPTR(entryCandidate)) {
        /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
        PHASE VI: Found a good candidate so either reuse a free entry OR split the last
        entry in the heap. We will also clear the memory at this time.
        * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

        /* If the entry candidate's next is null, then this is the last entry in the heap
        so split the entry blocks in two. */
        if (ISNULLPTR(entryCandidate->next)) {
          /* Set the entry candidate "next" to the new entry that will contain the remaining
          unused blocks. */
          entryCandidate->next = (HeapEntry_t *)((Byte_t *)entryCandidate + (requestedBlocksWithOverhead * CONFIG_HEAP_BLOCK_SIZE));

          /* Mark the new entry as free. */
          entryCandidate->next->free = true;

          /* Mark the new entry as unprotected. */
          entryCandidate->next->protected = false;

          /* Calculate how many remaining blocks there are and update the new entry. */
          entryCandidate->next->blocks = entryCandidate->blocks - requestedBlocksWithOverhead;

          /* Set the new entry's "next" to null since it is now the last entry in the heap. */
          entryCandidate->next->next = NULL;

          /* Mark the candidate entry as no longer free. */
          entryCandidate->free = false;

          /* Set the entry protection based on the privileged system flag. */
          if (SYSFLAG_PRIVILEGED() == true) {

            entryCandidate->protected = true;

          } else {

            entryCandidate->protected = false;
          }

          /* Store how many blocks the entry contains. */
          entryCandidate->blocks = requestedBlocks;

          /* Clear the memory by mem-setting it to all zeros. */
          memset_((void *)((Byte_t *)entryCandidate + (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE)), zero, requestedBlocks * CONFIG_HEAP_BLOCK_SIZE);

          /* Return the address of the memory but make sure we move it forward
          enough so the end-user doesn't write to the heap entry. */
          ret = (void *)((Byte_t *)entryCandidate + (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE));

        } else {
          /* Looks like we found a candidate that is NOT the last entry in the heap,
          so simply claim it for France. */
          entryCandidate->free = false;

          /* Set the entry protection based on the privileged system flag. */
          if (SYSFLAG_PRIVILEGED() == true) {

            entryCandidate->protected = true;

          } else {

            entryCandidate->protected = false;
          }


          /* Clear the memory by mem-setting it to all zeros. */
          memset_((void *)((Byte_t *)entryCandidate + (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE)), zero, requestedBlocks * CONFIG_HEAP_BLOCK_SIZE);

          /* Return the address of the memory but make sure we move it forward
          enough so the end-user doesn't write to the heap entry. */
          ret = (void *)((Byte_t *)entryCandidate + (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE));
        }
      }
    }
  }

  /* Exit protect and enable interrupts before returning. */

  EXIT_PRIVILEGED();

  ENABLE_INTERRUPTS();

  return ret;
}

/* The xMemFree() system call will free heap memory pointed to by the pointer parameter. */
void xMemFree(void *ptr_) {
  /* Disable interrupts because we can't be interrupted while modifying the heap. */
  DISABLE_INTERRUPTS();

  Word_t blocks = zero;

  HeapEntry_t *entryCursor = NULL;

  HeapEntry_t *entryToFree = NULL;

  SYSASSERT(ISNOTNULLPTR(ptr_));

  /* Check to make sure the end-user passed a pointer that is at least not null. If it is null,
  then move on and return. */
  if (ISNOTNULLPTR(ptr_)) {
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE I: Determine if the first heap entry has been created. If it hasn't then
    just return.
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    SYSASSERT(start->blocks != zero);

    /* Check if the entry at the start of the heap is un-initialized by looking
    at the blocks member. If it is zero, then the heap has not been initialized so
    just thrown in the towel. */
    if (start->blocks != zero) {
      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      PHASE II: Check the health of the heap by scanning through all of the heap entries
      counting how many blocks are in each entry then comparing that against the
      HEAP_SIZE_IN_BLOCKS setting. If the two do not match there is a problem!!
      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

      /* To scan the heap, set the heap entry cursor to the start of the heap. */
      entryCursor = start;

      /* While the heap entry cursor is not null, keep scanning. */
      while (ISNOTNULLPTR(entryCursor)) {
        blocks += entryCursor->blocks + entryBlocksNeeded; /* Assuming entry blocks needed has been
                                                                  calculated if the heap has been initialized. */

        /* Move on to the next heap entry. */
        entryCursor = entryCursor->next;
      }

      SYSASSERT(blocks == CONFIG_HEAP_SIZE_IN_BLOCKS);

      /* Check if the counted blocks matches the HEAP_SIZE_IN_BLOCKS setting,
      if it doesn't return (i.e., Houston, we've had a problem.) */
      if (blocks == CONFIG_HEAP_SIZE_IN_BLOCKS) {
        /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
        PHASE III: Check if the pointer paramater actually points to a heap entry that
        by scanning the heap for it. If it exists, free the entry.

        Don't ever just directly check that the pointer references what APPEARS to be a
        heap entry - always traverse the heap!!
        * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

        /* Determine the heap entry to free by moving back from the pointer by the byte size of one
        heap entry. */
        entryToFree = (HeapEntry_t *)((Byte_t *)ptr_ - (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE));

        /* To scan the heap, set the heap entry cursor to the start of the heap. */
        entryCursor = start;

        /* While the heap entry cursor is not null, keep scanning. */
        while (ISNOTNULLPTR(entryCursor)) {
          /* If the entry cursor equals the entry we want to free, then break out of the loop. */
          if (entryCursor == entryToFree) {
            break;
          }

          /* Move on to the next heap entry. */
          entryCursor = entryCursor->next;
        }

        SYSASSERT(ISNOTNULLPTR(entryCursor));

        /* Check if the entry cursor is null, if it is we couldn't find the entry to be freed. If it is
        not null then proceed with marking the entry free. */
        if (ISNOTNULLPTR(entryCursor)) {

          SYSASSERT(entryCursor == entryToFree);

          /* Check one last time if the entry cursor equals the entry we want to free, if it does,
           mark it free. */
          if (entryCursor == entryToFree) {

            SYSASSERT((entryCursor->protected == false) || ((entryCursor->protected == true) && (SYSFLAG_PRIVILEGED() == true)));

            /* If the entry is marked protected and the protect system flag is false,
            then return because a protected entry cannot be freed while the protect
            system flag is false. */
            if ((entryCursor->protected == false) || ((entryCursor->protected == true) && (SYSFLAG_PRIVILEGED() == true))) {
              /* Make the entry free by setting free to true. */
              entryCursor->free = true;

              /* Mark the entry as unprotected. */
              entryCursor->protected = false;
            }
          }
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

  HeapEntry_t *entryCursor = NULL;

  Word_t usedBlocks = zero;


  /* Assert if the end-user passed a null pointer. */
  SYSASSERT(ISNOTNULLPTR(ptr_));


  /* Check to make sure the end-user passed a pointer that is at least not null. */
  if (ISNOTNULLPTR(ptr_)) {

    /* Check if the heap is un-initialized or unhealthy. */
    SYSASSERT(CHECK_HEAP_HEALTH_SUCCESS == CheckHeapHealth(CHECK_HEAP_HEALTH_ONLY, NULL));


    /* If the heap is initialized and healthy, then proceed with summing up the
    blocks that are in use. */
    if (CHECK_HEAP_HEALTH_SUCCESS == CheckHeapHealth(CHECK_HEAP_HEALTH_ONLY, NULL)) {

      entryCursor = start;

      /* While the heap entry cursor is not null, keep scanning. */
      while (ISNOTNULLPTR(entryCursor)) {


        /* At each entry, check to see if it is in use. If it is, add the number
        of blocks it contains plus the number of blocks consumed by the heap entry
        block to the used block count. */
        if (entryCursor->free == false) {

          /* Sum the number of used blocks for each heap entry in use. */
          usedBlocks += entryCursor->blocks + entryBlocksNeeded;
        }

        /* Move on to the next heap entry. */
        entryCursor = entryCursor->next;
      }

      /* End-user is expecting bytes, so calculate it based on the
      block size. */
      ret = usedBlocks * CONFIG_HEAP_BLOCK_SIZE;
    }
  }

  return ret;
}


/* The xMemGetSize() system call returns the amount of memory in bytes that
is currently allocated to a specific pointer. */
size_t xMemGetSize(void *ptr_) {
  size_t ret = zero;


  HeapEntry_t *entryToSize = NULL;


  /* Assert if the end-user passed a null pointer. */
  SYSASSERT(ISNOTNULLPTR(ptr_));


  /* Check to make sure the end-user passed a pointer that is at least not null. */
  if (ISNOTNULLPTR(ptr_)) {


    /* Check if the heap is un-initialized, unhealthy or if the end-user passed
    pointer is an invalid pointer. */
    SYSASSERT(CHECK_HEAP_HEALTH_SUCCESS == CheckHeapHealth(CHECK_HEAP_HEALTH_AND_PTR, ptr_));


    /* If the heap is initialized, healthy and if the end-user passed pointer is
    valid then we can continue. */
    if (CHECK_HEAP_HEALTH_SUCCESS == CheckHeapHealth(CHECK_HEAP_HEALTH_AND_PTR, ptr_)) {

      /* Need to calculate the location of the heap entry for the end-user passed pointer. */
      entryToSize = (HeapEntry_t *)((Byte_t *)ptr_ - (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE));


      /* The entry should not be free, also check if it is protected because if it is
      then we must be in privileged mode. */
      SYSASSERT((entryToSize->free == false) && ((entryToSize->protected == false) || ((entryToSize->free == false) && (entryToSize->protected == true) && (SYSFLAG_PRIVILEGED() == true))));


      /* Make sure the entry is not free, also if protected we must also be in privileged
      mode. */
      if ((entryToSize->free == false) && ((entryToSize->protected == false) || ((entryToSize->free == false) && (entryToSize->protected == true) && (SYSFLAG_PRIVILEGED() == true)))) {



        /* We want to return the amount of BYTES in use by the pointer so multiply the
        blocks consumed by the entry by the HEAP_BLOCK_SIZE. */
        ret = entryToSize->blocks * CONFIG_HEAP_BLOCK_SIZE;
      }
    }
  }

  EXIT_PRIVILEGED();

  return ret;
}

/* The CheckHeapHealth() function checks the health of the heap and optionally
will check that a pointer is valid at the same time. */
Base_t CheckHeapHealth(Base_t option_, void *ptr_) {
  HeapEntry_t *entryCursor = NULL;

  HeapEntry_t *entryToFind = NULL;

  Base_t ptrFound = false;

  Word_t blocks = zero;

  Base_t ret = CHECK_HEAP_HEALTH_FAILURE;


  /* Assert if there is an invalid combination of arguments
  passed to function. */
  SYSASSERT(((CHECK_HEAP_HEALTH_ONLY == option_) && (ISNULLPTR(ptr_))) || ((CHECK_HEAP_HEALTH_AND_PTR == option_) && (ISNOTNULLPTR(ptr_))));


  /* Check if there is an invalid combination of arguments passed
  to function before proceeding with checks. */
  if (((CHECK_HEAP_HEALTH_ONLY == option_) && (ISNULLPTR(ptr_))) || ((CHECK_HEAP_HEALTH_AND_PTR == option_) && (ISNOTNULLPTR(ptr_)))) {


    /* Assert if the heap has not been initialized. */
    SYSASSERT(zero != start->blocks);


    /* Check if the heap has been initialized, if it has then
    proceed with the checks. */
    if (zero != start->blocks) {

      entryCursor = start;


      /* If we need to also check that a pointer is valid at the same time,
      then we must calculate where its heap entry would be. */
      if (CHECK_HEAP_HEALTH_AND_PTR == option_) {

        entryToFind = (HeapEntry_t *)((Byte_t *)ptr_ - (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE));
      }

      /* Traverse the heap and sum the blocks from
      each entry. */
      while (ISNOTNULLPTR(entryCursor)) {

        blocks += entryCursor->blocks + entryBlocksNeeded;

        /* At the same time if we are checking for a pointer, let's find it. If
        found then set the pointer found variable to true. */
        if ((CHECK_HEAP_HEALTH_AND_PTR == option_) && (entryCursor == entryToFind)) {

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


        /* If we only have to check the heap health then set the
        return value to success OR if we are also checking that
        the pointer was valid then set the return value to success
        if the pointer's heap entry was found. */
        if ((CHECK_HEAP_HEALTH_ONLY == option_) || ((CHECK_HEAP_HEALTH_AND_PTR == option_) && (true == ptrFound))) {

          ret = CHECK_HEAP_HEALTH_SUCCESS;
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