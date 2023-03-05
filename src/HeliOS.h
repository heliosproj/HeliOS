/*UNCRUSTIFY-OFF*/
/**
 * @file HeliOS.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Header file for end-user application code
 * @version 0.4.0
 * @date 2022-09-06
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
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
/*UNCRUSTIFY-ON*/
#ifndef HELIOS_H_
  #define HELIOS_H_

  #include "posix.h"

  #include <stdint.h>

  #include "config.h"
  #include "defines.h"

  typedef enum TaskState_e {
    TaskStateSuspended,
    TaskStateRunning,
    TaskStateWaiting
  } TaskState_t;
  typedef TaskState_t xTaskState;
  typedef enum SchedulerState_e {
    SchedulerStateSuspended,
    SchedulerStateRunning
  } SchedulerState_t;
  typedef SchedulerState_t xSchedulerState;
  typedef enum Return_e {
    ReturnOK,
    ReturnError
  } Return_t;
  typedef Return_t xReturn;
  typedef enum TimerState_e {
    TimerStateSuspended,
    TimerStateRunning
  } TimerState_t;
  typedef TimerState_t xTimerState;
  typedef enum DeviceState_e {
    DeviceStateSuspended,
    DeviceStateRunning
  } DeviceState_t;
  typedef DeviceState_t xDeviceState;
  typedef enum DeviceMode_e {
    DeviceModeReadOnly,
    DeviceModeWriteOnly,
    DeviceModeReadWrite
  } DeviceMode_t;
  typedef DeviceMode_t xDeviceMode;
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
  typedef VOID_TYPE Task_t;
  typedef Task_t *xTask;
  typedef VOID_TYPE Timer_t;
  typedef Timer_t *xTimer;
  typedef VOID_TYPE Queue_t;
  typedef Queue_t *xQueue;
  typedef VOID_TYPE StreamBuffer_t;
  typedef StreamBuffer_t *xStreamBuffer;
  typedef struct TaskNotification_s {
    Base_t notificationBytes;
    Byte_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES];
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
    Byte_t name[CONFIG_TASK_NAME_BYTES];
    TaskState_t state;
    Ticks_t lastRunTime;
    Ticks_t totalRunTime;
  } TaskInfo_t;
  typedef TaskInfo_t xTaskInfo;
  typedef struct QueueMessage_s {
    Base_t messageBytes;
    Byte_t messageValue[CONFIG_MESSAGE_VALUE_BYTES];
  } QueueMessage_t;
  typedef QueueMessage_t *xQueueMessage;
  typedef struct SystemInfo_s {
    Byte_t productName[OS_PRODUCT_NAME_SIZE];
    Base_t majorVersion;
    Base_t minorVersion;
    Base_t patchVersion;
    Base_t numberOfTasks;
  } SystemInfo_t;
  typedef SystemInfo_t *xSystemInfo;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */


  /**
   * @brief Syscall to register a device driver with the kernel
   *
   * The xDeviceRegisterDevice() syscall is a component of the HeliOS device
   * driver model which registers a device driver with the HeliOS kernel. This
   * syscall must be made before a device driver can be called by xDeviceRead(),
   * xDeviceWrite(), etc. If the device driver is successfully registered with
   * the kernel, xDeviceRegisterDevice() will return ReturnOK. Once a device is
   * registered, it cannot be un-registered - it can only be placed in a
   * suspended state which is done by calling xDeviceConfigDevice(). However, as
   * with most aspects of the device driver model in HeliOS, it is important to
   * note that the implementation of and support for device state and mode is up
   * to the device driver's author.
   *
   * @note A device driver unique identifier ("uid") *MUST* be a globally unique
   * identifier. No two device drivers in the same application can share the
   * same uid. This is best achieved by ensuring the device driver author
   * selects a uid for his device driver that is not in use by other device
   * drivers. A device driver template and device drivers can be found in
   * /drivers.
   *
   * @param  device_self_register_ A pointer to the device driver's self
   *                               registration function; often,
   *                               DRIVERNAME_self_register().
   * @return                       xReturn On success, the syscall returns
   *                               ReturnOK. On failure, the syscall returns
   *                               ReturnError. A failure is any condition in
   *                               which the syscall was unable to achieve its
   *                               intended objective. For example, if
   *                               xTaskGetId() was unable to locate the task by
   *                               the task handle (i.e., xTask) passed to the
   *                               syscall, because either the handle was null
   *                               or invalid (e.g., points to a deleted task),
   *                               xTaskGetId() would return ReturnError. All
   *                               HeliOS syscalls return the xReturn (a.k.a.,
   *                               Return_t) type which can either be ReturnOK
   *                               or ReturnError. The C macro OK() can be used
   *                               as a more concise way of checking the return
   *                               value of a syscall (e.g.,
   *                               if(OK(xMemGetUsed(&size))) {} ).
   */
  xReturn xDeviceRegisterDevice(xReturn (*device_self_register_)());
  xReturn xDeviceIsAvailable(const xHalfWord uid_, xBase *res_);
  xReturn xDeviceSimpleWrite(const xHalfWord uid_, xWord *data_);
  xReturn xDeviceWrite(const xHalfWord uid_, xSize *size_, xAddr data_);
  xReturn xDeviceSimpleRead(const xHalfWord uid_, xWord *data_);
  xReturn xDeviceRead(const xHalfWord uid_, xSize *size_, xAddr *data_);
  xReturn xDeviceInitDevice(const xHalfWord uid_);
  xReturn xDeviceConfigDevice(const xHalfWord uid_, xSize *size_, xAddr config_);
  xReturn xMemAlloc(volatile xAddr *addr_, const xSize size_);
  xReturn xMemFree(const volatile xAddr addr_);
  xReturn xMemGetUsed(xSize *size_);
  xReturn xMemGetSize(const volatile xAddr addr_, xSize *size_);
  xReturn xMemGetHeapStats(xMemoryRegionStats *stats_);
  xReturn xMemGetKernelStats(xMemoryRegionStats *stats_);
  xReturn xQueueCreate(xQueue *queue_, const xBase limit_);
  xReturn xQueueDelete(xQueue queue_);
  xReturn xQueueGetLength(const xQueue queue_, xBase *res_);
  xReturn xQueueIsQueueEmpty(const xQueue queue_, xBase *res_);
  xReturn xQueueIsQueueFull(const xQueue queue_, xBase *res_);
  xReturn xQueueMessagesWaiting(const xQueue queue_, xBase *res_);
  xReturn xQueueSend(xQueue queue_, const xBase bytes_, const xByte *value_);
  xReturn xQueuePeek(const xQueue queue_, xQueueMessage *message_);
  xReturn xQueueDropMessage(xQueue queue_);
  xReturn xQueueReceive(xQueue queue_, xQueueMessage *message_);
  xReturn xQueueLockQueue(xQueue queue_);
  xReturn xQueueUnLockQueue(xQueue queue_);
  xReturn xStreamCreate(xStreamBuffer *stream_);
  xReturn xStreamDelete(const xStreamBuffer stream_);
  xReturn xStreamSend(xStreamBuffer stream_, const xByte byte_);
  xReturn xStreamReceive(const xStreamBuffer stream_, xHalfWord *bytes_, xByte **data_);
  xReturn xStreamBytesAvailable(const xStreamBuffer stream_, xHalfWord *bytes_);
  xReturn xStreamReset(const xStreamBuffer stream_);
  xReturn xStreamIsEmpty(const xStreamBuffer stream_, xBase *res_);
  xReturn xStreamIsFull(const xStreamBuffer stream_, xBase *res_);
  xReturn xSystemAssert(const char *file_, const int line_);
  xReturn xSystemInit(void);
  xReturn xSystemHalt(void);
  xReturn xSystemGetSystemInfo(xSystemInfo *info_);
  xReturn xTaskCreate(xTask *task_, const xByte *name_, void (*callback_)(xTask task_, xTaskParm parm_), xTaskParm taskParameter_);
  xReturn xTaskDelete(const xTask task_);
  xReturn xTaskGetHandleByName(xTask *task_, const xByte *name_);
  xReturn xTaskGetHandleById(xTask *task_, const xBase id_);
  xReturn xTaskGetAllRunTimeStats(xTaskRunTimeStats *stats_, xBase *tasks_);
  xReturn xTaskGetTaskRunTimeStats(const xTask task_, xTaskRunTimeStats *stats_);
  xReturn xTaskGetNumberOfTasks(xBase *tasks_);
  xReturn xTaskGetTaskInfo(const xTask task_, xTaskInfo *info_);
  xReturn xTaskGetAllTaskInfo(xTaskInfo *info_, xBase *tasks_);
  xReturn xTaskGetTaskState(const xTask task_, xTaskState *state_);
  xReturn xTaskGetName(const xTask task_, xByte **name_);
  xReturn xTaskGetId(const xTask task_, xBase *id_);
  xReturn xTaskNotifyStateClear(xTask task_);
  xReturn xTaskNotificationIsWaiting(const xTask task_, xBase *res_);
  xReturn xTaskNotifyGive(xTask task_, const xBase bytes_, const xByte *value_);
  xReturn xTaskNotifyTake(xTask task_, xTaskNotification *notification_);
  xReturn xTaskResume(xTask task_);
  xReturn xTaskSuspend(xTask task_);
  xReturn xTaskWait(xTask task_);
  xReturn xTaskChangePeriod(xTask task_, const xTicks period_);
  xReturn xTaskChangeWDPeriod(xTask task_, const xTicks period_);
  xReturn xTaskGetPeriod(const xTask task_, xTicks *period_);
  xReturn xTaskResetTimer(xTask task_);
  xReturn xTaskStartScheduler(void);
  xReturn xTaskResumeAll(void);
  xReturn xTaskSuspendAll(void);
  xReturn xTaskGetSchedulerState(xSchedulerState *state_);
  xReturn xTaskGetWDPeriod(const xTask task_, xTicks *period_);
  xReturn xTimerCreate(xTimer *timer_, const xTicks period_);
  xReturn xTimerDelete(const xTimer timer_);
  xReturn xTimerChangePeriod(xTimer timer_, const xTicks period_);
  xReturn xTimerGetPeriod(const xTimer timer_, xTicks *period_);
  xReturn xTimerIsTimerActive(const xTimer timer_, xBase *res_);
  xReturn xTimerHasTimerExpired(const xTimer timer_, xBase *res_);
  xReturn xTimerReset(xTimer timer_);
  xReturn xTimerStart(xTimer timer_);
  xReturn xTimerStop(xTimer timer_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef HELIOS_H_ */