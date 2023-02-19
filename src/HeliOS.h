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
  Return_t xDeviceRegisterDevice(Return_t (*device_self_register_)());
  Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_);
  Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Word_t *data_);
  Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
  Return_t xDeviceSimpleRead(const HalfWord_t uid_, Word_t *data_);
  Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);
  Return_t xDeviceInitDevice(const HalfWord_t uid_);
  Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);
  Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);
  Return_t xMemFree(const volatile Addr_t *addr_);
  Return_t xMemGetUsed(Size_t *size_);
  Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_);
  Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_);
  Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_);
  Return_t xQueueCreate(Queue_t **queue_, const Base_t limit_);
  Return_t xQueueDelete(Queue_t *queue_);
  Return_t xQueueGetLength(const Queue_t *queue_, Base_t *res_);
  Return_t xQueueIsQueueEmpty(const Queue_t *queue_, Base_t *res_);
  Return_t xQueueIsQueueFull(const Queue_t *queue_, Base_t *res_);
  Return_t xQueueMessagesWaiting(const Queue_t *queue_, Base_t *res_);
  Return_t xQueueSend(Queue_t *queue_, const Base_t bytes_, const Byte_t *value_);
  Return_t xQueuePeek(const Queue_t *queue_, QueueMessage_t **message_);
  Return_t xQueueDropMessage(Queue_t *queue_);
  Return_t xQueueReceive(Queue_t *queue_, QueueMessage_t **message_);
  Return_t xQueueLockQueue(Queue_t *queue_);
  Return_t xQueueUnLockQueue(Queue_t *queue_);
  Return_t xStreamCreate(StreamBuffer_t **stream_);
  Return_t xStreamDelete(const StreamBuffer_t *stream_);
  Return_t xStreamSend(StreamBuffer_t *stream_, const Byte_t byte_);
  Return_t xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_, Byte_t **data_);
  Return_t xStreamBytesAvailable(const StreamBuffer_t *stream_, HalfWord_t *bytes_);
  Return_t xStreamReset(const StreamBuffer_t *stream_);
  Return_t xStreamIsEmpty(const StreamBuffer_t *stream_, Base_t *res_);
  Return_t xStreamIsFull(const StreamBuffer_t *stream_, Base_t *res_);
  Return_t xSystemAssert(const char *file_, const int line_);
  Return_t xSystemInit(void);
  void xSystemHalt(void);
  Return_t xSystemGetSystemInfo(SystemInfo_t **info_);
  Return_t xTaskCreate(Task_t **task_, const Byte_t *name_, void (*callback_)(Task_t *task_, TaskParm_t *parm_), TaskParm_t *taskParameter_);
  Return_t xTaskDelete(const Task_t *task_);
  Return_t xTaskGetHandleByName(Task_t **task_, const Byte_t *name_);
  Return_t xTaskGetHandleById(Task_t **task_, const Base_t id_);
  Return_t xTaskGetAllRunTimeStats(TaskRunTimeStats_t **stats_, Base_t *tasks_);
  Return_t xTaskGetTaskRunTimeStats(const Task_t *task_, TaskRunTimeStats_t **stats_);
  Return_t xTaskGetNumberOfTasks(Base_t *tasks_);
  Return_t xTaskGetTaskInfo(const Task_t *task_, TaskInfo_t **info_);
  Return_t xTaskGetAllTaskInfo(TaskInfo_t **info_, Base_t *tasks_);
  Return_t xTaskGetTaskState(const Task_t *task_, TaskState_t *state_);
  Return_t xTaskGetName(const Task_t *task_, Byte_t **name_);
  Return_t xTaskGetId(const Task_t *task_, Base_t *id_);
  Return_t xTaskNotifyStateClear(Task_t *task_);
  Return_t xTaskNotificationIsWaiting(const Task_t *task_, Base_t *res_);
  Return_t xTaskNotifyGive(Task_t *task_, const Base_t bytes_, const Byte_t *value_);
  Return_t xTaskNotifyTake(Task_t *task_, TaskNotification_t **notification_);
  Return_t xTaskResume(Task_t *task_);
  Return_t xTaskSuspend(Task_t *task_);
  Return_t xTaskWait(Task_t *task_);
  Return_t xTaskChangePeriod(Task_t *task_, const Ticks_t period_);
  Return_t xTaskChangeWDPeriod(Task_t *task_, const Ticks_t period_);
  Return_t xTaskGetPeriod(const Task_t *task_, Ticks_t *period_);
  Return_t xTaskResetTimer(Task_t *task_);
  Return_t xTaskStartScheduler(void);
  Return_t xTaskResumeAll(void);
  Return_t xTaskSuspendAll(void);
  Return_t xTaskGetSchedulerState(SchedulerState_t *state_);
  Return_t xTaskGetWDPeriod(const Task_t *task_, Ticks_t *period_);
  Return_t xTimerCreate(Timer_t **timer_, const Ticks_t period_);
  Return_t xTimerDelete(const Timer_t *timer_);
  Return_t xTimerChangePeriod(Timer_t *timer_, const Ticks_t period_);
  Return_t xTimerGetPeriod(const Timer_t *timer_, Ticks_t *period_);
  Return_t xTimerIsTimerActive(const Timer_t *timer_, Base_t *res_);
  Return_t xTimerHasTimerExpired(const Timer_t *timer_, Base_t *res_);
  Return_t xTimerReset(Timer_t *timer_);
  Return_t xTimerStart(Timer_t *timer_);
  Return_t xTimerStop(Timer_t *timer_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef HELIOS_H_ */