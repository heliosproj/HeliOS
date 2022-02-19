/**
 * @file task.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for task management
 * @version 0.3.1
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
TaskList_t *taskList = NULL;

/* The xTaskCreate() system call will create a new task. The task will be created with its
state set to suspended. The xTaskCreate() and xTaskDelete() system calls cannot be called within
a task. They MUST be called outside of the scope of the HeliOS scheduler. */
Task_t *xTaskCreate(const char *name_, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *taskParameter_) {
  Task_t *ret = NULL;

  Task_t *taskCursor = NULL;

  SYSASSERT(SYSFLAG_CRITICAL() == false);

  SYSASSERT(ISNOTNULLPTR(name_));

  SYSASSERT(ISNOTNULLPTR(callback_));

  /* Check if not in a critical section from CRITICAL_ENTER() and make sure the name and callback parameters
  are not null. */
  if ((SYSFLAG_CRITICAL() == false) && (ISNOTNULLPTR(name_)) && (ISNOTNULLPTR(callback_))) {
    /* Check if the task list is null, if it is call xMemAlloc() to allocate
    the dynamic memory for it. */
    if (ISNULLPTR(taskList)) {
    
      ENTER_PRIVILEGED();

      taskList = (TaskList_t *)xMemAlloc(sizeof(TaskList_t));
    }

    SYSASSERT(ISNOTNULLPTR(taskList));

    /* Check if the task list is still null in which case xMemAlloc() was unable to allocate
    the required memory. Enable interrupts and return null. */
    if (ISNOTNULLPTR(taskList)) {

      ENTER_PRIVILEGED();

      ret = (Task_t *)xMemAlloc(sizeof(Task_t));

      SYSASSERT(ISNOTNULLPTR(ret));

      /* Check if the task is not null. If it is, then xMemAlloc() was unable to allocate the required
      memory. */
      if (ISNOTNULLPTR(ret)) {

        taskList->nextId++;

        ret->id = taskList->nextId;

        memcpy_(ret->name, name_, CONFIG_TASK_NAME_BYTES);

        ret->state = TaskStateSuspended;

        ret->callback = callback_;

        ret->taskParameter = taskParameter_;

        ret->next = NULL;

        taskCursor = taskList->head;

        /* Check if the task list head is not null, if it is set it to the newly created task. If it
        isn't null then append the newly created task to the task list. */
        if (ISNOTNULLPTR(taskList->head)) {
          /* If the task cursor is not null, continue to traverse the list to find the end. */
          while (ISNOTNULLPTR(taskCursor->next)) {
            taskCursor = taskCursor->next;
          }

          taskCursor->next = ret;
        } else {
          taskList->head = ret;
        }

        taskList->length++;
      }
    }
  }

  return ret;
}

/* The xTaskDelete() system call will delete a task. The xTaskCreate() and xTaskDelete() system calls
cannot be called within a task. They MUST be called outside of the scope of the HeliOS scheduler. */
void xTaskDelete(Task_t *task_) {
  Task_t *taskCursor = NULL;

  Task_t *taskPrevious = NULL;

  SYSASSERT(SYSFLAG_CRITICAL() == false);

  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if not in a critical section from CRITICAL_ENTER() and make sure the task list is not null
  and that the task parameter is also not null. */
  if ((SYSFLAG_CRITICAL() == false) && (ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    taskPrevious = NULL;

    /* Check if the task cursor is not null and if the task cursor equals the task
    to be deleted. */
    if ((ISNOTNULLPTR(taskCursor)) && (taskCursor == task_)) {
      taskList->head = taskCursor->next;

      ENTER_PRIVILEGED();

      xMemFree(taskCursor);

      taskList->length--;

    } else {
      /* While the task cursor is not null and the task cursor is not equal to the
      task to be deleted. */
      while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
        taskPrevious = taskCursor;

        taskCursor = taskCursor->next;
      }

      SYSASSERT(ISNOTNULLPTR(taskCursor));

      /* If the task cursor is is not null, then delete the task otherwise return. */
      if (ISNOTNULLPTR(taskCursor)) {
        taskPrevious->next = taskCursor->next;

        ENTER_PRIVILEGED();

        xMemFree(taskCursor);

        taskList->length--;
      }
    }
  }

  return;
}

/* The xTaskGetHandleByName() system call will return the task handle pointer to the
task specified by its ASCII name. The length of the task name is dependent on the
CONFIG_TASK_NAME_BYTES setting. The name is compared byte-for-byte so the name is
case sensitive. */
Task_t *xTaskGetHandleByName(const char *name_) {
  Task_t *ret = NULL;

  Task_t *taskCursor = NULL;

  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(name_));

  /* Check if the task list is not null and the name parameter is also not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(name_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null, scan the task list for the task name. */
    while (ISNOTNULLPTR(taskCursor)) {
      /* Compare the task name of the task pointed to by the task cursor against the
      name parameter. */
      if (memcmp_(taskCursor->name, name_, CONFIG_TASK_NAME_BYTES) == zero) {
        ret = taskCursor;

        break;
      }

      taskCursor = taskCursor->next;
    }
  }

  return ret;
}

/* The xTaskGetHandleById() system call will return a pointer to the task handle
specified by its identifier. */
Task_t *xTaskGetHandleById(Base_t id_) {
  Task_t *ret = NULL;

  Task_t *taskCursor = NULL;

  SYSASSERT(ISNOTNULLPTR(taskList));

  /* Check if the task list is not null and the identifier parameter is greater than
  zero. */
  if (ISNOTNULLPTR(taskList)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null, check the task pointed to by the task cursor
    and compare its identifier against the identifier parameter being searched for. */
    while (ISNOTNULLPTR(taskCursor)) {
      if (taskCursor->id == id_) {
        ret = taskCursor;

        break;
      }

      taskCursor = taskCursor->next;
    }
  }

  return ret;
}

/* The xTaskGetAllRunTimeStats() system call will return the runtime statistics for all
 of the tasks regardless of their state. The xTaskGetAllRunTimeStats() system call returns
 the xTaskRunTimeStats type. An xBase variable must be passed by reference to xTaskGetAllRunTimeStats()
 which will contain the number of tasks so the end-user can iterate through the tasks. */
TaskRunTimeStats_t *xTaskGetAllRunTimeStats(Base_t *tasks_) {
  Base_t i = zero;

  Base_t tasks = zero;

  Task_t *taskCursor = NULL;

  TaskRunTimeStats_t *ret = NULL;

  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(tasks_));

  /* Check if the task list is not null and the tasks parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(tasks_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null, continue to traverse the task list counting
    the number of tasks in the list. */
    while (ISNOTNULLPTR(taskCursor)) {
      tasks++;

      taskCursor = taskCursor->next;
    }

    SYSASSERT(taskList->length == tasks);

    /* Check if the number of tasks is greater than zero and the length of the task list equals
    the number of tasks just counted (this is done as an integrity check). */
    if ((tasks > zero) && (taskList->length == tasks)) {
      ret = (TaskRunTimeStats_t *)xMemAlloc(tasks * sizeof(TaskRunTimeStats_t));


      SYSASSERT(ISNOTNULLPTR(ret));

      /* Check if xMemAlloc() successfully allocated the memory. */
      if (ISNOTNULLPTR(ret)) {
        taskCursor = taskList->head;

        /* While the task cursor is not null, continue to traverse the task list adding the
        runtime statistics of each task to the runtime stats array to be returned. */
        while (ISNOTNULLPTR(taskCursor)) {
          ret[i].id = taskCursor->id;

          ret[i].lastRunTime = taskCursor->lastRunTime;

          ret[i].totalRunTime = taskCursor->totalRunTime;

          taskCursor = taskCursor->next;

          i++;
        }

        *tasks_ = tasks;

      } else {
        *tasks_ = zero;
      }
    }
  }

  return ret;
}

/* The xTaskGetTaskRunTimeStats() system call returns the task runtime statistics for
one task. The xTaskGetTaskRunTimeStats() system call returns the xTaskRunTimeStats type.
The memory must be freed by calling xMemFree() after it is no longer needed. */
TaskRunTimeStats_t *xTaskGetTaskRunTimeStats(Task_t *task_) {
  Task_t *taskCursor = NULL;

  TaskRunTimeStats_t *ret = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal to the
    task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* If the task cursor is null, the task could not be found so return null. */
    if (ISNOTNULLPTR(taskCursor)) {
      ret = (TaskRunTimeStats_t *)xMemAlloc(sizeof(TaskRunTimeStats_t));

      SYSASSERT(ISNOTNULLPTR(ret));

      /* Check if xMemAlloc() successfully allocated the memory. */
      if (ISNOTNULLPTR(ret)) {
        ret->id = taskCursor->id;

        ret->lastRunTime = taskCursor->lastRunTime;

        ret->totalRunTime = taskCursor->totalRunTime;
      }
    }
  }

  return ret;
}

/* The xTaskGetNumberOfTasks() system call returns the current number of tasks
regardless of their state. */
Base_t xTaskGetNumberOfTasks(void) {
  Base_t ret = zero;

  Base_t tasks = zero;

  Task_t *taskCursor = NULL;

  SYSASSERT(ISNOTNULLPTR(taskList));

  /* Check if the task list is not null. */
  if (ISNOTNULLPTR(taskList)) {
    taskCursor = taskList->head;

    /* While the task cursor is not null, continue to count the number of
    tasks. */
    while (ISNOTNULLPTR(taskCursor)) {
      tasks++;

      taskCursor = taskCursor->next;
    }

    SYSASSERT(taskList->length == tasks);

    /* Check if the length of the task list equals the number of tasks counted
    (this is an integrity check). */
    if (taskList->length == tasks) {
      ret = tasks;
    }
  }

  return ret;
}

/* The xTaskGetTaskInfo() system call returns the xTaskInfo structure containing
the details of the task including its identifier, name, state and runtime statistics. */
TaskInfo_t *xTaskGetTaskInfo(Task_t *task_) {
  Task_t *taskCursor = NULL;

  TaskInfo_t *ret = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is null, if so the task could not be found
    so return null. */
    if (ISNOTNULLPTR(taskCursor)) {
      ret = (TaskInfo_t *)xMemAlloc(sizeof(TaskInfo_t));

      SYSASSERT(ISNOTNULLPTR(ret));

      /* Check if the task info memory has been allocated by xMemAlloc(). if it
      has then populate the task info and return. */
      if (ISNOTNULLPTR(ret)) {
        ret->id = taskCursor->id;

        ret->state = taskCursor->state;

        memcpy_(ret->name, taskCursor->name, CONFIG_TASK_NAME_BYTES);

        ret->lastRunTime = taskCursor->lastRunTime;

        ret->totalRunTime = taskCursor->totalRunTime;
      }
    }
  }

  return ret;
}

/* The xTaskGetAllTaskInfo() system call returns the xTaskInfo structure containing
the details of ALL tasks including its identifier, name, state and runtime statistics. */
TaskInfo_t *xTaskGetAllTaskInfo(Base_t *tasks_) {
  Base_t i = zero;

  Base_t tasks = zero;

  Task_t *taskCursor = NULL;

  TaskInfo_t *ret = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(tasks_));

  /* Check if the task list is not null and the tasks parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(tasks_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null, continue to traverse the task list counting
    the number of tasks in the list. */
    while (ISNOTNULLPTR(taskCursor)) {
      tasks++;

      taskCursor = taskCursor->next;
    }

    SYSASSERT(taskList->length == tasks);

    /* Check if the number of tasks is greater than zero and the length of the task list equals
    the number of tasks just counted (this is done as an integrity check). */
    if ((tasks > zero) && (taskList->length == tasks)) {
      ret = (TaskInfo_t *)xMemAlloc(tasks * sizeof(TaskInfo_t));

      SYSASSERT(ISNOTNULLPTR(ret));

      /* Check if xMemAlloc() successfully allocated the memory. */
      if (ISNOTNULLPTR(ret)) {
        taskCursor = taskList->head;

        /* While the task cursor is not null, continue to traverse the task list adding the
        runtime statistics of each task to the runtime stats array to be returned. */
        while (ISNOTNULLPTR(taskCursor)) {
          ret[i].id = taskCursor->id;

          ret[i].state = taskCursor->state;

          memcpy_(ret[i].name, taskCursor->name, CONFIG_TASK_NAME_BYTES);

          ret[i].lastRunTime = taskCursor->lastRunTime;

          ret[i].totalRunTime = taskCursor->totalRunTime;

          taskCursor = taskCursor->next;

          i++;
        }

        *tasks_ = tasks;

      } else {
        *tasks_ = zero;
      }
    }
  }

  return ret;
}


/* The xTaskGetTaskState() system call will return the state of the task. */
TaskState_t xTaskGetTaskState(Task_t *task_) {
  TaskState_t ret = TaskStateError;

  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so return the tasks's state
    otherwise return the error task state. */
    if (ISNOTNULLPTR(taskCursor)) {
      ret = taskCursor->state;

    } else {
      ret = TaskStateError;
    }
  }

  return ret;
}

/* The xTaskGetName() system call returns the ASCII name of the task. The size of the
task is dependent on the setting CONFIG_TASK_NAME_BYTES. The task name is NOT a null
terminated char array. */
char *xTaskGetName(Task_t *task_) {
  char *ret = NULL;

  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so allocate memory for the
    task name. */
    if (ISNOTNULLPTR(taskCursor)) {
      ret = (char *)xMemAlloc(CONFIG_TASK_NAME_BYTES);

      SYSASSERT(ISNOTNULLPTR(ret));

      /* Check if the task info memory has been allocated by xMemAlloc(). */
      if (ISNOTNULLPTR(ret)) {
        memcpy_(ret, taskCursor->name, CONFIG_TASK_NAME_BYTES);
      }
    }
  }

  return ret;
}

/* The xTaskGetId() system call returns the task identifier for the task. */
Base_t xTaskGetId(Task_t *task_) {
  Base_t ret = zero;

  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so set the return value
    to the task identifier. */
    if (ISNOTNULLPTR(taskCursor)) {
      ret = taskCursor->id;
    }
  }

  return ret;
}

/* The xTaskNotifyStateClear() system call will clear a waiting task notification if one
exists without returning the notification. */
void xTaskNotifyStateClear(Task_t *task_) {
  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so check if there is a waiting
    notification. */
    if (ISNOTNULLPTR(taskCursor)) {
      /* If the task notification bytes are greater than zero, then there is a notification
      to clear. */
      if (taskCursor->notificationBytes > zero) {
        taskCursor->notificationBytes = zero;
        memset_(taskCursor->notificationValue, zero, CONFIG_NOTIFICATION_VALUE_BYTES);
      }
    }
  }

  return;
}

/* The xTaskNotificationIsWaiting() system call will return true or false depending
on whether there is a task notification waiting for the task. */
Base_t xTaskNotificationIsWaiting(Task_t *task_) {
  Base_t ret = zero;

  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so check if there is a waiting
    task notification. */
    if (ISNOTNULLPTR(taskCursor)) {
      /* Check if the notification bytes are greater than zero. If so, there is a notification
      waiting so return true. */
      if (taskCursor->notificationBytes > zero) {
        ret = true;
      }
    }
  }

  return ret;
}

/* The xTaskNotifyGive() system call will send a task notification to the specified task. The
task notification bytes is the number of bytes contained in the notification value. The number of
notification bytes must be between one and the CONFIG_NOTIFICATION_VALUE_BYTES setting. The notification
value must contain a pointer to a char array containing the notification value. If the task already
has a waiting task notification, xTaskNotifyGive() will NOT overwrite the waiting task notification. */
Base_t xTaskNotifyGive(Task_t *task_, Base_t notificationBytes_, const char *notificationValue_) {
  Base_t ret = false;

  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  SYSASSERT(notificationBytes_ > zero);

  SYSASSERT(notificationBytes_ < CONFIG_NOTIFICATION_VALUE_BYTES);

  SYSASSERT(ISNOTNULLPTR(notificationValue_));

  /* Check if the task list is not null and the task parameter is not null, the notification bytes are between
  one and CONFIG_NOTIFICATION_VALUE_BYTES and that the notification value char array pointer is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_)) && (notificationBytes_ > zero) && (notificationBytes_ < CONFIG_NOTIFICATION_VALUE_BYTES) && (ISNOTNULLPTR(notificationValue_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so add the task notification to the task. */
    if (ISNOTNULLPTR(taskCursor)) {
      /* If the notification bytes are zero then there is not a notification already waiting,
      so copy the notification value into the task and set the notification bytes. */
      if (taskCursor->notificationBytes == zero) {
        taskCursor->notificationBytes = notificationBytes_;

        memcpy_(taskCursor->notificationValue, notificationValue_, CONFIG_NOTIFICATION_VALUE_BYTES);

        ret = true;
      }
    }
  }

  return ret;
}

/* The xTaskNotifyTake() system call will return the waiting task notification if there
is one. The xTaskNotifyTake() system call will return an xTaskNotification structure containing
the notification bytes and its value. */
TaskNotification_t *xTaskNotifyTake(Task_t *task_) {
  Task_t *taskCursor = NULL;

  TaskNotification_t *ret = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so check if there is a waiting
    task notification. */
    if (ISNOTNULLPTR(taskCursor)) {
      /* Check if the notification bytes are greater than zero, if so there is a waiting task
      notification. */
      if (taskCursor->notificationBytes > zero) {
        ret = (TaskNotification_t *)xMemAlloc(sizeof(TaskNotification_t));

        SYSASSERT(ISNOTNULLPTR(ret));

        /* Check if xMemAlloc() successfully allocated the memory for the task notification
        structure. */
        if (ISNOTNULLPTR(ret)) {
          ret->notificationBytes = taskCursor->notificationBytes;

          memcpy_(ret->notificationValue, taskCursor->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES);

          taskCursor->notificationBytes = zero;

          memset_(taskCursor->notificationValue, zero, CONFIG_NOTIFICATION_VALUE_BYTES);
        }
      }
    }
  }

  return ret;
}

/* The xTaskResume() system call will resume a suspended task. Tasks are suspended on creation
so either xTaskResume() or xTaskWait() must be called to place the task in a state that the scheduler
will execute. */
void xTaskResume(Task_t *task_) {
  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so set the task state. */
    if (ISNOTNULLPTR(taskCursor)) {
      taskCursor->state = TaskStateRunning;
    }
  }

  return;
}

/* The xTaskSuspend() system call will suspend a task. A task that has been suspended
will not be executed by the scheduler until xTaskResume() or xTaskWait() is called. */
void xTaskSuspend(Task_t *task_) {
  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so set the task state. */
    if (ISNOTNULLPTR(taskCursor)) {
      taskCursor->state = TaskStateSuspended;
    }
  }

  return;
}

/* The xTaskWait() system call will place a task in the waiting state. A task must
be in the waiting state for event driven multitasking with either direct to task
notifications OR setting the period on the task timer with xTaskChangePeriod(). A task
in the waiting state will not be executed by the scheduler until an event has occurred. */
void xTaskWait(Task_t *task_) {
  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so set the task state. */
    if (ISNOTNULLPTR(taskCursor)) {
      taskCursor->state = TaskStateWaiting;
    }
  }

  return;
}

/* The xTaskChangePeriod() system call will change the period (microseconds) on the task timer
for the specified task. The timer period must be greater than zero. To have any effect, the task
must be in the waiting state set by calling xTaskWait() on the task. Once the timer period is set
and the task is in the waiting state, the task will be executed every N microseconds based on the period.
Changing the period to zero will prevent the task from being executed even if it is in the waiting state. */
void xTaskChangePeriod(Task_t *task_, Time_t timerPeriod_) {
  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so set the task timer period. */
    if (ISNOTNULLPTR(taskCursor)) {
      taskCursor->timerPeriod = timerPeriod_;
    }
  }

  return;
}

/* The xTaskGetPeriod() will return the period for the timer for the specified task. See
xTaskChangePeriod() for more information on how the task timer works. */
Time_t xTaskGetPeriod(Task_t *task_) {
  Time_t ret = zero;

  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so set the return value to the
    task timer period. */
    if (ISNOTNULLPTR(taskCursor)) {
      ret = taskCursor->timerPeriod;
    }
  }

  return ret;
}

/* The xTaskResetTimer() system call will reset the task timer. xTaskResetTimer() does not change
the timer period or the task state when called. See xTaskChangePeriod() for more details on task timers. */
void xTaskResetTimer(Task_t *task_) {
  Task_t *taskCursor = NULL;


  SYSASSERT(ISNOTNULLPTR(taskList));

  SYSASSERT(ISNOTNULLPTR(task_));

  /* Check if the task list is not null and the task parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {
    taskCursor = taskList->head;

    /* While the task cursor is not null and the task cursor is not equal
    to the task being searched for. */
    while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {
      taskCursor = taskCursor->next;
    }

    SYSASSERT(ISNOTNULLPTR(taskCursor));

    /* Check if the task cursor is not null, if so set the start time to
    the current time. */
    if (ISNOTNULLPTR(taskCursor)) {
      taskCursor->timerStartTime = CURRENTTIME();
    }
  }
  return;
}

/* The xTaskStartScheduler() system call passes control to the HeliOS scheduler. */
void xTaskStartScheduler(void) {
  Task_t *runTask = NULL;

  Task_t *taskCursor = NULL;

  /* Underflow unsigned least runtime to get maximum value */
  Time_t leastRunTime = -1;

  /* Disable interrupts and set the critical section flag before entering into the scheduler main
  loop. */

  SYSASSERT(SYSFLAG_CRITICAL() == false);

  SYSASSERT(ISNOTNULLPTR(taskList));

  if ((SYSFLAG_CRITICAL() == false) && (ISNOTNULLPTR(taskList))) {

    DISABLE_INTERRUPTS();

    ENTER_CRITICAL();

    /* Continue to loop while the scheduler running flag is true. */
    while (SYSFLAG_RUNNING()) {
      /* If the runtime overflow flag is true. Reset the runtimes on all of the tasks. */
      if (SYSFLAG_OVERFLOW()) {
        RunTimeReset();
      }


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

    /* Enable interrupts and UNset the critical section flag before returning from the scheduler. */
    EXIT_CRITICAL();

    ENABLE_INTERRUPTS();
  }

  return;
}

/* If the runtime overflow flag is set, then RunTimeReset() is called to reset all of the
total runtimes on tasks to their last runtime. */
void RunTimeReset(void) {
  Task_t *taskCursor = NULL;

  SYSASSERT(ISNOTNULLPTR(taskList));

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

/* The xTaskGetSchedulerState() system call will return the state of the scheduler. */
SchedulerState_t xTaskGetSchedulerState(void) {
  SchedulerState_t ret = SchedulerStateError;

  if(SYSFLAG_RUNNING() == true) {

    ret = SchedulerStateRunning;

  } else {

    ret = SchedulerStateSuspended;

  }

  return ret;
}