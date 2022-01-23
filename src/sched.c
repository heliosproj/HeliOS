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

#include "sched.h"

volatile Flags_t flags = {
    .setupCalled = false,
    .critBlocking = false,
    .runtimeOverflow = false};

void xHeliOSSetup() {
  if (!flags.setupCalled) {
    MemInit();
    TaskListInit();
    TaskInit();
    flags.setupCalled = true;
  }
}

void xHeliOSLoop() {
  Task_t *runTask = null;
  Task_t *task = null;
  Time_t leastRuntime = TIME_T_MAX;

  flags.critBlocking = true;

  if (flags.runtimeOverflow)
    RuntimeReset();

  TaskListRewindPriv();
  do {
    task = TaskListGetPriv();
    if (task) {
      if (task->state == TaskStateWaiting && task->notifyBytes > 0) {
        TaskRun(task);
      } else if (task->state == TaskStateWaiting && task->timerInterval > 0 && CURRENTTIME() - task->timerStartTime > task->timerInterval) {
        TaskRun(task);
        task->timerStartTime = CURRENTTIME();
      } else if (task->state == TaskStateRunning && task->totalRuntime < leastRuntime) {
        leastRuntime = task->totalRuntime;
        runTask = task;
      }
    }
  } while (TaskListMoveNextPriv());

  if (runTask)
    TaskRun(runTask);

  flags.critBlocking = false;
}

HeliOSGetInfoResult_t *xHeliOSGetInfo() {
  int16_t tasks = 0;
  Task_t *task = null;
  HeliOSGetInfoResult_t *heliOSGetInfoResult = null;

  TaskListRewind();
  do {
    task = TaskListGet();
    if (task)
      tasks++;
  } while (TaskListMoveNext());
  heliOSGetInfoResult = (HeliOSGetInfoResult_t *)xMemAlloc(sizeof(HeliOSGetInfoResult_t));
  if (heliOSGetInfoResult) {
    heliOSGetInfoResult->tasks = tasks;
    strncpy_(heliOSGetInfoResult->productName, PRODUCT_NAME, PRODUCTNAME_SIZE);
    heliOSGetInfoResult->majorVersion = MAJOR_VERSION_NO;
    heliOSGetInfoResult->minorVersion = MINOR_VERSION_NO;
    heliOSGetInfoResult->patchVersion = PATCH_VERSION_NO;
  }
  return heliOSGetInfoResult;
}

bool IsCritBlocking() {
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
  LARGE_INTEGER pf;
  LARGE_INTEGER pc;
  QueryPerformanceFrequency(&pf);
  QueryPerformanceCounter(&pc);
  return pc.QuadPart;
#elif defined(OTHER_ARCH_LINUX)
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  return t.tv_sec * 1000000 + t.tv_nsec / 1000;
#else
  /*
     * Since CurrentTime() is not defined for CURRENTTIME() on
     * the Arduino architectures, just return zero.
     */
  return 0;
#endif
}

inline void TaskRun(Task_t *task_) {
  Time_t taskStartTime = 0;
  Time_t prevTotalRuntime = 0;

  prevTotalRuntime = task_->totalRuntime;
  taskStartTime = CURRENTTIME();
#if defined(ENABLE_TASK_PARAMETER)
  (*task_->callback)(task_->id, task_->taskParameter);
#else
  (*task_->callback)(task_->id);
#endif
  task_->lastRuntime = CURRENTTIME() - taskStartTime;
  task_->totalRuntime += task_->lastRuntime;
  if (task_->totalRuntime < prevTotalRuntime)
    flags.runtimeOverflow = true;
}

inline void RuntimeReset() {
  Task_t *task = null;

  TaskListRewindPriv();
  do {
    task = TaskListGetPriv();
    if (task)
      task->totalRuntime = task->lastRuntime;
  } while (TaskListMoveNextPriv());
  flags.runtimeOverflow = false;
}

void memcpy_(void *dest_, void *src_, size_t n_) {
  char *src = (char *)src_;
  char *dest = (char *)dest_;

  for (size_t i = 0; i < n_; i++)
    dest[i] = src[i];
}

void memset_(void *dest_, int16_t val_, size_t n_) {
  char *dest = (char *)dest_;

  for (size_t i = 0; i < n_; i++)
    dest[i] = (char)val_;
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

int16_t strncmp_(const char *str1_, const char *str2_, size_t n_) {
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