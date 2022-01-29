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

#include "timer.h"

TimerList_t *timerList = null;

Timer_t *xTaskCreate() {
  Timer_t *timer = null;
  Timer_t *taskCursor = null;
  if (IsNotCritBlocking() && name_ && callback_) {
    if (!taskList) {
      taskList = (TaskList_t *)xMemAlloc(sizeof(TaskList_t));
      if (!taskList) {
        return null;
      }
    }
    timer = (Task_t *)xMemAlloc(sizeof(Task_t));
    if (timer) {
      DISABLE_INTERRUPTS();
      taskList->nextId++;
      timer->id = taskList->nextId;
      strncpy_(timer->name, name_, TASKNAME_SIZE);
      timer->state = TaskStateSuspended;
      timer->callback = callback_;
      timer->taskParameter = taskParameter_;
      timer->next = null;
      taskCursor = taskList->head;
      if (taskList->head) {
        while (taskCursor->next) {
          taskCursor = taskCursor->next;
        }
        taskCursor->next = timer;
      } else {
        taskList->head = timer;
      }
      taskList->length++;
      ENABLE_INTERRUPTS();
      return timer;
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