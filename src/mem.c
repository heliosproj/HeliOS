/*
 * HeliOS Embedded Operating System
 * Copyright (C) 2020 Manny Peterson <me@mannypeterson.com>
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
 */

#include "HeliOS.h"
#include "list.h"
#include "mem.h"
#include "task.h"

volatile MemAllocRecord_t memAllocTable[MEMALLOCTABLE_SIZE];

void MemInit() {
  memset_(&memAllocTable, 0, MEMALLOCTABLE_SIZE * sizeof(MemAllocRecord_t));
}

void *xMemAlloc(size_t size_) {
  void *ptr = null;

  if (size_ > 0) {
    for (size_t i = 0; i < MEMALLOCTABLE_SIZE; i++) {
      if (!memAllocTable[i].ptr) {
        ptr = calloc(1, size_);
        if (ptr) {
          memAllocTable[i].size = size_;
          memAllocTable[i].ptr = ptr;
          return ptr;
        }
      }
    }
  }
  return null;
}

void xMemFree(void *ptr_) {
  if (ptr_) {
    for (size_t i = 0; i < MEMALLOCTABLE_SIZE; i++) {
      if (memAllocTable[i].ptr == ptr_) {
        free(memAllocTable[i].ptr);
        memAllocTable[i].size = 0;
        memAllocTable[i].ptr = null;
      }
    }
  }
}

size_t xMemGetUsed() {
  size_t used = 0;

  for (size_t i = 0; i < MEMALLOCTABLE_SIZE; i++)
    if (memAllocTable[i].ptr)
      used += memAllocTable[i].size;
  return used;
}

size_t xMemGetSize(void *ptr_) {
  if (ptr_) {
    for (size_t i = 0; i < MEMALLOCTABLE_SIZE; i++)
      if (memAllocTable[i].ptr == ptr_)
        return memAllocTable[i].size;
  }
  return 0;
}

void MemClear() {
  for (size_t i = 0; i < MEMALLOCTABLE_SIZE; i++) {
    if (memAllocTable[i].ptr) {
      free(memAllocTable[i].ptr);
      memAllocTable[i].size = 0;
      memAllocTable[i].ptr = null;
    }
  }
}
