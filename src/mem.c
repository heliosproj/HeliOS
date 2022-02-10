/**
 * @file mem.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for the management of heap memory in HeliOS
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
extern TaskList_t *taskList;

static Byte_t heap[HEAP_RAW_SIZE];

HeapEntry_t *heapStart = (HeapEntry_t *)heap;

Word_t entryBlocksNeeded = 0;

void *xMemAlloc(size_t size_) {
  /* Disable interrupts because we can't be interrupted while modifying the heap. */
  DISABLE_INTERRUPTS();

  Word_t blockCount = 0;

  Word_t requestedBlocks = 0;

  /* Requested blocks with overhead is the requested blocks + the number of blocks
  required for the heap entry. */
  Word_t requestedBlocksWithOverhead = 0;

  /* To get the maximum value of Word_t, we underflow the unsigned type. */
  Word_t leastBlocks = -1;

  HeapEntry_t *entryCursor = null;

  HeapEntry_t *entryCandidate = null;

  /* Confirm the requested size in bytes is greater than zero. */
  if (size_ > 0) {
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE I: Determine how many blocks a heap entry requires. One block is generally
    sufficient but we shouldn't assume. This only needs to be done once.
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* If we haven't calculated how many blocks a heap entry requires, calculate
    it now. */
    if (entryBlocksNeeded == 0) {
      /* Calculate the quotient of the blocks needed for the heap entry. */
      entryBlocksNeeded = sizeof(HeapEntry_t) / CONFIG_HEAP_BLOCK_SIZE;

      /* Calculate the remainder of the blocks needed for the heap entry. If there is
      a remainder add one more block to the blocks needed. */
      if (sizeof(HeapEntry_t) % CONFIG_HEAP_BLOCK_SIZE > 0) {
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
    been initialized yet, so do that now. */
    if (heapStart->blocks == 0) {
      /* Zero out the entire heap. HEAP_RAW_SIZE equates to CONFIG_HEAP_SIZE_IN_BLOCKS * CONFIG_HEAP_BLOCK_SIZE. */
      memset_(heap, 0, HEAP_RAW_SIZE);

      /* Set the heap entry to free because, it is free. */
      heapStart->free = true;

      /* Mark the entry unprotected by setting protected to false. An entry is protected if the macro ENTER_PROTECT()
      is called before invoking xMemAlloc(). A protected entry cannot be freed by xMemFree() unless ENTER_PROTECT()
      is called beforehand. */
      heapStart->protected = false;

      /* Set the number of blocks in the first entry to the total number of blocks
      in the heap heap minus one block which is occupied by the first heap entry. */
      heapStart->blocks = CONFIG_HEAP_SIZE_IN_BLOCKS - entryBlocksNeeded;

      /* There is only one heap entry at this point so set the next to null. */
      heapStart->next = null;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE III: Check the health of the heap by scanning through all of the heap entries
    counting how many blocks are in each entry then comparing that against the
    CONFIG_HEAP_SIZE_IN_BLOCKS setting. If the two do not match there is a problem!!
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* To scan the heap, set the heap entry cursor to the start of the heap. */
    entryCursor = heapStart;

    /* While the heap entry cursor is not null, keep scanning. */
    while (ISNOTNULLPTR(entryCursor)) {
      /* Continue to sum the blocks while keeping in mind that the heap entries
      consume blocks too. */
      blockCount += entryCursor->blocks + entryBlocksNeeded;

      /* Move on to the next heap entry. */
      entryCursor = entryCursor->next;
    }

    /* If the block count does not match CONFIG_HEAP_SIZE_IN_BLOCKS then we need
    to return because the heap is corrupt. */
    if (blockCount != CONFIG_HEAP_SIZE_IN_BLOCKS) {
      /* Exit protect and enable interrupts before returning. */

      EXIT_PROTECT();

      ENABLE_INTERRUPTS();

      return null;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE IV: Calculate how many blocks are needed for the requested size in bytes.
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* Calculate the quotient of the requested blocks by dividing the requested size
    paramater by the heap block size also in bytes. */
    requestedBlocks = size_ / CONFIG_HEAP_BLOCK_SIZE;

    /* Calculate the remainder of the requested blocks. If there is a remainder we
    need to add one more block. */
    if (size_ % CONFIG_HEAP_SIZE_IN_BLOCKS > 0) {
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
    entryCursor = heapStart;

    /* While the heap entry cursor is not null, keep scanning. */
    while (ISNOTNULLPTR(entryCursor)) {
      /* See if there is a candidate heap entry for the requested blocks by checking:
          1) The entry at the cursor is free.
          2) The entry has enough blocks to cover the requested blocks with overhead.
          3) The entry has the least possible number of blocks.*/
      if (entryCursor->free == true && entryCursor->blocks >= requestedBlocksWithOverhead && entryCursor->blocks < leastBlocks) {
        /* Seems like a good candidate so update the least blocks in case
        there is an entry with fewer blocks that is free yet will fit
        the requested blocks with overhead. */
        leastBlocks = entryCursor->blocks;

        /* Keep a copy of the entry cursor as the best entry candidate. */
        entryCandidate = entryCursor;
      }

      /* Move on to the next entry. */
      entryCursor = entryCursor->next;
    }

    /* If the entry candidate is null, well.... we can't fulfill the request so
    return null. */
    if (ISNULLPTR(entryCandidate)) {
      /* Exit protect and enable interrupts before returning. */

      EXIT_PROTECT();

      ENABLE_INTERRUPTS();

      return null;
    }

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

      /* Set the new entry's "next: to null since it is now the last entry in the heap. */
      entryCandidate->next->next = null;

      /* Mark the candidate entry as no longer free. */
      entryCandidate->free = false;

      /* Set the entry protection based on the protect system flag. */
      entryCandidate->protected = SYSFLAG_PROTECT();

      /* Store how many blocks the entry contains. */
      entryCandidate->blocks = requestedBlocks;

      /* Clear the memory by mem-setting it to all zeros. */
      memset_((void *)((Byte_t *)entryCandidate + (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE)), 0, requestedBlocks * CONFIG_HEAP_BLOCK_SIZE);

      /* Exit protect and enable interrupts before returning. */

      EXIT_PROTECT();

      ENABLE_INTERRUPTS();

      /* Return the address of the memory but make sure we move it forward
      enough so the end-user doesn't write to the heap entry. */
      return (void *)((Byte_t *)entryCandidate + (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE));

    } else {
      /* Looks like we found a candidate that is NOT the last entry in the heap,
      so simply mark it as no longer free. */
      entryCandidate->free = false;

      /* Set the entry protection based on the protect system flag. */
      entryCandidate->protected = SYSFLAG_PROTECT();

      /* Clear the memory by mem-setting it to all zeros. */
      memset_((void *)((Byte_t *)entryCandidate + (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE)), 0, requestedBlocks * CONFIG_HEAP_BLOCK_SIZE);

      /* Exit protect and enable interrupts before returning. */

      EXIT_PROTECT();

      ENABLE_INTERRUPTS();

      /* Return the address of the memory but make sure we move it forward
      enough so the end-user doesn't write to the heap entry. */
      return (void *)((Byte_t *)entryCandidate + (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE));
    }
  }

  /* Exit protect and enable interrupts before returning. */

  EXIT_PROTECT();

  ENABLE_INTERRUPTS();

  return null;
}

void xMemFree(void *ptr_) {
  /* Disable interrupts because we can't be interrupted while modifying the heap. */
  DISABLE_INTERRUPTS();

  Word_t blockCount = 0;

  HeapEntry_t *entryCursor = null;

  HeapEntry_t *entryToFree = null;

  /* Check to make sure the end-user passed a pointer that is at least not null. */
  if (ISNOTNULLPTR(ptr_)) {
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE I: Determine if the first heap entry has been created. If it hasn't then
    just return.
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* Check if the entry at the start of the heap is un-initialized by looking
    at the blocks member. If it is zero, then the heap has not been initialized so
    just thrown in the towel. */
    if (heapStart->blocks == 0) {
      /* Exit protect and enable interrupts before returning. */
      EXIT_PROTECT();

      ENABLE_INTERRUPTS();

      return;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE II: Check the health of the heap by scanning through all of the heap entries
    counting how many blocks are in each entry then comparing that against the
    CONFIG_HEAP_SIZE_IN_BLOCKS setting. If the two do not match there is a problem!!
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* To scan the heap, set the heap entry cursor to the start of the heap. */
    entryCursor = heapStart;

    /* While the heap entry cursor is not null, keep scanning. */
    while (ISNOTNULLPTR(entryCursor)) {
      blockCount += entryCursor->blocks + entryBlocksNeeded; /* Assuming entry blocks needed has been
                                                                calculated if the heap has been initialized. */

      /* Move on to the next heap entry. */
      entryCursor = entryCursor->next;
    }

    /* Check if the counted blocks matches the CONFIG_HEAP_SIZE_IN_BLOCKS setting,
    if it doesn't return (i.e., Houston, we've had a problem.) */
    if (blockCount != CONFIG_HEAP_SIZE_IN_BLOCKS) {
      /* Exit protect and enable interrupts before returning. */
      EXIT_PROTECT();

      ENABLE_INTERRUPTS();

      return;
    }

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
    entryCursor = heapStart;

    /* While the heap entry cursor is not null, keep scanning. */
    while (ISNOTNULLPTR(entryCursor)) {
      /* If the entry cursor equals the entry we want to free, then break out of the loop. */
      if (entryCursor == entryToFree) {
        break;
      }

      /* Move on to the next heap entry. */
      entryCursor = entryCursor->next;
    }

    /* Well, we didn't find the entry for the pointer the end-user wanted freed so
    return. */
    if (ISNULLPTR(entryCursor)) {
      /* Exit protect and enable interrupts before returning. */
      EXIT_PROTECT();

      ENABLE_INTERRUPTS();

      return;
    }

    /* Check one last time if the entry cursor equals the entry we want to free, if it does,
    mark it free. We are done here. */
    if (entryCursor == entryToFree) {
      /* If the entry is marked protected and the protect system flag is false,
      then return because a protected entry cannot be freed while the protect
      system flag is false. */
      if (entryCursor->protected == true && SYSFLAG_PROTECT() == false) {
        /* Exit protect and enable interrupts before returning. */

        EXIT_PROTECT();

        ENABLE_INTERRUPTS();

        return;
      }

      /* Make the entry free by setting free to true. */
      entryCursor->free = true;

      /* Mark the entry as unprotected. */
      entryCursor->protected = false;
    }
  }

  /* Exit protect and enable interrupts before returning. */
  EXIT_PROTECT();

  ENABLE_INTERRUPTS();

  return;
}

/* The xMemGetUsed() system call returns the amount of memory in bytes
that is currently allocated. */
size_t xMemGetUsed() {
  Word_t blockCount = 0;

  Word_t usedBlockCount = 0;

  HeapEntry_t *entryCursor = null;

  /* Check if the entry at the start of the heap is un-initialized by looking
  at the blocks member. If it is zero then the heap has not been initialized so
  just thrown in the towel. */
  if (heapStart->blocks == 0) {
    return 0;
  }

  /* To scan the heap, need to set the heap entry cursor to the start of the heap. */
  entryCursor = heapStart;

  /* While the heap entry cursor is not null, keep scanning. */
  while (ISNOTNULLPTR(entryCursor)) {
    blockCount += entryCursor->blocks + entryBlocksNeeded; /* Assuming entry blocks needed has been
                                                              calculated if the heap has been initialized. */

    if (entryCursor->free == false) {
      usedBlockCount += entryCursor->blocks + entryBlocksNeeded;
    }

    entryCursor = entryCursor->next;
  }

  /* Check if the counted blocks matches the CONFIG_HEAP_SIZE_IN_BLOCKS setting,
  if it doesn't return. */
  if (blockCount != CONFIG_HEAP_SIZE_IN_BLOCKS) {
    return 0;
  }

  return usedBlockCount * CONFIG_HEAP_BLOCK_SIZE;
}

/* The xMemGetSize() system call returns the amount of memory in bytes that
is currently allocated to a specific pointer. */
size_t xMemGetSize(void *ptr_) {
  Word_t blockCount = 0;

  HeapEntry_t *entryCursor = null;

  HeapEntry_t *entryToSize = null;

  /* Check to make sure the end-user passed a pointer that is at least not null. */
  if (ISNOTNULLPTR(ptr_)) {
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE I: Determine if the first heap entry has been created. If it hasn't then
    just return.
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* Check if the entry at the start of the heap is un-initialized by looking
    at the blocks member. If it is zero then the heap has not been initialized so
    just thrown in the towel. */
    if (heapStart->blocks == 0) {
      return 0;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE II: Check the health of the heap but scanning through all of the heap entries
    counting how many blocks are in each entry then comparing that against the
    CONFIG_HEAP_SIZE_IN_BLOCKS setting. If the two do not match there is a problem!!
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* To scan the heap, need to set the heap entry cursor to the start of the heap. */
    entryCursor = heapStart;

    /* While the heap entry cursor is not null, keep scanning. */
    while (ISNOTNULLPTR(entryCursor)) {
      blockCount += entryCursor->blocks + entryBlocksNeeded; /* Assuming entry blocks needed has been
                                                                calculated if the heap has been initialized. */
      entryCursor = entryCursor->next;
    }

    /* Check if the counted blocks matches the CONFIG_HEAP_SIZE_IN_BLOCKS setting,
    if it doesn't return. */
    if (blockCount != CONFIG_HEAP_SIZE_IN_BLOCKS) {
      return 0;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    PHASE III: Check if the pointer paramater actually points to a heap entry that
    by scanning the heap for it. If it exists, free the entry.

    Don't ever just directly check that the pointer references what APPEARS to be a
    heap entry!
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* Determine the heap entry to free by moving back from the pointer by the byte size of one
    heap entry. */
    entryToSize = (HeapEntry_t *)((Byte_t *)ptr_ - (entryBlocksNeeded * CONFIG_HEAP_BLOCK_SIZE));

    /* To scan the heap, need to set the heap entry cursor to the start of the heap. */
    entryCursor = heapStart;

    /* While the heap entry cursor is not null, keep scanning. */
    while (ISNOTNULLPTR(entryCursor)) {
      /* If the entry cursor equals the entry we want to free, then break out of the loop. */
      if (entryCursor == entryToSize) {
        break;
      }
      entryCursor = entryCursor->next;
    }

    /* Well, we didn't find the entry for the pointer the end-user wanted freed so
    return. */
    if (ISNULLPTR(entryCursor)) {
      return 0;
    }

    return entryCursor->blocks * CONFIG_HEAP_BLOCK_SIZE;
  }

  return 0;
}

/* The built-in utility function to copy memory between the source and destination pointers. */
void memcpy_(void *dest_, const void *src_, size_t n_) {
  char *src = (char *)src_;
  char *dest = (char *)dest_;

  for (size_t i = 0; i < n_; i++) {
    dest[i] = src[i];
  }
}

/* The built-in utility function to set the memory pointed to by the destination pointer
to the specified value. */
void memset_(void *dest_, int16_t val_, size_t n_) {
  char *dest = (char *)dest_;

  for (size_t i = 0; i < n_; i++) {
    dest[i] = (char)val_;
  }
}

/* The built-in utility function to compare the contents of memory at two locations pointed to by
the pointers s1 and s2. */
int16_t memcmp_(const void *s1_, const void *s2_, size_t n_) {
  char *s1 = (char *)s1_;
  char *s2 = (char *)s2_;

  for (size_t i = 0; i < n_; i++) {
    if (*s1 != *s2)
      return *s1 - *s2;
    s1++;
    s2++;
  }
  return 0;
}