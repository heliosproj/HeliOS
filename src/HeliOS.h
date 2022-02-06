/**
 * @file HeliOS.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Header file to be included in end-user code and contains all of the public data types and functions
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
#ifndef HELIOS_H_
#define HELIOS_H_

#include <stdint.h>

#include "config.h"
#include "defines.h"

/**
 * @brief Enumerated type for task states.
 * 
 * A task can be in one of the four possible states defined in the TaskState_t
 * enumerated type. The state of a task is changed by calling xTaskResume(),
 * xTaskSuspend() or xTaskWait(). The TaskState_t enumerated type should be declared
 * as xTaskState.
 * 
 * @sa xTaskState
 * @sa xTaskResume()
 * @sa xTaskSuspend()
 * @sa xTaskWait() 
 * 
 */
typedef enum {
  TaskStateError,      /**< Returned by xTaskGetTaskState() when task cannot be found. */
  TaskStateSuspended, /**< State a task is in when it is first created by xTaskCreate() or suspended by xTaskSuspend(). */
  TaskStateRunning,   /**< State a task is in after xTaskResume() is called. */
  TaskStateWaiting    /**< State a task is in after xTaskWait() is called. */
} TaskState_t;

/**
 * @brief Type definition for the task identifier.
 * 
 * A task handle can be found by either the task's ASCII name or its identifier by
 * calling xTaskGetHandleByName() or xTaskGetHandleById() respectively. The TaskId_t
 * type should be declared as xTaskId.
 * 
 * @sa xTaskId
 * @sa xTaskGetHandleByName()
 * @sa xTaskGetHandleById()
 * 
 */
typedef int16_t TaskId_t;

/**
 * @brief Type definition for the base data type.
 * 
 * A simple data type is often needed as an argument for a system call or a return type.
 * The Base_t type is used in such a case where there are no other structural data
 * requirements. The Base_t type should be declared as xBase.
 * 
 * @sa xBase
 *
 */
typedef int16_t Base_t;

/**
 * @brief Data structure for task runtime statistics.
 * 
 * The TaskRunTimeStats_t structure contains task runtime statistics and is returned by
 * xTaskGetAllRunTimeStats() and xTaskGetTaskRunTimeStats(). The TaskRunTimeStats_t type
 * should be declared as xTaskRunTimeStats.
 * 
 * @sa xTaskRunTimeStats
 * @sa xTaskGetTaskRunTimeStats()
 * @sa xTaskGetAllRunTimeStats()
 * 
 * @warning The memory allocated for an instance of xTaskRunTimeStats must be freed
 * using xMemFree().
 * 
 */
typedef struct TaskRunTimeStats_s {
  Time_t lastRunTime;  /**< The runtime duration in microseconds the last time the task was executed by the scheduler. */
  Time_t totalRunTime; /**< The total runtime duration in microseconds the task has been executed by the scheduler. */
} TaskRunTimeStats_t;

/**
 * @brief Data structure for information about a task.
 * 
 * The TaskInfo_t structure is similar to xTaskRuntimeStats_t in that it contains runtime statistics for
 * a task. However, TaskInfo_t also contains additional details about a task such as its identifier, ASCII name
 * and state. The TaskInfo_t structure is returned by xTaskGetTaskInfo(). If on runtime statistics are needed,
 * TaskRunTimeStats_t should be used because of its lower memory footprint. The TaskInfo_t type should be
 * declared as xTaskInfo.
 * 
 * @sa xTaskInfo
 * @sa xTaskGetTaskInfo()
 * 
 * @warning The memory allocated for an instance of xTaskInfo must be freed using
 * xMemFree().
 * 
 */
typedef struct TaskInfo_s {
  TaskId_t id;                       /**< The task identifier which is used by xTaskGetHandleById() to return the task handle. */
  char name[CONFIG_TASK_NAME_BYTES]; /**< The name of the task which is used by xTaskGetHandleByName() to return the task handle. */
  TaskState_t state;                 /**< The state the task is in which is one of four states specified in the TaskState_t enumerated data type. */
  Time_t lastRunTime;                /**< The runtime duration in microseconds the last time the task was executed by the scheduler. */
  Time_t totalRunTime;               /**< The total runtime duration in microseconds the task has been executed by the scheduler. */
} TaskInfo_t;

