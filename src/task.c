/**
 * @file task.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for tasks and managing tasks in HeliOS
 * @version 0.3.0
 * @date 2022-01-31
 *
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

#include "task.h"

/* Declare and initialize the task list to null. */
TaskList_t *taskList = null;

/**
 * @brief The xTaskCreate() system call will create a new task. The task will be created with its
 * state set to suspended. The xTaskCreate() and xTaskDelete() system calls cannot be called within
 * a task. They MUST be called outside of the scope of the HeliOS scheduler.
 *
 * @param name_ The ASCII name of the task which can be used by xTaskGetHandleByName() to obtain the task pointer. The
 * length of the name is depended on the CONFIG_TASK_NAME_BYTES. The task name is NOT a null terminated char array.
 * @param callback_ The callback pointer to the task main function. This is the function that will be invoked
 * by the scheduler when a task is scheduled for execution.
 * @param taskParameter_ A pointer to any type or structure that the end-user wants to pass into the task as
 * a parameter. The task parameter is not required and may simply be set to null.
 * @return Task_t* A pointer to the newly created task.
 */
Task_t *xTaskCreate(const char *name_, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *taskParameter_) {
  /* Disable interrupts while manipulating the task list to prevent corruption. */
  DISABLE_INTERRUPTS();

  Task_t *task = null;

  Task_t *taskCursor = null;

  /* Check if HeliOS is blocking from CRITICAL_ENTER() and make sure the name and callback parameters
  are not null. */
  if (IsNotCritBlocking() && ISNOTNULLPTR(name_) && ISNOTNULLPTR(callback_)) {
    /* Check if the task list is null, if it is call xMemAlloc() to allocate
    the dynamic memory for it. */
    if (ISNULLPTR(taskList)) {
      taskList = (TaskList_t *)xMemAlloc(sizeof(TaskList_t));

      /* Check if the task list is still null in which case xMemAlloc() was unable to allocate
      the required memory. Enable interrupts and return null. */
      if (ISNULLPTR(taskList)) {
        ENABLE_INTERRUPTS();

        return null;
      }
    }

    task = (Task_t *)xMemAlloc(sizeof(Task_t));

    /* Check if the task is not null. If it is, then xMemAlloc() was unable to allocate the required
    memory. */
    if (ISNOTNULLPTR(task)) {
      taskList->nextId++;

      task->id = taskList->nextId;

      memcpy_(task->name, name_, CONFIG_TASK_NAME_BYTES);

      task->state = TaskStateSuspended;

      task->callback = callback_;

      task->taskParameter = taskParameter_;

      task->next = null;

      taskCursor = taskList->head;

      /* Check if the task list head is not null, if it is set it to the newly created task. If it
      isn't null then append the newly created task to the task list. */
      if (ISNOTNULLPTR(taskList->head)) {
        /* If the task cursor is not null, continue to traverse the list to find the end. */
        while (ISNOTNULLPTR(taskCursor->next)) {
          taskCursor = taskCursor->next;
        }

        taskCursor->next = task;
      } else {
        taskList->head = task;
      }

      taskList->length++;

      ENABLE_INTERRUPTS();

      return task;
    }
  }

  ENABLE_INTERRUPTS();

  return null;
}

/**
 * @brief The xTaskDelete() system call will delete a task. The xTaskCreate() and xTaskDelete() system calls
 * cannot be called within a task. They MUST be called outside of the scope of the HeliOS scheduler.
 *
 * @param task_ A pointer to the task to be deleted.
 */
void xTaskDelete(Task_t *task_) {
  /* Disable interrupts while manipulating the task list to prevent corruption. */
  DISABLE_INTERRUPTS();

  Task_t *taskCursor = null;

  Task_t *taskPrevious = null;

  /* Check if HeliOS is blocking from CRITICAL_ENTER() and make sure the task list is not null
  and that the task parameter is also not null. */
  if (IsNotCritBlocking() && ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    taskPrevious = null;

    /* Check if the task cursor is not null and if the task cursor equals the task
    to be deleted. */
    if (ISNOTNULLPTR(taskCursor) && taskCursor == task_) {
      taskList->head = taskCursor->next;

      xMemFree(taskCursor);

      taskList->length--;

    } else {
      /* While the task cursor is not null and the task cursor is not equal to the
      task to be deleted. */
      while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
        taskPrevious = taskCursor;

        taskCursor = taskCursor->next;
      }

      /* If the task cursor is null then return because the task was never found. */
      if (ISNULLPTR(taskCursor)) {
        ENABLE_INTERRUPTS();

        return;
      }

      taskPrevious->next = taskCursor->next;

      xMemFree(taskCursor);

      taskList->length--;
    }
  }

  ENABLE_INTERRUPTS();

  return;
}

