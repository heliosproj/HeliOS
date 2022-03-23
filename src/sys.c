/**
 * @file sys.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources system related calls
 * @version 0.3.3
 * @date 2022-01-31
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
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

#include "sys.h"

/* "You are not expected to understand this."
Thank you for the best OS on Earth, Dennis. */



#if defined(ARDUINO_ARCH_AVR)

extern volatile unsigned long timer0_overflow_count;

#elif defined(ARDUINO_ARCH_SAM)
#elif defined(ARDUINO_ARCH_SAMD)
#elif defined(ARDUINO_ARCH_ESP8266)
#elif defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)
#elif defined(ESP32)
#elif defined(STM32)

static volatile Ticks_t sysTicks = zero;


void SysTick_Handler(void) {

  DISABLE_INTERRUPTS();

  sysTicks++;

  ENABLE_INTERRUPTS();

  return;
}

#elif defined(DEBUG)
#else
#endif



/* Declare and set the system flags to their default values. */
SysFlags_t sysFlags = {
    .running = false,
    .overflow = false,
    .corrupt = false};



/* The _SystemAssert_() system call will be called when
the SYSASSERT() macro evaluates false. In order for there
to be any effect, CONFIG_ENABLE_SYSTEM_ASSERT and
CONFIG_SYSTEM_ASSERT_BEHAVIOR must be defined.

_SystemAssert_() should NOT be called directly. Instead
use the SYSASSERT() C macro. */
void _SystemAssert_(const char *file_, int line_) {

  /* Do not modify this system call directly. Define
  the behavior (code) through the CONFIG_SYSTEM_ASSERT_BEHAVIOR
  setting in the config.h header file. */

#if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR)
  CONFIG_SYSTEM_ASSERT_BEHAVIOR(file_, line_);
#endif



  return;
}



Ticks_t _SysGetSysTicks_(void) {

#if defined(ARDUINO_ARCH_AVR)

  return timer0_overflow_count;

#elif defined(ARDUINO_ARCH_SAM)
#elif defined(ARDUINO_ARCH_SAMD)
#elif defined(ARDUINO_ARCH_ESP8266)
#elif defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)
#elif defined(ESP32)
#elif defined(STM32)

  return sysTicks;

#elif defined(DEBUG)

  struct timespec t;

  clock_gettime(CLOCK_MONOTONIC_RAW, &t);

  return (t.tv_sec * 1000000) + (t.tv_nsec / 1000);

#else

  return zero;

#endif
}



void xSystemInit(void) {

#if defined(ARDUINO_ARCH_AVR)
#elif defined(ARDUINO_ARCH_SAM)
#elif defined(ARDUINO_ARCH_SAMD)
#elif defined(ARDUINO_ARCH_ESP8266)
#elif defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)
#elif defined(ESP32)
#elif defined(STM32)

  SysTick_Config(SYSTEM_CORE_CLOCK_FREQUENCY / SYSTEM_CORE_CLOCK_PRESCALER);

  return;

#elif defined(DEBUG)
#else

  return;

#endif
}




/* The xSystemHalt() system call halts the system. */
void xSystemHalt(void) {


  /* Don't want to service interrupts anymore so disable them. */
  DISABLE_INTERRUPTS();



  /* Put the processor into an infinite loop. */
  for (;;) {
    /* Do nothing - literally. */
  }
}

/* The xSystemGetSystemInfo() system call will return the type xSystemInfo containing
information about the system including the OS (product) name, its version and how many tasks
are currently in the running, suspended or waiting states. */
SystemInfo_t *xSystemGetSystemInfo(void) {

  SystemInfo_t *ret = NULL;



  ret = (SystemInfo_t *)_HeapAllocateMemory_(sizeof(SystemInfo_t));



  /* Assert if xMemAlloc() failed to allocate the heap memory. */
  SYSASSERT(ISNOTNULLPTR(ret));



  /* Check if system info is not null to make sure xMemAlloc() successfully allocated
  the memory. */
  if (ISNOTNULLPTR(ret)) {


    _memcpy_(ret->productName, OS_PRODUCT_NAME, OS_PRODUCT_NAME_SIZE);

    ret->majorVersion = OS_MAJOR_VERSION_NO;

    ret->minorVersion = OS_MINOR_VERSION_NO;

    ret->patchVersion = OS_PATCH_VERSION_NO;

    ret->numberOfTasks = xTaskGetNumberOfTasks();
  }

  return ret;
}