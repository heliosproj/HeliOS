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
#include <stdbool.h>

#if defined(ARDUINO_ARCH_AVR)
  #include <Arduino.h>
  #define NOW() micros()
  #define DISABLE() noInterrupts()
  #define ENABLE() interrupts()
  typedef unsigned long Time;
  #define HELIOSTIMEMAX ULONG_MAX
#elif defined(ARDUINO_ARCH_SAM)
  #include <Arduino.h>
  #define NOW() micros()
  #define DISABLE() noInterrupts()
  #define ENABLE() interrupts()
  typedef unsigned long Time;
  #define HELIOSTIMEMAX ULONG_MAX
#elif defined(ARDUINO_ARCH_SAMD)
  #include <Arduino.h>
  #define NOW() micros()
  #define DISABLE() noInterrupts()
  #define ENABLE() interrupts()
  typedef unsigned long Time;
  #define HELIOSTIMEMAX ULONG_MAX
#else
  #if defined(OTHER_ARCH_WINDOWS)
    #include <Windows.h>
    #define NOW() HeliOSCurrTime()
    #define DISABLE()
    #define ENABLE()
    typedef uint64_t Time;
    #define HELIOSTIMEMAX UINT64_MAX
  #elif defined(OTHER_ARCH_LINUX)
    #include <time.h>
    #define NOW() HeliOSCurrTime()
    #define DISABLE()
    #define ENABLE()
    typedef uint64_t Time;
    #define HELIOSTIMEMAX UINT64_MAX
  #else
    #error "This architecture is currently unsupported by HeliOS."
  #endif
#endif

#ifndef NULL
  #define NULL 0x0
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
  int		id;
  char		name[TASKNAMESIZE];
  xTaskState	state;
  int		notifyBytes;
  char		notifyValue[NOTIFYVALUESIZE];
  Time		lastRuntime;
  Time		totalRuntime;
  Time		timerInterval;
  Time		timerStartTime;
} xTaskGetInfoResult;

typedef struct {
  int	notifyBytes;
  char	notifyValue[NOTIFYVALUESIZE];
} xTaskGetNotifResult;

typedef struct {
  int	tasks;
  char	productName[PRODUCTNAMESIZE];
  int	majorVersion;
  int	minorVersion;
  int	patchVersion;
} xHeliOSGetInfoResult;

typedef struct {
  int		id;
  char		name[TASKNAMESIZE];
  xTaskState	state;
  Time		lastRuntime;
  Time		totalRuntime;
} xTaskGetListResult;

struct TaskListItem_s;

typedef struct {
  int				id;
  char				name[TASKNAMESIZE];
  xTaskState			state;
  void (*callback)(int);
  int				notifyBytes;
  char				notifyValue[NOTIFYVALUESIZE];
  Time				lastRuntime;
  Time				totalRuntime;
  Time				timerInterval;
  Time				timerStartTime;
  struct TaskListItem_s *	next;
} Task;

typedef struct TaskListItem_s {
  Task *			task;
  struct TaskListItem_s *	next;
} TaskListItem;

typedef struct {
  size_t	size;
  void *	ptr;
} MemAllocRecord;

typedef struct {
  bool	setupCalled;
  bool	critBlocking;
  bool	runtimeOverflow;
} Flags;

#ifdef __cplusplus
  extern "C" {
#endif

void xHeliOSSetup();
void xHeliOSLoop();
xHeliOSGetInfoResult *xHeliOSGetInfo();
bool HeliOSIsCriticalBlocking();
void HeliOSReset();
Time HeliOSCurrTime();
void HeliOSRunTask(Task *);
void HeliOSResetRuntime();
void memcpy_(void *, void *, size_t);
void memset_(void *, int, size_t);
char *strncpy_(char *, const char *, size_t);
int strncmp_(const char *, const char *, size_t n);

#ifdef __cplusplus
} // extern "C" {
#endif
