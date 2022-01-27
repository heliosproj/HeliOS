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

TaskId_t nextTaskId = 1;
Task_t *taskHead = null;
Task_t *taskTail = null;

Task_t *xTaskCreate(const char *name_, void (*callback_)(TaskId_t, TaskParm_t), TaskParm_t taskParameter_) {
  Task_t *task = null;
  if (IsNotCritBlocking() && name_ && callback_) {
    task = (Task_t *)xMemAlloc(sizeof(Task_t));
    if (task) {
      task->id = taskNextId;
      strncpy_(task->name, name_, TASKNAME_SIZE);
      task->state = TaskStateStopped;
      task->callback = callback_;
      task->next = null;
      if (taskTail) {
        taskTail->next = task;
        taskTail = task;
      } else {
        taskHead = task;
        taskTail = task;
      }
      taskNextId++;
    }
  }
  return task;
}

Task_t *xTaskDestroy(Task_t *task_) {
  if (IsNotCritBlocking() && task_) {
    if(taskTail == task_ && taskHead == task_) {

    }
  }
}

  if (taskListCurr) {
    if (taskListCurr == taskListHead && taskListCurr == taskListTail) {
      TaskListItem_t *item = (TaskListItem_t *)taskListHead;
      TaskListInit();
      xMemFree(item->task);
      xMemFree(item);
    } else if (taskListCurr == taskListHead) {
      TaskListItem_t *item = (TaskListItem_t *)taskListHead;
      taskListHead = taskListHead->next;
      TaskListRewind();
      TaskListRewindPriv();
      xMemFree(item->task);
      xMemFree(item);
    } else if (taskListCurr == taskListTail) {
      TaskListItem_t *item = (TaskListItem_t *)taskListTail;
      taskListTail = taskListPrev;
      taskListPrev->next = null;
      TaskListRewind();
      TaskListRewindPriv();
      xMemFree(item->task);
      xMemFree(item);
    } else {
      TaskListItem_t *item = (TaskListItem_t *)taskListCurr;
      taskListPrev->next = taskListCurr->next;
      TaskListRewind();
      TaskListRewindPriv();
      xMemFree(item->task);
      xMemFree(item);
    }
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
