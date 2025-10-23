/*UNCRUSTIFY-OFF*/
/**
 * @file mem.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for memory management with multiple region support
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/

#include "mem.h"


/* Two separate memory regions for isolation: heap for user allocations, kernel
 * for system use */
static volatile MemoryRegion_t heap = { /* User memory pool structure */
  0 /* Zero-initialize all fields to ensure clean state */
};
static volatile MemoryRegion_t kernel = {  /* Kernel memory pool structure */
  0 /* Zero-initialize all fields to ensure clean state */
};


/* Macro to round up value to next alignment boundary - ensures all addresses
 * meet alignment requirements */
#define __AlignUp__(value_, alignment_) \
        (((value_) + ((alignment_) - 1)) & ~((alignment_) - 1)) /* Add
                                                                 * (alignment-1)
                                                                 * then mask off
                                                                 * lower bits */
/* Macro to round down value to alignment boundary - used for address validation
 */
#define __AlignDown__(value_, alignment_) \
        ((value_) & ~((alignment_) - 1)) /* Simply mask off the lower bits */


/* Macro to check if a value is aligned - returns true if no remainder when
 * divided by alignment */
#define __IsAligned__(value_, alignment_) \
        (((value_) & ((alignment_) - 1)) == 0) /* Check if lower bits are zero
                                                */
/* Calculate aligned block header size to ensure user data starts at aligned
 * boundary */
#define ALIGNED_HEADER_SIZE \
        (((sizeof(BlockHeader_t)) + (CONFIG_MEMORY_ALIGNMENT - 1)) & ~(CONFIG_MEMORY_ALIGNMENT - 1))


/* Convert user pointer to block header by subtracting aligned header size */
#define __OffsetPointerToBlockHeader__(ptr_) \
        ((BlockHeader_t *) (((Byte_t *) (ptr_)) - ALIGNED_HEADER_SIZE))


/* Convert block header to user pointer by adding aligned header size */
#define __OffsetBlockHeaderToPointer__(header_) \
        ((Addr_t *) (((Byte_t *) (header_)) + ALIGNED_HEADER_SIZE))


/* Check if block is currently allocated - INUSE is 0xAA pattern */
#define __BlockHeaderIsInUse__(header_) (INUSE == (header_)->free)


/* Check if block is currently free - FREE is 0xD5 pattern */
#define __BlockHeaderIsFree__(header_) (FREE == (header_)->free)


/* Forward declarations of internal functions */
static Return_t __ValidateBlockHeader__(const BlockHeader_t *header_, const volatile MemoryRegion_t *region_);
static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_);
static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_);
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_);
static Return_t __DefragMemoryRegion__(volatile MemoryRegion_t *region_);
static Return_t __MemoryRegionInit__(volatile MemoryRegion_t *region_);
static Return_t __DetectByteOrder__(ByteOrder_t *order_);
static Word_t __checksum__(const BlockHeader_t *header_);


/* Fletcher-32 inspired checksum for block header integrity verification */
static Word_t __checksum__(const BlockHeader_t *header_) {
  Word_t sum1 = 0xFFFFu;  /* Initialize first sum to all ones for better bit
                           * distribution */
  Word_t sum2 = 0xFFFFu; /* Initialize second sum to all ones for better bit
                          * distribution */
  Word_t temp; /* Temporary variable for multi-word pointer processing */


  /* Process pointer field based on architecture word size for efficiency */
#if UINTPTR_MAX == 0xFF


    /* 8-bit architecture: process pointer as single byte */
    sum1 = (sum1 + (Word_t) (uintptr_t) header_->next) & 0xFFFFu; /* Add pointer
                                                                   * value to
                                                                   * sum1, mask
                                                                   * to 16 bits
                                                                   */
    sum2 = (sum2 + sum1) & 0xFFFFu; /* Add sum1 to sum2, mask to 16 bits */
#elif UINTPTR_MAX == 0xFFFF


    /* 16-bit architecture: process pointer as single 16-bit word */
    temp = (Word_t) (uintptr_t) header_->next; /* Cast pointer to word */
    sum1 = (sum1 + temp) & 0xFFFFu; /* Add to sum1, mask to 16 bits */
    sum2 = (sum2 + sum1) & 0xFFFFu; /* Add sum1 to sum2, mask to 16 bits */
#elif UINTPTR_MAX == 0xFFFFFFFF


    /* 32-bit architecture: process pointer as two 16-bit words */
    temp = (Word_t) (uintptr_t) header_->next; /* Cast pointer to 32-bit value
                                                */
    sum1 = (sum1 + (temp & 0xFFFFu)) & 0xFFFFu; /* Add lower 16 bits to sum1 */
    sum2 = (sum2 + sum1) & 0xFFFFu; /* Update sum2 with new sum1 */
    sum1 = (sum1 + (temp >> 16)) & 0xFFFFu; /* Add upper 16 bits to sum1 */
    sum2 = (sum2 + sum1) & 0xFFFFu; /* Update sum2 with new sum1 */
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF


    /* 64-bit architecture: process pointer as four 16-bit words for maximum
     * efficiency */
    temp = (Word_t) ((uintptr_t) header_->next & 0xFFFFFFFFu); /* Get lower 32
                                                                * bits */
    sum1 = (sum1 + (temp & 0xFFFFu)) & 0xFFFFu; /* Add bits 0-15 to sum1 */
    sum2 = (sum2 + sum1) & 0xFFFFu; /* Update sum2 */
    sum1 = (sum1 + (temp >> 16)) & 0xFFFFu; /* Add bits 16-31 to sum1 */
    sum2 = (sum2 + sum1) & 0xFFFFu; /* Update sum2 */
    temp = (Word_t) ((uintptr_t) header_->next >> 32); /* Get upper 32 bits */
    sum1 = (sum1 + (temp & 0xFFFFu)) & 0xFFFFu; /* Add bits 32-47 to sum1 */
    sum2 = (sum2 + sum1) & 0xFFFFu; /* Update sum2 */
    sum1 = (sum1 + (temp >> 16)) & 0xFFFFu; /* Add bits 48-63 to sum1 */
    sum2 = (sum2 + sum1) & 0xFFFFu; /* Update sum2 */
#endif /* if UINTPTR_MAX == 0xFF */
  /* Process size field as two 16-bit words (size is always 32-bit) */
  sum1 = (sum1 + (header_->size & 0xFFFFu)) & 0xFFFFu; /* Add lower 16 bits of
                                                        * size */
  sum2 = (sum2 + sum1) & 0xFFFFu; /* Update sum2 */
  sum1 = (sum1 + (header_->size >> 16)) & 0xFFFFu; /* Add upper 16 bits of size
                                                    */
  sum2 = (sum2 + sum1) & 0xFFFFu; /* Update sum2 */
  /* Process free/inuse flag as single byte */
  sum1 = (sum1 + header_->free) & 0xFFFFu; /* Add free flag to sum1 */
  sum2 = (sum2 + sum1) & 0xFFFFu; /* Update sum2 */

  /* Combine sums and XOR with magic constant for better bit distribution */
  return(((sum2 << 16) | sum1) ^ 0xB16B00B5u); /* "BIG BOOBS" magic constant for
                                                * entropy */
}


