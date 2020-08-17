/*
 * HeliOS Embedded Operating System
 * Copyright (C) 2020 Manny Peterson <me@mannypeterson.com>
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

#include <stdlib.h>
#include <limits.h>

#if defined(ARDUINO_ARCH_AVR)
        #include <Arduino.h>
        #define NOW() micros()
        #define DISABLE() noInterrupts()
        #define ENABLE() interrupts()
#elif defined(ARDUINO_ARCH_SAM)
        #include <Arduino.h>
        #define NOW() micros()
        #define DISABLE() noInterrupts()
        #define ENABLE() interrupts()
#elif defined(ARDUINO_ARCH_SAMD)
        #include <Arduino.h>
        #define NOW() micros()
        #define DISABLE() noInterrupts()
        #define ENABLE() interrupts()
#else
        #error “HeliOS is currently supported on the Arduino AVR, SAM and SAMD architectures. Other architectures may require porting of HeliOS.”
#endif

#ifndef NULL
        #define NULL 0
#endif

#ifndef FALSE
        #define FALSE (1 != 1)
#endif

#ifndef TRUE
        #define TRUE (!FALSE)
#endif

#define TASKNAMESIZE 16
#define NOTIFYVALUESIZE 16
#define PRODUCTNAMESIZE 16
#define MAJORVERSION 0
#define MINORVERSION 2
#define PATCHVERSION 4
#define PRODUCTNAME "HeliOS"
#define MEMALLOCTABLESIZE 50
#define WAITINGTASKSIZE 8

typedef enum {
  TaskStateErrored,
  TaskStateStopped,
  TaskStateRunning,
  TaskStateWaiting
} xTaskState;

typedef struct {
  int id;
  char name[TASKNAMESIZE];
  xTaskState state;
  int notifyBytes;
  char notifyValue[NOTIFYVALUESIZE];
  unsigned long lastRuntime;
  unsigned long totalRuntime;
  unsigned long timerInterval;
  unsigned long timerStartTime;
} xTaskGetInfoResult;

typedef struct {
  int notifyBytes;
  char notifyValue[NOTIFYVALUESIZE];
} xTaskGetNotifResult;

typedef struct {
  int tasks;
  char productName[PRODUCTNAMESIZE];
  int majorVersion;
  int minorVersion;
  int patchVersion;
} xHeliOSGetInfoResult;

typedef struct {
  int id;
  char name[TASKNAMESIZE];
  xTaskState state;
  unsigned long lastRuntime;
  unsigned long totalRuntime;
} xTaskGetListResult;

struct tasklistitem_s;

typedef struct {
  int id;
  char name[TASKNAMESIZE];
  xTaskState state;
  void (*callback)(int);
  int notifyBytes;
  char notifyValue[NOTIFYVALUESIZE];
  unsigned long lastRuntime;
  unsigned long totalRuntime;
  unsigned long timerInterval;
  unsigned long timerStartTime;
  struct tasklistitem_s* next;
} Task;

typedef struct tasklistitem_s {
  Task* task;
  struct tasklistitem_s* next;
} TaskListItem;

typedef struct {
  size_t size;
  void* ptr;
} MemAllocRecord;

#ifdef __cplusplus
extern "C" {
#endif

void xHeliOSSetup();
void xHeliOSLoop();
xHeliOSGetInfoResult* xHeliOSGetInfo();
int HeliOSIsCriticalBlocking();
void HeliOSReset();
void memcpy_(void*, void*, size_t);
void memset_(void*, int, size_t);
char* strncpy_(char*, const char*, size_t);
int strncmp_(const char*, const char*, size_t n);

#ifdef __cplusplus
} // extern "C" {
#endif
