/**
 * @file task.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for tasks and managing tasks in HeliOS
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

#include "task.h"

extern SysFlags_t sysFlags;

/* Declare and initialize the task list to null. */
TaskList_t *taskList = null;

/* The xTaskCreate() system call will create a new task. The task will be created with its
state set to suspended. The xTaskCreate() and xTaskDelete() system calls cannot be called within
a task. They MUST be called outside of the scope of the HeliOS scheduler. */
Task_t *xTaskCreate(const char *name_, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *taskParameter_) {

  Task_t *task = null;

  Task_t *taskCursor = null;

  /* Check if not in a critical section from CRITICAL_ENTER() and make sure the name and callback parameters
  are not null. */
  if (!SYSFLAG_CRITICAL() && ISNOTNULLPTR(name_) && ISNOTNULLPTR(callback_)) {
    /* Check if the task list is null, if it is call xMemAlloc() to allocate
    the dynamic memory for it. */
    if (ISNULLPTR(taskList)) {
      taskList = (TaskList_t *)xMemAlloc(sizeof(TaskList_t));

      /* Check if the task list is still null in which case xMemAlloc() was unable to allocate
      the required memory. Enable interrupts and return null. */
      if (ISNULLPTR(taskList)) {

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

      return task;
    }
  }


  return null;
}

/* The xTaskDelete() system call will delete a task. The xTaskCreate() and xTaskDelete() system calls
cannot be called within a task. They MUST be called outside of the scope of the HeliOS scheduler. */
void xTaskDelete(Task_t *task_) {

  Task_t *taskCursor = null;

  Task_t *taskPrevious = null;

  /* Check if not in a critical section from CRITICAL_ENTER() and make sure the task list is not null
  and that the task parameter is also not null. */
  if (!SYSFLAG_CRITICAL() && ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
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

        return;
      }

      taskPrevious->next = taskCursor->next;

      xMemFree(taskCursor);

      taskList->length--;
    }
  }

  return;
}

/* The xTaskGetHandleByName() system call will return the task handle pointer to the
task specified by its ASCII name. The length of the task name is dependent on the
CONFIG_TASK_NAME_BYTES setting. The name is compared byte-for-byte so the name is
case sensitive. */
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

/* The xTaskGetHandleById() system call will return a pointer to the task handle
specified by its identifier. */
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

/* The xTaskGetAllRunTimeStats() system call will return the runtime statistics for all
 of the tasks regardless of their state. The xTaskGetAllRunTimeStats() system call returns
 the xTaskRunTimeStats type. An xBase variable must be passed by reference to xTaskGetAllRunTimeStats()
 which will contain the number of tasks so the end-user can iterate through the tasks. */
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

/* The xTaskGetTaskRunTimeStats() system call returns the task runtime statistics for
one task. The xTaskGetTaskRunTimeStats() system call returns the xTaskRunTimeStats type.
The memory must be freed by calling xMemFree() after it is no longer needed. */
TaskRunTimeStats_t *xTaskGetTaskRunTimeStats(Task_t *task_) {
  Task_t *taskCursor = null;

  TaskRunTimeStats_t *taskRunTimeStats = null;

  /* Check if the task list and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal to the
    task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* If the task cursor is null, the task could not be found so return null. */
    if (ISNULLPTR(taskCursor)) {
      return null;
    }

    taskRunTimeStats = (TaskRunTimeStats_t *)xMemAlloc(sizeof(TaskRunTimeStats_t));

    /* Check if xMemAlloc() successfully allocated the memory. */
    if (ISNOTNULLPTR(taskRunTimeStats)) {
      taskRunTimeStats->lastRunTime = taskCursor->lastRunTime;

      taskRunTimeStats->totalRunTime = taskCursor->totalRunTime;

      return taskRunTimeStats;
    }
  }

  return null;
}

