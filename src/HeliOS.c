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

volatile Flags_t flags = {
  .setupCalled		= false,
  .critBlocking		= false,
  .runtimeOverflow	= false
};

void xHeliOSSetup() {
  if (!flags.setupCalled) {
    MemInit();
    TaskListInit();
    TaskInit();
    flags.setupCalled = true;
  }
}

void xHeliOSLoop() {
  int waiting = 0;
  Task_t *waitingTask[WAITINGTASKSIZE];
  Task_t *runningTask = NULL;
  Task_t *task = NULL;
  Time_t leastRuntime = TIMEMAX;

  flags.critBlocking = true;

  if (flags.runtimeOverflow)
    RuntimeReset();

  /*
   * Disable interrupts while scheduler runs.
   */
  DISABLE();
  TaskListRewind();
  do {
    task = TaskListGet();
    if (task) {
      if (task->state == TaskStateRunning && task->totalRuntime < leastRuntime) {
        leastRuntime = task->totalRuntime;
        runningTask = task;
      } else if (task->state == TaskStateWaiting) {
        if (waiting < WAITINGTASKSIZE) {
          waitingTask[waiting] = task;
          waiting++;
        }
      }
    }
  } while (TaskListMoveNext());

  /*
   * Re-enable interrupts after sceduler runs.
   */
  ENABLE();
  for (int i = 0; i < waiting; i++) {
    if (waitingTask[i]->notifyBytes > 0) {
      TaskRun(waitingTask[i]);
      waitingTask[i]->notifyBytes = 0;
    } else if (waitingTask[i]->timerInterval > 0) {
      if (NOW() - waitingTask[i]->timerStartTime > waitingTask[i]->timerInterval) {
        TaskRun(waitingTask[i]);
        waitingTask[i]->timerStartTime = NOW();
      }
    }
  }
  if (runningTask)
    TaskRun(runningTask);
  flags.critBlocking = false;
}

xHeliOSGetInfoResult *xHeliOSGetInfo() {
  int tasks = 0;
  Task_t *task = NULL;
  xHeliOSGetInfoResult *heliOSGetInfoResult = NULL;

  TaskListRewind();
  do {
    task = TaskListGet();
    if (task)
      tasks++;
  } while (TaskListMoveNext());
  heliOSGetInfoResult = (xHeliOSGetInfoResult *)xMemAlloc(sizeof(xHeliOSGetInfoResult));
  if (heliOSGetInfoResult) {
    heliOSGetInfoResult->tasks = tasks;
    strncpy_(heliOSGetInfoResult->productName, PRODUCTNAME, PRODUCTNAMESIZE);
    heliOSGetInfoResult->majorVersion = MAJORVERSION;
    heliOSGetInfoResult->minorVersion = MINORVERSION;
    heliOSGetInfoResult->patchVersion = PATCHVERSION;
  }
  return heliOSGetInfoResult;
}

bool IsCriticalBlocking() {
  return flags.critBlocking;
}

void HeliOSReset() {
  MemClear();
  MemInit();
  TaskListInit();
  TaskInit();
  flags.setupCalled = false;
  flags.critBlocking = false;
  flags.runtimeOverflow = false;
}

inline Time_t CurrentTime() {
#if defined(OTHER_ARCH_WINDOWS)
    /*
     * Get time from Windows. Need to implement and test.
     */
    return 0;
#elif defined(OTHER_ARCH_LINUX)
    /*
     * Get time from Linux. Need to implement and test.
     */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
#else
    return 0;
#endif
}

inline void TaskRun(Task_t *task_) {
  Time_t taskStartTime = 0;
  Time_t prevTotalRuntime = 0;

  prevTotalRuntime = task_->totalRuntime;
  taskStartTime = NOW();
  (*task_->callback)(task_->id);
  task_->lastRuntime = NOW() - taskStartTime;
  task_->totalRuntime += task_->lastRuntime;
  if (task_->totalRuntime < prevTotalRuntime)
    flags.runtimeOverflow = true;
}

inline void RuntimeReset() {
  Task_t *task = NULL;

  TaskListRewind();
  do {
    task = TaskListGet();
    if (task)
      task->totalRuntime = task->lastRuntime;
  } while (TaskListMoveNext());
  flags.runtimeOverflow = false;
}

void memcpy_(void *dest_, void *src_, size_t n_) {
  char *src = (char *)src_;
  char *dest = (char *)dest_;

  for (unsigned int i = 0; i < n_; i++)
    dest[i] = src[i];
}

void memset_(void *dest_, int val_, size_t n_) {
  char *dest = (char *)dest_;

  for (unsigned int i = 0; i < n_; i++)
    dest[i] = val_;
}

char *strncpy_(char *dest_, const char *src_, size_t n_) {
  const char *src = src_;
  char *dest = dest_;

  while (*src && n_--) {
    *dest = *src;
    dest++;
    src++;
  }
  *dest = 0;
  return dest_;
}

int strncmp_(const char *str1_, const char *str2_, size_t n_) {
  const char *str2 = str2_;
  const char *str1 = str1_;

  while (*str1 && n_--) {
    if (*str1 != *str2)
      return *str1 - *str2;
    str1++;
    str2++;
  }
  return 0;
}
