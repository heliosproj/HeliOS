/**
 * @file mem.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for memory management
 * @version 0.3.6
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

static Base_t __MemoryRegionCheck__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, const Base_t option_);
static Base_t __MemoryRegionCheckAddr__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static Addr_t *__calloc__(volatile MemoryRegion_t *region_, const Size_t size_);
static void __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static MemoryRegionStats_t *__MemGetRegionStats__(const volatile MemoryRegion_t *region_);
static void __DefragMemoryRegion__(const volatile MemoryRegion_t *region_);


void __MemoryInit__(void) {

  __memset__(&heap, 0x0, sizeof(MemoryRegion_t));

  heap.minAvailableEver = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE;

  __memset__(&kernel, 0x0, sizeof(MemoryRegion_t));

  kernel.minAvailableEver = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS * CONFIG_MEMORY_REGION_BLOCK_SIZE;

  return;
}

/* System call used by end-user tasks to allocate memory
   from the heap memory region. */
Addr_t *xMemAlloc(const Size_t size_) {

  /* Just call __calloc__() with the heap memory region
     to allocate memory from the heap. */
  return __calloc__(&heap, size_);
}



/* System call used by end-user tasks to free memory from
   the heap memory region. */
void xMemFree(const volatile Addr_t *addr_) {


  /* Just call __free__() with the heap memory region to free
     from the heap. */
  __free__(&heap, addr_);

  return;
}



/* System call to find out how much memory is allocated
   in the heap memory region. */
Size_t xMemGetUsed(void) {


  Size_t ret = zero;


  MemoryEntry_t *cursor = NULL;


  HalfWord_t used = zero;



  /* Assert if any memory region is corrupt. */
  SYSASSERT(false == SYSFLAG_FAULT());



  /* Check to make sure no memory regions are
     corrupt before we do anything. */
  if (false == SYSFLAG_FAULT()) {


    /* Assert if the check of the heap memory region fails. */
    SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheck__(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR));



    /* Check if the heap memory region is consistent. */
    if (RETURN_SUCCESS == __MemoryRegionCheck__(&heap, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR)) {

      cursor = heap.start;


      /* Traverse the memory region as long as there is
         something to traverse. */
      while (ISNOTNULLPTR(cursor)) {



        /* If the entry pointed to by the cursor is not
           free, then add its blocks to the used block count. */
        if (false == cursor->free) {



          used += cursor->blocks;
        }


        /* Move on to the next entry. */
        cursor = cursor->next;
      }



      /* The end-user is expecting bytes, not blocks so multiply
         the block size by the number of used blocks. */
      ret = used * CONFIG_MEMORY_REGION_BLOCK_SIZE;
    }
  }

  return ret;
}



/* A system call used by end-user tasks to return the amount
   of memory in bytes assigned to an address in the heap memory
   region. */