/* The xTaskGetNumberOfTasks() system call returns the current number of tasks
regardless of their state. */
Base_t xTaskGetNumberOfTasks() {
  Base_t tasks = 0;

  Task_t *taskCursor = null;

  /* Check if the task list is not null. */
  if (ISNOTNULLPTR(taskList)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null, continue to count the number of
    tasks. */
    while (ISNOTNULLPTR(taskCursor)) {
      tasks++;

      taskCursor = taskCursor->next;
    }

    /* Check if the length of the task list equals the number of tasks counted
    (this is an integrity check). */
    if (taskList->length == tasks) {
      return tasks;
    }
  }

  return 0;
}

/* The xTaskGetTaskInfo() system call returns the xTaskInfo structure containing
the details of the task including its identifier, name, state and runtime statistics. */
TaskInfo_t *xTaskGetTaskInfo(Task_t *task_) {
  Task_t *taskCursor = null;

  TaskInfo_t *taskInfo = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNULLPTR(taskCursor)) {
      return null;
    }

    taskInfo = (TaskInfo_t *)xMemAlloc(sizeof(TaskInfo_t));

    /* Check if the task info memory has been allocated by xMemAlloc(). */
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

/* The xTaskGetTaskState() system call will return the state of the task. */
TaskState_t xTaskGetTaskState(Task_t *task_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNULLPTR(taskCursor)) {
      return TaskStateError;
    }

    return taskCursor->state;
  }

  return TaskStateError;
}

/* The xTaskGetName() system call returns the ASCII name of the task. The size of the
task is dependent on the setting CONFIG_TASK_NAME_BYTES. The task name is NOT a null
terminated char array. */
char *xTaskGetName(Task_t *task_) {
  char *name = null;

  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNULLPTR(taskCursor)) {
      return null;
    }

    name = (char *)xMemAlloc(CONFIG_TASK_NAME_BYTES);

    /* Check if the task info memory has been allocated by xMemAlloc(). */
    if (ISNOTNULLPTR(name)) {
      memcpy_(name, taskCursor->name, CONFIG_TASK_NAME_BYTES);

      return name;
    }
  }

  return null;
}

/* The xTaskGetId() system call returns the task identifier for the task. */
TaskId_t xTaskGetId(Task_t *task_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
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

/* The xTaskNotifyStateClear() system call will clear a waiting task notification if one
exists without returning the notification. */
void xTaskNotifyStateClear(Task_t *task_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return. */
    if (ISNULLPTR(taskCursor)) {
      return;
    }

    /* If the task notification bytes are greater than zero, then there is a notficiation
    to clear. */
    if (taskCursor->notificationBytes > 0) {
      taskCursor->notificationBytes = 0;
      memset_(taskCursor->notificationValue, 0, CONFIG_NOTIFICATION_VALUE_BYTES);
    }
  }

  return;
}

/* The xTaskNotificationIsWaiting() system call will return true or false depending
on whether there is a task notification waiting for the task. */
Base_t xTaskNotificationIsWaiting(Task_t *task_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return false. */
    if (ISNULLPTR(taskCursor)) {
      return false;
    }

    /* Check if the notification bytes are greater than zero. If so, there is a notification
    waiting so return true. */
    if (taskCursor->notificationBytes > 0) {
      return true;
    }
  }

  return false;
}

/* The xTaskNotifyGive() system call will send a task notification to the specified task. The
task notification bytes is the number of bytes contained in the notification value. The number of
notification bytes must be between one and the CONFIG_NOTIFICATION_VALUE_BYTES setting. The notification
value must contain a pointer to a char array containing the notification value. If the task already
has a waiting task notification, xTaskNotifyGive() will NOT overwrite the waiting task notification. */
void xTaskNotifyGive(Task_t *task_, Base_t notificationBytes_, const char *notificationValue_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null, the notification bytes are between
  one and CONFIG_NOTIFICATION_VALUE_BYTES and that the notification value char array pointer is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_) && notificationBytes_ > 0 && notificationBytes_ < CONFIG_NOTIFICATION_VALUE_BYTES && ISNOTNULLPTR(notificationValue_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNULLPTR(taskCursor)) {
      return;
    }

    /* If the notification bytes are zero then there is not a notification already waiting,
    so copy the notification value into the task and set the notification bytes. */
    if (taskCursor->notificationBytes == 0) {
      taskCursor->notificationBytes = notificationBytes_;

      memcpy_(taskCursor->notificationValue, notificationValue_, CONFIG_NOTIFICATION_VALUE_BYTES);
    }
  }

  return;
}

