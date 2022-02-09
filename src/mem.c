/**
 * @file mem.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for the management of dynamically allocated memory in HeliOS
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

void *xMemAlloc(size_t size_) {
  return null;
}
void xMemFree(void *ptr) {
  return;
}

/* The xMemGetUsed() system call returns the amount of memory in bytes
that is currently allocated. */
size_t xMemGetUsed() {
  return 0;
}

/* The xMemGetSize() system call returns the amount of memory in bytes that
is currently allocated to a specific pointer. */
size_t xMemGetSize(void *ptr_) {
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