/**
 * @brief The xTaskGetHandleByName() system call will return the task handle pointer to the
 * task specified by its ASCII name. The length of the task name is dependent on the
 * CONFIG_TASK_NAME_BYTES setting. The name is compared byte-for-byte so the name is
 * case sensitive.
 *
 * @param name_ The ASCII name of the task to return the handle pointer for.
 * @return Task_t* A pointer to the task handle. xTaskGetHandleByName() returns null if the
 * name cannot be found.
 */
Task_t *xTaskGetHandleByName(const char *name_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the name parameter is also not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(name_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null, scan the task list for the task name. */
    while (ISNOTNULLPTR(taskCursor)) {
      /* Compare the task name of the task pointed to by the task cursor against the
      name parameter. */
      if (memcmp_(taskCursor->name, name_, CONFIG_TASK_NAME_BYTES) == 0) {
        return taskCursor;
      }

      taskCursor = taskCursor->next;
    }
  }

  return null;
}

/**
 * @brief The xTaskGetHandleById() system call will return a pointer to the task handle
 * specified by its identifier.
 *
 * @param id_ The identifier of the task to return the handle pointer for.
 * @return Task_t* A  pointer to the task handle. xTaskGetHandleById() returns null if the
 * the task identifier cannot be found.
 */
Task_t *xTaskGetHandleById(TaskId_t id_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the identifier parameter is greater than
  zero. */
  if (ISNOTNULLPTR(taskList) && id_ > 0) {
    taskCursor = taskList->head;

    /* While the task cursor is not null, check the task pointed to by the task cursor
    and compare its identifier against the identifier parameter being searched for. */
    while (ISNOTNULLPTR(taskCursor)) {
      if (taskCursor->id == id_) {
        return taskCursor;
      }

      taskCursor = taskCursor->next;
    }
  }

  return null;
}

/**
 * @brief The xTaskGetAllRunTimeStats() system call will return the runtime statistics for all
 * of the tasks regardless of their state. The xTaskGetAllRunTimeStats() system call returns
 * the xTaskRunTimeStats type. An xBase variable must be passed by reference to xTaskGetAllRunTimeStats()
 * which will contain the number of tasks so the end-user can iterate through the tasks. The
 * xTaskRunTimeStats memory must be freed by xMemFree() after it is no longer needed.
 *
 * @param tasks_ An variable of type xBase passed by reference which will contain the number of tasks
 * upon return. If no tasks currently exist, this variable will not be modified.
 * @return TaskRunTimeStats_t* The runtime stats returned by xTaskGetAllRunTimeStats(). If there are
 * currently no tasks then this will be null. This memory must be freed by xMemFree().
 */
TaskRunTimeStats_t *xTaskGetAllRunTimeStats(Base_t *tasks_) {
  Base_t i = 0;

  Base_t tasks = 0;

  Task_t *taskCursor = null;

  TaskRunTimeStats_t *taskRunTimeStats = null;

  /* Check if the task list is not null and the tasks parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(tasks_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null, continue to traverse the task list counting
    the number of tasks in the list. */
    while (ISNOTNULLPTR(taskCursor)) {
      tasks++;

      taskCursor = taskCursor->next;
    }

    /* Check if the number of tasks is greater than zero and the length of the task list equals
    the number of tasks just counted (this is done as an integrity check). */
    if (tasks > 0 && taskList->length == tasks) {
      taskRunTimeStats = (TaskRunTimeStats_t *)xMemAlloc(tasks * sizeof(TaskRunTimeStats_t));

      /* Check if xMemAlloc() successfully allocated the memory. */
      if (ISNOTNULLPTR(taskRunTimeStats)) {
        taskCursor = taskList->head;

        /* While the task cursor is not null, continue to traverse the task list adding the
        runtime statistics of each task to the runtime stats array to be returned. */
        while (ISNOTNULLPTR(taskCursor)) {
          taskRunTimeStats[i].lastRunTime = taskCursor->lastRunTime;

          taskRunTimeStats[i].totalRunTime = taskCursor->totalRunTime;

          taskCursor = taskCursor->next;

          i++;
        }

        *tasks_ = tasks;

        return taskRunTimeStats;
      }
    }

    *tasks_ = 0;
  }

  return null;
}

TaskRunTimeStats_t *xTaskGetTaskRunTimeStats(Task_t *task_) {
  Task_t *taskCursor = null;
  TaskRunTimeStats_t *taskRunTimeStats = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return null;
    }
    taskRunTimeStats = (TaskRunTimeStats_t *)xMemAlloc(sizeof(TaskRunTimeStats_t));
    if (ISNOTNULLPTR(taskRunTimeStats)) {
      taskRunTimeStats->lastRunTime = taskCursor->lastRunTime;
      taskRunTimeStats->totalRunTime = taskCursor->totalRunTime;
      return taskRunTimeStats;
    }
  }
  return null;
}

