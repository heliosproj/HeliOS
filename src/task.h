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
   * @details Allocates memory for a task control block and registers it with the scheduler.
   *          The task starts in suspended state and must be resumed explicitly.
   *
   * @param[out] task_ Pointer to store the created task handle
   * @param[in] name_ Human-readable task name (null-terminated string)
   * @param[in] callback_ Function pointer to task callback
   * @param[in] taskParameter_ Optional parameter passed to task callback (can be NULL)
   *
   * @return ReturnOK if task was created successfully
   * @return ReturnError if allocation failed or invalid parameters
   *
   * @note The task must be resumed with xTaskResume() before it will execute
   * @warning Caller is responsible for deleting the task with xTaskDelete()
   */
  Return_t xTaskCreate(Task_t **task_, const Byte_t *name_, void (*callback_)(Task_t *task_, TaskParm_t *parm_), TaskParm_t *taskParameter_);

  /**
   * @brief Deletes a task
   * @details Removes the task from the scheduler and frees its allocated memory.
   *
   * @param[in] task_ Pointer to the task to delete
   *
   * @return ReturnOK if task was deleted successfully
   * @return ReturnError if task is invalid or deallocation failed
   *
   * @warning Using the task handle after deletion results in undefined behavior
   */
  Return_t xTaskDelete(const Task_t *task_);

  /**
   * @brief Gets task handle by name
   * @details Searches for a task by its name and returns the task handle.
   *
   * @param[out] task_ Pointer to store the found task handle
   * @param[in] name_ Task name to search for (null-terminated string)
   *
   * @return ReturnOK if task was found
   * @return ReturnError if task not found or invalid parameters
   */
  Return_t xTaskGetHandleByName(Task_t **task_, const Byte_t *name_);

  /**
   * @brief Gets task handle by ID
   * @details Searches for a task by its unique identifier and returns the task handle.
   *
   * @param[out] task_ Pointer to store the found task handle
   * @param[in] id_ Task identifier to search for
   *
   * @return ReturnOK if task was found
   * @return ReturnError if task not found or invalid parameters
   */
  Return_t xTaskGetHandleById(Task_t **task_, const Base_t id_);

  /**
   * @brief Gets runtime statistics for all tasks
   * @details Allocates and returns an array of runtime statistics for all registered tasks.
   *
   * @param[out] stats_ Pointer to store allocated statistics array
   * @param[out] tasks_ Pointer to store the number of tasks
   *
   * @return ReturnOK if statistics were retrieved successfully
   * @return ReturnError if allocation failed or invalid parameters
   *
   * @warning Caller is responsible for freeing the allocated array
   */
  Return_t xTaskGetAllRunTimeStats(TaskRunTimeStats_t **stats_, Base_t *tasks_);

  /**
   * @brief Gets runtime statistics for a specific task
   * @details Allocates and returns runtime statistics for the specified task.
   *
   * @param[in] task_ Pointer to the task to query
   * @param[out] stats_ Pointer to store allocated statistics structure
   *
   * @return ReturnOK if statistics were retrieved successfully
   * @return ReturnError if task is invalid or allocation failed
   *
   * @warning Caller is responsible for freeing the allocated structure
   */
  Return_t xTaskGetTaskRunTimeStats(const Task_t *task_, TaskRunTimeStats_t **stats_);

  /**
   * @brief Gets the total number of registered tasks
   * @details Returns the count of all tasks currently in the scheduler.
   *
   * @param[out] tasks_ Pointer to store the task count
   *
   * @return ReturnOK if count was retrieved successfully
   * @return ReturnError if invalid parameter
   */
  Return_t xTaskGetNumberOfTasks(Base_t *tasks_);

  /**
   * @brief Gets information about a specific task
   * @details Allocates and returns detailed information about the specified task.
   *
   * @param[in] task_ Pointer to the task to query
   * @param[out] info_ Pointer to store allocated information structure
   *
   * @return ReturnOK if information was retrieved successfully
   * @return ReturnError if task is invalid or allocation failed
   *
   * @warning Caller is responsible for freeing the allocated structure
   */
  Return_t xTaskGetTaskInfo(const Task_t *task_, TaskInfo_t **info_);

  /**
   * @brief Gets information about all tasks
   * @details Allocates and returns an array of information structures for all registered tasks.
   *
   * @param[out] info_ Pointer to store allocated information array
   * @param[out] tasks_ Pointer to store the number of tasks
   *
   * @return ReturnOK if information was retrieved successfully
   * @return ReturnError if allocation failed or invalid parameters
   *
   * @warning Caller is responsible for freeing the allocated array
   */
  Return_t xTaskGetAllTaskInfo(TaskInfo_t **info_, Base_t *tasks_);

  /**
   * @brief Gets the current state of a task
   * @details Returns the execution state (running, suspended, or waiting) of the specified task.
   *
   * @param[in] task_ Pointer to the task to query
   * @param[out] state_ Pointer to store the task state
   *
   * @return ReturnOK if state was retrieved successfully
   * @return ReturnError if task is invalid or invalid parameter
   */
  Return_t xTaskGetTaskState(const Task_t *task_, TaskState_t *state_);

  /**
   * @brief Gets the name of a task
   * @details Returns a pointer to the task's name string.
   *
   * @param[in] task_ Pointer to the task to query
   * @param[out] name_ Pointer to store the name string pointer
   *
   * @return ReturnOK if name was retrieved successfully
   * @return ReturnError if task is invalid or invalid parameter
   *
   * @note The returned pointer points to the internal task structure; do not modify or free
   */
  Return_t xTaskGetName(const Task_t *task_, Byte_t **name_);

  /**
   * @brief Gets the unique identifier of a task
   * @details Returns the task's unique ID number.
   *
   * @param[in] task_ Pointer to the task to query
   * @param[out] id_ Pointer to store the task ID
   *
   * @return ReturnOK if ID was retrieved successfully
   * @return ReturnError if task is invalid or invalid parameter
   */
  Return_t xTaskGetId(const Task_t *task_, Base_t *id_);

  /**
   * @brief Clears a task's notification state
   * @details Removes any pending notification from the specified task.
   *
   * @param[in,out] task_ Pointer to the task to clear
   *
   * @return ReturnOK if notification was cleared successfully
   * @return ReturnError if task is invalid
   */
  Return_t xTaskNotifyStateClear(Task_t *task_);

  /**
   * @brief Checks if a task has a pending notification
   * @details Returns true if the task has a notification waiting to be consumed.
   *
   * @param[in] task_ Pointer to the task to check
   * @param[out] res_ Pointer to store the result (true if notification pending, false otherwise)
   *
   * @return ReturnOK if check was successful
   * @return ReturnError if task is invalid or invalid parameter
   */
  Return_t xTaskNotificationIsWaiting(const Task_t *task_, Base_t *res_);

  /**
   * @brief Sends a notification to a task
   * @details Delivers a notification with optional data to the specified task.
   *
   * @param[in,out] task_ Pointer to the task to notify
   * @param[in] bytes_ Number of bytes in the notification value
   * @param[in] value_ Pointer to notification data (can be NULL if bytes_ is 0)
   *
   * @return ReturnOK if notification was sent successfully
   * @return ReturnError if task is invalid, buffer full, or invalid parameters
   */
  Return_t xTaskNotifyGive(Task_t *task_, const Base_t bytes_, const Byte_t *value_);

  /**
   * @brief Receives a notification from a task
   * @details Allocates and returns the pending notification for the specified task.
   *
   * @param[in,out] task_ Pointer to the task to receive from
   * @param[out] notification_ Pointer to store allocated notification structure
   *
   * @return ReturnOK if notification was received successfully
   * @return ReturnError if no notification pending, task is invalid, or allocation failed
   *
   * @warning Caller is responsible for freeing the allocated notification structure
   */
  Return_t xTaskNotifyTake(Task_t *task_, TaskNotification_t **notification_);

  /**
   * @brief Resumes a suspended task
   * @details Changes the task state to running, allowing it to be scheduled for execution.
   *
   * @param[in,out] task_ Pointer to the task to resume
   *
   * @return ReturnOK if task was resumed successfully
   * @return ReturnError if task is invalid
   */
  Return_t xTaskResume(Task_t *task_);

  /**
   * @brief Suspends a task
   * @details Changes the task state to suspended, preventing it from being scheduled.
   *
   * @param[in,out] task_ Pointer to the task to suspend
   *
   * @return ReturnOK if task was suspended successfully
   * @return ReturnError if task is invalid
   */
  Return_t xTaskSuspend(Task_t *task_);

  /**
   * @brief Puts a task into waiting state
   * @details Changes the task state to waiting for a notification or event.
   *
   * @param[in,out] task_ Pointer to the task to wait
   *
   * @return ReturnOK if task state was changed successfully
   * @return ReturnError if task is invalid
   */
  Return_t xTaskWait(Task_t *task_);

  /**
   * @brief Changes the period of a task's timer
   * @details Updates the task's periodic timer period without affecting its running state.
   *
   * @param[in,out] task_ Pointer to the task to modify
   * @param[in] period_ New timer period in system ticks
   *
   * @return ReturnOK if period was changed successfully
   * @return ReturnError if task is invalid
   */
  Return_t xTaskChangePeriod(Task_t *task_, const Ticks_t period_);

  /**
   * @brief Changes the watchdog timer period for a task
   * @details Updates the task's watchdog timer period for task monitoring.
   *
   * @param[in,out] task_ Pointer to the task to modify
   * @param[in] period_ New watchdog timer period in system ticks
   *
   * @return ReturnOK if period was changed successfully
   * @return ReturnError if task is invalid or watchdog not enabled
   */
  Return_t xTaskChangeWDPeriod(Task_t *task_, const Ticks_t period_);

  /**
   * @brief Gets the current period of a task's timer
   * @details Retrieves the configured timer period for the specified task.
   *
   * @param[in] task_ Pointer to the task to query
   * @param[out] period_ Pointer to store the timer period in system ticks
   *
   * @return ReturnOK if period was retrieved successfully
   * @return ReturnError if task is invalid or invalid parameter
   */
  Return_t xTaskGetPeriod(const Task_t *task_, Ticks_t *period_);

  /**
   * @brief Resets a task's timer
   * @details Resets the task timer start time to the current system ticks.
   *
   * @param[in,out] task_ Pointer to the task to reset
   *
   * @return ReturnOK if timer was reset successfully
   * @return ReturnError if task is invalid
   */
  Return_t xTaskResetTimer(Task_t *task_);

  /**
   * @brief Starts the task scheduler
   * @details Initiates the cooperative scheduler to begin executing tasks.
   *
   * @return ReturnOK if scheduler was started successfully
   * @return ReturnError if scheduler is already running or initialization failed
   *
   * @note This function should be called after all initial tasks are created
   */
  Return_t xTaskStartScheduler(void);

  /**
   * @brief Resumes all suspended tasks
   * @details Changes all tasks in suspended state to running state.
   *
   * @return ReturnOK if tasks were resumed successfully
   * @return ReturnError if operation failed
   */
  Return_t xTaskResumeAll(void);

  /**
   * @brief Suspends all running tasks
   * @details Changes all tasks in running state to suspended state.
   *
   * @return ReturnOK if tasks were suspended successfully
   * @return ReturnError if operation failed
   */
  Return_t xTaskSuspendAll(void);

  /**
   * @brief Gets the current scheduler state
   * @details Returns whether the scheduler is running or suspended.
   *
   * @param[out] state_ Pointer to store the scheduler state
   *
   * @return ReturnOK if state was retrieved successfully
   * @return ReturnError if invalid parameter
   */
  Return_t xTaskGetSchedulerState(SchedulerState_t *state_);

  /**
   * @brief Gets the watchdog timer period for a task
   * @details Retrieves the configured watchdog timer period for the specified task.
   *
   * @param[in] task_ Pointer to the task to query
   * @param[out] period_ Pointer to store the watchdog period in system ticks
   *
   * @return ReturnOK if period was retrieved successfully
   * @return ReturnError if task is invalid, watchdog not enabled, or invalid parameter
   */
  Return_t xTaskGetWDPeriod(const Task_t *task_, Ticks_t *period_);

  #if defined(POSIX_ARCH_OTHER)
    void __TaskStateClear__(void);

  #endif /* if defined(POSIX_ARCH_OTHER) */

  #ifdef __cplusplus

    }

  #endif /* ifdef __cplusplus */

#endif /* ifndef TASK_H_ */