/**
 * @file HeliOS.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Header file for end-user application code
 * @version 0.3.5
 * @date 2022-09-06
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
#ifndef HELIOS_H_
#define HELIOS_H_

#include "posix.h"

#include <stdint.h>

#include "config.h"
#include "defines.h"

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

/* Base types */
typedef VOID_TYPE TaskParm_t;
typedef TaskParm_t *xTaskParm;
typedef UINT8_TYPE Base_t;
typedef Base_t xBase;
typedef UINT8_TYPE Byte_t;
typedef Byte_t xByte;
typedef VOID_TYPE Addr_t;
typedef Addr_t *xAddr;
typedef SIZE_TYPE Size_t;
typedef Size_t xSize;
typedef UINT16_TYPE HalfWord_t;
typedef HalfWord_t xHalfWord;
typedef UINT32_TYPE Word_t;
typedef Word_t xWord;
typedef UINT32_TYPE Ticks_t;
typedef Ticks_t xTicks;
typedef UCHAR_TYPE Char_t;
typedef Char_t xChar;
typedef VOID_TYPE TaskParm_t;
typedef TaskParm_t *xTaskParm;

/* Stub types */
typedef VOID_TYPE Device_t;
typedef Device_t *xDevice;
typedef VOID_TYPE Task_t;
typedef Task_t *xTask;
typedef VOID_TYPE StreamBuffer_t;
typedef StreamBuffer_t *xStreamBuffer;
typedef VOID_TYPE Queue_t;
typedef Queue_t *xQueue;
typedef VOID_TYPE Timer_t;
typedef Timer_t *xTimer;

/* Struct types */
typedef struct TaskNotification_s {
  Base_t notificationBytes;
  Char_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES];
} TaskNotification_t;
typedef TaskNotification_t *xTaskNotification;

typedef struct TaskRunTimeStats_s {
  Base_t id;
  Ticks_t lastRunTime;
  Ticks_t totalRunTime;
} TaskRunTimeStats_t;
typedef TaskRunTimeStats_t *xTaskRunTimeStats;

typedef struct MemoryRegionStats_s {
  Word_t largestFreeEntryInBytes;
  Word_t smallestFreeEntryInBytes;
  Word_t numberOfFreeBlocks;
  Word_t availableSpaceInBytes;
  Word_t successfulAllocations;
  Word_t successfulFrees;
  Word_t minimumEverFreeBytesRemaining;
} MemoryRegionStats_t;
typedef MemoryRegionStats_t *xMemoryRegionStats;

typedef struct TaskInfo_s {
  Base_t id;
  Char_t name[CONFIG_TASK_NAME_BYTES];
  TaskState_t state;
  Ticks_t lastRunTime;
  Ticks_t totalRunTime;
} TaskInfo_t;
typedef TaskInfo_t *xTaskInfo;

typedef struct QueueMessage_s {
  Base_t messageBytes;
  Char_t messageValue[CONFIG_MESSAGE_VALUE_BYTES];
} QueueMessage_t;
typedef QueueMessage_t *xQueueMessage;

typedef struct SystemInfo_s {
  Char_t productName[OS_PRODUCT_NAME_SIZE];
  Base_t majorVersion;
  Base_t minorVersion;
  Base_t patchVersion;
  Base_t numberOfTasks;
} SystemInfo_t;
typedef SystemInfo_t *xSystemInfo;

