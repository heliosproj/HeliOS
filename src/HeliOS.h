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
#elif defined(ARDUINO_ARCH_SAM)
	#include <Arduino.h>
	#define NOW() micros()
#elif defined(ARDUINO_ARCH_SAMD)
	#include <Arduino.h>
	#define NOW() micros()
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
#define PATCHVERSION 3
#define PRODUCTNAME "HeliOS"
#define MEMALLOCTABLESIZE 50
#define WAITINGTASKSIZE 8

enum xTaskState {
	TaskStateErrored,
	TaskStateStopped,
	TaskStateRunning,
	TaskStateWaiting
};

struct xTaskGetInfoResult {
	int id;
	char name[TASKNAMESIZE];
	enum xTaskState state;
	int notifyBytes;
	char notifyValue[NOTIFYVALUESIZE];
	unsigned long lastRuntime;
	unsigned long totalRuntime;
	unsigned long timerInterval;
	unsigned long timerStartTime;
};

struct xTaskGetNotifResult {
	int notifyBytes;
	char notifyValue[NOTIFYVALUESIZE];
};

struct xHeliOSGetInfoResult {
	int tasks;
	char productName[PRODUCTNAMESIZE];
	int majorVersion;
	int minorVersion;
	int patchVersion;
};

struct xTaskGetListResult {
	int id;
	char name[TASKNAMESIZE];
	enum xTaskState state;
	unsigned long lastRuntime;
	unsigned long totalRuntime;
};

struct Task {
	int id;
	char name[TASKNAMESIZE];
	enum xTaskState state;
	void (*callback)(int);
	int notifyBytes;
	char notifyValue[NOTIFYVALUESIZE];
	unsigned long lastRuntime;
	unsigned long totalRuntime;
	unsigned long timerInterval;
	unsigned long timerStartTime;
	struct TaskListItem* next;
};

struct TaskListItem {
	struct Task* task;
	struct TaskListItem* next;
};

struct MemAllocRecord {
	size_t size;
	void* ptr;
};

#ifdef __cplusplus
extern "C" {
#endif

void xHeliOSSetup();
void xHeliOSLoop();
struct xHeliOSGetInfoResult* xHeliOSGetInfo();
int HeliOSIsCriticalBlocking();
void HeliOSReset();
void memcpy_(void*, void*, size_t);
void memset_(void*, int, size_t);
char* strncpy_(char*, const char*, size_t);
int strncmp_(const char*, const char*, size_t n);

#ifdef __cplusplus
} // extern "C" {
#endif
