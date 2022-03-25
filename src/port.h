/**
 * @file port.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for portability layer
 * @version 0.3.3
 * @date 2022-03-24
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
#ifndef PORT_H_
#define PORT_H_

#include "config.h"
#include "defines.h"
#include "types.h"
#include "mem.h"
#include "queue.h"
#include "sys.h"
#include "task.h"
#include "timer.h"



#if defined(ARDUINO_ARCH_AVR) /* TESTED 2022-03-24 */

/*

  https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/wiring.c

  Copyright (c) 2005-2006 David A. Mellis

  volatile unsigned long timer0_overflow_count = 0;
  volatile unsigned long timer0_millis = 0;
  static unsigned char timer0_fract = 0;

  #if defined(TIM0_OVF_vect)
  ISR(TIM0_OVF_vect)
  #else
  ISR(TIMER0_OVF_vect)
  #endif
  {
    // copy these to local variables so they can be stored in registers
    // (volatile variables must be read from memory on every access)
    unsigned long m = timer0_millis;
    unsigned char f = timer0_fract;

    m += MILLIS_INC;
    f += FRACT_INC;
    if (f >= FRACT_MAX) {
      f -= FRACT_MAX;
      m += 1;
    }

    timer0_fract = f;
    timer0_millis = m;
    timer0_overflow_count++;
  }

*/

extern unsigned long timer0_overflow_count;

#define DISABLE_INTERRUPTS() __asm__ __volatile__("cli")

#define ENABLE_INTERRUPTS() __asm__ __volatile__("sei")

#elif defined(ARDUINO_ARCH_SAM)

/*

  https://github.com/arduino/ArduinoCore-sam/blob/master/cores/arduino/cortex_handlers.c

  Copyright (c) 2012 Arduino.  All right reserved.

  void SysTick_Handler(void)
  {
    if (sysTickHook())
      return;

    tickReset();

    // Increment tick count each ms
    TimeTick_Increment();
  }

  https://github.com/arduino/ArduinoCore-sam/blob/master/system/libsam/source/timetick.c

  Copyright (c) 2011-2012, Atmel Corporation

  static volatile uint32_t _dwTickCount=0 ;

  extern void TimeTick_Increment( void )
  {
      _dwTickCount++ ;
  }

  extern uint32_t GetTickCount( void )
  {
      return _dwTickCount ;
  }

*/

extern uint32_t GetTickCount(void);

#define DISABLE_INTERRUPTS() __asm volatile ("cpsid i")

#define ENABLE_INTERRUPTS() __asm volatile ("cpsie i")

#elif defined(ARDUINO_ARCH_SAMD) /* TESTED 2022-03-24 */

/*

  https://github.com/arduino/ArduinoCore-samd/blob/master/cores/arduino/delay.c

  Copyright (c) 2015 Arduino LLC.  All right reserved.

  static volatile uint32_t _ulTickCount=0 ;

  unsigned long millis( void )
  {
  // todo: ensure no interrupts
    return _ulTickCount ;
  }

  void SysTick_DefaultHandler(void)
  {
    // Increment tick count each ms
    _ulTickCount++;
    tickReset();
  }

*/

extern unsigned long millis(void);

#define DISABLE_INTERRUPTS() __asm volatile ("cpsid i")

#define ENABLE_INTERRUPTS() __asm volatile ("cpsie i")

#elif defined(ARDUINO_ARCH_ESP8266)

#define DISABLE_INTERRUPTS()

#define ENABLE_INTERRUPTS()

#elif defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)

/*

  https://github.com/PaulStoffregen/cores/blob/master/teensy4/EventResponder.cpp

  Copyright 2017 Paul Stoffregen

  extern "C" volatile uint32_t systick_millis_count;
  extern "C" volatile uint32_t systick_cycle_count;
  extern "C" uint32_t systick_safe_read; // micros() synchronization
  extern "C" void systick_isr(void)
  {
    systick_cycle_count = ARM_DWT_CYCCNT;
    systick_millis_count++;
  }

  https://github.com/PaulStoffregen/cores/blob/master/teensy3/EventResponder.cpp

  Copyright 2017 Paul Stoffregen

  extern "C" volatile uint32_t systick_millis_count;

  void systick_isr(void)
  {
    systick_millis_count++;
  }

*/

extern uint32_t systick_millis_count;

#define DISABLE_INTERRUPTS() __asm volatile ("cpsid i")

#define ENABLE_INTERRUPTS() __asm volatile ("cpsie i")

#elif defined(ESP32)

#define DISABLE_INTERRUPTS()

#define ENABLE_INTERRUPTS()

#elif defined(STM32) /* TESTED 2022-03-24 */

/* ld linker script section

  .kernel_mem_region (NOLOAD):
  {
    . = ALIGN(0x8000);
    _start_kernel_mem_region = .;
    *(.kernel_mem_region*);
    . = ALIGN(0x8000);
    _end_kernel_mem_region = .;
    _size_kernel_mem_region = _end_kernel_mem_region - _start_kernel_mem_region;
  } > RAM

*/

#include "stm32f429xx.h"

#define DISABLE_INTERRUPTS() __asm volatile ("cpsid i")

#define ENABLE_INTERRUPTS() __asm volatile ("cpsie i")

#define SYSTEM_CORE_CLOCK_FREQUENCY 0xF42400u /* 16000000u */

#define SYSTEM_CORE_CLOCK_PRESCALER 0x3E8u /* 1000u */

#elif defined(DEBUG_ON) /* TESTED 2022-03-24 */

#include <stdio.h>
#include <time.h>

#define DISABLE_INTERRUPTS()

#define ENABLE_INTERRUPTS()

#define CONFIG_ENABLE_SYSTEM_ASSERT

#define CONFIG_SYSTEM_ASSERT_BEHAVIOR(file_, line_) printf("assert: %s:%d\n", file_, line_)

#endif



#ifdef __cplusplus
extern "C" {
#endif

Ticks_t _SysGetSysTicks_(void);
void _SysInit_(void);

#ifdef __cplusplus
}  // extern "C" {
#endif

#endif