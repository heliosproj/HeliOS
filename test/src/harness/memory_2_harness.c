/*UNCRUSTIFY-OFF*/
/**
 * @file memory_2_harness.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.4.0
 * @date 2022-08-27
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
/*UNCRUSTIFY-ON*/
#include "memory_2_harness.h"


void memory_2_harness(void) {
  unit_begin("unit name");


  /*UNCRUSTIFY-OFF*/
  /*
    1) Create something in the heap
    2) Call a memory syscall like xMemGetUsed() which should return ReturnOK
    3) Check the memfault flag (should be "false" at this point)
    4) Modify some part of the memory entry
    5) Call a memory syscall like xMemGetUsed() which should return ReturnError
    6) Check the memfault flag (should be "true" at this point)
    7) Call __MemoryClear__() and __SysStateClear__()

    Things we can screw with in the memory entry
    1) magic
    2) free
    3) blocks
    4) next
  */
  /*UNCRUSTIFY-ON*/
  unit_end();

  return;
}