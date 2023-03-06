/*UNCRUSTIFY-OFF*/
/**
 * @file memory_2_harness.h
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
#ifndef MEMORY_2_HARNESS_H_
  #define MEMORY_2_HARNESS_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "stream.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"

  #include "unit.h"

  #if defined(ENTRYSIZE)
    #undef ENTRYSIZE
  #endif /* if defined(ENTRYSIZE) */
  #define ENTRYSIZE ((sizeof(MemoryEntry_t) / CONFIG_MEMORY_REGION_BLOCK_SIZE) + (zero < (sizeof(MemoryEntry_t) % CONFIG_MEMORY_REGION_BLOCK_SIZE) ? 1 : 0))

  #if defined(ADDR2ENTRY)
    #undef ADDR2ENTRY
  #endif /* if defined(ADDR2ENTRY) */
  #define ADDR2ENTRY(addr_) ((MemoryEntry_t *) ((Byte_t *) (addr_) - (ENTRYSIZE * CONFIG_MEMORY_REGION_BLOCK_SIZE)))

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  void memory_2_harness(void);
  void test_magic(void);
  void test_free(void);
  void test_blocks(void);
  void test_next(void);


  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef MEMORY_2_HARNESS_H_ */