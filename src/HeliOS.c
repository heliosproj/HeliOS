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

#include "HeliOS.h"
#include "list.h"
#include "mem.h"
#include "task.h"
#include "timer.h"

volatile int heliOSSetupCalled = FALSE;
volatile int heliOSCriticalBlocking = FALSE;

void xHeliOSSetup() {
  if(!heliOSSetupCalled) {
    MemInit();
    TaskListInit();
    TaskInit();
    heliOSSetupCalled = TRUE;
  }
}

void xHeliOSLoop() {
  int waiting = 0;
  Task* waitingTask[WAITINGTASKSIZE];
  Task* runningTask = NULL;
  Task* task = NULL;
  unsigned long taskStartTime = 0;
  unsigned long leastRuntime = ULONG_MAX;
  heliOSCriticalBlocking = TRUE;

  /*
   * Disable interrupts while scheduler runs.
   */
#if defined(ARDUINO_ARCH_AVR)
  noInterrupts();
#elif defined(ARDUINO_ARCH_SAM)
  noInterrupts();
#elif defined(ARDUINO_ARCH_SAMD)
  noInterrupts();
#else
        #error “HeliOS is currently supported on the Arduino AVR, SAM and SAMD architectures. Other architectures may require porting of HeliOS.”
#endif
  TaskListRewind();
  do {
    task = TaskListGet();
    if (task) {
      if (task->state == TaskStateRunning && task->totalRuntime < leastRuntime) {
        leastRuntime = task->totalRuntime;
        runningTask = task;
      } else if (task->state == TaskStateWaiting) {
        if(waiting < WAITINGTASKSIZE) {
          waitingTask[waiting] = task;
          waiting++;
        }
      }
    }
  } while (TaskListMoveNext());

  /*
   * Re-enable interrupts after sceduler runs.
   */
#if defined(ARDUINO_ARCH_AVR)
  interrupts();
#elif defined(ARDUINO_ARCH_SAM)
  interrupts();
#elif defined(ARDUINO_ARCH_SAMD)
  interrupts();
#else
        #error “HeliOS is currently supported on the Arduino AVR, SAM and SAMD architectures. Other architectures may require porting of HeliOS.”
#endif
  for(int i = 0; i < waiting; i++) {
    if (waitingTask[i]->notifyBytes > 0) {
      taskStartTime = NOW();
      (*waitingTask[i]->callback)(waitingTask[i]->id);
      waitingTask[i]->lastRuntime = NOW() - taskStartTime;
      waitingTask[i]->totalRuntime += waitingTask[i]->lastRuntime;
      waitingTask[i]->notifyBytes = 0;
    } else if (waitingTask[i]->timerInterval > 0) {
      if (NOW() - waitingTask[i]->timerStartTime > waitingTask[i]->timerInterval) {
        taskStartTime = NOW();
        (*waitingTask[i]->callback)(waitingTask[i]->id);
        waitingTask[i]->lastRuntime = NOW() - taskStartTime;
        waitingTask[i]->totalRuntime += waitingTask[i]->lastRuntime;
        waitingTask[i]->timerStartTime = NOW();
      }
    }
  }
  if(runningTask) {
    taskStartTime = NOW();
    (*runningTask->callback)(runningTask->id);
    runningTask->lastRuntime = NOW() - taskStartTime;
    runningTask->totalRuntime += runningTask->lastRuntime;
  }
  heliOSCriticalBlocking = FALSE;
}

xHeliOSGetInfoResult* xHeliOSGetInfo() {
  int tasks = 0;
  Task* task = NULL;
  xHeliOSGetInfoResult* heliOSGetInfoResult = NULL;
  TaskListRewind();
  do {
    task = TaskListGet();
    if (task) {
      tasks++;
    }
  } while (TaskListMoveNext());
  heliOSGetInfoResult = (xHeliOSGetInfoResult*)xMemAlloc(sizeof(xHeliOSGetInfoResult));
  if (heliOSGetInfoResult) {
    heliOSGetInfoResult->tasks = tasks;
    strncpy_(heliOSGetInfoResult->productName, PRODUCTNAME, PRODUCTNAMESIZE);
    heliOSGetInfoResult->majorVersion = MAJORVERSION;
    heliOSGetInfoResult->minorVersion = MINORVERSION;
    heliOSGetInfoResult->patchVersion = PATCHVERSION;
  }
  return heliOSGetInfoResult;
}

int HeliOSIsCriticalBlocking() {
  return heliOSCriticalBlocking;
}

void HeliOSReset() {
  MemClear();
  MemInit();
  TaskListInit();
  TaskInit();
  heliOSSetupCalled = FALSE;
  heliOSCriticalBlocking = FALSE;
}

void memcpy_(void* dest_, void* src_, size_t n_) {
  char* src = (char*)src_;
  char* dest = (char*)dest_;
  for (unsigned int i = 0; i < n_; i++) {
    dest[i] = src[i];
  }
}

void memset_(void* dest_, int val_, size_t n_) {
  char* dest = (char*)dest_;
  for (unsigned int i = 0; i < n_; i++) {
    dest[i] = val_;
  }
}

char* strncpy_(char* dest_, const char* src_, size_t n) {
  const char* src = src_;
  char* dest = dest_;
  while (*src && n--) {
    *dest = *src;
    dest++;
    src++;
  }
  *dest = 0;
  return dest_;
}

int strncmp_(const char* str1_, const char* str2_, size_t n) {
  const char* str2 = str2_;
  const char* str1 = str1_;
  while (*str1 && n--) {
    if (*str1 != *str2) {
      return *str1 - *str2;
    }
    str1++;
    str2++;
  }
  return 0;
}
