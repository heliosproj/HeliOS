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
#ifndef TYPES_H_
#define TYPES_H_

typedef enum {
  TaskStateSuspended,
  TaskStateRunning,
  TaskStateWaiting
} TaskState_t;

typedef void TaskParm_t;

typedef int16_t TaskId_t;

typedef struct TaskNotification_s {
  int16_t notificationBytes;
  char notificationValue[TNOTIFYVALUE_SIZE];
} TaskNotification_t;

typedef struct Task_s {
  int16_t id;
  char name[TASKNAME_SIZE];
  TaskState_t state;
  TaskParm_t *taskParameter;
  void (*callback)(struct Task_s *, TaskParm_t *);
  int16_t notificationBytes;
  char notificationValue[TNOTIFYVALUE_SIZE];
  Time_t lastRunTime;
  Time_t totalRunTime;
  Time_t timerInterval;
  Time_t timerStartTime;
  struct Task_s *next;
} Task_t;

typedef struct TaskRunTimeStats_s {
  Time_t lastRunTime;
  Time_t totalRunTime;
} TaskRunTimeStats_t;

typedef struct TaskInfo_s {
  int16_t id;
  char name[TASKNAME_SIZE];
  TaskState_t state;
  Time_t lastRunTime;
  Time_t totalRunTime;
} TaskInfo_t;

typedef struct TaskList_s {
  int16_t nextId;
  int16_t length;
  Task_t *head;
} TaskList_t;

typedef int16_t Flag_t;

typedef struct Flags_s {
  Flag_t schedulerRunning;
  Flag_t critBlocking;
  Flag_t runTimeOverflow;
} Flags_t;

typedef struct QueueMessage_s {
  int16_t messageBytes;
  char messageValue[TNOTIFYVALUE_SIZE];
  struct QueueMessage_s *next;
} QueueMessage_t;

typedef struct Queue_s {
  int16_t length;
  QueueMessage_t *head;
  QueueMessage_t *tail;
} Queue_t;

typedef struct MemAllocRecord_s {
  size_t size;
  void *ptr;
} MemAllocRecord_t;

#endif