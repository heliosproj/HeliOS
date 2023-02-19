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

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  xReturn xDeviceRegisterDevice(xReturn (*device_self_register_)());
  xReturn xDeviceIsAvailable(const xHalfWord uid_, xBase *res_);
  xReturn xDeviceSimpleWrite(const xHalfWord uid_, xWord *data_);
  xReturn xDeviceWrite(const xHalfWord uid_, xSize *size_, xAddr *data_);
  xReturn xDeviceSimpleRead(const xHalfWord uid_, xWord *data_);
  xReturn xDeviceRead(const xHalfWord uid_, xSize *size_, xAddr *data_);
  xReturn xDeviceInitDevice(const xHalfWord uid_);
  xReturn xDeviceConfigDevice(const xHalfWord uid_, xSize *size_, xAddr *config_);
  xReturn xMemAlloc(volatile xAddr **addr_, const xSize size_);
  xReturn xMemFree(const volatile xAddr *addr_);
  xReturn xMemGetUsed(xSize *size_);
  xReturn xMemGetSize(const volatile xAddr *addr_, xSize *size_);
  xReturn xMemGetHeapStats(xMemoryRegionStats **stats_);
  xReturn xMemGetKernelStats(xMemoryRegionStats **stats_);
  xReturn xQueueCreate(xQueue **queue_, const xBase limit_);
  xReturn xQueueDelete(xQueue *queue_);
  xReturn xQueueGetLength(const xQueue *queue_, xBase *res_);
  xReturn xQueueIsQueueEmpty(const xQueue *queue_, xBase *res_);
  xReturn xQueueIsQueueFull(const xQueue *queue_, xBase *res_);
  xReturn xQueueMessagesWaiting(const xQueue *queue_, xBase *res_);
  xReturn xQueueSend(xQueue *queue_, const xBase bytes_, const xByte *value_);
  xReturn xQueuePeek(const xQueue *queue_, xQueueMessage **message_);
  xReturn xQueueDropMessage(xQueue *queue_);
  xReturn xQueueReceive(xQueue *queue_, xQueueMessage **message_);
  xReturn xQueueLockQueue(xQueue *queue_);
  xReturn xQueueUnLockQueue(xQueue *queue_);
  xReturn xStreamCreate(xStreamBuffer **stream_);
  xReturn xStreamDelete(const xStreamBuffer *stream_);
  xReturn xStreamSend(xStreamBuffer *stream_, const xByte byte_);
  xReturn xStreamReceive(const xStreamBuffer *stream_, xHalfWord *bytes_, xByte **data_);
  xReturn xStreamBytesAvailable(const xStreamBuffer *stream_, xHalfWord *bytes_);
  xReturn xStreamReset(const xStreamBuffer *stream_);
  xReturn xStreamIsEmpty(const xStreamBuffer *stream_, xBase *res_);
  xReturn xStreamIsFull(const xStreamBuffer *stream_, xBase *res_);
  xReturn xSystemAssert(const char *file_, const int line_);
  xReturn xSystemInit(void);
  void xSystemHalt(void);
  xReturn xSystemGetSystemInfo(xSystemInfo **info_);
  xReturn xTaskCreate(xTask **task_, const xByte *name_, void (*callback_)(xTask *task_, xTaskParm *parm_), xTaskParm *taskParameter_);
  xReturn xTaskDelete(const xTask *task_);
  xReturn xTaskGetHandleByName(xTask **task_, const xByte *name_);
  xReturn xTaskGetHandleById(xTask **task_, const xBase id_);
  xReturn xTaskGetAllRunTimeStats(xTaskRunTimeStats **stats_, xBase *tasks_);
  xReturn xTaskGetTaskRunTimeStats(const xTask *task_, xTaskRunTimeStats **stats_);
  xReturn xTaskGetNumberOfTasks(xBase *tasks_);
  xReturn xTaskGetTaskInfo(const xTask *task_, xTaskInfo **info_);
  xReturn xTaskGetAllTaskInfo(xTaskInfo **info_, xBase *tasks_);
  xReturn xTaskGetTaskState(const xTask *task_, xTaskState *state_);
  xReturn xTaskGetName(const xTask *task_, xByte **name_);
  xReturn xTaskGetId(const xTask *task_, xBase *id_);
  xReturn xTaskNotifyStateClear(xTask *task_);
  xReturn xTaskNotificationIsWaiting(const xTask *task_, xBase *res_);
  xReturn xTaskNotifyGive(xTask *task_, const xBase bytes_, const xByte *value_);
  xReturn xTaskNotifyTake(xTask *task_, xTaskNotification **notification_);
  xReturn xTaskResume(xTask *task_);
  xReturn xTaskSuspend(xTask *task_);
  xReturn xTaskWait(xTask *task_);
  xReturn xTaskChangePeriod(xTask *task_, const xTicks period_);
  xReturn xTaskChangeWDPeriod(xTask *task_, const xTicks period_);
  xReturn xTaskGetPeriod(const xTask *task_, xTicks *period_);
  xReturn xTaskResetTimer(xTask *task_);
  xReturn xTaskStartScheduler(void);
  xReturn xTaskResumeAll(void);
  xReturn xTaskSuspendAll(void);
  xReturn xTaskGetSchedulerState(xSchedulerState *state_);
  xReturn xTaskGetWDPeriod(const xTask *task_, xTicks *period_);
  xReturn xTimerCreate(xTimer **timer_, const xTicks period_);
  xReturn xTimerDelete(const xTimer *timer_);
  xReturn xTimerChangePeriod(xTimer *timer_, const xTicks period_);
  xReturn xTimerGetPeriod(const xTimer *timer_, xTicks *period_);
  xReturn xTimerIsTimerActive(const xTimer *timer_, xBase *res_);
  xReturn xTimerHasTimerExpired(const xTimer *timer_, xBase *res_);
  xReturn xTimerReset(xTimer *timer_);
  xReturn xTimerStart(xTimer *timer_);
  xReturn xTimerStop(xTimer *timer_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef HELIOS_H_ */