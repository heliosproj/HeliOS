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
    .runTimeOverflow = false
};

void xHeliOSLoop() {
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
        if (taskCursor->state == TaskStateWaiting && taskCursor->notifyBytes > 0) {
          TaskRun(taskCursor);
        } else if (taskCursor->state == TaskStateWaiting && taskCursor->timerInterval > 0 && CURRENTTIME() - taskCursor->timerStartTime > taskCursor->timerInterval) {
          TaskRun(taskCursor);
          taskCursor->timerStartTime = CURRENTTIME();
        } else if (taskCursor->state == TaskStateRunning && taskCursor->totalRunTime < leastRunTime) {
          leastRunTime = taskCursor->totalRunTime;
          runTask = taskCursor;
        }
        taskCursor = taskCursor->next;
      }
      if (runTask)
        TaskRun(runTask);
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