Size_t xMemGetSize(const volatile Addr_t *addr_) {


  Size_t ret = zero;


  MemoryEntry_t *tosize = NULL;



  /* Assert if any memory region is corrupt. */
  SYSASSERT(false == SYSFLAG_FAULT());



  /* Check to make sure no memory regions are
     corrupt before we do anything. */
  if (false == SYSFLAG_FAULT()) {



    /* Assert if the heap memory region fails its consistency check OR if the memory address is not
       valid for the heap memory region. */
    SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheck__(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR));



    /* Check if the heap memory region is consistent AND if the memory address is valid for the heap
       memory region. */
    if (RETURN_SUCCESS == __MemoryRegionCheck__(&heap, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {



      /* Get the memory region entry for the address to get the size of. */
      tosize = ADDR2ENTRY(addr_, &heap);



      /* Assert if it is free. */
      SYSASSERT(false == tosize->free);



      /* Check to make sure the entry is not free. */
      if (false == tosize->free) {



        /* The end-user is expecting the size in bytes so multiple the block
           size by the number of blocks the entry contains. */
        ret = tosize->blocks * CONFIG_MEMORY_REGION_BLOCK_SIZE;
      }
    }
  }



  return ret;
}



/* The __MemoryRegionCheck__() function checks the consistency of a memory region. If specified, it will also check
   that an address points to valid memory that was previously allocated by __calloc__() for the respective memory
   region. */
static Base_t __MemoryRegionCheck__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_, const Base_t option_) {


  MemoryEntry_t *cursor = NULL;

  MemoryEntry_t *find = NULL;

  Base_t found = false;

  HalfWord_t blocks = zero;


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
        SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheckAddr__(region_, cursor));



        /* Check if the memory address of the cursor is inside the scope of the
           memory region, this is important in case an entry is corrupt we don't
           want to inadvertently access some other area of memory. */
        if (RETURN_SUCCESS == __MemoryRegionCheckAddr__(region_, cursor)) {

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
          SYSFLAG_FAULT() = true;


          /* No point in continuing to traverse the memory entries in the memory region
             so break out of the loop. */
          break;
        }
      }


      /* Assert if the memory region blocks does not match the setting because this would
         indicate a serious issued. */
      SYSASSERT(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks);


      /* Check if the number of blocks in the memory region matches the setting before
         we give the memory region a clean bill of health. */
      if (CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS == blocks) {


        /* Assert if the memory region is flagged corrupt or if the address we were looking
           for was not found. */
        SYSASSERT(((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == SYSFLAG_FAULT())) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false == SYSFLAG_FAULT()) && (true == found)));


        /* Check that the memory region is not flagged corrupt and we found the memory address
           we were looking before. */
        if (((MEMORY_REGION_CHECK_OPTION_WO_ADDR == option_) && (false == SYSFLAG_FAULT())) || ((MEMORY_REGION_CHECK_OPTION_W_ADDR == option_) && (false == SYSFLAG_FAULT()) && (true == found))) {


          /* Things look good so set the return value to success. */
          ret = RETURN_SUCCESS;



        } /* Never use an else statement here to mark SYSFLAG_FAULT() = true. Just because an address wasn't
             found does not mean the memory region is corrupt. */



      } else {


        /* The number of blocks counted in the memory region does not match the setting
           so something is seriously wrong, so set the corrupt system flag. */
        SYSFLAG_FAULT() = true;
      }
    }
  }

  return ret;
}



/* Function to check if an address falls within the scope of a memory region. This
   function is used exclusively by __MemoryRegionCheck__(). */
