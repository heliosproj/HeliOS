/*UNCRUSTIFY-OFF*/
/**
 * @file task.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Task scheduler API header
 * @details
 * Defines task structures, scheduler states, and function prototypes for the cooperative multitasking scheduler and task management.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef TASK_H_

  #define TASK_H_

  #include "config.h"

  #include "defines.h"

  #include "types.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "console.h"

    #include "device.h"

    #include "fat.h"

    #include "fs.h"

  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

  #include "mem.h"

  #include "port.h"

  #include "posix.h"

  #include "queue.h"

  #include "streams.h"

  #include "sys.h"

  #include "timer.h"

  #ifdef __cplusplus

    extern "C" {

  #endif /* ifdef __cplusplus */
  /**
   * @brief Creates a new task
   * @details Allocates memory for a task control block and registers it with
   * the scheduler. The task starts in suspended state and must be resumed
   * explicitly.
   *
   * @param[out] task_          Pointer to store the created task handle
   * @param[in]  name_          Human-readable task name (null-terminated
   *                            string)
   * @param[in]  callback_      Function pointer to task callback
   * @param[in]  taskParameter_ Optional parameter passed to task callback (can
   *                            be NULL)
   *
   * @return                    ReturnOK if task was created successfully
   * @return                    ReturnError if allocation failed or invalid
   *                            parameters
   *
   * @note The task must be resumed with xTaskResume() before it will execute
   * @warning Caller is responsible for deleting the task with xTaskDelete()
   */
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

  #if defined(POSIX_ARCH_OTHER)
    void __TaskStateClear__(void);

  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus

    }

  #endif /* ifdef __cplusplus */

#endif /* ifndef TASK_H_ */