/* The xTaskNotifyTake() system call will return the waiting task notification if there
is one. The xTaskNotifyTake() system call will return an xTaskNotification structure containing
the notification bytes and its value. */
TaskNotification_t *xTaskNotifyTake(Task_t *task_) {
  Task_t *taskCursor = null;

  TaskNotification_t *taskNotification = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNULLPTR(taskCursor)) {
      return null;
    }

    /* Check if the notification bytes are greater than zero, if so there is a waiting task
    notification. */
    if (taskCursor->notificationBytes > 0) {
      taskNotification = (TaskNotification_t *)xMemAlloc(sizeof(TaskNotification_t));

      /* Check if xMemAlloc() successfully allocated the memory for the task notification
      structure. */
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

/* Returns the task list. This function is predominantly used by the scheduler to access
the tasks linked list. */
TaskList_t *TaskListGet() {
  return taskList;
}

/* The xTaskResume() system call will resume a suspended task. Tasks are suspended on creation
so either xTaskResume() or xTaskWait() must be called to place the task in a state that the scheduler
will execute. */
void xTaskResume(Task_t *task_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return. */
    if (ISNULLPTR(taskCursor)) {
      return;
    }

    taskCursor->state = TaskStateRunning;
  }

  return;
}

/* The xTaskSuspend() system call will suspend a task. A task that has been suspended
will not be executed by the scheduler until xTaskResume() or xTaskWait() is called. */
void xTaskSuspend(Task_t *task_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return. */
    if (ISNULLPTR(taskCursor)) {
      return;
    }

    taskCursor->state = TaskStateSuspended;
  }

  return;
}

/* The xTaskWait() system call will place a task in the waiting state. A task must
be in the waiting state for event driven multitasking with either direct to task
notifications OR setting the period on the task timer with xTaskChangePeriod(). A task
in the waiting state will not be executed by the scheduler until an event has occurred. */
void xTaskWait(Task_t *task_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNULLPTR(taskCursor)) {
      return;
    }

    taskCursor->state = TaskStateWaiting;
  }

  return;
}

/* The xTaskChangePeriod() system call will change the period (microseconds) on the task timer
for the specified task. The timer period must be greater than zero. To have any effect, the task
must be in the waiting state set by calling xTaskWait() on the task. Once the timer period is set
and the task is in the waiting state, the task will be executed every N microseconds based on the period.
Changing the period to zero will prevent the task from being executed even if it is in the waiting state. */
void xTaskChangePeriod(Task_t *task_, Time_t timerPeriod_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_) && timerPeriod_ >= 0) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNULLPTR(taskCursor)) {
      return;
    }

    taskCursor->timerPeriod = timerPeriod_;
  }

  return;
}

/* The xTaskGetPeriod() will return the period for the timer for the specified task. See
xTaskChangePeriod() for more information on how the task timer works. */
Time_t xTaskGetPeriod(Task_t *task_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNULLPTR(taskCursor)) {
      return 0;
    }
    return taskCursor->timerPeriod;
  }

  return 0;
}

/* The xTaskResetTimer() system call will reset the task timer. xTaskResetTimer() does not change
the timer period or the task state when called. See xTaskChangePeriod() for more details on task timers. */
void xTaskResetTimer(Task_t *task_) {
  Task_t *taskCursor = null;

  /* Check if the task list is not null and the task parameter is not null. */
  if (ISNOTNULLPTR(taskList) && ISNOTNULLPTR(task_)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while (ISNOTNULLPTR(taskCursor) && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNULLPTR(taskCursor)) {
      return;
    }
    taskCursor->timerStartTime = CURRENTTIME();
  }
  return;
}