/**
 * @brief Data structure for direct to task notifications.
 * 
 * The TaskNotification_t data structure contains the direct to task notification returned by xTaskNotifyTake().
 * The TaskNotification_t type should be declared as xTaskNotification.
 * 
 * @sa xTaskNotification
 * @sa xTaskNotifyTake()
 * 
 * @warning The memory allocated for an instance of xTaskNotification must be freed using
 * xMemFree().
 * 
 */
typedef struct TaskNotification_s {
  Base_t notificationBytes;                                /**< The number of bytes in the notificationValue member that makes up the notification value. This cannot exceed CONFIG_NOTIFICATION_VALUE_BYTES. */
  char notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES]; /**< The char array that contains the actual notification value. */
} TaskNotification_t;

/**
 * @brief Data structure for a message queue message.
 * 
 * The QueueMessage_t data structure contains the message queue message returned by xQueuePeek() and
 * xQueueReceive(). The QueueMessage_t type should be declared as xQueueMessage.
 * 
 * @sa xQueueMessage
 * @sa xQueuePeek()
 * @sa xQueueReceive()
 * 
 * @warning The memory allocated for an instance of xQueueMessage must be freed using xMemFree().
 * 
 */
typedef struct QueueMessage_s {
  Base_t messageBytes;                           /**< The number of bytes in the messageValue member that makes up the message value. This cannot exceed CONFIG_MESSAGE_VALUE_BYTES. */
  char messageValue[CONFIG_MESSAGE_VALUE_BYTES]; /**< the char array that contains the actual message value. */
} QueueMessage_t;

/**
 * @brief Data structure for system informaiton.
 * 
 * The SystemInfo_t data structure contains information about the HeliOS system and is returned
 * by xSystemGetSystemInfo(). The SystemInfo_t should be declared as xSystemInfo.
 * 
 * @sa xSystemInfo
 * @sa xSystemGetSystemInfo()
 * 
 * @warning The memory allocated for an instance of xSystemInfo must be freed using xMemFree().
 * 
 */
typedef struct SystemInfo_s {
  char productName[PRODUCTNAME_SIZE]; /**< The name of the operating system or product. This is always HeliOS. */
  Base_t majorVersion;                /**< The major version number of HeliOS and is Symantec Versioning Specification (SemVer) compliant. */
  Base_t minorVersion;                /**< The minor version number of HeliOS and is Symantec Versioning Specification (SemVer) compliant. */
  Base_t patchVersion;                /**< The patch version number of HeliOS and is Symantec Versioning Specification (SemVer) compliant. */
  Base_t numberOfTasks;               /**< The number of tasks presently in a suspended, running or waiting state. */
} SystemInfo_t;


typedef void Task_t;
typedef void TaskParm_t;
typedef void Queue_t;
typedef void Timer_t;

/* Type defines for the HeliOS API. */
typedef Base_t xBase;
typedef TaskId_t xTaskId;
typedef Timer_t *xTimer;
typedef Queue_t *xQueue;
typedef QueueMessage_t *xQueueMessage;
typedef TaskNotification_t *xTaskNotification;
typedef TaskInfo_t *xTaskInfo;
typedef TaskRunTimeStats_t *xTaskRunTimeStats;
typedef Task_t *xTask;
typedef TaskParm_t *xTaskParm;
typedef Time_t xTime;
typedef TaskState_t xTaskState;
typedef SystemInfo_t *xSystemInfo;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The xMemAlloc() system call will dynamically allocate memory for HeliOS
 * system calls and end-user tasks. The number of concurrently allocated pointers
 * is dependent on the setting CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES.
 *
 * @param size_ The amount (size) of the memory to be dynamically allocated in bytes.
 * @return void* If successful, xMemAlloc() returns a pointer to the dynamically
 * allocated memory. If unsuccessful, the system call will return null.
 */
