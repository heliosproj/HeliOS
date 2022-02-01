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

DynamicMemoryAllocEntry_t dynamicMemoryAllocTable[CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES];

void *xMemAlloc(size_t size_) {
  void *ptr = null;

  if (size_ > 0) {
    for (size_t i = 0; i < CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES; i++) {
      if (!dynamicMemoryAllocTable[i].ptr) {
        ptr = calloc(1, size_);
        if (ptr) {
          dynamicMemoryAllocTable[i].size = size_;
          dynamicMemoryAllocTable[i].ptr = ptr;
          return ptr;
        }
      }
    }
  }
  return null;
}

void xMemFree(void *ptr_) {
  if (ptr_) {
    for (size_t i = 0; i < CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES; i++) {
      if (dynamicMemoryAllocTable[i].ptr == ptr_) {
        free(dynamicMemoryAllocTable[i].ptr);
        dynamicMemoryAllocTable[i].size = 0;
        dynamicMemoryAllocTable[i].ptr = null;
      }
    }
  }
}

size_t xMemGetUsed() {
  size_t used = 0;

  for (size_t i = 0; i < CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES; i++)
    if (dynamicMemoryAllocTable[i].ptr)
      used += dynamicMemoryAllocTable[i].size;
  return used;
}

size_t xMemGetSize(void *ptr_) {
  if (ptr_) {
    for (size_t i = 0; i < CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES; i++)
      if (dynamicMemoryAllocTable[i].ptr == ptr_)
        return dynamicMemoryAllocTable[i].size;
  }
  return 0;
}