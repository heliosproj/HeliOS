/*
 * HeliOS Embedded Operating System
 * Copyright (C) 2020 Manny Peterson <me@mannypeterson.com>
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

#include "HeliOS.h"
#include "list.h"
#include "mem.h"
#include "task.h"

volatile TaskId_t taskNextId;

void TaskInit() {
  taskNextId = 1;
}

TaskId_t xTaskAdd(const char *name_, void (*callback_)(TaskId_t)) {
  if (!IsCritBlocking()) {
    Task_t *task = (Task_t *)xMemAlloc(sizeof(Task_t));
    if (task) {
      task->id = taskNextId;
      taskNextId++;
      if (name_ && callback_) {
        strncpy_(task->name, name_, TASKNAME_SIZE);
        task->state = TaskStateStopped;
      } else {
        strncpy_(task->name, "__NOTASK__", TASKNAME_SIZE);
        task->state = TaskStateInvalid;
      }
      task->callback = callback_;
      TaskListAdd(task);
      return task->id;
    }
  }
  return 0;
}

void xTaskRemove(TaskId_t id_) {
  if (!IsCritBlocking())
    if (TaskListSeek(id_))
      TaskListRemove();
}

void xTaskClear() {
  if (!IsCritBlocking())
    TaskListClear();
}

void xTaskStart(TaskId_t id_) {
  Task_t *task = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task)
      if (task->state != TaskStateInvalid)
        task->state = TaskStateRunning;
  }
}

void xTaskStop(TaskId_t id_) {
  Task_t *task = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task)
      if (task->state != TaskStateInvalid)
        task->state = TaskStateStopped;
  }
}

void xTaskWait(TaskId_t id_) {
  Task_t *task = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task)
      if (task->state != TaskStateInvalid)
        task->state = TaskStateWaiting;
  }
}

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

void xTaskResetTimer(TaskId_t id_) {
  Task_t *task = null;

  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task)
      if (task->state != TaskStateInvalid)
        task->timerStartTime = CURRENTTIME();
  }
}