void *xMemAlloc(size_t size_);

/**
 * @brief The xMemFree() system call will free memory dynamically allocated by
 * xMemAlloc() and other HeliOS system calls such as xSystemGetSystemInfo().
 *
 * @param ptr_ The pointer to the dynamically allocated memory to be freed.
 */
void xMemFree(void *ptr_);

/**
 * @brief The xMemGetUsed() system call returns the amount of memory in bytes
 * that is currently allocated. Calls to xMemAlloc() increases and xMemFree()
 * decreases the amount.
 *
 * @return size_t The amount of memory currently allocated in bytes. If no dynamic
 * memory is currently allocated, xMemGetUsed() will return zero.
 */
size_t xMemGetUsed();

/**
 * @brief The xMemGetSize() system call returns the amount of memory in bytes that
 * is currently allocated to a specific pointer. If the pointer is null or invalid,
 * xMemGetSize() will return zero bytes.
 *
 * @param ptr_ The pointer to the dynamically allocated memory to obtain the size of the
 * memory that is allocated.
 * @return size_t The amount of memory currently allocated to the specific pointer in bytes. If
 * the pointer is invalid or null, xMemGetSize() will return zero.
 */
size_t xMemGetSize(void *ptr_);

/**
 * @brief The xQueueCreate() system call creates a message queue for inter-task
 * communication. The queue should only be deleted by xQueueDelete() and NOT
 * xMemFree().
 *
 * @param limit_ The message limit for the queue. When this number is reach, the queue
 * is considered full and xQueueSend() will fail. The minimum limit for queues is dependent
 * on the  setting CONFIG_QUEUE_MINIMUM_LIMIT.
 * @return xQueue A queue is returned if successful, otherwise null is returned if unsuccessful.
 */
xQueue xQueueCreate(xBase limit_);

/**
 * @brief The xQueueDelete() system call will delete a queue created by xQueueCreate(). xQueueDelete()
 * will delete a queue regardless of how many messages the queue contains at the time xQueueDelete()
 * is called.
 *
 * @param queue_ The queue to be deleted.
 */
void xQueueDelete(xQueue queue_);

/**
 * @brief The xQueueGetLength() system call returns the length of the queue (the number of messages
 * the queue currently contains).
 *
 * @param queue_ The queue to return the length of.
 * @return xBase The number of messages in the queue. If unsuccessful or if the queue is empty,
 * xQueueGetLength() returns zero.
 */
xBase xQueueGetLength(xQueue queue_);

/**
 * @brief The xQueueIsEmpty() system call will return a true or false dependent on whether the queue is
 * empty or contains one or more messages.
 *
 * @param queue_ The queue to determine whether it is empty.
 * @return xBase True if the queue is empty. False if the queue has one or more messages. xQueueIsQueueEmpty()
 * will also return false if the queue parameter is invalid.
 */
xBase xQueueIsQueueEmpty(xQueue queue_);

/**
 * @brief The xQueueIsFull() system call will return a true or false dependent on whether the queue is
 * full or contains zero messages. A queue is considered full if the number of messages in the queue
 * is equal to the queue's length limit.
 *
 * @param queue_ The queue to determine whether it is full.
 * @return xBase True if the queue is full. False if the queue has zero. xQueueIsQueueEmpty()
 * will also return false if the queue parameter is invalid.
 */
xBase xQueueIsQueueFull(xQueue queue_);

/**
 * @brief The xQueueMessageWaiting() system call returns true or false dependent on whether
 * there is at least one message waiting. The queue does not have to be full to return true.
 *
 * @param queue_ The queue to determine whether one or more messages are waiting.
 * @return xBase True if one or more messages are waiting. False if there are no
 * messages waiting of the queue parameter is invalid.
 */
xBase xQueueMessagesWaiting(xQueue queue_);

