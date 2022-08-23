/**
 * @file types.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel header for kernel type definitions
 * @version 0.3.5
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


#include "posix.h"

#include <stdint.h>




typedef enum {
  TaskStateError,
  TaskStateSuspended,
  TaskStateRunning,
  TaskStateWaiting
} TaskState_t;




typedef enum {
  SchedulerStateError,
  SchedulerStateSuspended,
  SchedulerStateRunning
} SchedulerState_t;




typedef enum {
  TimerStateError,
  TimerStateSuspended,
  TimerStateRunning
} TimerState_t;




/* WARNING: Modifying these type defintions may cause serious headaches. You've been warned! */
typedef void TaskParm_t;
typedef uint8_t Base_t;
typedef uint8_t Byte_t;
typedef void Addr_t;
typedef size_t Size_t;
typedef uint16_t HWord_t;
typedef uint32_t Word_t;
typedef uint32_t Ticks_t;


typedef struct MemoryEntry_s {
  Byte_t free : 1;
  Byte_t reserved : 7;
  HWord_t blocks;
  struct MemoryEntry_s *next;
} MemoryEntry_t;



typedef struct MemoryRegion_s {
  volatile Byte_t mem[MEMORY_REGION_SIZE_IN_BYTES];
  MemoryEntry_t *start;
  HWord_t entrySize;
  HWord_t allocations;
  HWord_t frees;
  Word_t minAvailableEver;
} MemoryRegion_t;




typedef struct TaskNotification_s {
  Base_t notificationBytes;
  char notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES];
} TaskNotification_t;




typedef struct Task_s {
  Base_t id;
  char name[CONFIG_TASK_NAME_BYTES];
  TaskState_t state;
  TaskParm_t *taskParameter;
  void (*callback)(struct Task_s *, TaskParm_t *);
  Base_t notificationBytes;
  char notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES];
  Ticks_t lastRunTime;
  Ticks_t totalRunTime;
  Ticks_t timerPeriod;
  Ticks_t timerStartTime;
  struct Task_s *next;
} Task_t;




typedef struct TaskRunTimeStats_s {
  Base_t id;
  Ticks_t lastRunTime;
  Ticks_t totalRunTime;
} TaskRunTimeStats_t;



typedef struct MemoryRegionStats_s {
  Word_t largestFreeEntryInBytes;
  Word_t smallestFreeEntryInBytes;
  Word_t numberOfFreeBlocks;
  Word_t availableSpaceInBytes;
  Word_t successfulAllocations;
  Word_t successfulFrees;
  Word_t minimumEverFreeBytesRemaining;
} MemoryRegionStats_t;



typedef struct TaskInfo_s {
  Base_t id;
  char name[CONFIG_TASK_NAME_BYTES];
  TaskState_t state;
  Ticks_t lastRunTime;
  Ticks_t totalRunTime;
} TaskInfo_t;




typedef struct TaskList_s {
  Base_t nextId;
  Base_t length;
  Task_t *head;
} TaskList_t;




typedef struct Timer_s {
  TimerState_t state;
  Ticks_t timerPeriod;
  Ticks_t timerStartTime;
  struct Timer_s *next;
} Timer_t;




typedef struct TimerList_s {
  Base_t length;
  Timer_t *head;
} TimerList_t;




typedef struct SysFlags_s {
  Byte_t running : 1;
  Byte_t overflow : 1;
  Byte_t corrupt : 1;
  Byte_t reserved : 5;
} SysFlags_t;




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
  Base_t locked;
  Message_t *head;
  Message_t *tail;
} Queue_t;




typedef struct SystemInfo_s {
  char productName[OS_PRODUCT_NAME_SIZE];
  Base_t majorVersion;
  Base_t minorVersion;
  Base_t patchVersion;
  Base_t numberOfTasks;
} SystemInfo_t;
#endif