#ifdef __cplusplus
extern "C" {
#endif

Base_t xDeviceRegisterDevice(Base_t (*device_self_register_)());
Base_t xDeviceIsAvailable(const HalfWord_t uid_);
Base_t xDeviceSimpleWrite(const HalfWord_t uid_, Word_t *data_);
Base_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
Base_t xDeviceSimpleRead(const HalfWord_t uid_, Word_t *data_);
Base_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
Base_t xDeviceInitDevice(const HalfWord_t uid_);
Base_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);
Addr_t *xMemAlloc(const Size_t size_);
void xMemFree(const volatile Addr_t *addr_);
Size_t xMemGetUsed(void);
Size_t xMemGetSize(const volatile Addr_t *addr_);
MemoryRegionStats_t *xMemGetHeapStats(void);
MemoryRegionStats_t *xMemGetKernelStats(void);
Queue_t *xQueueCreate(const Base_t limit_);
void xQueueDelete(Queue_t *queue_);
Base_t xQueueGetLength(const Queue_t *queue_);
Base_t xQueueIsQueueEmpty(const Queue_t *queue_);
Base_t xQueueIsQueueFull(const Queue_t *queue_);
Base_t xQueueMessagesWaiting(const Queue_t *queue_);
Base_t xQueueSend(Queue_t *queue_, const Base_t messageBytes_, const Char_t *messageValue_);
QueueMessage_t *xQueuePeek(const Queue_t *queue_);
void xQueueDropMessage(Queue_t *queue_);
QueueMessage_t *xQueueReceive(Queue_t *queue_);
void xQueueLockQueue(Queue_t *queue_);
void xQueueUnLockQueue(Queue_t *queue_);
StreamBuffer_t *xStreamCreate(void);
void xStreamDelete(const StreamBuffer_t *stream_);
Base_t xStreamSend(StreamBuffer_t *stream_, const Byte_t byte_);
Byte_t *xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_);
HalfWord_t xStreamBytesAvailable(const StreamBuffer_t *stream_);
void xStreamReset(const StreamBuffer_t *stream_);
Base_t xStreamIsEmpty(const StreamBuffer_t *stream_);
Base_t xStreamIsFull(const StreamBuffer_t *steam_);
void xSystemInit(void);
void xSystemHalt(void);
SystemInfo_t *xSystemGetSystemInfo(void);
Task_t *xTaskCreate(const Char_t *name_, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *taskParameter_);
void xTaskDelete(const Task_t *task_);
Task_t *xTaskGetHandleByName(const Char_t *name_);
Task_t *xTaskGetHandleById(const Base_t id_);
TaskRunTimeStats_t *xTaskGetAllRunTimeStats(Base_t *tasks_);
TaskRunTimeStats_t *xTaskGetTaskRunTimeStats(const Task_t *task_);
Base_t xTaskGetNumberOfTasks(void);
TaskInfo_t *xTaskGetTaskInfo(const Task_t *task_);
TaskInfo_t *xTaskGetAllTaskInfo(Base_t *tasks_);
TaskState_t xTaskGetTaskState(const Task_t *task_);
Char_t *xTaskGetName(const Task_t *task_);
Base_t xTaskGetId(const Task_t *task_);
void xTaskNotifyStateClear(Task_t *task_);
Base_t xTaskNotificationIsWaiting(const Task_t *task_);
Base_t xTaskNotifyGive(Task_t *task_, const Base_t notificationBytes_, const Char_t *notificationValue_);
TaskNotification_t *xTaskNotifyTake(Task_t *task_);
void xTaskResume(Task_t *task_);
void xTaskSuspend(Task_t *task_);
void xTaskWait(Task_t *task_);
void xTaskChangePeriod(Task_t *task_, const Ticks_t timerPeriod_);
Ticks_t xTaskGetPeriod(const Task_t *task_);
void xTaskResetTimer(Task_t *task_);
void xTaskStartScheduler(void);
void xTaskResumeAll(void);
void xTaskSuspendAll(void);
SchedulerState_t xTaskGetSchedulerState(void);
void xTaskChangeWDPeriod(Task_t *task_, const Ticks_t wdTimerPeriod_);
Ticks_t xTaskGetWDPeriod(const Task_t *task_);
Timer_t *xTimerCreate(const Ticks_t timerPeriod_);
void xTimerDelete(const Timer_t *timer_);
void xTimerChangePeriod(Timer_t *timer_, const Ticks_t timerPeriod_);
Ticks_t xTimerGetPeriod(const Timer_t *timer_);
Base_t xTimerIsTimerActive(const Timer_t *timer_);
Base_t xTimerHasTimerExpired(const Timer_t *timer_);
void xTimerReset(Timer_t *timer_);
void xTimerStart(Timer_t *timer_);
void xTimerStop(Timer_t *timer_);

#if defined(POSIX_ARCH_OTHER)
void __MemoryClear__(void);
void __SysStateClear__(void);
void __TimerStateClear__(void);
void __TaskStateClear__(void);
void __MemoryRegionDumpKernel__(void);
void __MemoryRegionDumpHeap__(void);
void __DeviceStateClear__(void);
#endif

#ifdef __cplusplus
}
#endif
#endif