/*UNCRUSTIFY-OFF*/
/**
 * @file port.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Hardware abstraction layer implementation
 * @details
 * Implements platform-specific functions for system ticks, critical sections, memory barriers, and hardware initialization to support multiple architectures.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#include "port.h"

#if defined(CMSIS_ARCH_CORTEXM)

  static volatile Ticks_t sysTicks = 0x0u;
  void SysTick_Handler(void) {

    __DisableInterrupts__();

    sysTicks++;

    __EnableInterrupts__();

    return;

  }


#endif /* if defined(CMSIS_ARCH_CORTEXM) */
Ticks_t __PortGetSysTicks__(void) {

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || \
  defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || \
  defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)

    return(millis());

#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)

    return(0x0u);

#elif defined(CMSIS_ARCH_CORTEXM)

    return(sysTicks);

#elif defined(POSIX_ARCH_OTHER)

    struct timeval t;

    gettimeofday(&t, null);

    return((t.tv_sec) * 0x3E8 + (t.tv_usec) / 0x3E8);

#endif /* if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) ||
        * defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) ||
        * defined(ARDUINO_ARCH_STM32) ||  */

}


/**
* @brief Initializes platform-specific hardware
* @details Sets up timers, interrupts, and other hardware required for HeliOS
* operation.
*
* @return ReturnOK if initialization was successful
* @return ReturnError if initialization failed
*
* @note This is an internal function called during system startup
*/
Return_t __PortInit__(void) {

  FUNCTION_ENTER;

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || \
  defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || \
  defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)

    __ReturnOk__();

#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)

    __ReturnOk__();

#elif defined(CMSIS_ARCH_CORTEXM)

    SysTick_Config(SYSTEM_CORE_CLOCK_FREQUENCY / SYSTEM_CORE_CLOCK_PRESCALER);

    __ReturnOk__();

#elif defined(POSIX_ARCH_OTHER)

    __ReturnOk__();

#endif /* if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) ||
        * defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) ||
        * defined(ARDUINO_ARCH_STM32) ||  */

  FUNCTION_EXIT;

}