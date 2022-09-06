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

/* START OF ENUM TYPES */

/**
 * @brief
 *
 */
typedef enum {
  TaskStateError,
  TaskStateSuspended,
  TaskStateRunning,
  TaskStateWaiting
} TaskState_t;

/**
 * @brief
 *
 */
typedef enum {
  SchedulerStateError,
  SchedulerStateSuspended,
  SchedulerStateRunning
} SchedulerState_t;

/* START OF BASIC TYPES */

/**
 * @brief
 *
 */
typedef VOID_TYPE TaskParm_t;

/**
 * @brief
 *
 */
typedef TaskParm_t *xTaskParm;

/**
 * @brief
 *
 */
typedef UINT8_TYPE Base_t;

/**
 * @brief
 *
 */
typedef Base_t xBase;

/**
 * @brief
 *
 */
typedef UINT8_TYPE Byte_t;

/**
 * @brief
 *
 */
typedef Byte_t xByte;

/**
 * @brief
 *
 */
typedef VOID_TYPE Addr_t;

/**
 * @brief
 *
 */
typedef Addr_t *xAddr;

/**
 * @brief
 *
 */
typedef SIZE_TYPE Size_t;

/**
 * @brief
 *
 */
typedef Size_t xSize;

/**
 * @brief
 *
 */
typedef UINT16_TYPE HalfWord_t;

/**
 * @brief
 *
 */
typedef HalfWord_t xHalfWord;

/**
 * @brief
 *
 */
typedef UINT32_TYPE Word_t;

/**
 * @brief
 *
 */
typedef Word_t xWord;

/**
 * @brief
 *
 */
typedef UINT32_TYPE Ticks_t;

/**
 * @brief
 *
 */
typedef Ticks_t xTicks;

/**
 * @brief
 *
 */
typedef UCHAR_TYPE Char_t;

/**
 * @brief
 *
 */
typedef Char_t xChar;

/**
 * @brief
 *
 */
typedef VOID_TYPE TaskParm_t;

/**
 * @brief
 *
 */
typedef TaskParm_t *xTaskParm;

/* START OF STUB TYPES */

/**
 * @brief
 *
 */
typedef VOID_TYPE Device_t;

/**
 * @brief
 *
 */
typedef Device_t *xDevice;

/**
 * @brief
 *
 */
typedef VOID_TYPE Task_t;

/**
 * @brief
 *
 */
typedef Task_t *xTask;

/**
 * @brief
 *
 */
typedef VOID_TYPE StreamBuffer_t;

/**
 * @brief
 *
 */
typedef StreamBuffer_t *xStreamBuffer;

/**
 * @brief
 *
 */
typedef VOID_TYPE Queue_t;

/**
 * @brief
 *
 */
typedef Queue_t *xQueue;

/**
 * @brief
 *
 */
typedef VOID_TYPE Timer_t;

/**
 * @brief
 *
 */
typedef Timer_t *xTimer;

/* START OF STRUCT TYPES */

/**
 * @brief
 *
 */
typedef struct TaskNotification_s {
  Base_t notificationBytes;
  Char_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES];
} TaskNotification_t;
typedef TaskNotification_t *xTaskNotification;

/**
 * @brief
 *
 */
typedef struct TaskRunTimeStats_s {
  Base_t id;
  Ticks_t lastRunTime;
  Ticks_t totalRunTime;
} TaskRunTimeStats_t;
typedef TaskRunTimeStats_t *xTaskRunTimeStats;

/**
 * @brief
 *
 */
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

/**
 * @brief
 *
 */
typedef struct TaskInfo_s {
  Base_t id;
  Char_t name[CONFIG_TASK_NAME_BYTES];
  TaskState_t state;
  Ticks_t lastRunTime;
  Ticks_t totalRunTime;
} TaskInfo_t;
typedef TaskInfo_t *xTaskInfo;

/**
 * @brief
 *
 */
typedef struct QueueMessage_s {
  Base_t messageBytes;
  Char_t messageValue[CONFIG_MESSAGE_VALUE_BYTES];
} QueueMessage_t;
typedef QueueMessage_t *xQueueMessage;

/**
 * @brief
 *
 */
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

/**
 * @brief
 *
 * @param device_self_register_
 * @return xBase
 */
xBase xDeviceRegisterDevice(xBase (*device_self_register_)());

/**
 * @brief
 *
 * @param uid_
 * @return xBase
 */
xBase xDeviceIsAvailable(const xHalfWord uid_);

/**
 * @brief
 *
 * @param uid_
 * @param data_
 * @return xBase
 */
xBase xDeviceSimpleWrite(const xHalfWord uid_, xWord *data_);

/**
 * @brief
 *
 * @param uid_
 * @param size_
 * @param data_
 * @return xBase
 */
xBase xDeviceWrite(const xHalfWord uid_, xSize *size_, xAddr data_);

