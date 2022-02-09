/**
 * @file defines.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Header file containing all of the C preprocessor define statements for HeliOS
 * @version 0.3.0
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
#ifndef DEFINES_H_
#define DEFINES_H_

#include <stdint.h>

// TO-DO: REMOVE BEFORE RELEASE!!
#define OTHER_ARCH_LINUX

/* Definition blocks for embedded platform and/or tool-chain
specific headers and functions to compile and run HeliOS. When a
new embedded platform and/or tool-chain is added, the following
defines and typedef must be included:

* CURRENTTIME()
* DISABLE_INTERRUPTS()
* ENABLE_INTERRUPTS()
* TIME_T_MAX
* Time_t

If no definition block for the embedded platform and/or tool-chain
is matched, the "else" block will print a compiler warning and attempt
to default to the Arduino platform and/or tool-chain. */
#if defined(ARDUINO_ARCH_AVR)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE_INTERRUPTS() noInterrupts()
#define ENABLE_INTERRUPTS() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(ARDUINO_ARCH_SAM)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE_INTERRUPTS() noInterrupts()
#define ENABLE_INTERRUPTS() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(ARDUINO_ARCH_SAMD)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE_INTERRUPTS() noInterrupts()
#define ENABLE_INTERRUPTS() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(ARDUINO_ARCH_ESP8266)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE_INTERRUPTS() noInterrupts()
#define ENABLE_INTERRUPTS() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(OTHER_ARCH_LINUX)
#include <time.h>
#define CURRENTTIME() CurrentTime()
#define DISABLE_INTERRUPTS()
#define ENABLE_INTERRUPTS()
typedef uint64_t Time_t;
#define TIME_T_MAX UINT64_MAX
#elif defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE_INTERRUPTS() noInterrupts()
#define ENABLE_INTERRUPTS() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(ESP32)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE_INTERRUPTS() noInterrupts()
#define ENABLE_INTERRUPTS() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#else
#pragma message("WARNING: This embedded platform and/or tool-chain is not currently supported by HeliOS - proceed with caution.")
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE_INTERRUPTS() noInterrupts()
#define ENABLE_INTERRUPTS() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#endif

/* Define *null* if not defined. */
#if !defined(null)
#define null ((void *)0x0)
#endif

/* Define "true" if not defined. */
#if !defined(true)
#define true 0x1
#endif

/* Define "false" if not defined. */
#if !defined(false)
#define false 0x0
#endif

/* Define the raw size of the heap in bytes based on the number of blocks
the heap contains and the size of each block in bytes. */
#if !defined(HEAP_RAW_SIZE)
#define HEAP_RAW_SIZE CONFIG_HEAP_SIZE_IN_BLOCKS * CONFIG_HEAP_BLOCK_SIZE
#endif

/* Define the size in bytes of the product name which is accessible through
xSystemGetSystemInfo(). */
#if !defined(PRODUCTNAME_SIZE)
#define PRODUCTNAME_SIZE 6
#endif

/* Define the product name which is accessible through xSystemGetSystemInfo(). */
#if !defined(PRODUCT_NAME)
#define PRODUCT_NAME "HeliOS"
#endif

/* Define the product major version number which is accessible through
xSystemGetSystemInfo(). */
#if !defined(MAJOR_VERSION_NO)
#define MAJOR_VERSION_NO 0
#endif

/* Define the product minor version number which is accessible through
xSystemGetSystemInfo(). */
#if !defined(MINOR_VERSION_NO)
#define MINOR_VERSION_NO 3
#endif

/* Define the product patch version number which is accessible through
xSystemGetSystemInfo(). */
#if !defined(PATCH_VERSION_NO)
#define PATCH_VERSION_NO 0
#endif

/* Define the macro which sets the critical flag to true when
entering a critical section of code. */
#if !defined(ENTER_CRITICAL)
#define ENTER_CRITICAL() sysFlags.critical = true;
#endif

/* Define the macro which sets the critical flag to false when
exiting a critical section of code. */
#if !defined(EXIT_CRITICAL)
#define EXIT_CRITICAL() sysFlags.critical = false;
#endif

/* Define a marco which makes null pointer checks more readable and
concise */
#if !defined(ISNOTNULLPTR)
#define ISNOTNULLPTR(p) null != p
#endif

/* Define a marco which makes null pointer checks more readable and
concise */
#if !defined(ISNULLPTR)
#define ISNULLPTR(p) null == p
#endif
#endif