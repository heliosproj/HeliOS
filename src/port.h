#ifndef PORT_H_

  #define PORT_H_

  #include "config.h"

  #include "defines.h"

  #include "types.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "console.h"

    #include "device.h"

    #include "fat.h"

    #include "fs.h"

  #endif 

  #include "mem.h"

  #include "posix.h"

  #include "queue.h"

  #include "streams.h"

  #include "sys.h"

  #include "task.h"

  #include "timer.h"

  #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || \

  defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || \

  defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)

    #include <Arduino.h>

    #define __DisableInterrupts__() noInterrupts()

    #define __EnableInterrupts__() interrupts()

  #elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)

    #pragma message("WARNING: The ESP32 Arduino core uses FreeRTOS. HeliOS and FreeRTOS cannot coexist in the same application.")

    #define __DisableInterrupts__()

    #define __EnableInterrupts__()

  #elif defined(CMSIS_ARCH_CORTEXM)

    #include "stm32f429xx.h"

    #define __DisableInterrupts__() __disable_irq()

    #define __EnableInterrupts__() __enable_irq()

    #define SYSTEM_CORE_CLOCK_FREQUENCY 0xF42400u 

    #define SYSTEM_CORE_CLOCK_PRESCALER 0x3E8u 

  #elif defined(POSIX_ARCH_OTHER)

    #include "posix.h"

    #include <stdio.h>

    #include <sys/time.h>

    #define __DisableInterrupts__()

    #define __EnableInterrupts__()

    #if defined(UNIT_TEST_COLORIZE)

      #define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l) printf("\033[95mkernel:\033[93m assert at %s:%d\n\033[39m", f, l)

    #else  

      #define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l) printf("kernel: assert at %s:%d\n", f, l)

    #endif 

  #endif 

  #ifdef __cplusplus

    extern "C" {

  #endif 

  Ticks_t __PortGetSysTicks__(void);

  Return_t __PortInit__(void);

  #ifdef __cplusplus

    }

  #endif 

#endif 