/**
 * @brief
 *
 * @param uid_
 * @param data_
 * @return xBase
 */
xBase xDeviceSimpleRead(const xHalfWord uid_, xWord *data_);

/**
 * @brief
 *
 * @param uid_
 * @param size_
 * @param data_
 * @return xBase
 */
xBase xDeviceRead(const xHalfWord uid_, xSize *size_, xAddr data_);

/**
 * @brief
 *
 * @param uid_
 * @return xBase
 */
xBase xDeviceInitDevice(const xHalfWord uid_);

/**
 * @brief
 *
 * @param uid_
 * @param size_
 * @param config_
 * @return xBase
 */
xBase xDeviceConfigDevice(const xHalfWord uid_, xSize *size_, xAddr config_);

/**
 * @brief
 *
 * @param size_
 * @return xAddr
 */
xAddr xMemAlloc(const xSize size_);

/**
 * @brief
 *
 * @param addr_
 */
void xMemFree(const volatile xAddr addr_);

/**
 * @brief
 *
 * @return xSize
 */
xSize xMemGetUsed(void);

/**
 * @brief
 *
 * @param addr_
 * @return xSize
 */
xSize xMemGetSize(const volatile xAddr addr_);

/**
 * @brief
 *
 * @return xMemoryRegionStats
 */
xMemoryRegionStats xMemGetHeapStats(void);

/**
 * @brief
 *
 * @return xMemoryRegionStats
 */
xMemoryRegionStats xMemGetKernelStats(void);

/**
 * @brief
 *
 * @param limit_
 * @return xQueue
 */
xQueue xQueueCreate(const xBase limit_);

/**
 * @brief
 *
 * @param queue_
 */
void xQueueDelete(xQueue queue_);

/**
 * @brief
 *
 * @param queue_
 * @return xBase
 */
xBase xQueueGetLength(const xQueue queue_);

/**
 * @brief
 *
 * @param queue_
 * @return xBase
 */
xBase xQueueIsQueueEmpty(const xQueue queue_);

/**
 * @brief
 *
 * @param queue_
 * @return xBase
 */
xBase xQueueIsQueueFull(const xQueue queue_);

/**
 * @brief
 *
 * @param queue_
 * @return xBase
 */
xBase xQueueMessagesWaiting(const xQueue queue_);

/**
 * @brief
 *
 * @param queue_
 * @param messageBytes_
 * @param messageValue_
 * @return xBase
 */
xBase xQueueSend(xQueue queue_, const xBase messageBytes_, const xChar *messageValue_);

/**
 * @brief
 *
 * @param queue_
 * @return xQueueMessage
 */
xQueueMessage xQueuePeek(const xQueue queue_);

/**
 * @brief
 *
 * @param queue_
 */
void xQueueDropMessage(xQueue queue_);

/**
 * @brief
 *
 * @param queue_
 * @return xQueueMessage
 */
xQueueMessage xQueueReceive(xQueue queue_);

/**
 * @brief
 *
 * @param queue_
 */
void xQueueLockQueue(xQueue queue_);

/**
 * @brief
 *
 * @param queue_
 */
void xQueueUnLockQueue(xQueue queue_);

/**
 * @brief
 *
 * @return xStreamBuffer
 */
xStreamBuffer xStreamCreate(void);

/**
 * @brief
 *
 * @param stream_
 */
void xStreamDelete(const xStreamBuffer stream_);

/**
 * @brief
 *
 * @param stream_
 * @param byte_
 * @return xBase
 */
xBase xStreamSend(xStreamBuffer stream_, const xByte byte_);

/**
 * @brief
 *
 * @param stream_
 * @param bytes_
 * @return xByte*
 */
xByte *xStreamReceive(const xStreamBuffer stream_, xHalfWord *bytes_);

/**
 * @brief
 *
 * @param stream_
 * @return xHalfWord
 */
xHalfWord xStreamBytesAvailable(const xStreamBuffer stream_);

/**
 * @brief
 *
 * @param stream_
 */
void xStreamReset(const xStreamBuffer stream_);

/**
 * @brief
 *
 * @param stream_
 * @return xBase
 */
xBase xStreamIsEmpty(const xStreamBuffer stream_);

/**
 * @brief
 *
 * @param steam_
 * @return xBase
 */
xBase xStreamIsFull(const xStreamBuffer steam_);

/**
 * @brief
 *
 */
void xSystemInit(void);

/**
 * @brief
 *
 */
void xSystemHalt(void);

/**
 * @brief
 *
 * @return xSystemInfo
 */
xSystemInfo xSystemGetSystemInfo(void);

/**
 * @brief
 *
 * @param name_
 * @param callback_
 * @param taskParameter_
 * @return xTask
 */
Task_t *xTaskCreate(const xChar *name_, void (*callback_)(xTask task_, xTaskParm parm_), xTaskParm taskParameter_);