/* Validate block header integrity - critical security function */
static Return_t __ValidateBlockHeader__(const BlockHeader_t *header_, const volatile MemoryRegion_t *region_) {
  FUNCTION_ENTER;  /* Track function entry for debugging */


  Word_t expectedChecksum = 0x0u; /* Variable to store calculated checksum */


  /* Verify header pointer is within valid memory region bounds */
  if(!(((const Byte_t *) header_ < (const Byte_t *) region_->mem) || /* Check
                                                                      * not
                                                                      * before
                                                                      * region
                                                                      * start */
    ((const Byte_t *) header_ >= ((const Byte_t *) region_->mem + MEMORY_REGION_SIZE_IN_BYTES - ALIGNED_HEADER_SIZE)))) { /*
                                                                                                                           *
                                                                                                                           *
                                                                                                                           * Check
                                                                                                                           * not
                                                                                                                           * after
                                                                                                                           * region
                                                                                                                           * end
                                                                                                                           */
    /* Header is within bounds - now validate integrity */
    expectedChecksum = __checksum__(header_); /* Calculate what checksum should
                                               * be */

    if(header_->checksum == expectedChecksum) { /* Compare stored vs calculated
                                                 * checksum */
      /* Checksum valid - verify free/inuse flag has legal value */
      if((header_->free == FREE) || (header_->free == INUSE)) {
        /* Only FREE (0xD5) or INUSE (0xAA) are valid */
        __ReturnOk__(); /* All validation checks passed */
      } else {
        /* Invalid free/inuse flag indicates memory corruption */
#if !defined(POSIX_ARCH_OTHER)
          __SetFlag__(MEMFAULT); /* Set global memory fault flag for embedded
                                  * systems */
#endif /* if !defined(POSIX_ARCH_OTHER) */
        __AssertOnElse__(); /* Trigger assertion failure */
      }
    } else {
      /* Checksum mismatch indicates header corruption */
#if !defined(POSIX_ARCH_OTHER)
        __SetFlag__(MEMFAULT); /* Set global memory fault flag for embedded
                                * systems */
#endif /* if !defined(POSIX_ARCH_OTHER) */
      __AssertOnElse__(); /* Trigger assertion failure */
    }
  } else {
    /* Header pointer is outside valid region bounds */
    __AssertOnElse__(); /* Trigger assertion failure */
  }

  FUNCTION_EXIT;  /* Track function exit for debugging */
}


