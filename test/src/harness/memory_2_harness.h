/*UNCRUSTIFY-OFF*/
/**
 * @file memory_2_harness.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.4.1
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
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