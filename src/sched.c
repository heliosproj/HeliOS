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

Flags_t flags = {
    .schedulerRunning = true,
    .critBlocking = false,
    .runTimeOverflow = false};

void xTaskStartScheduler() {
  Task_t *runTask = null;
  Task_t *taskCursor = null;
  TaskList_t *taskList = null;
  Time_t leastRunTime = TIME_T_MAX;

  DISABLE_INTERRUPTS();
  ENTER_CRITICAL()

  while (flags.schedulerRunning) {
    if (flags.runTimeOverflow)
      RunTimeReset();

    taskList = TaskListGet();
    if (taskList) {
      taskCursor = taskList->head;
      while (taskCursor) {
        if (taskCursor->state == TaskStateWaiting && taskCursor->notificationBytes > 0) {
          TaskRun(taskCursor);
        } else if (taskCursor->state == TaskStateWaiting && taskCursor->timerPeriod > 0 && CURRENTTIME() - taskCursor->timerStartTime > taskCursor->timerPeriod) {
          TaskRun(taskCursor);
          taskCursor->timerStartTime = CURRENTTIME();
        } else if (taskCursor->state == TaskStateRunning && taskCursor->totalRunTime < leastRunTime) {
          leastRunTime = taskCursor->totalRunTime;
          runTask = taskCursor;
        }
        taskCursor = taskCursor->next;
      }
      if (runTask) {
        TaskRun(runTask);
        runTask = null;
      }
      leastRunTime = TIME_T_MAX;
    }
  }

  EXIT_CRITICAL();
  ENABLE_INTERRUPTS();
}

Flag_t IsNotCritBlocking() {
  return !flags.critBlocking;
}

void RunTimeReset() {
  Task_t *taskCursor = null;
  TaskList_t *taskList = null;

  taskList = TaskListGet();
  if (taskList) {
    taskCursor = taskList->head;
    while (taskCursor) {
      taskCursor->totalRunTime = taskCursor->lastRunTime;
      taskCursor = taskCursor->next;
    }
    flags.runTimeOverflow = false;
  }
}

Time_t CurrentTime() {
#if defined(OTHER_ARCH_LINUX)
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  return t.tv_sec * 1000000 + t.tv_nsec / 1000;
#else
  return 0;
#endif
}

void TaskRun(Task_t *task_) {
  Time_t taskStartTime = 0;
  Time_t prevTotalRunTime = 0;

  prevTotalRunTime = task_->totalRunTime;
  taskStartTime = CURRENTTIME();
  ENABLE_INTERRUPTS();
  (*task_->callback)(task_, task_->taskParameter);
  DISABLE_INTERRUPTS();
  task_->lastRunTime = CURRENTTIME() - taskStartTime;
  task_->totalRunTime += task_->lastRunTime;
  if (task_->totalRunTime < prevTotalRunTime)
    flags.runTimeOverflow = true;
}

void xTaskResumeAll() {
  flags.schedulerRunning = true;
}

void xTaskSuspendAll() {
  flags.schedulerRunning = false;
}

SystemInfo_t *xSystemGetSystemInfo() {
  SystemInfo_t *systemInfo = null;
  systemInfo = (SystemInfo_t *)xMemAlloc(sizeof(SystemInfo_t));
  if (systemInfo) {
    memcpy_(systemInfo->productName, PRODUCT_NAME, PRODUCTNAME_SIZE);
    systemInfo->majorVersion = MAJOR_VERSION_NO;
    systemInfo->minorVersion = MINOR_VERSION_NO;
    systemInfo->patchVersion = PATCH_VERSION_NO;
    systemInfo->numberOfTasks = xTaskGetNumberOfTasks();
    return systemInfo;
  }
  return null;
}

void memcpy_(void *dest_, const void *src_, size_t n_) {
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

int16_t memcmp_(const void *s1_, const void *s2_, size_t n_) {
  char *s1 = (char *)s1_;
  char *s2 = (char *)s2_;

  for (size_t i = 0; i < n_; i++) {
    if (*s1 != *s2)
      return *s1 - *s2;
    s1++;
    s2++;
  }
  return 0;
}