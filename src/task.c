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
  int16_t i = 0;
  int16_t tasks = 0;
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

int16_t xTaskGetNumberOfTasks() {
  return taskList->length;
}

TaskState_t xTaskGetTaskState(Task_t *task_) {
  Task_t *taskCursor = null;
  taskCursor = taskList->head;
  while (taskCursor && taskCursor != task_) {
    taskCursor = taskCursor->next;
  }
  if (!taskCursor)
    return 0;
  return taskCursor->state;
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
    taskInfo->id;
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
    return 0;
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
  name = (TaskInfo_t *)xMemAlloc(TASKNAME_SIZE);
  if (name) {
    memcpy_(name, taskCursor->name, TASKNAME_SIZE);
    return name;
  }
  return null;
}

char *xTaskList() {
  return null;
}

void xTaskNotifyGive(Task_t *task_, int16_t notificationBytes_, char *notificationValue_) {
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

void xTaskNotifyGive(Task_t *task_, int16_t notificationBytes_, char *notificationValue_) {
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
      memcpy_(taskCursor->notificationBytes, notificationBytes_, TNOTIFYVALUE_SIZE);
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
}


/*
TaskId_t xTaskGetId(const char *name_) {
  Task_t *task = null;

  TaskListRewind();
  do {
    task = TaskListGet();
    if (task)
      if (strncmp_(task->name, name_, TASKNAME_SIZE) == 0)
        return task->id;
  } while (TaskListMoveNext());
  return 0;
}

void xTaskNotify(TaskId_t id_, int16_t notifyBytes_, char *notifyValue_) {
  Task_t *task = null;

  if (notifyBytes_ > 0 && notifyBytes_ <= TNOTIFYVALUE_SIZE && notifyValue_) {
    if (TaskListSeek(id_)) {
      task = TaskListGet();
      if (task) {
        if (task->state != TaskStateInvalid) {
          task->notifyBytes = notifyBytes_;
          memcpy_(task->notifyValue, notifyValue_, TNOTIFYVALUE_SIZE);
        }
      }
    }
  }
}

void xTaskNotifyClear(TaskId_t id_) {
  Task_t *task = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      if (task->state != TaskStateInvalid) {
        task->notifyBytes = 0;
        memset_(task->notifyValue, 0, TNOTIFYVALUE_SIZE);
      }
    }
  }
}

TaskGetNotifResult_t *xTaskGetNotif(TaskId_t id_) {
  Task_t *task = null;
  TaskGetNotifResult_t *taskGetNotifResult = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      taskGetNotifResult = (TaskGetNotifResult_t *)xMemAlloc(sizeof(TaskGetNotifResult_t));
      if (taskGetNotifResult) {
        taskGetNotifResult->notifyBytes = task->notifyBytes;
        memcpy_(taskGetNotifResult->notifyValue, task->notifyValue, TNOTIFYVALUE_SIZE);
      }
    }
  }
  return taskGetNotifResult;
}

TaskGetInfoResult_t *xTaskGetInfo(TaskId_t id_) {
  Task_t *task = null;
  TaskGetInfoResult_t *taskGetInfoResult = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      taskGetInfoResult = (TaskGetInfoResult_t *)xMemAlloc(sizeof(TaskGetInfoResult_t));
      if (taskGetInfoResult) {
        taskGetInfoResult->id = task->id;
        memcpy_(taskGetInfoResult->name, task->name, TASKNAME_SIZE);
        taskGetInfoResult->state = task->state;
        taskGetInfoResult->notifyBytes = task->notifyBytes;
        memcpy_(taskGetInfoResult->notifyValue, task->notifyValue, TNOTIFYVALUE_SIZE);
        taskGetInfoResult->lastRuntime = task->lastRuntime;
        taskGetInfoResult->totalRuntime = task->totalRuntime;
        taskGetInfoResult->timerInterval = task->timerInterval;
        taskGetInfoResult->timerStartTime = task->timerStartTime;
      }
    }
  }
  return taskGetInfoResult;
}

bool TaskListSeek(TaskId_t id_) {
  Task_t *task = null;

  TaskListRewind();
  do {
    task = TaskListGet();
    if (task)
      if (task->id == id_)
        return true;
  } while (TaskListMoveNext());
  return false;
}

TaskGetListResult_t *xTaskGetList(int16_t *tasks_) {
  int16_t i = 0;
  int16_t tasks = 0;
  Task_t *task = null;
  TaskGetListResult_t *taskGetListResult = null;

  *tasks_ = 0;
  TaskListRewind();
  do {
    task = TaskListGet();
    if (task)
      tasks++;
  } while (TaskListMoveNext());
  if (tasks > 0) {
    taskGetListResult = (TaskGetListResult_t *)xMemAlloc(tasks * sizeof(TaskGetListResult_t));
    if (taskGetListResult) {
      TaskListRewind();
      do {
        task = TaskListGet();
        if (task) {
          taskGetListResult[i].id = task->id;
          memcpy_(taskGetListResult[i].name, task->name, TASKNAME_SIZE);
          taskGetListResult[i].state = task->state;
          taskGetListResult[i].lastRuntime = task->lastRuntime;
          taskGetListResult[i].totalRuntime = task->totalRuntime;
          i++;
        }
      } while (TaskListMoveNext());
      *tasks_ = tasks;
    }
  }
  return taskGetListResult;
}

void xTaskSetTimer(TaskId_t id_, Time_t timerInterval_) {
  Task_t *task = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      if (task->state != TaskStateInvalid) {
        task->timerInterval = timerInterval_;
        task->timerStartTime = CURRENTTIME();
      }
    }
  }
}

void xTaskSetTimerWOReset(TaskId_t id_, Time_t timerInterval_) {
  Task_t *task = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task)
      if (task->state != TaskStateInvalid)
        task->timerInterval = timerInterval_;
  }
}

void xTaskResetTimer(TaskId_t id_) {
  Task_t *task = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task)
      if (task->state != TaskStateInvalid)
        task->timerStartTime = CURRENTTIME();
  }
}

*/