static Base_t __MemoryRegionCheckAddr__(const volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {




  Base_t ret = RETURN_FAILURE;


  /* Assert if the address is outside of the scope of the memory region. */
  SYSASSERT((addr_ >= (Addr_t *)(region_->mem)) && (addr_ < (Addr_t *)(region_->mem + MEMORY_REGION_SIZE_IN_BYTES)));



  /* Check if the address is inside the scope of the memory region, if it is
     then return success. */
  if ((addr_ >= (Addr_t *)(region_->mem)) && (addr_ < (Addr_t *)(region_->mem + MEMORY_REGION_SIZE_IN_BYTES))) {


    ret = RETURN_SUCCESS;
  }


  return ret;
}



/* A function to allocate memory and is similar to the standard libc calloc() but supports multiple memory regions. */
static Addr_t *__calloc__(volatile MemoryRegion_t *region_, const Size_t size_) {

  Addr_t *ret = NULL;

  HalfWord_t requested = zero;

  HalfWord_t free = zero;

  /* Intentionally underflow an unsigned data type
     to get its maximum value. */
  HalfWord_t fewest = -1;


  MemoryEntry_t *cursor = NULL;


  MemoryEntry_t *candidate = NULL;


  MemoryEntry_t *candidateNext = NULL;

  /* Need to disable interrupts while modifying entries in
     a memory region. */
  DISABLE_INTERRUPTS();


  /* Assert if zero bytes are requested because, well... we can't
     allocate zero bytes of memory. */
  SYSASSERT(zero < size_);


  /* Check to make sure one or more bytes was requested. */
  if (zero < size_) {


    /* Assert if the memory corruption flag is true. */
    SYSASSERT(false == SYSFLAG_FAULT());


    /* Check to make sure the memory corruption flag is not true because
       we can't allocate memory if a memory region is corrupt. */
    if (false == SYSFLAG_FAULT()) {


      /* Check if we have calculated how many blocks are needed to story
         a memory entry, if we haven't then we must calculate it now and save
         it for later. */
      if (zero == region_->entrySize) {


        /* Calculate the quotient part of the blocks. */
        region_->entrySize = ((HalfWord_t)(sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE));



        /* Check if there is a remainder, if so we need to add one block. */
        if (zero < ((HalfWord_t)(sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {



          region_->entrySize++;
        }
      }


      /* Check to see if the region has been initialized, if it hasn't then
         we must initialize it. */
      if (ISNULLPTR(region_->start)) {


        /* Start by setting the start entry to the start address of the memory
           region. We do this so we don't have to cast the type every time we wan't
           to address the first entry in the memory region. */
        region_->start = (MemoryEntry_t *)region_->mem;


        /* Zero out all of the memory in the memory region. */
        __memset__(region_->mem, zero, MEMORY_REGION_SIZE_IN_BYTES);

        /* Mark the first entry in the memory region free. */
        region_->start->free = true;



        /* Give the first entry in the memory region all of the blocks. */
        region_->start->blocks = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS;



        /* Because there is no next entry set it to null. */
        region_->start->next = NULL;
      }



      /* Assert if the memory region fails its consistency check. */
      SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR));


      /* Check if the memory region passes its consistency check. */
      if (RETURN_SUCCESS == __MemoryRegionCheck__(region_, NULL, MEMORY_REGION_CHECK_OPTION_WO_ADDR)) {


        /* Calculate the number of blocks requested. */
        requested = ((HalfWord_t)(size_ / CONFIG_MEMORY_REGION_BLOCK_SIZE));


        /* Check if there is a remainder, if so add one more block. */
        if (zero < ((HalfWord_t)(size_ % CONFIG_MEMORY_REGION_BLOCK_SIZE))) {



          requested++;
        }


        /* We need to include the number of blocks required by the entry. */
        requested += region_->entrySize;



        /* To traverse the entries in the memory region we need to start off with
           the cursor set to the starting entry. */
        cursor = region_->start;



        /* Keep traversing the memory region while this is an entry to traverse. */
        while (ISNOTNULLPTR(cursor)) {



          /* Check to see if the entry is free and if it is a good candidate. */
          if ((true == cursor->free) && (requested <= cursor->blocks) && (fewest > cursor->blocks)) {




            fewest = cursor->blocks;


            /* Let's remember this entry as a possible candidate. */
            candidate = cursor;
          }


          /* Count how many blocks are free so we can update the minimum bytes free ever
             for the memory region. */
          if (true == cursor->free) {

            free += cursor->blocks;
          }


          cursor = cursor->next;
        }


        /* Assert if we didn't find a good candidate. */
        SYSASSERT(ISNOTNULLPTR(candidate));



        /* Check if we found a good candidate, if not then head toward the exit. */
        if (ISNOTNULLPTR(candidate)) {



          /* Check if we can split the blocks in the entry. If we can then proceed with
             splitting the blocks by putting the remainder in a new entry. */
          if ((region_->entrySize + 1) <= (candidate->blocks - requested)) {



            /* Save the next of the candidate because we will need it later. */
            candidateNext = candidate->next;



            /* Calculate the location of the new entry based on the blocks requested. */
            candidate->next = (MemoryEntry_t *)((Byte_t *)candidate + (requested * CONFIG_MEMORY_REGION_BLOCK_SIZE));



            /* Now used the "candidate->next" saved earlier. We are basically inserting
               a new node in a linked list. */
            candidate->next->next = candidateNext;



            /* Mark the new entry as free. */
            candidate->next->free = true;



            /* Give the new entry the remaining blocks. */
            candidate->next->blocks = candidate->blocks - requested;



            /* Mark the candidate entry as in use. */
            candidate->free = false;



            /* Set the blocks of the candidate to requested. */
            candidate->blocks = requested;



            /* Clear the memory allocated. */
            __memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE);



            /* Convert the candidate memory entry address to the starting address of the
               newly allocated memory. */
            ret = ENTRY2ADDR(candidate, region_);



          } else {


            /* Because we are unable to split the candidate, let's just claim it for France. */
            candidate->free = false;



            /* Clear the memory allocated. */
            __memset__(ENTRY2ADDR(candidate, region_), zero, (requested - region_->entrySize) * CONFIG_MEMORY_REGION_BLOCK_SIZE);



            /* Convert the candidate memory entry address to the starting address of the
               newly allocated memory. */
            ret = ENTRY2ADDR(candidate, region_);
          }

          /* Update some memory region statistics before we are done. */

          /* Increment the allocations count for the region. */
          region_->allocations++;


          /* We just allocated memory, so subtract back out the requested blocks
             before we set the minimum bytes available ever. */
          free -= requested;


          if ((free * CONFIG_MEMORY_REGION_BLOCK_SIZE) < region_->minAvailableEver) {


            region_->minAvailableEver = (free * CONFIG_MEMORY_REGION_BLOCK_SIZE);
          }
        }
      }
    }
  }




  ENABLE_INTERRUPTS();

  return ret;
}



/* Function to free memory allocated by __calloc__(). */
static void __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {


  MemoryEntry_t *free = NULL;

  /* Need to disable interrupts while modifying entries in
     a memory region. */
  DISABLE_INTERRUPTS();


  /* Assert if a memory region is corrupt. */
  SYSASSERT(false == SYSFLAG_FAULT());


  /* Check if a memory region is corrupt. If it is, we might as well head toward
     the exit because we can't reliably modify the memory region. */
  if (false == SYSFLAG_FAULT()) {



    /* Assert if the memory region fails a consistency check. */
    SYSASSERT(RETURN_SUCCESS == __MemoryRegionCheck__(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR));



    /* Check the consistency of the memory region before we modify it. We are also checking
       that the memory address is valid. */
    if (RETURN_SUCCESS == __MemoryRegionCheck__(region_, addr_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {



      /* Convert the address of the allocated memory to its
         respective memory entry address. */
      free = ADDR2ENTRY(addr_, region_);


      /* Mark the memory entry as free. */
      free->free = true;


      region_->frees++;


      /* After freeing blocks we should defrag the memory
         region. */
      __DefragMemoryRegion__(region_);
    }
  }




  ENABLE_INTERRUPTS();

  return;
}



/* A wrapper function for __calloc__() because the memory
   regions cannot be accessed outside the scope of mem.c. */
Addr_t *__KernelAllocateMemory__(const Size_t size_) {

  return __calloc__(&kernel, size_);
}



/* A wrapper function for __free__() because the memory
   regions cannot be accessed outside the scope of mem.c. */
void __KernelFreeMemory__(const volatile Addr_t *addr_) {

  __free__(&kernel, addr_);


  return;
}



/* A wrapper function for __MemoryRegionCheck__() because the memory
   regions cannot be accessed outside the scope of mem.c. */
Base_t __MemoryRegionCheckKernel__(const volatile Addr_t *addr_, const Base_t option_) {

  return __MemoryRegionCheck__(&kernel, addr_, option_);
}



/* A wrapper function for __calloc__() because the memory
   regions cannot be accessed outside the scope of mem.c. */
Addr_t *__HeapAllocateMemory__(const Size_t size_) {


  return __calloc__(&heap, size_);
}



/* A wrapper function for __free__() because the memory
   regions cannot be accessed outside the scope of mem.c. */
void __HeapFreeMemory__(const volatile Addr_t *addr_) {

  __free__(&heap, addr_);


  return;
}



/* A wrapper function for __MemoryRegionCheck__() because the memory
   regions cannot be accessed outside the scope of mem.c. */
Base_t __MemoryRegionCheckHeap__(const volatile Addr_t *addr_, const Base_t option_) {

  return __MemoryRegionCheck__(&heap, addr_, option_);
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