/**
 * @brief The xQueueSend() system call will send a message to the queue. The size of the message
 * value is passed in the message bytes parameter. The message value size in byes is dependent
 * on the CONFIG_MESSAGE_VALUE_BYTES setting.
 *
 * @param queue_ The queue to send the message to.
 * @param messageBytes_ The number of bytes contained in the message value. The number of bytes must be greater than
 * zero and less than or equal to the setting CONFIG_MESSAGE_VALUE_BYTES.
 * @param messageValue_ The message value. If the message value is greater than defined in CONFIG_MESSAGE_VALUE_BYTES,
 * only the number of bytes defined in CONFIG_MESSAGE_VALUE_BYTES will be copied into the message value.
 * @return xBase xQueueSend() returns true if the message was sent to the queue successfully. Otherwise
 * false if unsuccessful.
 */
xBase xQueueSend(xQueue queue_, xBase messageBytes_, const char *messageValue_);

/**
 * @brief The xQueuePeek() system call will return the next message in the queue without
 * dropping the message.
 *
 * @param queue_ The queue to return the next message from.
 * @return xQueueMessage The next message in the queue. If the queue is empty or the queue
 * parameter is invalid, xQueuePeek() will return null.
 */
xQueueMessage xQueuePeek(xQueue queue_);

/**
 * @brief The xQueueDropMessage() system call will drop the next message from the queue without
 * returning the message.
 *
 * @param queue_ The queue to drop the next message from.
 */
void xQueueDropMessage(xQueue queue_);

/**
 * @brief The xQueueReceive() system call will return the next message in the queue and drop
 * it from the queue.
 *
 * @param queue_ The queue to return the next message from.
 * @return xQueueMessage The message returned from the queue. If the queue is empty
 * of the queue parameter is invalid, xQueueReceive() will return null.
 */
xQueueMessage xQueueReceive(xQueue queue_);

/**
 * @brief The xTaskStartScheduler() system call passes control to the HeliOS scheduler. This system
 * call will not return until xTaskSuspendAll() is called. If xTaskSuspendAll() is called, xTaskResumeAll()
 * must be called before xTaskStartScheduler() can be called again.
 *
 */
void xTaskStartScheduler();

/**
 * @brief The xTaskResumeAll() system call will set the scheduler system flag so the next
 * call to xTaskStartScheduler() will resume execute of all tasks. The state of each task
 * is not altered by xTaskSuspendAll() or xTaskResumeAll().
 *
 */
void xTaskResumeAll();

/**
 * @brief The xTaskSuspendAll() system call will set the scheduler system flag so the scheduler
 * will stop and return. The state of each task is not altered by xTaskSuspendAll() or xTaskResumeAll().
 *
 */
void xTaskSuspendAll();

/**
 * @brief The xSystemGetSystemInfo() system call will return the type xSystemInfo containing
 * information about the system including the OS (product) name, its version and how many tasks
 * are currently in the running, suspended or waiting states.
 *
 * @return xSystemInfo The system info is returned if successful, otherwise null is
 * returned if unsuccessful.
 */
xSystemInfo xSystemGetSystemInfo();

/**
 * @brief The xTaskCreate() system call will create a new task. The task will be created with its
 * state set to suspended. The xTaskCreate() and xTaskDelete() system calls cannot be called within
 * a task. They MUST be called outside of the scope of the HeliOS scheduler.
 *
 * @param name_ The ASCII name of the task which can be used by xTaskGetHandleByName() to obtain the task pointer. The
 * length of the name is depended on the CONFIG_TASK_NAME_BYTES. The task name is NOT a null terminated char array.
 * @param callback_ The callback pointer to the task main function. This is the function that will be invoked
 * by the scheduler when a task is scheduled for execution.
 * @param taskParameter_ A pointer to any type or structure that the end-user wants to pass into the task as
 * a parameter. The task parameter is not required and may simply be set to null.
 * @return xTask A pointer to the newly created task.
 */
xTask xTaskCreate(const char *name_, void (*callback_)(xTask, xTaskParm), xTaskParm taskParameter_);

