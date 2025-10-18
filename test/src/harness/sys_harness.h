/*UNCRUSTIFY-OFF*/
/**
 * @file sys_harness.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit test harness header for system API
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef SYS_HARNESS_H_
  #define SYS_HARNESS_H_


  #include "../unit/unit.h"
  void sys_harness(void);
  void test_system_init(void);
  void test_system_info(void);
  void test_system_halt(void);
  void test_system_assert(void);
  void test_enhanced_assertions(void);


#endif /* ifndef SYS_HARNESS_H_ */