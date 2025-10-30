/*UNCRUSTIFY-OFF*/
/**
 * @file console_harness.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for console
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef CONSOLE_HARNESS_H_
  #define CONSOLE_HARNESS_H_

  #include "defines.h"
  #include "unit.h"
  #include "mock_usart.h"
  #include "../../../src/console.h"

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  /* Console harness entry point */
  void console_harness(void);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */

#endif /* ifndef CONSOLE_HARNESS_H_ */