/**
 * @file types.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Header file containing all of the private type definitions in HeliOS
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
#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

typedef enum {
  TaskStateError,
  TaskStateSuspended,
  TaskStateRunning,
  TaskStateWaiting
} TaskState_t;

typedef enum {
  TimerStateError,
  TimerStateStopped,
  TimerStateRunning
} TimerState_t;

typedef void TaskParm_t;
typedef uint8_t TaskId_t;
typedef uint8_t Flag_t;
typedef uint8_t Base_t;
typedef uint8_t Byte_t;
typedef uint16_t Word_t;

typedef struct HeapEntry_s
{
    Byte_t free : 1;
    Byte_t protected : 1;
    Byte_t reserved : 6;
    Word_t blocks;
    struct HeapEntry_s *next;
} HeapEntry_t;

typedef struct TaskNotification_s {
  Base_t notificationBytes;
  char notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES];
} TaskNotification_t;

typedef struct Task_s {
  TaskId_t id;
  char name[CONFIG_TASK_NAME_BYTES];
  TaskState_t state;
  TaskParm_t *taskParameter;
  void (*callback)(struct Task_s *, TaskParm_t *);
  Base_t notificationBytes;
  char notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES];
  Time_t lastRunTime;
  Time_t totalRunTime;
  Time_t timerPeriod;
  Time_t timerStartTime;
  struct Task_s *next;
} Task_t;

typedef struct TaskRunTimeStats_s {
  Time_t lastRunTime;
  Time_t totalRunTime;
} TaskRunTimeStats_t;

typedef struct TaskInfo_s {
  TaskId_t id;
  char name[CONFIG_TASK_NAME_BYTES];
  TaskState_t state;
  Time_t lastRunTime;
  Time_t totalRunTime;
} TaskInfo_t;

typedef struct TaskList_s {
  Base_t nextId;
  Base_t length;
  Task_t *head;
} TaskList_t;

typedef struct Timer_s {
  TimerState_t state;
  Time_t timerPeriod;
  Time_t timerStartTime;
  struct Timer_s *next;
} Timer_t;

typedef struct TimerList_s {
  Base_t length;
  Timer_t *head;
} TimerList_t;

typedef struct Flags_s {
  Flag_t schedulerRunning;
  Flag_t critBlocking;
  Flag_t runTimeOverflow;
} Flags_t;

typedef struct QueueMessage_s {
  Base_t messageBytes;
  char messageValue[CONFIG_MESSAGE_VALUE_BYTES];
} QueueMessage_t;

typedef struct Message_s {
  Base_t messageBytes;
  char messageValue[CONFIG_MESSAGE_VALUE_BYTES];
  struct Message_s *next;
} Message_t;

typedef struct Queue_s {
  Base_t length;
  Base_t limit;
  Message_t *head;
  Message_t*tail;
} Queue_t;

typedef struct DynamicMemoryAllocEntry_s {
  size_t size;
  void *ptr;
} DynamicMemoryAllocEntry_t;

typedef struct SystemInfo_s {
  char productName[PRODUCTNAME_SIZE];
  Base_t majorVersion;
  Base_t minorVersion;
  Base_t patchVersion;
  Base_t numberOfTasks;
} SystemInfo_t;
#endif