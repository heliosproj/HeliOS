/*UNCRUSTIFY-OFF*/
/**
 * @file port.c
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
#include "port.h"


#if defined(CMSIS_ARCH_CORTEXM)
  static volatile Ticks_t sysTicks = zero;


  void SysTick_Handler(void) {
    DISABLE_INTERRUPTS();
    sysTicks++;
    ENABLE_INTERRUPTS();

    return;
  }


#endif /* if defined(CMSIS_ARCH_CORTEXM) */


Ticks_t __PortGetSysTicks__(void) {


#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || \
  defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || \
  defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)

    return(millis());

#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)

    return(0);

#elif defined(CMSIS_ARCH_CORTEXM)

    return(sysTicks);


#elif defined(POSIX_ARCH_OTHER)

    struct timeval t;


    gettimeofday(&t, null);

    return((t.tv_sec) * 1000 + (t.tv_usec) / 1000);

#endif /* if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) ||
        * defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) ||
        * defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_TEENSY_MICROMOD) ||
        * defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) ||
        * defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) ||
        * defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) ||
        * defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC) */
}


Return_t __PortInit__(void) {
  RET_DEFINE;

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || \
  defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || \
  defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)

    RET_OK;

#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)

    RET_OK;

#elif defined(CMSIS_ARCH_CORTEXM)

    SysTick_Config(SYSTEM_CORE_CLOCK_FREQUENCY / SYSTEM_CORE_CLOCK_PRESCALER);
    RET_OK;

#elif defined(POSIX_ARCH_OTHER)

    RET_OK;

#endif /* if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) ||
        * defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) ||
        * defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_TEENSY_MICROMOD) ||
        * defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) ||
        * defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) ||
        * defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) ||
        * defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC) */
  RET_RETURN;
}