Base_t xTaskGetNumberOfTasks() {
  Base_t tasks = 0;
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor)) {
      tasks++;
      taskCursor = taskCursor->next;
    }
    if (taskList->length == tasks) {
      return tasks;
    }
  }
  return 0;
}

TaskInfo_t *xTaskGetTaskInfo(Task_t *task_) {
  Task_t *taskCursor = null;
  TaskInfo_t *taskInfo = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return null;
    }
    taskInfo = (TaskInfo_t *)xMemAlloc(sizeof(TaskInfo_t));
    if (ISNOTNULLPTR(taskInfo)) {
      taskInfo->id = taskCursor->id;
      taskInfo->state = taskCursor->state;
      memcpy_(taskInfo->name, taskCursor->name, CONFIG_TASK_NAME_BYTES);
      taskInfo->lastRunTime = taskCursor->lastRunTime;
      taskInfo->totalRunTime = taskCursor->totalRunTime;
      return taskInfo;
    }
  }
  return null;
}

TaskState_t xTaskGetTaskState(Task_t *task_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return TaskStateNone;
    }
    return taskCursor->state;
  }
  return TaskStateNone;
}

char *xTaskGetName(Task_t *task_) {
  char *name = null;
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return null;
    }
    name = (char *)xMemAlloc(CONFIG_TASK_NAME_BYTES);
    if (ISNOTNULLPTR(name)) {
      memcpy_(name, taskCursor->name, CONFIG_TASK_NAME_BYTES);
      return name;
    }
  }
  return null;
}

TaskId_t xTaskGetId(Task_t *task_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return 0;
    }
    return taskCursor->id;
  }
  return 0;
}

/* TO-DO: Implement xTaskList(). */
char *xTaskList() {
  return null;
}

void xTaskNotifyStateClear(Task_t *task_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return;
    }
    if (taskCursor->notificationBytes > 0) {
      taskCursor->notificationBytes = 0;
      memset_(taskCursor->notificationValue, 0, CONFIG_NOTIFICATION_VALUE_BYTES);
    }
  }
  return;
}

Base_t xTaskNotificationIsWaiting(Task_t *task_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return false;
    }
    if (taskCursor->notificationBytes > 0) {
      return true;
    }
  }
  return false;
}

void xTaskNotifyGive(Task_t *task_, Base_t notificationBytes_, const char *notificationValue_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_) && notificationBytes_ > 0 && notificationBytes_ < CONFIG_NOTIFICATION_VALUE_BYTES && ISNOTNULLPTR(notificationValue_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return;
    }
    if (taskCursor->notificationBytes == 0) {
      taskCursor->notificationBytes = notificationBytes_;
      memcpy_(taskCursor->notificationValue, notificationValue_, CONFIG_NOTIFICATION_VALUE_BYTES);
    }
  }
  return;
}

TaskNotification_t *xTaskNotifyTake(Task_t *task_) {
  Task_t *taskCursor = null;
  TaskNotification_t *taskNotification = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return null;
    }
    if (taskCursor->notificationBytes > 0) {
      taskNotification = (TaskNotification_t *)xMemAlloc(sizeof(TaskNotification_t));
      if (ISNOTNULLPTR(taskNotification)) {
        taskNotification->notificationBytes = taskCursor->notificationBytes;
        memcpy_(taskNotification->notificationValue, taskCursor->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES);
        taskCursor->notificationBytes = 0;
        memset_(taskCursor->notificationValue, 0, CONFIG_NOTIFICATION_VALUE_BYTES);
        return taskNotification;
      }
    }
  }
  return null;
}

TaskList_t *TaskListGet() {
  return taskList;
}

void xTaskResume(Task_t *task_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return;
    }
    taskCursor->state = TaskStateRunning;
  }
  return;
}

void xTaskSuspend(Task_t *task_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return;
    }
    taskCursor->state = TaskStateSuspended;
  }
  return;
}

void xTaskWait(Task_t *task_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return;
    }
    taskCursor->state = TaskStateWaiting;
  }
  return;
}

void xTaskChangePeriod(Task_t *task_, Time_t timerPeriod_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_) && timerPeriod_ >= 0) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return;
    }
    taskCursor->timerPeriod = timerPeriod_;
  }
  return;
}

Time_t xTaskGetPeriod(Task_t *task_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return 0;
    }
    return taskCursor->timerPeriod;
  }
  return 0;
}

void xTaskResetTimer(Task_t *task_) {
  Task_t *taskCursor = null;
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (ISNULLPTR(taskCursor)) {
      return;
    }
    taskCursor->timerStartTime = CURRENTTIME();
  }
  return;
}