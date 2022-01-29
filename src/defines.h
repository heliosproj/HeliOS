/*
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
 */
#ifndef DEFINES_H_
#define DEFINES_H_

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
#elif defined(OTHER_ARCH_WINDOWS)
#include <Windows.h>
#define CURRENTTIME() CurrentTime()
#define DISABLE_INTERRUPTS()
#define ENABLE_INTERRUPTS()
typedef int64_t Time_t;
#define TIME_T_MAX INT64_MAX
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
#pragma message("WARNING: This architecture is currently unsupported by HeliOS. If targeting Linux or Microsoft Windows, make sure to un-comment OTHER_ARCH_LINUX or OTHER_ARCH_WINDOWS in HeliOS.h.")
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE_INTERRUPTS() noInterrupts()
#define ENABLE_INTERRUPTS() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#endif

#if !defined(null)
#define null 0x0
#endif

#if !defined(true)
#define true 0x1
#endif

#if !defined(false)
#define false 0x0
#endif

#if !defined(PRODUCTNAME_SIZE)
#define PRODUCTNAME_SIZE 16
#endif
#if !defined(PRODUCT_NAME)
#define PRODUCT_NAME "HeliOS"
#endif
#if !defined(MAJOR_VERSION_NO)
#define MAJOR_VERSION_NO 0
#endif
#if !defined(MINOR_VERSION_NO)
#define MINOR_VERSION_NO 3
#endif
#if !defined(PATCH_VERSION_NO)
#define PATCH_VERSION_NO 0
#endif

#if !defined(ENTER_CRITICAL)
#define ENTER_CRITICAL() flags.critBlocking = true;
#endif
#if !defined(EXIT_CRITICAL)
#define EXIT_CRITICAL() flags.critBlocking = false;
#endif
#endif