/*UNCRUSTIFY-OFF*/
/**
 * @file port.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for portability layer
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


  #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || \
  defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || \
  defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)

    #include <Arduino.h>

    #define DISABLE_INTERRUPTS() noInterrupts()

    #define ENABLE_INTERRUPTS() interrupts()

  #elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)

    #pragma message("WARNING: The ESP32 Arduino core uses FreeRTOS. HeliOS and FreeRTOS cannot coexist in the same application.")

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

  #endif /* if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) ||
          * defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) ||
          * defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_TEENSY_MICROMOD) ||
          * defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) ||
          * defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) ||
          * defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) ||
          * defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC) */


  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  Ticks_t __PortGetSysTicks__(void);
  Return_t __PortInit__(void);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */

#endif /* ifndef PORT_H_ */