/**
 * @brief
 *
 * @param task_
 */
void xTaskDelete(const xTask task_);

/**
 * @brief
 *
 * @param name_
 * @return xTask
 */
xTask xTaskGetHandleByName(const xChar *name_);

/**
 * @brief
 *
 * @param id_
 * @return xTask
 */
xTask xTaskGetHandleById(const xBase id_);

/**
 * @brief
 *
 * @param tasks_
 * @return xTaskRunTimeStats
 */
xTaskRunTimeStats xTaskGetAllRunTimeStats(xBase *tasks_);

/**
 * @brief
 *
 * @param task_
 * @return xTaskRunTimeStats
 */
xTaskRunTimeStats xTaskGetTaskRunTimeStats(const xTask task_);

/**
 * @brief
 *
 * @return xBase
 */
xBase xTaskGetNumberOfTasks(void);

/**
 * @brief
 *
 * @param task_
 * @return xTaskInfo
 */
xTaskInfo xTaskGetTaskInfo(const xTask task_);

/**
 * @brief
 *
 * @param tasks_
 * @return xTaskInfo
 */
xTaskInfo xTaskGetAllTaskInfo(xBase *tasks_);

/**
 * @brief
 *
 * @param task_
 * @return TaskState_t
 */
TaskState_t xTaskGetTaskState(const xTask task_);

/**
 * @brief
 *
 * @param task_
 * @return xChar*
 */
xChar *xTaskGetName(const xTask task_);

/**
 * @brief
 *
 * @param task_
 * @return xBase
 */
xBase xTaskGetId(const xTask task_);

/**
 * @brief
 *
 * @param task_
 */
void xTaskNotifyStateClear(xTask task_);

/**
 * @brief
 *
 * @param task_
 * @return xBase
 */
xBase xTaskNotificationIsWaiting(const xTask task_);

/**
 * @brief
 *
 * @param task_
 * @param notificationBytes_
 * @param notificationValue_
 * @return xBase
 */
xBase xTaskNotifyGive(xTask task_, const xBase notificationBytes_, const xChar *notificationValue_);

/**
 * @brief
 *
 * @param task_
 * @return xTaskNotification
 */
xTaskNotification xTaskNotifyTake(xTask task_);

/**
 * @brief
 *
 * @param task_
 */
void xTaskResume(xTask task_);

/**
 * @brief
 *
 * @param task_
 */
void xTaskSuspend(xTask task_);

/**
 * @brief
 *
 * @param task_
 */
void xTaskWait(xTask task_);

/**
 * @brief
 *
 * @param task_
 * @param timerPeriod_
 */
void xTaskChangePeriod(xTask task_, const xTicks timerPeriod_);

/**
 * @brief
 *
 * @param task_
 * @return xTicks
 */
xTicks xTaskGetPeriod(const xTask task_);

/**
 * @brief
 *
 * @param task_
 */
void xTaskResetTimer(xTask task_);

/**
 * @brief
 *
 */
void xTaskStartScheduler(void);

/**
 * @brief
 *
 */
void xTaskResumeAll(void);

/**
 * @brief
 *
 */
void xTaskSuspendAll(void);

/**
 * @brief
 *
 * @return SchedulerState_t
 */
SchedulerState_t xTaskGetSchedulerState(void);

/**
 * @brief
 *
 * @param task_
 * @param wdTimerPeriod_
 */
void xTaskChangeWDPeriod(xTask task_, const xTicks wdTimerPeriod_);

/**
 * @brief
 *
 * @param task_
 * @return xTicks
 */
xTicks xTaskGetWDPeriod(const xTask task_);

/**
 * @brief
 *
 * @param timerPeriod_
 * @return xTimer
 */
xTimer xTimerCreate(const xTicks timerPeriod_);

/**
 * @brief
 *
 * @param timer_
 */
void xTimerDelete(const xTimer timer_);

/**
 * @brief
 *
 * @param timer_
 * @param timerPeriod_
 */
void xTimerChangePeriod(xTimer timer_, const xTicks timerPeriod_);

/**
 * @brief
 *
 * @param timer_
 * @return xTicks
 */
xTicks xTimerGetPeriod(const xTimer timer_);

/**
 * @brief
 *
 * @param timer_
 * @return xBase
 */
xBase xTimerIsTimerActive(const xTimer timer_);

/**
 * @brief
 *
 * @param timer_
 * @return xBase
 */
xBase xTimerHasTimerExpired(const xTimer timer_);

/**
 * @brief
 *
 * @param timer_
 */
void xTimerReset(xTimer timer_);

/**
 * @brief
 *
 * @param timer_
 */
void xTimerStart(xTimer timer_);

/**
 * @brief
 *
 * @param timer_
 */
void xTimerStop(xTimer timer_);

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