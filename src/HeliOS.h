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
#ifndef HELIOS_H_
#define HELIOS_H_

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(ARDUINO_ARCH_AVR)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE() noInterrupts()
#define ENABLE() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(ARDUINO_ARCH_SAM)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE() noInterrupts()
#define ENABLE() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(ARDUINO_ARCH_SAMD)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE() noInterrupts()
#define ENABLE() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(ARDUINO_ARCH_ESP8266)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE() noInterrupts()
#define ENABLE() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(OTHER_ARCH_LINUX)
#include <time.h>
#define CURRENTTIME() CurrentTime()
#define DISABLE()
#define ENABLE()
typedef uint64_t Time_t;
#define TIME_T_MAX UINT64_MAX
#elif defined(OTHER_ARCH_WINDOWS)
#include <Windows.h>
#define CURRENTTIME() CurrentTime()
#define DISABLE()
#define ENABLE()
typedef int64_t Time_t;
#define TIME_T_MAX INT64_MAX
#elif defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE() noInterrupts()
#define ENABLE() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#elif defined(ESP32)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE() noInterrupts()
#define ENABLE() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#else
#pragma message("WARNING: This architecture is currently unsupported by HeliOS. If targeting Linux or Microsoft Windows, make sure to un-comment OTHER_ARCH_LINUX or OTHER_ARCH_WINDOWS in HeliOS.h.")
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE() noInterrupts()
#define ENABLE() interrupts()
typedef uint32_t Time_t;
#define TIME_T_MAX UINT32_MAX
#endif

#if !defined(null)
#define null 0x0
#endif

#define PRODUCTNAME_SIZE 16
#define PRODUCT_NAME "HeliOS"
#define MAJOR_VERSION_NO 0
#define MINOR_VERSION_NO 2
#define PATCH_VERSION_NO 8

typedef enum {
  TaskStateStopped,
  TaskStateRunning,
  TaskStateWaiting
} TaskState_t;

typedef struct TaskInfo_s {
  TaskId_t id;
  char name[TASKNAME_SIZE];
  TaskState_t state;
  int16_t notifyBytes;
  char notifyValue[TNOTIFYVALUE_SIZE];
  Time_t lastRuntime;
  Time_t totalRuntime;
  Time_t timerInterval;
  Time_t timerStartTime;
} TaskInfo_t;

typedef struct HeliOSInfo_s {
  int16_t tasks;
  char productName[PRODUCTNAME_SIZE];
  int16_t majorVersion;
  int16_t minorVersion;
  int16_t patchVersion;
} HeliOSInfo_t;

typedef struct {
  TaskId_t id;
  char name[TASKNAME_SIZE];
  TaskState_t state;
  Time_t lastRuntime;
  Time_t totalRuntime;
} TaskList_t;

typedef struct MemAllocRecord_s {
  size_t size;
  void *ptr;
} MemAllocRecord_t;

typedef int16_t Flag_t;

typedef struct Flags_s {
  Flag_t schedulerRunning;
  Flag_t critBlocking;
  Flag_t runtimeOverflow;
} Flags_t;

typedef void TaskParm_t;

typedef struct Task_s {
  int16_t id;
  char name[TASKNAME_SIZE];
  TaskState_t state;
  TaskParm_t taskParameter;
  void (*callback)(Task_t *, TaskParm_t *);
  int16_t notifyBytes;
  char notifyValue[TNOTIFYVALUE_SIZE];
  Time_t lastRuntime;
  Time_t totalRuntime;
  Time_t timerInterval;
  Time_t timerStartTime;
  struct Task_s *next;
} Task_t;

typedef struct TaskList_s {
  int16_t length;
  Task_t *head;
  Task_t *tail;
} TaskList_t;

typedef struct TaskNotification_s {
  int16_t notifyBytes;
  char notifyValue[TNOTIFYVALUE_SIZE];
} TaskNotification_t;

typedef struct QueueMessage_s {
  int16_t messageBytes;
  char messageValue[TNOTIFYVALUE_SIZE];
  struct QueueMessage_s *next;
} QueueMessage_t;

typedef struct Queue_s {
  int16_t length;
  QueueMessage_t *head;
  QueueMessage_t *tail;
} Queue_t;

typedef int16_t TaskId_t;
typedef Queue_t *xQueue;
typedef QueueMessage_t *xQueueItem;
typedef Taxk_t *xTask;
typedef TaskId_t xTaskId;
typedef TaskGetInfoResult_t *xTaskGetInfoResult;
typedef TaskGetNotifResult_t *xTaskGetNotifResult;
typedef HeliOSGetInfoResult_t *xHeliOSGetInfoResult;
typedef TaskGetListResult_t *xTaskGetListResult;
#endif