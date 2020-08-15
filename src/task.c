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
#include "timer.h"

volatile int taskNextId;

void TaskInit() {
  taskNextId = 1;
}

int xTaskAdd(const char* name_, void (*callback_)(int)) {
  if(!HeliOSIsCriticalBlocking()) {
    struct Task* task = (struct Task*)xMemAlloc(sizeof(struct Task));
    if (task) {
      task->id = taskNextId;
      taskNextId++;
      if (name_ && callback_) {
        strncpy_(task->name, name_, TASKNAMESIZE);
        task->state = TaskStateStopped;
      } else {
        strncpy_(task->name, "__NOTASK__", TASKNAMESIZE);
        task->state = TaskStateErrored;
      }
      task->callback = callback_;
      TaskListAdd(task);
      return task->id;
    }
  }
  return 0;
}

void xTaskRemove(int id_) {
  if(!HeliOSIsCriticalBlocking()) {
    if (TaskListSeek(id_)) {
      TaskListRemove();
    }
  }
}

void xTaskClear() {
  if(!HeliOSIsCriticalBlocking()) {
    TaskListClear();
  }
}

void xTaskStart(int id_) {
  struct Task* task = NULL;
  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      if (task->state != TaskStateErrored) {
        task->state = TaskStateRunning;
      }
    }
  }
}

void xTaskStop(int id_) {
  struct Task* task = NULL;
  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      if (task->state != TaskStateErrored) {
        task->state = TaskStateStopped;
      }
    }
  }
}

void xTaskWait(int id_) {
  struct Task* task = NULL;
  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      if (task->state != TaskStateErrored) {
        task->state = TaskStateWaiting;
      }
    }
  }
}

int xTaskGetId(const char* name_) {
  struct Task* task = NULL;
  TaskListRewind();
  do {
    task = TaskListGet();
    if (task) {
      if (strncmp_(task->name, name_, TASKNAMESIZE) == 0) {
        return task->id;
      }
    }
  } while (TaskListMoveNext());
  return 0;
}

void xTaskNotify(int id_, int notifyBytes_, char* notifyValue_) {
  struct Task* task = NULL;
  if(notifyBytes_ > 0 && notifyBytes_ <= NOTIFYVALUESIZE && notifyValue_) {
    if (TaskListSeek(id_)) {
      task = TaskListGet();
      if (task) {
        if (task->state != TaskStateErrored) {
          task->notifyBytes = notifyBytes_;
          memcpy_(task->notifyValue, notifyValue_, NOTIFYVALUESIZE);
        }
      }
    }
  }
}

void xTaskNotifyClear(int id_) {
  struct Task* task = NULL;
  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      if (task->state != TaskStateErrored) {
        task->notifyBytes = 0;
        memset_(task->notifyValue, 0, NOTIFYVALUESIZE);
      }
    }
  }
}

struct xTaskGetNotifResult* xTaskGetNotif(int id_) {
  struct Task* task = NULL;
  struct xTaskGetNotifResult* taskGetNotifResult = NULL;
  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      taskGetNotifResult = (struct xTaskGetNotifResult*)xMemAlloc(sizeof(struct xTaskGetNotifResult));
      if (taskGetNotifResult) {
        taskGetNotifResult->notifyBytes = task->notifyBytes;
        memcpy_(taskGetNotifResult->notifyValue, task->notifyValue, NOTIFYVALUESIZE);
      }
    }
  }
  return taskGetNotifResult;
}

struct xTaskGetInfoResult* xTaskGetInfo(int id_) {
  struct Task* task = NULL;
  struct xTaskGetInfoResult* taskGetInfoResult = NULL;
  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      taskGetInfoResult = (struct xTaskGetInfoResult*)xMemAlloc(sizeof(struct xTaskGetInfoResult));
      if (taskGetInfoResult) {
        taskGetInfoResult->id = task->id;
        memcpy_(taskGetInfoResult->name, task->name, TASKNAMESIZE);
        taskGetInfoResult->state = task->state;
        taskGetInfoResult->notifyBytes = task->notifyBytes;
        memcpy_(taskGetInfoResult->notifyValue, task->notifyValue, NOTIFYVALUESIZE);
        taskGetInfoResult->lastRuntime = task->lastRuntime;
        taskGetInfoResult->totalRuntime = task->totalRuntime;
        taskGetInfoResult->timerInterval = task->timerInterval;
        taskGetInfoResult->timerStartTime = task->timerStartTime;
      }
    }
  }
  return taskGetInfoResult;
}

int TaskListSeek(int id_) {
  struct Task* task = NULL;
  TaskListRewind();
  do {
    task = TaskListGet();
    if (task) {
      if (task->id == id_) {
        return TRUE;
      }
    }
  } while (TaskListMoveNext());
  return FALSE;
}

struct xTaskGetListResult* xTaskGetList(int* tasks_) {
  int i = 0;
  int tasks = 0;
  struct Task* task = NULL;
  struct xTaskGetListResult* taskGetListResult = NULL;
  *tasks_ = 0;
  TaskListRewind();
  do {
    task = TaskListGet();
    if (task) {
      tasks++;
    }
  } while (TaskListMoveNext());
  if (tasks > 0) {
    taskGetListResult = (struct xTaskGetListResult*)xMemAlloc(tasks * sizeof(struct xTaskGetListResult));
    if (taskGetListResult) {
      TaskListRewind();
      do {
        task = TaskListGet();
        if (task) {
          taskGetListResult[i].id = task->id;
          memcpy_(taskGetListResult[i].name, task->name, TASKNAMESIZE);
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

void xTaskSetTimer(int id_, unsigned long timerInterval_) {
  struct Task* task = NULL;
  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      if (task->state != TaskStateErrored) {
        task->timerInterval = timerInterval_;
        task->timerStartTime = NOW();
      }
    }
  }
}

void xTaskResetTimer(int id_) {
  struct Task* task = NULL;
  if (TaskListSeek(id_)) {
    task = TaskListGet();
    if (task) {
      if (task->state != TaskStateErrored) {
        task->timerStartTime = NOW();
      }
    }
  }
}
