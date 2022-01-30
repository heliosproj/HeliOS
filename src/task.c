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

#include "task.h"

TaskList_t *taskList = null;

Task_t *xTaskCreate(const char *name_, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *taskParameter_) {
  Task_t *task = null;
  Task_t *taskCursor = null;
  if (IsNotCritBlocking() && name_ && callback_) {
    if (!taskList) {
      taskList = (TaskList_t *)xMemAlloc(sizeof(TaskList_t));
      if (!taskList) {
        return null;
      }
    }
    task = (Task_t *)xMemAlloc(sizeof(Task_t));
    if (task) {
      DISABLE_INTERRUPTS();
      taskList->nextId++;
      task->id = taskList->nextId;
      strncpy_(task->name, name_, TASKNAME_SIZE);
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
  return null;
}

void xTaskDelete(Task_t *task_) {
  Task_t *taskCursor = null;
  Task_t *taskPrevious = null;
  if (IsNotCritBlocking() && task_) {
    DISABLE_INTERRUPTS();
    taskCursor = taskList->head;
    taskPrevious = null;
    if (taskCursor && taskCursor == task_) {
      taskList->head = taskCursor->next;
      xMemFree(taskCursor);
      taskList->length--;
      task_ = null;
    } else {
      while (taskCursor && taskCursor != task_) {
        taskPrevious = taskCursor;
        taskCursor = taskCursor->next;
      }
      if (!taskCursor)
        return;
      taskPrevious->next = taskCursor->next;
      xMemFree(taskCursor);
      taskList->length--;
      task_ = null;
      ENABLE_INTERRUPTS();
    }
  }
  return;
}

Task_t *xTaskGetHandleByName(const char *name_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor) {
    if (strncmp_(taskCursor->name, name_, TASKNAME_SIZE) == 0)
      return taskCursor;
    taskCursor = taskCursor->next;
  }
  return null;
}

Task_t *xTaskGetHandleById(TaskId_t id_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor) {
    if (taskCursor->id == id_)
      return taskCursor;
    taskCursor = taskCursor->next;
  }
  return null;
}

TaskRunTimeStats_t *xTaskGetAllRunTimeStats() {
  Base_t i = 0;
  Base_t tasks = 0;
  Task_t *taskCursor = null;
  TaskRunTimeStats_t *taskRunTimeStats = null;
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
      return taskRunTimeStats;
    }
    return null;
  }
  return null;
}

TaskRunTimeStats_t *xTaskGetTaskRunTimeStats(Task_t *task_) {
  Task_t *taskCursor = null;
  TaskRunTimeStats_t *taskRunTimeStats = null;
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
  return null;
}

Base_t xTaskGetNumberOfTasks() {
  return taskList->length;
}

TaskInfo_t *xTaskGetTaskInfo(Task_t *task_) {
  Task_t *taskCursor = null;
  TaskInfo_t *taskInfo = null;
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
    memcpy_(taskInfo->name, taskCursor->name, TASKNAME_SIZE);
    taskInfo->lastRunTime = taskCursor->lastRunTime;
    taskInfo->totalRunTime = taskCursor->totalRunTime;
    return taskInfo;
  }
  return null;
}

TaskState_t xTaskGetTaskState(Task_t *task_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return TaskStateNone;
  return taskCursor->state;
}

char *xTaskGetName(Task_t *task_) {
  char *name = null;
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return null;
  name = (char *)xMemAlloc(TASKNAME_SIZE);
  if (name) {
    memcpy_(name, taskCursor->name, TASKNAME_SIZE);
    return name;
  }
  return null;
}

TaskId_t xTaskGetId(Task_t *task_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return null;
  return taskCursor->id;
}

char *xTaskList() {
  return null;
}

void xTaskNotifyStateClear(Task_t *task_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return;
  if (taskCursor->notificationBytes > 0) {
    taskCursor->notificationBytes = 0;
    memset_(taskCursor->notificationValue, 0, TNOTIFYVALUE_SIZE);
  }
  return;
}

Base_t xTaskNotificationWaiting(Task_t *task_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return false;
  if (taskCursor->notificationBytes > 0) {
    return true;
  }
  return false;
}

void xTaskNotifyGive(Task_t *task_, Base_t notificationBytes_, char *notificationValue_) {
  Task_t *taskCursor = null;
  if (notificationBytes_ > 0 && notificationBytes_ < TNOTIFYVALUE_SIZE && notificationValue_) {
    taskCursor = taskList->head;
    while (taskCursor && taskCursor != task_) {
      taskCursor = taskCursor->next;
    }
    if (!taskCursor)
      return;
    if (taskCursor->notificationBytes == 0) {
      taskCursor->notificationBytes = notificationBytes_;
      memcpy_(taskCursor->notificationValue, notificationValue_, TNOTIFYVALUE_SIZE);
    }
  }
  return;
}

TaskNotification_t *xTaskNotifyTake(Task_t *task_) {
  Task_t *taskCursor = null;
  TaskNotification_t *taskNotification = null;
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
      memcpy_(taskNotification->notificationValue, taskCursor->notificationValue, TNOTIFYVALUE_SIZE);
      return taskNotification;
    }
  }
  return null;
}

TaskList_t *TaskListGet() {
  return taskList;
}

void xTaskResume(Task_t *task_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return;
  taskCursor->state = TaskStateRunning;
  return;
}

void xTaskSuspend(Task_t *task_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return;
  taskCursor->state = TaskStateSuspended;
  return;
}

void xTaskWait(Task_t *task_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return;
  taskCursor->state = TaskStateWaiting;
  return;
}

void xTaskSetTimer(Task_t *task_, Time_t timerPeriod_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return;
  taskCursor->timerPeriod = timerPeriod_;
  taskCursor->timerStartTime = CURRENTTIME();
  return;
}

void xTaskSetTimerWOReset(Task_t *task_, Time_t timerPeriod_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return;
  taskCursor->timerPeriod = timerPeriod_;
  return;
}

void xTaskResetTimer(Task_t *task_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return;
  taskCursor->timerStartTime = CURRENTTIME();
  return;
}