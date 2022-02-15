/**
 * @file sched.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for the HeliOS scheduler
 * @version 0.3.0
 * @date 2022-01-31
 *
 * @copyright
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
 *
 */

#include "sched.h"

/* "You are not expected to understand this."
Thank you for the best OS on Earth, Dennis.
May you forever rest in peace. */

extern TaskList_t *taskList;

/* Declare and set the system flags to their default values. */
SysFlags_t sysFlags = {
    .running = true,
    .critical = false,
    .overflow = false,
    .protect = false};

/* The xTaskStartScheduler() system call passes control to the HeliOS scheduler. */
void xTaskStartScheduler(void) {
  Task_t *runTask = NULL;

  Task_t *taskCursor = NULL;

  /* Underflow unsigned least runtime to get maximum value */
  Time_t leastRunTime = -1;

  /* Disable interrupts and set the critical section flag before entering into the scheduler main
  loop. */
  DISABLE_INTERRUPTS();

  ENTER_CRITICAL();

  /* Continue to loop while the scheduler running flag is true. */
  while (SYSFLAG_RUNNING()) {
    /* If the runtime overflow flag is true. Reset the runtimes on all of the tasks. */
    if (SYSFLAG_OVERFLOW()) {
      RunTimeReset();
    }

    /* Check if the task list returned by TaskListGet() is not null before accessing it. */
    if (ISNOTNULLPTR(taskList)) {
      taskCursor = taskList->head;

      /* While the task cursor is not null (i.e., there are further tasks in the task list). */
      while (ISNOTNULLPTR(taskCursor)) {
        /* If the task pointed to by the task cursor is waiting and it has a notification waiting, then execute it. */
        if ((taskCursor->state == TaskStateWaiting) && (taskCursor->notificationBytes > zero)) {
          TaskRun(taskCursor);

          /* If the task pointed to by the task cursor is waiting and its timer has expired, then execute it. */
        } else if ((taskCursor->state == TaskStateWaiting) && (taskCursor->timerPeriod > zero) && ((CURRENTTIME() - taskCursor->timerStartTime) > taskCursor->timerPeriod)) {
          TaskRun(taskCursor);

          taskCursor->timerStartTime = CURRENTTIME();

          /* If the task pointed to by the task cursor is running and it's total runtime is less than the
          least runtime from previous tasks, then set the run task pointer to the task cursor. This logic
          is used to achieve the runtime balancing. */
        } else if ((taskCursor->state == TaskStateRunning) && (taskCursor->totalRunTime < leastRunTime)) {
          leastRunTime = taskCursor->totalRunTime;

          runTask = taskCursor;
        } else {
          /* Nothing to do here.. Just for MISRA C:2012 compliance. */
        }

        taskCursor = taskCursor->next;
      }

      /* If the run task pointer is not null, then there is a running tasks to execute. */
      if (ISNOTNULLPTR(runTask)) {
        TaskRun(runTask);

        runTask = NULL;
      }

      leastRunTime = -1;
    }
  }

  /* Enable interrupts and UNset the critical section flag before returning from the scheduler. */
  EXIT_CRITICAL();

  ENABLE_INTERRUPTS();
}

/* If the runtime overflow flag is set, then RunTimeReset() is called to reset all of the
total runtimes on tasks to their last runtime. */
void RunTimeReset(void) {
  Task_t *taskCursor = NULL;

  /* Check if task list is not null before accessing it. */
  if (ISNOTNULLPTR(taskList)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null (i.e., there are further tasks in the task list). */
    while (ISNOTNULLPTR(taskCursor)) {
      taskCursor->totalRunTime = taskCursor->lastRunTime;

      taskCursor = taskCursor->next;
    }

    SYSFLAG_OVERFLOW() = false;
  }
  return;
}

/* Used only for when testing HeliOS on Linux, then get the time from clock_gettime(). */
Time_t CurrentTime(void) {
#if defined(OTHER_ARCH_LINUX)
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  return (t.tv_sec * 1000000) + (t.tv_nsec / 1000);
#else
  return zero;
#endif
}

/* Called by the xTaskStartScheduler() system call, TaskRun() executes a task and updates all of its
runtime statistics. */
void TaskRun(Task_t *task_) {
  Time_t taskStartTime = zero;

  Time_t prevTotalRunTime = zero;

  /* Record the total runtime before executing the task. */
  prevTotalRunTime = task_->totalRunTime;

  /* Record the start time of the task. */
  taskStartTime = CURRENTTIME();

  /* Enable interrupts for the task. */
  ENABLE_INTERRUPTS();

  /* Call the task from its callback pointer. */
  (*task_->callback)(task_, task_->taskParameter);

  /* Disable interrupts now that the task has returned. */
  DISABLE_INTERRUPTS();

  /* Calculate the runtime and store it in last runtime. */
  task_->lastRunTime = CURRENTTIME() - taskStartTime;

  /* Add last runtime to the total runtime. */
  task_->totalRunTime += task_->lastRunTime;

  /* Check if the new total runtime is less than the previous total runtime,
  if so an overflow has occurred so set the runtime over flow system flag. */
  if (task_->totalRunTime < prevTotalRunTime) {
    SYSFLAG_OVERFLOW() = true;
  }

  return;
}

/* The xTaskResumeAll() system call will set the scheduler system flag so the next
call to xTaskStartScheduler() will resume execute of all tasks. */
void xTaskResumeAll(void) {
  SYSFLAG_RUNNING() = true;
  return;
}

/* The xTaskSuspendAll() system call will set the scheduler system flag so the scheduler
will stop and return. */
void xTaskSuspendAll(void) {
  SYSFLAG_RUNNING() = false;
  return;
}

/* The xSystemHalt() system call stops the system by putting HeliOS into an infinite loop. */
void xSystemHalt(void) {
  DISABLE_INTERRUPTS();
  for (;;) {
  }
}

/* TO-DO: Implement xTaskStopScheduler(). */
void xTaskStopScheduler(void) {
  return;
}

/* The xSystemGetSystemInfo() system call will return the type xSystemInfo containing
information about the system including the OS (product) name, its version and how many tasks
are currently in the running, suspended or waiting states. */
SystemInfo_t *xSystemGetSystemInfo(void) {
  SystemInfo_t *ret = NULL;

  ret = (SystemInfo_t *)xMemAlloc(sizeof(SystemInfo_t));

  /* Check if system info is not null to make sure xMemAlloc() successfully allocated
  the memory. */
  if (ISNOTNULLPTR(ret)) {
    memcpy_(ret->productName, PRODUCT_NAME, PRODUCTNAME_SIZE);

    ret->majorVersion = MAJOR_VERSION_NO;

    ret->minorVersion = MINOR_VERSION_NO;

    ret->patchVersion = PATCH_VERSION_NO;

    ret->numberOfTasks = xTaskGetNumberOfTasks();
  }

  return ret;
}