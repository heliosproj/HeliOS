/*UNCRUSTIFY-OFF*/
/**
 * @file mem.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Memory allocator API header
 * @details
 * Defines memory region structures, allocation functions, and interfaces for heap and kernel memory management with statistics.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef MEM_H_

  #define MEM_H_

  #include "config.h"

  #include "defines.h"

  #include "types.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "console.h"

    #include "device.h"

    #include "fat.h"

    #include "fs.h"

  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

  #include "port.h"

  #include "posix.h"

  #include "queue.h"

  #include "streams.h"

  #include "sys.h"

  #include "task.h"

  #include "timer.h"

  #include <stdint.h>

  #include <stddef.h>

  #if defined(INUSE)

    #undef INUSE

  #endif /* if defined(INUSE) */

  #define INUSE 0xAAu

  #if defined(FREE)

    #undef FREE

  #endif /* if defined(FREE) */

  #define FREE 0x55u

  #ifndef BLOCKHEADER_T_

    #define BLOCKHEADER_T_

    /**
     * @brief Memory block header structure
     * @details Internal structure prepended to each memory block for allocation
     * tracking and integrity checking.
     */
    typedef struct BlockHeader_s {

      struct BlockHeader_s *next; /**< Pointer to next block in free list */

      Word_t checksum; /**< Integrity checksum for block validation */

      Word_t size; /**< Size of the data portion in bytes */

      Byte_t free; /**< Flag indicating if block is free (FREE) or in use
                    * (INUSE) */

    } BlockHeader_t;

  #endif /* ifndef BLOCKHEADER_T_ */

  #ifndef MEMORYREGION_T_

    #define MEMORYREGION_T_

    /**
     * @brief Memory region structure
     * @details Manages a contiguous region of memory with block allocation and
     * usage statistics.
     */
    typedef struct MemoryRegion_s {

      volatile Byte_t mem[MEMORY_REGION_SIZE]; /**< Raw memory buffer for
                                                * allocations */

      BlockHeader_t *first; /**< Pointer to first block in the region */

      HalfWord_t allocations; /**< Count of successful allocations */

      HalfWord_t frees; /**< Count of successful frees */

      Word_t minAvailableEver; /**< Low water mark of available memory */

    } MemoryRegion_t;

  #endif /* ifndef MEMORYREGION_T_ */

  #ifdef __cplusplus

    extern "C" {

  #endif /* ifdef __cplusplus */
  /**
   * @brief Allocates memory from the heap
   * @details Allocates a block of memory of the requested size and returns a
   * pointer to it.
   *
   * @param[out] addr_ Pointer to store the allocated memory address
   * @param[in]  size_ Size of memory to allocate in bytes
   *
   * @return           ReturnOK if memory was allocated successfully
   * @return           ReturnError if allocation failed or invalid parameters
   *
   * @note Allocated memory must be freed with xMemFree()
   * @warning Always check the return value before using the allocated memory
   */
  Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);
  /**
   * @brief Frees previously allocated memory
   * @details Returns a memory block to the heap for reuse.
   *
   * @param[in] addr_ Pointer to the memory to free
   *
   * @return          ReturnOK if memory was freed successfully
   * @return          ReturnError if address is invalid or already freed
   *
   * @warning Using memory after freeing results in undefined behavior
   */
  Return_t xMemFree(const volatile Addr_t *addr_);
  /**
   * @brief Frees all allocated heap memory
   * @details Resets the entire heap to its initial state, freeing all
   * allocations.
   *
   * @return ReturnOK if heap was reset successfully
   * @return ReturnError if operation failed
   *
   * @warning All existing pointers to heap memory become invalid
   */
  Return_t xMemFreeAll(void);
  /**
   * @brief Gets the total amount of used heap memory
   * @details Returns the sum of all currently allocated memory blocks.
   *
   * @param[out] size_ Pointer to store the used memory size in bytes
   *
   * @return           ReturnOK if size was retrieved successfully
   * @return           ReturnError if invalid parameter
   */
  Return_t xMemGetUsed(Size_t *size_);
  /**
   * @brief Gets the size of an allocated memory block
   * @details Returns the size of the data portion of the specified memory
   * block.
   *
   * @param[in]  addr_ Pointer to the allocated memory
   * @param[out] size_ Pointer to store the block size in bytes
   *
   * @return           ReturnOK if size was retrieved successfully
   * @return           ReturnError if address is invalid or invalid parameter
   */
  Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_);
  /**
   * @brief Gets heap memory statistics
   * @details Allocates and returns detailed statistics about heap memory usage.
   *
   * @param[out] stats_ Pointer to store allocated statistics structure
   *
   * @return            ReturnOK if statistics were retrieved successfully
   * @return            ReturnError if allocation failed or invalid parameter
   *
   * @warning Caller is responsible for freeing the allocated statistics
   * structure
   */
  Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_);
  /**
   * @brief Gets kernel memory statistics
   * @details Allocates and returns detailed statistics about kernel memory
   * usage.
   *
   * @param[out] stats_ Pointer to store allocated statistics structure
   *
   * @return            ReturnOK if statistics were retrieved successfully
   * @return            ReturnError if allocation failed or invalid parameter
   *
   * @warning Caller is responsible for freeing the allocated statistics
   * structure
   */
  Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_);
  /**
   * @brief Allocates memory from kernel region
   * @details Internal function to allocate memory from the kernel's dedicated
   * memory region.
   *
   * @param[out] addr_ Pointer to store the allocated memory address
   * @param[in]  size_ Size of memory to allocate in bytes
   *
   * @return           ReturnOK if memory was allocated successfully
   * @return           ReturnError if allocation failed or invalid parameters
   *
   * @note This is an internal function for kernel use only
   */
  Return_t __KernelAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
  /**
   * @brief Frees kernel-allocated memory
   * @details Internal function to return memory to the kernel region.
   *
   * @param[in] addr_ Pointer to the memory to free
   *
   * @return          ReturnOK if memory was freed successfully
   * @return          ReturnError if address is invalid or already freed
   *
   * @note This is an internal function for kernel use only
   */
  Return_t __KernelFreeMemory__(const volatile Addr_t *addr_);
  /**
   * @brief Allocates memory from heap region
   * @details Internal function to allocate memory from the heap memory region.
   *
   * @param[out] addr_ Pointer to store the allocated memory address
   * @param[in]  size_ Size of memory to allocate in bytes
   *
   * @return           ReturnOK if memory was allocated successfully
   * @return           ReturnError if allocation failed or invalid parameters
   *
   * @note This is an internal function called by xMemAlloc()
   */
  Return_t __HeapAllocateMemory__(volatile Addr_t **addr_, const Size_t size_);
  /**
   * @brief Frees heap-allocated memory
   * @details Internal function to return memory to the heap region.
   *
   * @param[in] addr_ Pointer to the memory to free
   *
   * @return          ReturnOK if memory was freed successfully
   * @return          ReturnError if address is invalid or already freed
   *
   * @note This is an internal function called by xMemFree()
   */
  Return_t __HeapFreeMemory__(const volatile Addr_t *addr_);
  /**
   * @brief Copies memory from source to destination
   * @details Internal memory copy implementation that handles volatile
   * pointers.
   *
   * @param[out] dest_ Destination memory address
   * @param[in]  src_  Source memory address
   * @param[in]  size_ Number of bytes to copy
   *
   * @return           ReturnOK if copy was successful
   * @return           ReturnError if invalid parameters
   *
   * @note This is an internal function similar to standard memcpy
   */
  Return_t __memcpy__(const volatile Addr_t *dest_, const volatile Addr_t *src_, const Size_t size_);
  /**
   * @brief Sets memory to a specified value
   * @details Internal memory set implementation that handles volatile pointers.
   *
   * @param[out] dest_ Destination memory address
   * @param[in]  val_  Byte value to set
   * @param[in]  size_ Number of bytes to set
   *
   * @return           ReturnOK if set was successful
   * @return           ReturnError if invalid parameters
   *
   * @note This is an internal function similar to standard memset
   */
  Return_t __memset__(const volatile Addr_t *dest_, const Byte_t val_, const Size_t size_);
  /**
   * @brief Compares two memory regions
   * @details Internal memory comparison implementation that handles volatile
   * pointers.
   *
   * @param[in]  s1_   First memory address
   * @param[in]  s2_   Second memory address
   * @param[in]  size_ Number of bytes to compare
   * @param[out] res_  Pointer to store comparison result (0 if equal, non-zero
   *                   otherwise)
   *
   * @return           ReturnOK if comparison was successful
   * @return           ReturnError if invalid parameters
   *
   * @note This is an internal function similar to standard memcmp
   */
  Return_t __memcmp__(const volatile Addr_t *s1_, const volatile Addr_t *s2_, const Size_t size_, Base_t *res_);
  /**
   * @brief Initializes the memory management system
   * @details Internal function to set up heap and kernel memory regions.
   *
   * @return ReturnOK if initialization was successful
   * @return ReturnError if initialization failed
   *
   * @note This is an internal function called during system startup
   */
  Return_t __MemoryInit__(void);

  #if defined(POSIX_ARCH_OTHER)
    void __MemoryClear__(void);

  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus

    }

  #endif /* ifdef __cplusplus */

#endif /* ifndef MEM_H_ */