/**
 * @brief The xTaskDelete() system call will delete a task. The xTaskCreate() and xTaskDelete() system calls
 * cannot be called within a task. They MUST be called outside of the scope of the HeliOS scheduler.
 *
 * @param task_ A pointer to the task to be deleted.
 */
void xTaskDelete(xTask task_);

/**
 * @brief The xTaskGetHandleByName() system call will return the task handle pointer to the
 * task specified by its ASCII name. The length of the task name is dependent on the
 * CONFIG_TASK_NAME_BYTES setting. The name is compared byte-for-byte so the name is
 * case sensitive.
 *
 * @param name_ The ASCII name of the task to return the handle pointer for.
 * @return xTask A pointer to the task handle. xTaskGetHandleByName() returns null if the
 * name cannot be found.
 */
xTask xTaskGetHandleByName(const char *name_);

/**
 * @brief The xTaskGetHandleById() system call will return a pointer to the task handle
 * specified by its identifier.
 *
 * @param id_ The identifier of the task to return the handle pointer for.
 * @return xTask A  pointer to the task handle. xTaskGetHandleById() returns null if the
 * the task identifier cannot be found.
 */
xTask xTaskGetHandleById(xTaskId id_);

/**
 * @brief The xTaskGetAllRunTimeStats() system call will return the runtime statistics for all
 * of the tasks regardless of their state. The xTaskGetAllRunTimeStats() system call returns
 * the xTaskRunTimeStats type. An xBase variable must be passed by reference to xTaskGetAllRunTimeStats()
 * which will contain the number of tasks so the end-user can iterate through the tasks. The
 * xTaskRunTimeStats memory must be freed by xMemFree() after it is no longer needed.
 *
 * @param tasks_ An variable of type xBase passed by reference which will contain the number of tasks
 * upon return. If no tasks currently exist, this variable will not be modified.
 * @return xTaskRunTimeStats The runtime stats returned by xTaskGetAllRunTimeStats(). If there are
 * currently no tasks then this will be null. This memory must be freed by xMemFree().
 */
xTaskRunTimeStats xTaskGetAllRunTimeStats(xBase *tasks_);

/**
 * @brief The xTaskGetTaskRunTimeStats() system call returns the task runtime statistics for
 * one task. The xTaskGetTaskRunTimeStats() system call returns the xTaskRunTimeStats type.
 * The memory must be freed by calling xMemFree() after it is no longer needed.
 *
 * @param task_ The task to get the runtime statistics for.
 * @return xTaskRunTimeStats The runtime stats returned by xTaskGetTaskRunTimeStats().
 * xTaskGetTaskRunTimeStats() will return null of the task cannot be found.
 */
xTaskRunTimeStats xTaskGetTaskRunTimeStats(xTask task_);

/**
 * @brief The xTaskGetNumberOfTasks() system call returns the current number of tasks
 * regardless of their state.
 *
 * @return xBase The number of tasks.
 */
xBase xTaskGetNumberOfTasks();

/**
 * @brief The xTaskGetTaskInfo() system call returns the xTaskInfo structure containing
 * the details of the task including its identifier, name, state and runtime statistics.
 *
 * @param task_ The task to return the details of.
 * @return TaskInfo_t* The xTaskInfo structure containing the task details. xTaskGetTaskInfo()
 * returns null if the task cannot be found.
 */
TaskInfo_t *xTaskGetTaskInfo(xTask task_);

/**
 * @brief The xTaskGetTaskState() system call will return the state of the task.
 *
 * @param task_ The task to return the state of.
 * @return xTaskState The xTaskState of the task. If the task cannot be found, xTaskGetTaskState()
 * will return null.
 */
xTaskState xTaskGetTaskState(xTask task_);

/**
 * @brief The xTaskGetName() system call returns the ASCII name of the task. The size of the
 * task is dependent on the setting CONFIG_TASK_NAME_BYTES. The task name is NOT a null
 * terminated char array. The memory allocated for the char array must be freed by
 * xMemFree() when no longer needed.
 *
 * @param task_ The task to return the name of.
 * @return char* A pointer to the char array containing the ASCII name of the task. The task name
 * is NOT a null terminated char array. xTaskGetName() will return null if the task cannot be found.
 */
