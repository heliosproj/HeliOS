/**
 * @file mem.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for the management of dynamically allocated memory in HeliOS
 * @version 0.3.0
 * @date 2022-01-31
 *
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

/* Declare a static array of type DynamicMemoryAllocEntry_t to store
the pointer and size of the memory allocated from the heap by xMemAlloc(). The
size of the table is dependent on the setting CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES. */
DynamicMemoryAllocEntry_t dynamicMemoryAllocTable[CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES];

/**
 * @brief The xMemAlloc() system call will dynamically allocate memory for HeliOS
 * system calls and end-user tasks. The number of concurrently allocated pointers
 * is dependent on the setting CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES.
 *
 * @param size_ The amount (size) of the memory to be dynamically allocated in bytes.
 * @return void* If successful, xMemAlloc() returns a pointer to the dynamically
 * allocated memory. If unsuccessful, the system call will return null.
 */
void *xMemAlloc(size_t size_) {
  void *ptr = null;

  /* Make sure the requested memory size is greater than zero. */
  if (size_ > 0) {
    for (size_t i = 0; i < CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES; i++) {
      /* Check if entry at index a empty entry in the dynamicMemoryAllocTable to use. */
      if (ISNULLPTR(dynamicMemoryAllocTable[i].ptr)) {
        ptr = calloc(1, size_);

        /* Ensure that calloc() successfully allocated the requested memory. If
        successful, add the pointer and size to the table and return the pointer. */
        if (ISNOTNULLPTR(ptr)) {
          dynamicMemoryAllocTable[i].size = size_;

          dynamicMemoryAllocTable[i].ptr = ptr;

          return ptr;
        }
      }
    }
  }

  return null;
}

/**
 * @brief The xMemFree() system call will free memory dynamically allocated by
 * xMemAlloc() and other HeliOS system calls such as xSystemGetSystemInfo().
 *
 * @param ptr_ The pointer to the dynamically allocated memory to be freed.
 */
void xMemFree(void *ptr_) {
  /* Check to make sure the pointer passed to xMemFree() is not null. */
  if (ISNOTNULLPTR(ptr_)) {
    for (size_t i = 0; i < CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES; i++) {
      /* Check to see if the pointer at the current index in the dynamicMemoryAllocTable
      matches the pointer parameter. If so, free() the memory and reset the entry
      at the current index. */
      if (dynamicMemoryAllocTable[i].ptr == ptr_) {
        free(dynamicMemoryAllocTable[i].ptr);

        dynamicMemoryAllocTable[i].size = 0;

        dynamicMemoryAllocTable[i].ptr = null;
      }
    }
  }
}

/**
 * @brief The xMemGetUsed() system call returns the amount of memory in bytes
 * that is currently allocated. Calls to xMemAlloc() increases and xMemFree()
 * decreases the amount.
 *
 * @return size_t The amount of memory currently allocated in bytes. If no dynamic
 * memory is currently allocated, xMemGetUsed() will return zero.
 */
size_t xMemGetUsed() {
  size_t used = 0;

  for (size_t i = 0; i < CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES; i++) {
    /* If the pointer at the current index in the dynamicMemoryAllocTable
    is not null, then sum the size and store the result in used. */
    if (ISNOTNULLPTR(dynamicMemoryAllocTable[i].ptr)) {
      used += dynamicMemoryAllocTable[i].size;
    }
  }

  return used;
}

/**
 * @brief The xMemGetSize() system call returns the amount of memory in bytes that
 * is currently allocated to a specific pointer. If the pointer is null or invalid,
 * xMemGetSize() will return zero bytes.
 *
 * @param ptr_ The pointer to the dynamically allocated memory to obtain the size of the
 * memory that is allocated.
 * @return size_t The amount of memory currently allocated to the specific pointer in bytes. If
 * the pointer is invalid or null, xMemGetSize() will return zero.
 */
size_t xMemGetSize(void *ptr_) {
  /* Check to see if the pointer paramater is null, if it is not null, then search the
  table for the pointer and return the size. */
  if (ISNOTNULLPTR(ptr_)) {
    for (size_t i = 0; i < CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES; i++) {
      if (dynamicMemoryAllocTable[i].ptr == ptr_) {
        return dynamicMemoryAllocTable[i].size;
      }
    }
  }

  return 0;
}