/* Initialize the memory management subsystem */
Return_t __MemoryInit__(void) {
  FUNCTION_ENTER;  /* Track function entry */


  ByteOrder_t order; /* Variable to store detected byte order */


  /* Initialize heap memory region for user allocations */
  if(OK(__MemoryRegionInit__(&heap))) { /* Set up heap region structure */
    /* Initialize kernel memory region for system allocations */
    if(OK(__MemoryRegionInit__(&kernel))) { /* Set up kernel region structure */
      /* Detect system byte order for proper data handling */
      if(OK(__DetectByteOrder__(&order))) { /* Determine endianness */
        if(ByteOrderLittleEndian == order) { /* Check if little-endian */
          __SetFlag__(LITTLEEND); /* Set little-endian flag */
        } else { /* System is big-endian */
          __UnsetFlag__(LITTLEEND); /* Clear little-endian flag */
        }

        __UnsetFlag__(MEMFAULT);  /* Clear any memory fault flags */
        __ReturnOk__(); /* Initialization successful */
      } else {
        __AssertOnElse__();  /* Byte order detection failed */
      }
    } else {
      __AssertOnElse__();  /* Kernel region initialization failed */
    }
  } else {
    __AssertOnElse__();  /* Heap region initialization failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Initialize a memory region with a single free block */
static Return_t __MemoryRegionInit__(volatile MemoryRegion_t *region_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(__PointerIsNotNull__(region_)) { /* Validate region pointer */
    /* Set first block header at beginning of memory region */
    region_->first = (BlockHeader_t *) region_->mem; /* Point to start of memory
                                                      * array */


    /* Initialize statistics */
    region_->minAvailableEver = MEMORY_REGION_SIZE_IN_BYTES; /* Track minimum
                                                             * free memory */
    region_->allocations = 0; /* Reset allocation counter */
    region_->frees = 0; /* Reset free counter */

    /* Clear entire memory region to zero for security */
    if(OK(__memset__((volatile Addr_t *) region_->mem, nil, MEMORY_REGION_SIZE_IN_BYTES))) { /*
                                                                                              *
                                                                                              *
                                                                                              * Zero
                                                                                              * all
                                                                                              * bytes
                                                                                              */
      /* Create initial free block spanning entire region */
      BlockHeader_t *first = region_->first; /* Get pointer to first block */


      first->next = null; /* No next block - this is the only one */
      /* Size is total region minus one header for this block */
      first->size = MEMORY_REGION_SIZE_IN_BYTES - ALIGNED_HEADER_SIZE; /*
                                                                        * Available
                                                                        * user
                                                                        * space
                                                                        */
      first->free = FREE; /* Mark block as free */
      /* Calculate and store checksum for integrity */
      first->checksum = __checksum__(first); /* Store checksum for validation */
      __ReturnOk__(); /* Initialization successful */
    } else {
      __AssertOnElse__();  /* Memory clearing failed */
    }
  } else {
    __AssertOnElse__();  /* Null region pointer */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Allocate memory from specified region - core allocation function */
static Return_t __calloc__(volatile MemoryRegion_t *region_, volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t requested; /* Aligned size actually needed */
  Size_t available = 0; /* Total free memory available */
  BlockHeader_t *cursor = null; /* Iterator for traversing block list */
  BlockHeader_t *candidate = null; /* Best-fit block found */
  BlockHeader_t *next = null; /* Temporary for list manipulation */
  BlockHeader_t *first = null; /* First block pointer */
  Size_t candidateSize = (Size_t) -1; /* Size of best-fit block (start with max)
                                       */
  Size_t traversedSize = 0; /* Bytes traversed for cycle detection */
  Base_t cycleDetected = false; /* Flag to track if cycle was detected */


  /* Align requested size up to alignment boundary */
  requested = __AlignUp__(size_, CONFIG_MEMORY_ALIGNMENT); /* Ensure proper
                                                            * alignment */


  /* Disable interrupts to make allocation atomic */
  __DisableInterrupts__(); /* Prevent race conditions */

  /* Comprehensive validation of all parameters and preconditions */
  if(__FlagIsNotSet__(MEMFAULT) && /* No memory fault active */
    __PointerIsNotNull__(region_) && /* Valid region pointer */
    __PointerIsNotNull__(addr_) && /* Valid output pointer */
    (nil < size_) && /* Non-zero size requested */
    (requested >= size_) && /* Check alignment didn't overflow */
    ((requested < MEMORY_REGION_SIZE_IN_BYTES) && /* Request fits in region */
    ((requested + ALIGNED_HEADER_SIZE) <= MEMORY_REGION_SIZE_IN_BYTES))) { /*
                                                                            * Including
                                                                            * header
                                                                            * still
                                                                            * fits
                                                                            */
    /* Lazy initialization if region was never used before */
    if(__PointerIsNull__(region_->first)) { /* Check if first allocation ever */
      /* Initialize region with single free block */
      region_->first = (BlockHeader_t *) region_->mem; /* Point to start */
      first = region_->first; /* Get first block pointer */
      first->next = null; /* No next block */
      first->size = MEMORY_REGION_SIZE_IN_BYTES - ALIGNED_HEADER_SIZE; /* All
                                                                        * space
                                                                        * minus
                                                                        * header
                                                                        */
      first->free = FREE; /* Mark as free */
      /* Set checksum for new block */
      first->checksum = __checksum__(first); /* Calculate integrity checksum */
    }

    cursor = region_->first;  /* Start at first block */

    /* Traverse linked list to find best-fit free block */
    while(__PointerIsNotNull__(cursor)) { /* While not at end of list */
      /* Validate block header before accessing to prevent crashes from
       * corruption */
      if(!OK(__ValidateBlockHeader__(cursor, region_))) { /* Check block
                                                          * integrity */
        /* Corruption detected - cannot trust any candidates found so far */
        candidate = null; /* Clear any candidate to prevent using corrupted
                           * memory */
        __SetFlag__(MEMFAULT); /* Set memory fault flag */
        __AssertOnElse__(); /* Trigger assertion */
        break; /* Exit loop */
      }


      /* Track total memory traversed to detect circular lists */
      traversedSize += ALIGNED_HEADER_SIZE + cursor->size; /* Add this block's
                                                            * total size */

      if(traversedSize > MEMORY_REGION_SIZE_IN_BYTES) { /* More traversed than
                                                         * exists */
        /* Circular list detected - memory corruption */
        cycleDetected = true; /* Set cycle flag */
        candidate = null; /* Clear candidate to prevent allocation */
        __SetFlag__(MEMFAULT); /* Set memory fault flag */
        __AssertOnElse__(); /* Trigger assertion */
        break; /* Exit loop */
      }


      /* Check if this block is suitable (free and big enough) */
      if(__BlockHeaderIsFree__(cursor) && /* Block is free */
        (requested <= cursor->size) && /* Block is big enough */
        (cursor->size < candidateSize)) {
        /* Block is smaller than current best (best-fit) */
        candidateSize = cursor->size; /* Update best size */
        candidate = cursor; /* Save this block as candidate */
      }


      /* Track total available free memory */
      if(__BlockHeaderIsFree__(cursor)) { /* If block is free */
        available += cursor->size; /* Add to available total */
      }

      cursor = cursor->next;  /* Move to next block */
    }


    /* Only proceed if no cycle detected and suitable block found */
    if((cycleDetected == false) && __PointerIsNotNull__(candidate)) { /* Valid
                                                                       * candidate
                                                                       * and no
                                                                       * corruption
                                                                       */
      /* Determine if block should be split or used whole */


      /* Split only if remaining space is enough for minimum block plus header
       */
      if((candidate->size >= requested) && /* Verify no underflow */
        ((candidate->size - requested) >= (ALIGNED_HEADER_SIZE + CONFIG_MEMORY_MINIMUM_BLOCK_SIZE))) { /*
                                                                                                        *
                                                                                                        *
                                                                                                        * Remainder
                                                                                                        * is
                                                                                                        * useful
                                                                                                        */
        /* Split the block - create new free block from remainder */
        next = candidate->next; /* Save original next pointer */
        /* Calculate new block location after allocated space */
        candidate->next = (BlockHeader_t *) (((Byte_t *) candidate) + ALIGNED_HEADER_SIZE + requested);


        /* Initialize the new free block */
        candidate->next->next = next; /* Link to original next */
        candidate->next->size = candidate->size - requested - ALIGNED_HEADER_SIZE; /*
                                                                                    *
                                                                                    *
                                                                                    * Remaining
                                                                                    * size
                                                                                    */
        candidate->next->free = FREE; /* Mark as free */
        candidate->next->checksum = __checksum__(candidate->next); /* Set
                                                                    * checksum
                                                                    */


        /* Update allocated block size */
        candidate->size = requested; /* Exact size requested */
      }


      /* Mark block as allocated */
      candidate->free = INUSE; /* Set in-use flag */
      candidate->checksum = __checksum__(candidate); /* Update checksum */

      /* Clear allocated memory for security */
      if(OK(__memset__(__OffsetBlockHeaderToPointer__(candidate), nil, requested))) {
        /*
         * Zero memory
         */
        *addr_ = __OffsetBlockHeaderToPointer__(candidate); /* Return user
                                                             * pointer */


        /* Verify pointer alignment as final check */
        if(!__IsAligned__((Size_t) *addr_, CONFIG_MEMORY_ALIGNMENT)) { /* Check
                                                                        * alignment
                                                                        */
          /* Critical error - alignment guarantee violated */
          __SetFlag__(MEMFAULT); /* Set memory fault */
          __AssertOnElse__(); /* Trigger assertion */
        }


        /* Update statistics */
        region_->allocations++; /* Increment allocation count */
        available -= requested; /* Reduce available memory */

        if(available < region_->minAvailableEver) { /* Check if new low */
          region_->minAvailableEver = available; /* Update minimum */
        }

        __ReturnOk__();  /* Allocation successful */
      } else {
        __AssertOnElse__();  /* Memory clearing failed */
      }
    } else {
      __AssertOnElse__();  /* No suitable block found or corruption detected */
    }
  } else {
    __AssertOnElse__();  /* Parameter validation failed */
  }


  /* Re-enable interrupts */
  __EnableInterrupts__(); /* Restore interrupt state */
  FUNCTION_EXIT; /* Track function exit */
}


/* Free memory block and merge adjacent free blocks */
static Return_t __free__(volatile MemoryRegion_t *region_, const volatile Addr_t *addr_) {
  FUNCTION_ENTER;  /* Track function entry */


  BlockHeader_t *header = null; /* Block header pointer */


  /* Disable interrupts to make free atomic */
  __DisableInterrupts__(); /* Prevent race conditions */

  if(__PointerIsNotNull__(addr_)) { /* Check for null pointer free */
    if(__FlagIsNotSet__(MEMFAULT) && __PointerIsNotNull__(region_)) { /*
                                                                       * Validate
                                                                       * preconditions
                                                                       */
      /* Convert user pointer back to block header */
      header = __OffsetPointerToBlockHeader__(addr_); /* Subtract header size */

      /* Comprehensive validation of block header */
      if(OK(__ValidateBlockHeader__(header, region_))) { /* Validate integrity
                                                          */
        /* Verify block is actually allocated (prevent double-free) */
        if(__BlockHeaderIsInUse__(header)) { /* Check INUSE flag */
          /* Mark block as free */
          header->free = FREE; /* Set free flag */
          header->checksum = __checksum__(header); /* Update checksum */
          region_->frees++; /* Increment free counter */

          /* Merge adjacent free blocks to reduce fragmentation */
          if(OK(__DefragMemoryRegion__(region_))) { /* Coalesce free blocks */
            __ReturnOk__(); /* Free successful */
          } else {
            __AssertOnElse__();  /* Defragmentation failed */
          }
        } else {
          /* Block already free - double free error */
#if !defined(POSIX_ARCH_OTHER)
            __SetFlag__(MEMFAULT); /* Set memory fault for embedded */
#endif /* if !defined(POSIX_ARCH_OTHER) */
          __AssertOnElse__(); /* Trigger assertion */
        }
      } else {
        /* Invalid block header - bad pointer */
        __AssertOnElse__(); /* Trigger assertion */
      }
    } else {
      __AssertOnElse__();  /* Invalid region or memory fault */
    }
  } else {
    /* Null pointer free - silently succeed per standard */
    __ReturnOk__(); /* Return success */
    __AssertOnElse__(); /* But also assert for debugging */
  }


  /* Re-enable interrupts */
  __EnableInterrupts__(); /* Restore interrupt state */
  FUNCTION_EXIT; /* Track function exit */
}


/* Merge adjacent free blocks to reduce fragmentation */
static Return_t __DefragMemoryRegion__(volatile MemoryRegion_t *region_) {
  FUNCTION_ENTER;  /* Track function entry */


  BlockHeader_t *cursor = null; /* Current block pointer */
  BlockHeader_t *nextBlock = null; /* Next block pointer */
  Base_t merged = true; /* Flag to track if any merges occurred */
  Base_t cycleDetected = false; /* Flag for cycle detection */
  Size_t traversedSize = 0; /* For cycle detection */


  if(__PointerIsNotNull__(region_)) { /* Validate region pointer */
    /* Keep merging until no more merges possible or error detected */
    while(merged && !cycleDetected) {
      /* Continue while merges happening and no errors */
      merged = false; /* Reset merge flag */
      cursor = region_->first; /* Start at beginning */
      traversedSize = 0; /* Reset traversal counter */

      /* Traverse list looking for adjacent free blocks */
      while(__PointerIsNotNull__(cursor) && __PointerIsNotNull__(cursor->next) && !cycleDetected) { /*
                                                                                                     *
                                                                                                     *
                                                                                                     * Need
                                                                                                     * current
                                                                                                     * and
                                                                                                     * next
                                                                                                     */
        /* Track traversed memory for cycle detection */
        traversedSize += ALIGNED_HEADER_SIZE + cursor->size; /* Add block size
                                                              */

        if(traversedSize > MEMORY_REGION_SIZE_IN_BYTES) { /* Cycle detected */
          /* Circular reference - memory corruption */
          __SetFlag__(MEMFAULT); /* Set fault flag */
          cycleDetected = true; /* Set flag to exit loops */
        } else {
          /* Check if current and next blocks are both free */
          if(__BlockHeaderIsFree__(cursor) && __BlockHeaderIsFree__(cursor->next)) {
            /*
             * Both free
             */
            nextBlock = cursor->next; /* Get next block pointer */

            /* Check for integer overflow before merging */
            if(cursor->size > ((Size_t) -1) - ALIGNED_HEADER_SIZE - nextBlock->size) { /*
                                                                                        *
                                                                                        *
                                                                                        * Would
                                                                                        * overflow
                                                                                        */
              /* Skip this merge to prevent overflow */
              cursor = cursor->next; /* Move to next block */
              continue; /* Skip to next iteration */
            }


            /* Merge the blocks */
            cursor->size += ALIGNED_HEADER_SIZE + nextBlock->size; /* Combine
                                                                   * sizes */
            cursor->next = nextBlock->next; /* Skip over merged block */
            /* Update checksum for merged block */
            cursor->checksum = __checksum__(cursor); /* Recalculate checksum */
            /* Mark that merge occurred */
            merged = true; /* Set flag to continue merging */
          } else {
            cursor = cursor->next;  /* Move to next block */
          }
        }
      }
    }

    if(cycleDetected) {
      __AssertOnElse__(); /* Trigger assertion for cycle detection */
    } else {
      __ReturnOk__();  /* Defragmentation complete */
    }
  } else {
    __AssertOnElse__();  /* Null region pointer */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Public API: Allocate memory from heap */
Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(OK(__calloc__(&heap, addr_, size_))) { /* Allocate from heap region */
    __ReturnOk__(); /* Allocation successful */
  } else {
    __AssertOnElse__();  /* Allocation failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Public API: Free heap memory */
Return_t xMemFree(const volatile Addr_t *addr_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(OK(__free__(&heap, addr_))) { /* Free from heap region */
    __ReturnOk__(); /* Free successful */
  } else {
    __AssertOnElse__();  /* Free failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Public API: Free all heap memory */
Return_t xMemFreeAll(void) {
  FUNCTION_ENTER;  /* Track function entry */

  if(OK(__MemoryRegionInit__(&heap))) {
    /* Reinitialize heap to single free block */
    __ReturnOk__(); /* Reset successful */
  } else {
    __AssertOnElse__();  /* Reset failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Public API: Get total used heap memory */
Return_t xMemGetUsed(Size_t *size_) {
  FUNCTION_ENTER;  /* Track function entry */


  BlockHeader_t *cursor = null; /* Block iterator */
  Size_t used = 0; /* Total used memory */
  Size_t traversedSize = 0; /* For cycle detection */
  Base_t cycleDetected = false; /* Flag for cycle detection */


  if(__PointerIsNotNull__(size_)) { /* Validate output pointer */
    cursor = heap.first; /* Start at first block */

    /* Traverse all blocks counting used memory */
    while(__PointerIsNotNull__(cursor) && !cycleDetected) { /* While blocks
                                                             * remain and no
                                                             * errors */
      /* Track traversed memory for cycle detection */
      traversedSize += ALIGNED_HEADER_SIZE + cursor->size; /* Add block total */

      if(traversedSize > MEMORY_REGION_SIZE_IN_BYTES) { /* Cycle detected */
        /* Circular reference - corruption */
        __SetFlag__(MEMFAULT); /* Set fault flag */
        cycleDetected = true; /* Set flag to exit loop */
      } else {
        if(__BlockHeaderIsInUse__(cursor)) {  /* Block is allocated */
          /* Include both user data and header overhead */
          used += cursor->size + ALIGNED_HEADER_SIZE; /* Add total block size */
        }

        cursor = cursor->next;  /* Move to next block */
      }
    }

    if(cycleDetected) {
      __AssertOnElse__(); /* Trigger assertion for cycle detection */
    } else {
      *size_ = used;  /* Return total used */
      __ReturnOk__(); /* Success */
    }
  } else {
    __AssertOnElse__();  /* Null output pointer */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Public API: Get size of specific allocation */
Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_) {
  FUNCTION_ENTER;  /* Track function entry */


  BlockHeader_t *header = null; /* Block header pointer */


  if(__PointerIsNotNull__(addr_) && __PointerIsNotNull__(size_)) { /* Validate
                                                                    * parameters
                                                                    */
    /* Convert user pointer to block header */
    header = __OffsetPointerToBlockHeader__(addr_); /* Get header */

    /* Validate block header integrity */
    if(OK(__ValidateBlockHeader__(header, &heap))) { /* Check validity */
      /* Verify block is allocated */
      if(__BlockHeaderIsInUse__(header)) { /* Check in-use flag */
        *size_ = header->size; /* Return block size */
        __ReturnOk__(); /* Success */
      } else {
        /* Block not allocated - invalid operation */
        __AssertOnElse__(); /* Trigger assertion */
      }
    } else {
      /* Invalid header - corruption or bad pointer */
      __AssertOnElse__(); /* Trigger assertion */
    }
  } else {
    __AssertOnElse__();  /* Null parameters */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Kernel API: Allocate from kernel region */
Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(OK(__calloc__(&kernel, addr_, size_))) { /* Allocate from kernel region */
    if(__PointerIsNotNull__(*addr_)) { /* Verify allocation succeeded */
      __ReturnOk__(); /* Success */
    } else {
      __AssertOnElse__();  /* Null allocation */
    }
  } else {
    __AssertOnElse__();  /* Allocation failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Kernel API: Free kernel memory */
Return_t __KernelFreeMemory__(const volatile Addr_t *addr_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(OK(__free__(&kernel, addr_))) { /* Free from kernel region */
    __ReturnOk__(); /* Success */
  } else {
    __AssertOnElse__();  /* Free failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Heap allocation function for internal use */
Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(OK(__calloc__(&heap, addr_, size_))) { /* Allocate from heap */
    if(__PointerIsNotNull__(*addr_)) { /* Verify success */
      __ReturnOk__(); /* Success */
    } else {
      __AssertOnElse__();  /* Null allocation */
    }
  } else {
    __AssertOnElse__();  /* Allocation failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Heap free function for internal use */
Return_t __HeapFreeMemory__(const volatile Addr_t *addr_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(OK(__free__(&heap, addr_))) { /* Free from heap */
    __ReturnOk__(); /* Success */
  } else {
    __AssertOnElse__();  /* Free failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Get statistics for a memory region */
static Return_t __MemGetRegionStats__(const volatile MemoryRegion_t *region_, MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;  /* Track function entry */


  MemoryRegionStats_t *stats = null; /* Stats structure pointer */
  BlockHeader_t *cursor = null; /* Block iterator */
  Word_t largestFree = 0; /* Largest free block size */
  Word_t smallestFree = (Word_t) -1; /* Smallest free block (start at max) */
  Word_t freeBlocks = 0; /* Number of free blocks */
  Word_t availableBytes = 0; /* Total free bytes */
  Size_t traversedSize = 0; /* For cycle detection */
  Base_t cycleDetected = false; /* Flag for cycle detection */


  if(__PointerIsNotNull__(region_) && __PointerIsNotNull__(stats_)) { /*
                                                                       * Validate
                                                                       * parameters
                                                                       */
    /* Allocate memory for stats structure */
    if(OK(xMemAlloc((volatile Addr_t **) &stats, sizeof(MemoryRegionStats_t)))) {
      /*
       * Allocate stats
       */
      cursor = region_->first; /* Start at first block */

      /* Traverse blocks gathering statistics */
      while(__PointerIsNotNull__(cursor) && !cycleDetected) { /* While blocks
                                                               * remain and no
                                                               * errors */
        /* Track traversed memory for cycle detection */
        traversedSize += ALIGNED_HEADER_SIZE + cursor->size; /* Add block size
                                                              */

        if(traversedSize > MEMORY_REGION_SIZE_IN_BYTES) { /* Cycle detected */
          /* Circular reference - corruption */
          __SetFlag__(MEMFAULT); /* Set fault flag */
          /* Free allocated stats before exit */
          xMemFree((const volatile Addr_t *) stats); /* Clean up */
          cycleDetected = true; /* Set flag to exit loop */
        } else {
          if(__BlockHeaderIsFree__(cursor)) {  /* Block is free */
            freeBlocks++; /* Count free block */
            availableBytes += cursor->size; /* Add to total free */

            if(cursor->size > largestFree) { /* Check if largest */
              largestFree = cursor->size; /* Update largest */
            }

            if(cursor->size < smallestFree) {  /* Check if smallest */
              smallestFree = cursor->size; /* Update smallest */
            }
          }

          cursor = cursor->next;  /* Move to next block */
        }
      }

      if(cycleDetected) {
        __AssertOnElse__(); /* Trigger assertion for cycle detection */
      } else {
        /* Fill in statistics structure */
        stats->largestFreeEntryInBytes = largestFree; /* Largest free block */
        stats->smallestFreeEntryInBytes = (smallestFree == (Word_t) -1) ? 0 : smallestFree; /*
                                                                                             *
                                                                                             *
                                                                                             * Smallest
                                                                                             * or
                                                                                             * 0
                                                                                             */
        stats->numberOfFreeBlocks = freeBlocks; /* Free block count */
        stats->availableSpaceInBytes = availableBytes; /* Total free space */
        stats->successfulAllocations = region_->allocations; /* Allocation count
                                                              */
        stats->successfulFrees = region_->frees; /* Free count */
        stats->minimumEverFreeBytesRemaining = region_->minAvailableEver; /* Low
                                                                           * water
                                                                           * mark
                                                                           */
        *stats_ = stats; /* Return stats pointer */
        __ReturnOk__(); /* Success */
      }
    } else {
      __AssertOnElse__();  /* Stats allocation failed */
    }
  } else {
    __AssertOnElse__();  /* Null parameters */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Public API: Get heap statistics */
Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(OK(__MemGetRegionStats__(&heap, stats_))) { /* Get heap stats */
    __ReturnOk__(); /* Success */
  } else {
    __AssertOnElse__();  /* Failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Public API: Get kernel statistics */
Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(OK(__MemGetRegionStats__(&kernel, stats_))) { /* Get kernel stats */
    __ReturnOk__(); /* Success */
  } else {
    __AssertOnElse__();  /* Failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Memory copy function - byte-by-byte copy */
Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t i = nil; /* Loop counter */
  volatile Byte_t *src = null; /* Source byte pointer */
  volatile Byte_t *dest = null; /* Destination byte pointer */


  if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (nil < size_)) {
    /*
     * Validate parameters
     */
    src = (Byte_t *) src_; /* Cast to byte pointer */
    dest = (Byte_t *) dest_; /* Cast to byte pointer */

    for(i = nil; i < size_; i++) { /* Copy each byte */
      dest[i] = src[i]; /* Byte-by-byte copy */
    }

    __ReturnOk__();  /* Success */
  } else {
    __AssertOnElse__();  /* Invalid parameters */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Memory set function - fill memory with byte value */
Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t i = nil; /* Loop counter */
  volatile Byte_t *dest = null; /* Destination byte pointer */


  if(__PointerIsNotNull__(dest_) && (nil < size_)) { /* Validate parameters */
    dest = (Byte_t *) dest_; /* Cast to byte pointer */

    for(i = nil; i < size_; i++) { /* Fill each byte */
      dest[i] = (Byte_t) val_; /* Set byte value */
    }

    __ReturnOk__();  /* Success */
  } else {
    __AssertOnElse__();  /* Invalid parameters */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Memory compare function - compare two memory regions */
Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t i = nil; /* Loop counter */
  volatile Byte_t *s1 = null; /* First region pointer */
  volatile Byte_t *s2 = null; /* Second region pointer */


  if(__PointerIsNotNull__(s1_) && __PointerIsNotNull__(s2_) && (nil < size_) && __PointerIsNotNull__(res_)) {
    /*
     * Validate
     */
    *res_ = true; /* Assume equal initially */
    s1 = (Byte_t *) s1_; /* Cast to byte pointer */
    s2 = (Byte_t *) s2_; /* Cast to byte pointer */

    for(i = nil; i < size_; i++) { /* Compare each byte */
      if(*s1 != *s2) { /* Bytes differ */
        *res_ = false; /* Not equal */
        break; /* Exit early */
      }

      s1++;  /* Advance first pointer */
      s2++; /* Advance second pointer */
    }

    __ReturnOk__();  /* Success */
  } else {
    __AssertOnElse__();  /* Invalid parameters */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Detect system byte order (endianness) */
static Return_t __DetectByteOrder__(ByteOrder_t *order_) {
  FUNCTION_ENTER;  /* Track function entry */

  if(__PointerIsNotNull__(order_)) { /* Validate parameter */
    /* Cast string "\xFF\x00" to 16-bit value and check result */
    if((*(uint16_t *) "\xFF\x00") < 0x100) { /* If value is less than 256 */
      *order_ = ByteOrderLittleEndian; /* System is little-endian */
      __ReturnOk__(); /* Success */
    } else { /* Value is 0xFF00 */
      *order_ = ByteOrderBigEndian; /* System is big-endian */
      __ReturnOk__(); /* Success */
    }
  } else {
    __AssertOnElse__();  /* Null parameter */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


#if defined(POSIX_ARCH_OTHER)


/* Test function: Clear memory regions for testing */
  void __MemoryClear__(void) {
    __MemoryRegionInit__(&heap); /* Reset heap region */
    __MemoryRegionInit__(&kernel); /* Reset kernel region */
  }


#endif /* if defined(POSIX_ARCH_OTHER) */
/* String utility constants */
#define CHAR_NULL 0x00u /* Null terminator character */
#define CHAR_SLASH 0x2Fu /* Forward slash character '/' */
#define CHAR_DOT 0x2Eu /* Dot character '.' */

#if !defined(CONFIG_FS_MAX_PATH_LENGTH)
  #define CONFIG_FS_MAX_PATH_LENGTH 256u /* Default max path length */
#endif /* if !defined(CONFIG_FS_MAX_PATH_LENGTH) */


/* Calculate string length */
Size_t __strlen__(const Byte_t *str_) {
  Size_t len = 0x0u;  /* Length counter */


  if(__PointerIsNotNull__(str_)) { /* Validate pointer */
    while(CHAR_NULL != str_[len]) { /* Until null terminator */
      len++; /* Count character */
    }
  }

  return (len);  /* Return length */
}


/* Safe string copy with size limit */
Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t i = 0x0u; /* Character index */


  if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (0x0u < destSize_)) { /*
                                                                                         *
                                                                                         *
                                                                                         * Validate
                                                                                         * parameters
                                                                                         */
    /* Copy characters until null or size limit */
    while((CHAR_NULL != src_[i]) && (i < (destSize_ - 0x1u))) {
      /* Room for null terminator */
      dest_[i] = src_[i]; /* Copy character */
      i++; /* Next character */
    }

    dest_[i] = CHAR_NULL;  /* Add null terminator */
    __ReturnOk__(); /* Success */
  } else {
    __AssertOnElse__(); /* Invalid parameters */
  }

  FUNCTION_EXIT; /* Track function exit */
}


/* String copy with exact length */
Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t n_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t i = 0x0u; /* Character index */


  if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (0x0u < n_)) { /*
                                                                                  *
                                                                                  *
                                                                                  * Validate
                                                                                  * parameters
                                                                                  */
    /* Copy up to n characters or until null */
    for(i = 0x0u; (i < n_) && (CHAR_NULL != src_[i]); i++) { /* Copy loop */
      dest_[i] = src_[i]; /* Copy character */
    }


    /* Pad with nulls if source was shorter */
    for(; i < n_; i++) { /* Padding loop */
      dest_[i] = CHAR_NULL; /* Add null */
    }

    __ReturnOk__();  /* Success */
  } else {
    __AssertOnElse__(); /* Invalid parameters */
  }

  FUNCTION_EXIT; /* Track function exit */
}


/* Compare two strings */
Base_t __strcmp__(const Byte_t *s1_, const Byte_t *s2_) {
  Size_t i = 0x0u;  /* Character index */


  if(__PointerIsNull__(s1_) || __PointerIsNull__(s2_)) { /* Validate pointers */
    return (0x0u); /* Treat nulls as equal */
  }


  /* Compare characters until mismatch or end */
  while((CHAR_NULL != s1_[i]) && (CHAR_NULL != s2_[i])) {
    /* Both have characters */
    if(s1_[i] != s2_[i]) { /* Characters differ */
      return ((s1_[i] < s2_[i]) ? -0x1 : 0x1); /* Return comparison result */
    }

    i++;  /* Next character */
  }

  if(s1_[i] == s2_[i]) {  /* Both ended at same point */
    return (0x0u); /* Strings are equal */
  }

  return ((s1_[i] < s2_[i]) ? -0x1 : 0x1);  /* One string is shorter */
}


/* Compare strings up to n characters */
Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t n_) {
  Size_t i = 0x0u;  /* Character index */


  if(__PointerIsNull__(s1_) || __PointerIsNull__(s2_) || (0x0u == n_)) {
    /*
     * Validate parameters
     */
    return (0x0u); /* Treat as equal */
  }

  for(i = 0x0u; i < n_; i++) {  /* Compare up to n characters */
    if((CHAR_NULL == s1_[i]) || (s1_[i] != s2_[i])) { /* End or mismatch */
      return ((s1_[i] < s2_[i]) ? (Base_t) -0x1 : ((s1_[i] > s2_[i]) ? (Base_t) 0x1 : (Base_t) 0x0));
    }
  }

  return (0x0u);  /* First n characters match */
}


/* Concatenate strings with size limit */
Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t destLen = 0x0u; /* Destination length */
  Size_t i = 0x0u; /* Source index */


  if(__PointerIsNotNull__(dest_) && __PointerIsNotNull__(src_) && (0x0u < destSize_)) {
    /*
     * Validate
     */
    destLen = __strlen__(dest_); /* Get current length */

    if(destLen < destSize_) { /* Have room */
      /* Append source to destination */
      while((CHAR_NULL != src_[i]) && ((destLen + i) < (destSize_ - 0x1u))) {
        /*
         * Room for null
         */
        dest_[destLen + i] = src_[i]; /* Copy character */
        i++; /* Next character */
      }

      dest_[destLen + i] = CHAR_NULL;  /* Add terminator */
      __ReturnOk__(); /* Success */
    } else {
      __AssertOnElse__(); /* No room */
    }
  } else {
    __AssertOnElse__(); /* Invalid parameters */
  }

  FUNCTION_EXIT; /* Track function exit */
}


/* Find first occurrence of character */
Byte_t * __strchr__(const Byte_t *str_, const Byte_t ch_) {
  Size_t i = 0x0u;  /* Character index */


  if(__PointerIsNull__(str_)) { /* Validate pointer */
    return (null); /* Not found */
  }

  while(CHAR_NULL != str_[i]) {  /* Search string */
    if(str_[i] == ch_) { /* Found character */
      return ((Byte_t *) &str_[i]); /* Return pointer to it */
    }

    i++;  /* Next character */
  }

  if(CHAR_NULL == ch_) {  /* Looking for null terminator */
    return ((Byte_t *) &str_[i]); /* Return pointer to it */
  }

  return (null);  /* Not found */
}


/* Find last occurrence of character */
Byte_t * __strrchr__(const Byte_t *str_, const Byte_t ch_) {
  Size_t len = 0x0u;  /* String length */
  Size_t i = 0x0u; /* Loop index */


  if(__PointerIsNull__(str_)) { /* Validate pointer */
    return (null); /* Not found */
  }

  len = __strlen__(str_);  /* Get string length */

  /* Search backwards through string */
  for(i = len; i > 0x0u; i--) { /* From end to start */
    if(str_[i - 0x1u] == ch_) { /* Found character */
      return ((Byte_t *) &str_[i - 0x1u]); /* Return pointer */
    }
  }

  if((CHAR_NULL == ch_) && (len > 0x0u)) {  /* Looking for terminator */
    return ((Byte_t *) &str_[len]); /* Return end pointer */
  }

  return (null);  /* Not found */
}


/* Join two path components */
Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, const Size_t destSize_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t baseLen = 0x0u; /* Base path length */
  Size_t pathLen = 0x0u; /* Path component length */
  Base_t needSlash = false; /* Whether to add separator */


  if(__PointerIsNull__(dest_) || __PointerIsNull__(base_) || __PointerIsNull__(path_) || (0x0u == destSize_)) {
    __AssertOnElse__();  /* Invalid parameters */
    FUNCTION_EXIT; /* Exit function */
  }

  baseLen = __strlen__(base_);  /* Get base length */
  pathLen = __strlen__(path_); /* Get path length */

  if((0x0u == baseLen) || (0x0u == pathLen)) { /* Empty components */
    __AssertOnElse__(); /* Invalid paths */
    FUNCTION_EXIT; /* Exit function */
  }


  /* Handle absolute path - just return it */
  if(CHAR_SLASH == path_[0x0u]) { /* Path is absolute */
    if(pathLen >= destSize_) { /* Won't fit */
      __AssertOnElse__(); /* Buffer too small */
      __AssertOnElse__(); /* Double assert for emphasis */
      FUNCTION_EXIT; /* Exit function */
    }

    if(OK(__strcpy__(dest_, path_, destSize_))) {  /* Copy path */
      __ReturnOk__(); /* Success */
    } else {
      __AssertOnElse__();  /* Copy failed */
    }

    FUNCTION_EXIT;  /* Exit function */
  }


  /* Determine if separator needed between components */
  needSlash = (CHAR_SLASH != base_[baseLen - 0x1u]) && (CHAR_SLASH != path_[0x0u]);


  /* Check if result will fit */
  if((baseLen + pathLen + (needSlash ? 0x1u : 0x0u)) >= destSize_) {
    /* Too long
     */
    __AssertOnElse__(); /* Buffer too small */
    FUNCTION_EXIT; /* Exit function */
  }

  if(OK(__strcpy__(dest_, base_, destSize_))) {  /* Copy base */
    if(needSlash) { /* Add separator if needed */
      dest_[baseLen] = CHAR_SLASH; /* Add slash */
      dest_[baseLen + 0x1u] = CHAR_NULL; /* Terminate */
    }

    if(OK(__strcat__(dest_, path_, destSize_))) {  /* Append path */
      __ReturnOk__(); /* Success */
    } else {
      __AssertOnElse__();  /* Concatenation failed */
    }
  } else {
    __AssertOnElse__();  /* Copy failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Normalize a path by resolving . and .. components */
Return_t __path_normalize__(Byte_t *path_, const Size_t pathSize_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t i = 0x0u; /* Input index */
  Size_t j = 0x0u; /* Output index */
  Size_t len = 0x0u; /* Path length */
  Byte_t temp[CONFIG_FS_MAX_PATH_LENGTH]; /* Temporary buffer */
  Byte_t segments[CONFIG_FS_MAX_PATH_LENGTH / 2][CONFIG_FS_MAX_PATH_LENGTH]; /*
                                                                              * Path
                                                                              * segments
                                                                              */
  Size_t segmentCount = 0x0u; /* Number of segments */
  Size_t k = 0x0u; /* Segment index */
  Size_t segLen = 0x0u; /* Segment length */
  Size_t segIdx = 0x0u; /* Character in segment */


  if(__PointerIsNull__(path_) || (0x0u == pathSize_)) {
    /* Validate parameters
     */
    __AssertOnElse__(); /* Invalid parameters */
    FUNCTION_EXIT; /* Exit function */
  }

  len = __strlen__(path_);  /* Get path length */

  if((0x0u == len) || (len >= CONFIG_FS_MAX_PATH_LENGTH)) { /* Invalid length */
    __AssertOnElse__(); /* Path too long or empty */
    FUNCTION_EXIT; /* Exit function */
  }


  /* Copy to temporary buffer */
  for(i = 0x0u; i <= len; i++) { /* Include terminator */
    temp[i] = path_[i]; /* Copy character */
  }


  /* Parse path into segments */
  i = 0x0u; /* Reset index */

  if(CHAR_SLASH == temp[0x0u]) { /* Absolute path */
    i = 0x1u; /* Skip leading slash */
  }

  segIdx = 0x0u;  /* Start new segment */

  for(; i <= len; i++) { /* Process all characters */
    if((CHAR_SLASH == temp[i]) || (CHAR_NULL == temp[i])) { /* End of segment */
      if(segIdx > 0x0u) { /* Non-empty segment */
        segments[segmentCount][segIdx] = CHAR_NULL; /* Terminate segment */

        /* Process special segments */
        if((segments[segmentCount][0x0u] == '.') && (segments[segmentCount][0x1u] == '.') && (segments[segmentCount][0x2u] == CHAR_NULL)) { /*
                                                                                                                                             *
                                                                                                                                             *
                                                                                                                                             * ".."
                                                                                                                                             *
                                                                                                                                             *
                                                                                                                                             * segment
                                                                                                                                             */
          /* Go up one directory if possible */
          if(segmentCount > 0x0u) { /* Have parent to remove */
            segmentCount--; /* Remove last segment */
          }
        } else if(!((segments[segmentCount][0x0u] == '.') && (segments[segmentCount][0x1u] == CHAR_NULL))) { /*
                                                                                                              *
                                                                                                              *
                                                                                                              * Not
                                                                                                              *
                                                                                                              *
                                                                                                              * "."
                                                                                                              */
          /* Keep this segment */
          segmentCount++; /* Add to list */
        }


        /* "." segments are ignored */
        segIdx = 0x0u; /* Start new segment */
      }
    } else {
      segments[segmentCount][segIdx++] = temp[i];  /* Add to current segment */
    }
  }


  /* Rebuild normalized path */
  j = 0x0u; /* Output index */

  if(CHAR_SLASH == path_[0x0u]) { /* Preserve absolute path */
    path_[j++] = CHAR_SLASH; /* Add leading slash */
  }

  for(k = 0x0u; k < segmentCount; k++) {  /* Add each segment */
    Size_t m; /* Character index */


    segLen = __strlen__(segments[k]); /* Get segment length */

    if(k > 0x0u) { /* Not first segment */
      path_[j++] = CHAR_SLASH; /* Add separator */
    }

    for(m = 0x0u; m < segLen; m++) {  /* Copy segment */
      path_[j++] = segments[k][m]; /* Copy character */
    }
  }


  /* Handle empty result (root directory) */
  if((0x0u == j) || ((0x1u == j) && (CHAR_SLASH == path_[0x0u]))) {
    /* Empty or just "/"
     */
    path_[0x0u] = CHAR_SLASH; /* Set to root */
    j = 0x1u; /* One character */
  }

  path_[j] = CHAR_NULL;  /* Terminate path */
  __ReturnOk__(); /* Success */
  FUNCTION_EXIT; /* Track function exit */
}


/* Check if path is absolute */
Base_t __path_is_absolute__(const Byte_t *path_) {
  if(__PointerIsNull__(path_)) {  /* Validate pointer */
    return (false); /* Null is not absolute */
  }

  return ((CHAR_SLASH == path_[0x0u]) ? true : false);  /* Check for leading
                                                         * slash */
}


/* Extract directory part of path */
Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t len = 0x0u; /* Path length */
  Size_t i = 0x0u; /* Loop index */


  if(__PointerIsNull__(dest_) || __PointerIsNull__(path_) || (0x0u == destSize_)) {
    /*
     * Validate
     */
    __AssertOnElse__(); /* Invalid parameters */
    FUNCTION_EXIT; /* Exit function */
  }

  len = __strlen__(path_);  /* Get path length */

  if(0x0u == len) { /* Empty path */
    if(OK(__strcpy__(dest_, (const Byte_t *) ".", destSize_))) {
      /* Return current directory */
      __ReturnOk__(); /* Success */
    } else {
      __AssertOnElse__();  /* Copy failed */
    }

    FUNCTION_EXIT;  /* Exit function */
  }


  /* Find last slash */
  for(i = len; i > 0x0u; i--) { /* Search backwards */
    if(CHAR_SLASH == path_[i - 0x1u]) { /* Found slash */
      break; /* Stop searching */
    }
  }

  if(0x0u == i) {  /* No slash found */
    if(OK(__strcpy__(dest_, (const Byte_t *) ".", destSize_))) {
      /* Return current directory */
      __ReturnOk__(); /* Success */
    } else {
      __AssertOnElse__();  /* Copy failed */
    }
  } else {
    if(i > destSize_) {  /* Won't fit */
      __AssertOnElse__(); /* Buffer too small */
      __AssertOnElse__(); /* Double assert */
      FUNCTION_EXIT; /* Exit function */
    }

    if(OK(__strncpy__(dest_, path_, i - 0x1u))) {  /* Copy up to last slash */
      dest_[i - 0x1u] = CHAR_NULL; /* Terminate */
      __ReturnOk__(); /* Success */
    } else {
      __AssertOnElse__();  /* Copy failed */
    }
  }

  FUNCTION_EXIT;  /* Track function exit */
}


/* Extract filename part of path */
Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_) {
  FUNCTION_ENTER;  /* Track function entry */


  Size_t len = 0x0u; /* Path length */
  Size_t i = 0x0u; /* Loop index */
  Size_t start = 0x0u; /* Start of filename */


  if(__PointerIsNull__(dest_) || __PointerIsNull__(path_) || (0x0u == destSize_)) {
    /*
     * Validate
     */
    __AssertOnElse__(); /* Invalid parameters */
    FUNCTION_EXIT; /* Exit function */
  }

  len = __strlen__(path_);  /* Get path length */

  if(0x0u == len) { /* Empty path */
    if(OK(__strcpy__(dest_, (const Byte_t *) ".", destSize_))) {
      /* Return current directory */
      __ReturnOk__(); /* Success */
    } else {
      __AssertOnElse__();  /* Copy failed */
    }

    FUNCTION_EXIT;  /* Exit function */
  }


  /* Find last slash */
  for(i = len; i > 0x0u; i--) { /* Search backwards */
    if(CHAR_SLASH == path_[i - 0x1u]) { /* Found slash */
      start = i; /* Filename starts after slash */
      break; /* Stop searching */
    }
  }

  if((len - start) >= destSize_) {  /* Won't fit */
    __AssertOnElse__(); /* Buffer too small */
    FUNCTION_EXIT; /* Exit function */
  }

  if(OK(__strcpy__(dest_, &path_[start], destSize_))) {  /* Copy filename */
    __ReturnOk__(); /* Success */
  } else {
    __AssertOnElse__();  /* Copy failed */
  }

  FUNCTION_EXIT;  /* Track function exit */
}