char *xTaskGetName(xTask task_);

/**
 * @brief The xTaskGetId() system call returns the task identifier for the task.
 *
 * @param task_ The task to return the identifier of.
 * @return xTaskId The identifier of the task. If the task cannot be found, xTaskGetId()
 * returns zero (all tasks identifiers are 1 or greater).
 */
xTaskId xTaskGetId(xTask task_);

/**
 * @brief The xTaskNotifyStateClear() system call will clear a waiting task notification if one
 * exists without returning the notification.
 *
 * @param task_ The task to clear the notification for.
 */
void xTaskNotifyStateClear(xTask task_);

/**
 * @brief The xTaskNotificationIsWaiting() system call will return true or false depending
 * on whether there is a task notification waiting for the task.
 *
 * @param task_ The task to check for a waiting task notification.
 * @return xBase Returns true if there is a task notification. False if there is no notification
 * or if the task could not be found.
 */
xBase xTaskNotificationIsWaiting(xTask task_);

/**
 * @brief The xTaskNotifyGive() system call will send a task notification to the specified task. The
 * task notification bytes is the number of bytes contained in the notification value. The number of
 * notification bytes must be between one and the CONFIG_NOTIFICATION_VALUE_BYTES setting. The notification
 * value must contain a pointer to a char array containing the notification value. If the task already
 * has a waiting task notification, xTaskNotifyGive() will NOT overwrite the waiting task notification.
 *
 * @param task_ The task to send the task notification to.
 * @param notificationBytes_ The number of bytes contained in the notification value. The number must be
 * between one and the CONFIG_NOTIFICATION_VALUE_BYTES setting.
 * @param notificationValue_ A char array containing the notification value.
 */
void xTaskNotifyGive(xTask task_, xBase notificationBytes_, const char *notificationValue_);

/**
 * @brief The xTaskNotifyTake() system call will return the waiting task notification if there
 * is one. The xTaskNotifyTake() system call will return an xTaskNotification structure containing
 * the notification bytes and its value. The memory allocated by xTaskNotifyTake() must be freed
 * by xMemFree().
 *
 * @param task_ The task to return a waiting task notification.
 * @return xTaskNotification The xTaskNotification structure containing the notification bytes
 * and value. xTaskNotifyTake() will return null if no waiting task notification exists or if
 * the task cannot be found.
 */
xTaskNotification xTaskNotifyTake(xTask task_);

/**
 * @brief The xTaskResume() system call will resume a suspended task. Tasks are suspended on creation
 * so either xTaskResume() or xTaskWait() must be called to place the task in a state that the scheduler
 * will execute.
 *
 * @param task_ The task to set its state to running.
 */
void xTaskResume(xTask task_);

/**
 * @brief The xTaskSuspend() system call will suspend a task. A task that has been suspended
 * will not be executed by the scheduler until xTaskResume() or xTaskWait() is called.
 *
 * @param task_ The task to suspend.
 */
void xTaskSuspend(xTask task_);

/**
 * @brief The xTaskWait() system call will place a task in the waiting state. A task must
 * be in the waiting state for event driven multitasking with either direct to task
 * notifications OR setting the period on the task timer with xTaskChangePeriod(). A task
 * in the waiting state will not be executed by the scheduler until an event has occurred.
 *
 * @param task_ The task to place in the waiting state.
 */
void xTaskWait(xTask task_);

/**
 * @brief The xTaskChangePeriod() system call will change the period (microseconds) on the task timer
 * for the specified task. The timer period must be greater than zero. To have any effect, the task
 * must be in the waiting state set by calling xTaskWait() on the task. Once the timer period is set
 * and the task is in the waiting state, the task will be executed every N microseconds based on the period.
 * Changing the period to zero will prevent the task from being executed even if it is in the waiting state.
 *
 * @param task_ The task to change the timer period for.
 * @param timerPeriod_ The timer period in microseconds.
 */
