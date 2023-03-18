/*UNCRUSTIFY-OFF*/
/**
 * @file port.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for portability layer
 * @version 0.4.0
 * @date 2022-03-24
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
#ifndef PORT_H_
  #define PORT_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "stream.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"


  #if defined(ARDUINO_ARCH_AVR)

    #include <Arduino.h>

    #define DISABLE_INTERRUPTS() interrupts()

    #define ENABLE_INTERRUPTS() noInterrupts()

  #elif defined(ARDUINO_ARCH_SAM)

    #include <Arduino.h>

    #define DISABLE_INTERRUPTS() interrupts()

    #define ENABLE_INTERRUPTS() noInterrupts()

  #elif defined(ARDUINO_ARCH_SAMD)

    #include <Arduino.h>

    #define DISABLE_INTERRUPTS() interrupts()

    #define ENABLE_INTERRUPTS() noInterrupts()

  #elif defined(ARDUINO_ARCH_ESP8266)

    #include <Arduino.h>

    #define DISABLE_INTERRUPTS() interrupts()

    #define ENABLE_INTERRUPTS() noInterrupts()

  #elif defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || \
  defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || \
  defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)

    #include <Arduino.h>

    #define DISABLE_INTERRUPTS() interrupts()

    #define ENABLE_INTERRUPTS() noInterrupts()

  #elif defined(ESP32)

    #pragma \
  message("WARNING: The ESP32 Arduino core uses FreeRTOS. HeliOS and FreeRTOS cannot coexist in the same application. If your application requires an embedded operating system, use the built-in FreeRTOS included with the ESP32 Arduino core.")

    #define DISABLE_INTERRUPTS()

    #define ENABLE_INTERRUPTS()

  #elif defined(CMSIS_ARCH_CORTEXM)

    #include "stm32f429xx.h"

    #define DISABLE_INTERRUPTS() __disable_irq()

    #define ENABLE_INTERRUPTS() __enable_irq()

    #define SYSTEM_CORE_CLOCK_FREQUENCY 0xF42400u /* 16000000u */

    #define SYSTEM_CORE_CLOCK_PRESCALER 0x3E8u /* 1000u */

  #elif defined(POSIX_ARCH_OTHER)

    #include "posix.h"

    #include <stdio.h>
    #include <sys/time.h>

    #define DISABLE_INTERRUPTS()

    #define ENABLE_INTERRUPTS()
    #if defined(UNIT_TEST_COLORIZE)
      #define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l) printf("\033[95mkernel:\033[93m assert at %s:%d\n\033[39m", f, l)
    #else  /* if defined(UNIT_TEST_COLORIZE) */
      #define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l) printf("kernel: assert at %s:%d\n", f, l)
    #endif /* if defined(UNIT_TEST_COLORIZE) */

  #elif defined(ARDUINO_ARCH_STM32)

    #include <Arduino.h>

    #define DISABLE_INTERRUPTS() interrupts()

    #define ENABLE_INTERRUPTS() noInterrupts()

  #endif /* if defined(ARDUINO_ARCH_AVR) */


  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  Ticks_t __PortGetSysTicks__(void);
  Return_t __PortInit__(void);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */

#endif /* ifndef PORT_H_ */