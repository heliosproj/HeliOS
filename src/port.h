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



#if defined(ARDUINO_ARCH_AVR)

#include <Arduino.h>

extern volatile unsigned long timer0_overflow_count;

#define DISABLE_INTERRUPTS() __asm__ __volatile__("cli")

#define ENABLE_INTERRUPTS() __asm__ __volatile__("sei")

#elif defined(ARDUINO_ARCH_SAM)

#include <Arduino.h>

extern uint32_t GetTickCount(void);

#define DISABLE_INTERRUPTS() __disable_irq()

#define ENABLE_INTERRUPTS() __enable_irq()

#elif defined(ARDUINO_ARCH_SAMD)

#include <Arduino.h>

extern unsigned long millis(void);

#define DISABLE_INTERRUPTS() __disable_irq()

#define ENABLE_INTERRUPTS() __enable_irq()

#elif defined(ARDUINO_ARCH_ESP8266)

#define DISABLE_INTERRUPTS()

#define ENABLE_INTERRUPTS()

#elif defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)

#include <Arduino.h>

extern volatile uint32_t systick_millis_count;

#define DISABLE_INTERRUPTS() __disable_irq()

#define ENABLE_INTERRUPTS() __enable_irq()

#elif defined(ESP32)

#define DISABLE_INTERRUPTS()

#define ENABLE_INTERRUPTS()

#elif defined(STM32)

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

#define DISABLE_INTERRUPTS() __disable_irq()

#define ENABLE_INTERRUPTS() __enable_irq()

#define SYSTEM_CORE_CLOCK_FREQUENCY 0xF42400u /* 16000000u */

#define SYSTEM_CORE_CLOCK_PRESCALER 0x3E8u /* 1000u */

#elif defined(DEBUG_ON)

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