void xTaskChangePeriod(xTask task_, xTime timerPeriod_);

/**
 * @brief The xTaskGetPeriod() will return the period for the timer for the specified task. See
 * xTaskChangePeriod() for more information on how the task timer works.
 *
 * @param task_ The task to return the timer period for.
 * @return xTime The timer period in microseconds. xTaskGetPeriod() will return zero
 * if the timer period is zero or if the task could not be found.
 */
xTime xTaskGetPeriod(xTask task_);

/**
 * @brief The xTaskResetTimer() system call will reset the task timer. xTaskResetTimer() does not change
 * the timer period or the task state when called. See xTaskChangePeriod() for more details on task timers.
 *
 * @param task_ The task to reset the task timer for.
 */
void xTaskResetTimer(xTask task_);

/**
 * @brief The xTimerCreate() system call will create a new timer. Timers differ from
 * task timers in that they do not create events that effect the scheduling of a task.
 * Timers can be used by tasks to initiate various task activities based on a specified
 * time period represented in microseconds. The memory allocated by xTimerCreate() must
 * be freed by xTimerDelete(). Unlike tasks, timers may be created and deleted within
 * tasks.
 *
 * @param timerPeriod_ The number of microseconds before the timer expires.
 * @return xTimer The newly created timer. If the timer period parameter is less than zero
 * or xTimerCreate() was unable to allocate the required memory, xTimerCreate() will return null.
 */
xTimer xTimerCreate(xTime timerPeriod_);

/**
 * @brief The xTimerDelete() system call will delete a timer. For more information on timers see the
 * xTaskTimerCreat() system call.
 *
 * @param timer_ The timer to be deleted.
 */
void xTimerDelete(xTimer timer_);

/**
 * @brief The xTimerChangePeriod() system call will change the period of the specified timer.
 * The timer period is measured in microseconds. If the timer period is zero, the xTimerHasTimerExpired()
 * system call will always return false.
 *
 *
 * @param timer_ The timer to change the period for.
 * @param timerPeriod_ The timer period in is microseconds. Timer period must be zero or greater.
 */
void xTimerChangePeriod(xTimer timer_, xTime timerPeriod_);

/**
 * @brief The xTimerGetPeriod() system call will return the current timer period
 * for the specified timer.
 *
 * @param timer_ The timer to get the timer period for.
 * @return xTime The timer period. If the timer cannot be found, xTimerGetPeriod()
 * will return zero.
 */
xTime xTimerGetPeriod(xTimer timer_);

/**
 * @brief The xTimerIsTimerActive() system call will return true of the timer has been
 * started with xTimerStart().
 *
 * @param timer_ The timer to check if active.
 * @return xBase True if active, false if not active or if the timer could not be found.
 */
xBase xTimerIsTimerActive(xTimer timer_);

/**
 * @brief The xTimerHasTimerExpired() system call will return true or false dependent on whether
 * the timer period for the specified timer has elapsed. xTimerHasTimerExpired() will NOT
 * reset the timer. Timers must be reset with xTimerReset().
 *
 * @param timer_ The timer to determine if the period has expired.
 * @return xBase True if the timer has expired, false if the timer has not expired or could not be found.
 */
xBase xTimerHasTimerExpired(xTimer timer_);

/**
 * @brief The xTimerReset() system call will reset the start time of the timer to zero.
 *
 * @param timer_ The timer to be reset.
 */
void xTimerReset(xTimer timer_);

/**
 * @brief The xTimerStart() system call will place the timer in the running state. Neither xTaskStart() nor
 * xTaskStop() will reset the timer. Timers can only be reset with xTimerReset().
 *
 * @param timer_ The timer to be started.
 */
void xTimerStart(xTimer timer_);

/**
 * @brief The xTimerStop() system call will place the timer in the stopped state. Neither xTaskStart() nor
 * xTaskStop() will reset the timer. Timers can only be reset with xTimerReset().
 *
 * @param timer_ The timer to be stopped.
 */
void xTimerStop(xTimer timer_);

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif