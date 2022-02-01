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

TaskList_t *taskList = null;

Task_t *xTaskCreate(const char *name_, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *taskParameter_) {
  DISABLE_INTERRUPTS();
  Task_t *task = null;
  Task_t *taskCursor = null;
  if (IsNotCritBlocking() && name_ && callback_) {
    if (!taskList) {
      taskList = (TaskList_t *)xMemAlloc(sizeof(TaskList_t));
      if (!taskList) {
        ENABLE_INTERRUPTS();
        return null;
      }
    }
    task = (Task_t *)xMemAlloc(sizeof(Task_t));
    if (task) {
      taskList->nextId++;
      task->id = taskList->nextId;
      memcpy_(task->name, name_, CONFIG_TASK_NAME_BYTES);
      task->state = TaskStateSuspended;
      task->callback = callback_;
      task->taskParameter = taskParameter_;
      task->next = null;
      taskCursor = taskList->head;
      if (taskList->head) {
        while (taskCursor->next) {
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

void xTaskDelete(Task_t *task_) {
  DISABLE_INTERRUPTS();
  Task_t *taskCursor = null;
  Task_t *taskPrevious = null;
  if (IsNotCritBlocking() && taskList && task_) {
    taskCursor = taskList->head;
    taskPrevious = null;
    if (taskCursor && taskCursor == task_) {
      taskList->head = taskCursor->next;
      xMemFree(taskCursor);
      taskList->length--;
    } else {
      while (taskCursor && taskCursor != task_) {
        taskPrevious = taskCursor;
        taskCursor = taskCursor->next;
      }
      if (!taskCursor) {
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

Task_t *xTaskGetHandleByName(const char *name_) {
  Task_t *taskCursor = null;
  if (taskList && name_) {
    taskCursor = taskList->head;
    while (taskCursor) {
      if (memcmp_(taskCursor->name, name_, CONFIG_TASK_NAME_BYTES) == 0)
        return taskCursor;
      taskCursor = taskCursor->next;
    }
  }
  return null;
}

Task_t *xTaskGetHandleById(TaskId_t id_) {
  Task_t *taskCursor = null;
  if (taskList && id_ > 0) {
    taskCursor = taskList->head;
    while (taskCursor) {
      if (taskCursor->id == id_)
        return taskCursor;
      taskCursor = taskCursor->next;
    }
  }
  return null;
}

TaskRunTimeStats_t *xTaskGetAllRunTimeStats(Base_t *tasks_) {
  Base_t i = 0;
  Base_t tasks = 0;
  Task_t *taskCursor = null;
  TaskRunTimeStats_t *taskRunTimeStats = null;
  if (taskList && tasks_) {
    taskCursor = taskList->head;
    while (taskCursor) {
      tasks++;
      taskCursor = taskCursor->next;
    }
    if (tasks > 0 && taskList->length == tasks) {
      taskRunTimeStats = (TaskRunTimeStats_t *)xMemAlloc(tasks * sizeof(TaskRunTimeStats_t));
      if (taskRunTimeStats) {
        taskCursor = taskList->head;
        while (taskCursor) {
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
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return null;
    taskRunTimeStats = (TaskRunTimeStats_t *)xMemAlloc(sizeof(TaskRunTimeStats_t));
    if (taskRunTimeStats) {
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
  if (taskList) {
    taskCursor = taskList->head;
    while (taskCursor) {
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
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return null;
    taskInfo = (TaskInfo_t *)xMemAlloc(sizeof(TaskInfo_t));
    if (taskInfo) {
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
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return TaskStateNone;
    return taskCursor->state;
  }
  return TaskStateNone;
}

char *xTaskGetName(Task_t *task_) {
  char *name = null;
  Task_t *taskCursor = null;
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return null;
    name = (char *)xMemAlloc(CONFIG_TASK_NAME_BYTES);
    if (name) {
      memcpy_(name, taskCursor->name, CONFIG_TASK_NAME_BYTES);
      return name;
    }
  }
  return null;
}

TaskId_t xTaskGetId(Task_t *task_) {
  Task_t *taskCursor = null;
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return 0;
    return taskCursor->id;
  }
  return 0;
}

char *xTaskList() {
  return null;
}

void xTaskNotifyStateClear(Task_t *task_) {
  Task_t *taskCursor = null;
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return;
    if (taskCursor->notificationBytes > 0) {
      taskCursor->notificationBytes = 0;
      memset_(taskCursor->notificationValue, 0, CONFIG_NOTIFICATION_VALUE_BYTES);
    }
  }
  return;
}

Base_t xTaskNotificationIsWaiting(Task_t *task_) {
  Task_t *taskCursor = null;
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return false;
    if (taskCursor->notificationBytes > 0) {
      return true;
    }
  }
  return false;
}

void xTaskNotifyGive(Task_t *task_, Base_t notificationBytes_, const char *notificationValue_) {
  Task_t *taskCursor = null;
  if (taskList && task_ && notificationBytes_ > 0 && notificationBytes_ < CONFIG_NOTIFICATION_VALUE_BYTES && notificationValue_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return;
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
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return null;
    if (taskCursor->notificationBytes > 0) {
      taskNotification = (TaskNotification_t *)xMemAlloc(sizeof(TaskNotification_t));
      if (taskNotification) {
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
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return;
    taskCursor->state = TaskStateRunning;
  }
  return;
}

void xTaskSuspend(Task_t *task_) {
  Task_t *taskCursor = null;
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return;
    taskCursor->state = TaskStateSuspended;
  }
  return;
}

void xTaskWait(Task_t *task_) {
  Task_t *taskCursor = null;
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return;
    taskCursor->state = TaskStateWaiting;
  }
  return;
}

void xTaskChangePeriod(Task_t *task_, Time_t timerPeriod_) {
  Task_t *taskCursor = null;
  if (taskList && task_ && timerPeriod_ >= 0) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return;
    taskCursor->timerPeriod = timerPeriod_;
  }
  return;
}

Time_t xTaskGetPeriod(Task_t *task_) {
  Task_t *taskCursor = null;
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return 0;
    return taskCursor->timerPeriod;
  }
  return 0;
}

void xTaskResetTimer(Task_t *task_) {
  Task_t *taskCursor = null;
  if (taskList && task_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return;
    taskCursor->timerStartTime = CURRENTTIME();
  }
  return;
}