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
 * @brief Enumerated data type for task states
 *
 * A task can be in one of four possible states as defined by the TaskState_t
 * enumerated data type. The state a task is in is changed by calling xTaskResume(),
 * xTaskSuspend() or xTaskWait(). The HeliOS scheduler will only schedule, for execution,
 * tasks in either the TaskStateRunning or TaskStateWaiting state. TaskState_t should
 * be declared (i.e., used) as xTaskState.
 *
 * @sa xTaskState
 * @sa xTaskResume()
 * @sa xTaskSuspend()
 * @sa xTaskWait()
 * @sa xTaskGetTaskState()
 *
 */
typedef enum {
  TaskStateError,     /**< Returned by xTaskGetTaskState() when the task cannot be found */
  TaskStateSuspended, /**< State a task is in when it is first created OR after calling xTaskSuspend() - tasks in the TaskStateSuspended state will not be scheduled for execution*/
  TaskStateRunning,   /**< State a task is in after calling xTaskResume() - tasks in the TaskStateRunning state will be scheduled co-operatively */
  TaskStateWaiting    /**< State a task is in after calling xTaskWait() - tasks in the TaskStateWaiting state will be scheduled as event driven */
} TaskState_t;

/**
 * @brief Enumerated data type for task states
 *
 * @sa TaskState_t
 *
 */
typedef TaskState_t xTaskState;

/**
 * @brief Enumerated data type for scheduler state
 *
 * The scheduler can be in one of three possible states as defined by the SchedulerState_t
 * enumerated data type. The state the scheduler is in is changed by calling xTaskSuspendAll()
 * and xTaskResumeAll(). The state the scheduler is in can be obtained by calling xTaskGetSchedulerState().
 * SchedulerState_t should be declared (i.e., used) as xSchedulerState.
 *
 * @sa xSchedulerState
 * @sa xTaskSuspendAll()
 * @sa xTaskResumeAll()
 * @sa xTaskGetSchedulerState()
 * @sa xTaskStartScheduler()
 *
 */
typedef enum {
  SchedulerStateError,     /**< Not used - reserved for future use */
  SchedulerStateSuspended, /**< State the scheduler is in after calling xTaskSuspendAll() - xTaskStartScheduler() will stop scheduling tasks for execution and relinquish control when xTaskSuspendAll() is called */
  SchedulerStateRunning    /**< State the scheduler is in after calling xTaskResumeAll() - xTaskStartScheduler() will continue to schedule tasks for execution until xTaskSuspendAll() is called */
} SchedulerState_t;

/**
 * @brief Enumerated data type for the scheduler state
 *
 * @sa SchedulerState_t
 *
 */
typedef SchedulerState_t xSchedulerState;

/* START OF BASIC TYPES */

/**
 * @brief Data type for the task paramater
 *
 * The TaskParm_t type is used to pass a paramater to a task at the time of task
 * creation using xTaskCreate(). A task paramater is a pointer of type void and
 * can point to any number of types, arrays and/or data structures that will be
 * passed to the task. It is up to the end-user to manage, allocate and free the
 * memory related to these objects using xMemAlloc() and xMemFree(). TaskParm_t
 * should be declared as xTaskParm.
 *
 * @sa xTaskParm
 * @sa xTaskCreate()
 * @sa xMemAlloc()
 * @sa xMemFree()
 *
 */
typedef VOID_TYPE TaskParm_t;

/**
 * @brief Data type for the task paramater
 *
 * @sa TaskParm_t
 *
 */
typedef TaskParm_t *xTaskParm;

/**
 * @brief Data type for the base type
 *
 * The Base_t type is a simple data type often used as an argument or return type
 * for system calls when the value is known not to exceed its 8-bit width and no
 * data structure requirements exist. There are no guarantees the Base_t will
 * always be 8-bits wide. If an 8-bit data type is needed that is guaranteed
 * to remain 8-bits wide, the Byte_t data type should be used. Base_t should be
 * declared as xBase.
 *
 * @sa xBase
 * @sa Byte_t
 *
 */
typedef UINT8_TYPE Base_t;

/**
 * @brief Data type for the base type
 *
 * @sa Base_t
 *
 */
typedef Base_t xBase;

/**
 * @brief Data type for an 8-bit wide byte
 *
 * The Byte_t type is an 8-bit wide data type and is guaranteed to always be
 * 8-bits wide. Byte_t should be declared as xByte.
 *
 * @sa xByte
 *
 */
typedef UINT8_TYPE Byte_t;

/**
 * @brief Data type for an 8-bit wide byte
 *
 * @sa Byte_t
 *
 */
typedef Byte_t xByte;

/**
 * @brief Data type for a pointer to an address
 *
 * The Addr_t type is a pointer of type void and is used to pass
 * addresses between the end-user application and system calls. It is not
 * necessary to use the Addr_t ype within the end-user application as long as
 * the type is not used to interact with the kernel through system calls. Addr_t
 * should be declared as xAddr.
 *
 * @sa xAddr
 *
 */
typedef VOID_TYPE Addr_t;

/**
 * @brief Data type for a pointer to an address
 *
 * @sa Addr_t
 *
 */
typedef Addr_t *xAddr;

/**
 * @brief Data type for the storage requirements of an object in memory
 *
 * The Size_t type is used for the storage requirements of an object in
 * memory and is always represented in bytes. Size_t should be declared as
 * xSize.
 *
 * @sa xSize
 *
 */
typedef SIZE_TYPE Size_t;

/**
 * @brief Data type for the storage requirements of an object in memory
 *
 * @sa Size_t
 *
 */
typedef Size_t xSize;

/**
 * @brief Data type for a 16-bit half word
 *
 * The HalfWord_t type is a 16-bit wide data type and is guaranteed to always be
 * 16-bits wide. HalfWord_t should be declared as xHalfWord.
 *
 * @sa xHalfWord
 *
 */
typedef UINT16_TYPE HalfWord_t;

/**
 * @brief Data type for a 16-bit half word
 *
 * @sa HalfWord_t
 *
 */
typedef HalfWord_t xHalfWord;

/**
 * @brief Data type for a 32-bit word
 *
 * The Word_t type is a 32-bit wide data type and is guaranteed to always be
 * 32-bits wide. Word_t should be declared as xWord.
 *
 * @sa xWord
 *
 */
typedef UINT32_TYPE Word_t;

/**
 * @brief Data type for a 32-bit word
 *
 * @sa Word_t
 *
 */
typedef Word_t xWord;

/**
 * @brief Data type for system ticks
 *
 * The Ticks_t type is used to store ticks from the system clock. Ticks
 * is not bound to any one unit of measure for time though most systems
 * are configured for millisecond resolution, milliseconds is not guaranteed
 * and is dependent on the system clock frequency and prescaler. Ticks_t should
 * be declared as xTicks.
 *
 * @sa xTicks
 *
 */
typedef UINT32_TYPE Ticks_t;

/**
 * @brief Data type for system ticks
 *
 * @sa Ticks_t
 *
 */
typedef Ticks_t xTicks;

/**
 * @brief Data type for a character
 *
 * The Char_t data type is used to store an 8-bit char and is
 * typically used for char arrays for ASCII names (e.g., task name).
 * Char_t should be declared as xChar.
 *
 * @sa xChar
 *
 */
typedef UCHAR_TYPE Char_t;

/**
 * @brief Data type for a character
 *
 * @sa Data_t
 *
 */
typedef Char_t xChar;

/* START OF HANDLE TYPES */

/**
 * @brief Data type for a device handle
 *
 * The Device_t data type is used as a device handle. The device handle
 * is created when xDeviceRegisterDevice() is called. For more information
 * about devices and device drivers, see xDeviceRegisterDevice() for more
 * information. Device_t should be declared as xDevice.
 *
 * @sa xDevice
 * @sa xDeviceRegisterDevice()
 *
 */
typedef VOID_TYPE Device_t;

/**
 * @brief Data type for a device handle
 *
 * @sa Device_t
 *
 */
typedef Device_t *xDevice;

/**
 * @brief Data type for a task handle
 *
 * The Task_t data type is used as a task handle. The task handle is created
 * when xTaskCreate() is called. For more information about tasks, see xTaskCreate().
 * Task_t should be declared as xTask.
 *
 * @sa xTask
 * @sa xTaskCreate()
 * @attention The memory referenced by the task handle must be freed by calling xTaskDelete()
 * @sa xTaskDelete()
 *
 */
typedef VOID_TYPE Task_t;

/**
 * @brief Data type for a task handle
 *
 * @sa Task_t
 * @attention The memory referenced by the task handle must be freed by calling xTaskDelete()
 * @sa xTaskDelete()
 *
 */
typedef Task_t *xTask;

/**
 * @brief Data type for a stream buffer handle
 *
 * The StreamBuffer_t data type is used as a stream buffer handle. The stream buffer
 * handle is created when xStreamCreate() is called. For more information about
 * stream buffers, see xStreamCreate(). Stream_t should be declared as xStream.
 *
 * @sa xStream
 * @sa xStreamCreate()
 * @attention The memory referenced by the stream buffer handle must be freed by calling xStreamDelete()
 * @sa xStreamDelete()
 *
 */
typedef VOID_TYPE StreamBuffer_t;

/**
 * @brief Data type for a stream buffer handle
 *
 * @sa StreamBuffer_t
 * @attention The memory referenced by the stream buffer handle must be freed by calling xStreamDelete()
 * @sa xStreamDelete()
 *
 */
typedef StreamBuffer_t *xStreamBuffer;

/**
 * @brief Data type for a queue handle
 *
 * The Queue_t data type is used as a queue handle. The queue handle is created
 * when xQueueCreate() is called. For more information about queues, see xQueueCreate().
 * Queue_t should be declared as xQueue.
 *
 * @sa xQueue
 * @sa xQueueCreate()
 * @attention The memory referenced by the queue handle must be freed by calling xQueueDelete()
 * @sa xQueueDelete()
 *
 */
typedef VOID_TYPE Queue_t;

/**
 * @brief Data type for a queue handle
 *
 * @sa Queue_t
 * @attention The memory referenced by the queue handle must be freed by calling xQueueDelete()
 * @sa xQueueDelete()
 *
 */
typedef Queue_t *xQueue;

/**
 * @brief Data type for a timer handle
 *
 * The Timer_t data type is used as a timer handle. The timer handle is created when xTimerCreate()
 * is called. For more information about timers, see xTimerCreate(). Timer_t should be declared
 * as xTimer.
 *
 * @sa xTimer
 * @sa xTimerCreate()
 * @attention The memory referenced by the timer handle must be freed by calling xTimerDelete()
 * @sa xTimerDelete()
 *
 */
typedef VOID_TYPE Timer_t;

/**
 * @brief Data type for a timer handle
 *
 * @sa Timer_t
 * @attention The memory referenced by the timer handle must be freed by calling xTimerDelete()
 * @sa xTimerDelete()
 *
 */
typedef Timer_t *xTimer;

/* START OF STRUCT TYPES */

/**
 * @brief Data structure for a direct to task notification
 *
 * The TaskNotification_t data structure is used by xTaskNotifyGive() and xTaskNotifyTake()
 * to send and receive direct to task notifications. Direct to task notifications are part of
 * the event-driven multitasking model. A direct to task notifcation may be received by event-driven
 * and co-operative tasks alike. However, the benefit of direct to task notifications may only be
 * realized by tasks scheduled as event-driven. In order to wait for a direct to task notification,
 * the task must be in a "waiting" state which is set by xTaskWait(). The TaskNotification_t type
 * should be declared as xTaskNotification.
 *
 * @sa xTaskNotification
 * @attention The memory allocated for the data struture must be freed by calling xMemFree()
 * @sa xMemFree()
 * @sa xTaskNotifyGive()
 * @sa xTaskNotifyTake()
 * @sa xTaskWait()
 *
 * @attention The notification value is *NOT* null terminated and thus Standard C Library string functions
 * such as strcmp(), strcpy() and strlen(), which expect a null terminated char array, must not be used to
 * manipulate the notification value
 *
 */
typedef struct TaskNotification_s {
  Base_t notificationBytes;                                  /**< The length in bytes of the notification value which cannot exceed CONFIG_NOTIFICATION_VALUE_BYTES */
  Char_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES]; /**< The notification value whose length is specified by the notification bytes member */
} TaskNotification_t;

/**
 * @brief Data structure for a direct to task notification
 *
 * @sa TaskNotification_t
 * @attention The memory allocated for the data struture must be freed by calling xMemFree()
 * @sa xMemFree()
 *
 */
typedef TaskNotification_t *xTaskNotification;

/**
 * @brief Data structure for task runtime statistics
 *
 * The TaskRunTimeStats_t data structure is used by xTaskGetTaskRunTimeStats() and xTaskGetAllRuntimeStats() to obtain
 * runtime statistics about a task. The TaskRunTimeStats_t type should be declared as xTaskRunTimeStats.
 *
 * @sa xTaskRunTimeStats
 * @sa xTaskGetTaskRunTimeStats()
 * @sa xTaskGetAllRunTimeStats()
 * @attention The memory allocated for the data struture must be freed by calling xMemFree()
 * @sa xMemFree()
 *
 */
typedef struct TaskRunTimeStats_s {
  Base_t id;            /**< The ID of the task referenced by the task handle */
  Ticks_t lastRunTime;  /**< The duration in ticks of the task's last runtime */
  Ticks_t totalRunTime; /**< The duration in ticks of the task's total runtime */
} TaskRunTimeStats_t;

/**
 * @brief Data structure for task runtime statistics
 *
 * @sa TaskRunTimeStats_t
 * @attention The memory allocated for the data struture must be freed by calling xMemFree()
 * @sa xMemFree()
 *
 */
typedef TaskRunTimeStats_t *xTaskRunTimeStats;

/**
 * @brief Data structure for memory region statistics
 *
 * The MemoryRegionStats_t data structure is used by xMemGetHeapStats() and xMemGetKernelStats()
 * to obtain statistics about either memory region. The MemoryRegionStats_t type should be declared
 * as xMemoryRegionStats.
 *
 * @sa xMemoryRegionStats
 * @sa xMemGetHeapStats()
 * @sa xMemGetKernelStats()
 * @attention The memory allocated for the data structure must be freed by calling xMemFree()
 * @sa xMemFree()
 *
 */
typedef struct MemoryRegionStats_s {
  Word_t largestFreeEntryInBytes;       /**< The largest free entry in bytes */
  Word_t smallestFreeEntryInBytes;      /**< The smallest free entry in bytes */
  Word_t numberOfFreeBlocks;            /**< The number of free blocks - see CONFIG_MEMORY_REGION_BLOCK_SIZE for block size in bytes */
  Word_t availableSpaceInBytes;         /**< The amount of free memory in bytes (i.e., numberOfFreeBlocks * CONFIG_MEMORY_REGION_BLOCK_SIZE) */
  Word_t successfulAllocations;         /**< Number of successful memory allocations */
  Word_t successfulFrees;               /**< Number of successful memory "frees" */
  Word_t minimumEverFreeBytesRemaining; /**< Lowest water lever since system initialization of free bytes of memory */
} MemoryRegionStats_t;

/**
 * @brief Data structure for memory region statistics
 *
 * @sa MemoryRegionStats_t
 * @attention The memory allocated for the data struture must be freed by calling xMemFree()
 * @sa xMemFree()
 *
 */
typedef MemoryRegionStats_t *xMemoryRegionStats;

/**
 * @brief Data structure for information about a task
 *
 * The TaskInfo_t structure is similar to xTaskRuntimeStats_t in that it contains runtime statistics for
 * a task. However, TaskInfo_t also contains additional details about a task such as its ASCII name
 * and state. The TaskInfo_t structure is returned by xTaskGetTaskInfo() and xTaskGetAllTaskInfo().
 * If only runtime statistics are needed, then TaskRunTimeStats_t should be used because of its smaller
 * memory footprint. The TaskInfo_t should be declared as xTaskInfo
 *
 * @sa xTaskInfo
 * @sa xTaskGetTaskInfo()
 * @sa xTaskGetAllTaskInfo()
 * @sa CONFIG_TASK_NAME_BYTES
 * @sa xMemFree()
 *
 * @attention The memory allocated for the data structure must be freed by calling xMemFree()
 *
 * @attention The task name is *NOT* null terminated and thus Standard C Library string functions
 * such as strcmp(), strcpy() and strlen(), which expect a null terminated char array, must not be used to
 * manipulate the task name
 *
 */
typedef struct TaskInfo_s {
  Base_t id;                           /**< The task identifier which is used by xTaskGetHandleById() to return the task handle */
  Char_t name[CONFIG_TASK_NAME_BYTES]; /**< The ASCII name of the task which is used by xTaskGetHandleByName() to return the task handle - this is *NOT* a null terminated char array */
  TaskState_t state;                   /**< The state the task is in which is one of four states specified in the TaskState_t enumerated data type */
  Ticks_t lastRunTime;                 /**< The duration in ticks of the task's last runtime */
  Ticks_t totalRunTime;                /**< The duration in ticks of the task's total runtime */
} TaskInfo_t;

/**
 * @brief Data structure for information about a task
 *
 * @sa TaskInfo_t
 * @attention The memory allocated for the data struture must be freed by calling xMemFree()
 * @sa xMemFree()
 *
 */
typedef TaskInfo_t *xTaskInfo;

/**
 * @brief Data structure for a queue message
 *
 * The QueueMessage_t stucture is used to store a queue message and is returned by xQueueReceive() and xQueuePeek().
 * The QueueMessage_t stucture should be declared as xQueueMessage.
 *
 * @sa xQueueMessage
 * @sa xQueueReceive()
 * @sa xQueuePeek()
 * @sa CONFIG_MESSAGE_VALUE_BYTES
 * @sa xMemFree()
 *
 * @attention The memory allocated for the data structure must be freed by calling xMemFree()
 *
 * @attention The message value is *NOT* null terminated and thus Standard C Library string functions
 * such as strcmp(), strcpy() and strlen(), which expect a null terminated char array, must not be used to
 * manipulate the message value
 *
 *
 */
typedef struct QueueMessage_s {
  Base_t messageBytes;                             /**< The number of bytes contained in the message value which cannot exceed CONFIG_MESSAGE_VALUE_BYTES */
  Char_t messageValue[CONFIG_MESSAGE_VALUE_BYTES]; /**< The ASCII queue message value - this is *NOT* a null terminated character array */
} QueueMessage_t;

/**
 * @brief Data structure for a queue message
 *
 * @sa QueueMessage_t
 * @attention The memory allocated for the data structure must be freed by calling xMemFree()
 * @sa xMemFree()
 *
 */
typedef QueueMessage_t *xQueueMessage;

/**
 * @brief Data structure for information about the HeliOS system
 *
 * The SystemInfo_t data structure is used to store information about the HeliOS system and
 * is returned by xSystemGetSystemInfo(). The SystemInfo_t structure should be declared as
 * xSystemInfo.
 *
 * @sa xSystemInfo
 * @sa xSystemGetSystemInfo()
 * @sa OS_PRODUCT_NAME_SIZE
 * @sa xMemFree()
 *
 * @attention The memory allocated for the data structure must be freed by calling xMemFree()
 *
 * @attention The product name is *NOT* null terminated and thus Standard C Library string functions
 * such as strcmp(), strcpy() and strlen(), which expect a null terminated char array, must not be used to
 * manipulate the product name
 *
 */
typedef struct SystemInfo_s {
  Char_t productName[OS_PRODUCT_NAME_SIZE]; /**< The ASCII product name of the operating system (always "HeliOS)") */
  Base_t majorVersion;                      /**< The SemVer major version number of HeliOS */
  Base_t minorVersion;                      /**< The SemVer minor version number of HeliOS */
  Base_t patchVersion;                      /**< The SemVer patch version number of HeliOS */
  Base_t numberOfTasks;                     /**< The number of tasks regardless of their state */
} SystemInfo_t;

/**
 * @brief Data structure for information about the HeliOS system
 *
 * @sa SystemInfo_t
 * @attention The memory allocated for the data structure must be freed by calling xMemFree()
 * @sa xMemFree()
 *
 */
typedef SystemInfo_t *xSystemInfo;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System call to register a device driver
 *
 * The xDeviceRegisterDevice() system call, as part of the HeliOS device driver model, registers a
 * device driver with the HeliOS kernel. This system call must be made before a device driver can
 * be called by xDeviceRead(), xDeviceWrite(), etc. If the device driver is successfully registered
 * with the kernel, xDeviceRegisterDevice() will return RETURN_SUCCESS. Once a device is registered
 * it cannot be un-registered - it can only be placed in a suspended state which is done by calling
 * xDeviceConfigDevice(). However, as with most aspects of the device driver model in HeliOS, it is
 * important to note that the implementation of the device state and mode is up to the device driver
 * author. A quick word about device driver universal identifiers (uid). A device driver uid *MUST* be
 * a globally unique identifier. No two device drivers in the same application can share the same uid.
 * This is best achieved by ensuring the device driver author selects a uid for his device driver
 * that is not in use by another device driver. A device driver template and pre-packaged device drivers
 * can be found in /drivers.
 *
 * @param device_self_register_ A pointer to the self registration function for the device driver
 * @return xBase If the device driver is successfully registered with the kernel, xDeviceRegisterDevice()
 * returns RETURN_SUCCESS. Otherwise RETURN_FAILURE is returned.
 */
xBase xDeviceRegisterDevice(xBase (*device_self_register_)());

/**
 * @brief System call to check if a device is available
 * 
 * The xDeviceIsAvailable() system call checks to see if a device driver is "available", generally
 * for a read, write or read/write operation. What "available" means is up to the device driver author and
 * may change based on what mode the device driver is in. For example, if the device driver is in read-only
 * mode, then xDeviceIsAvailable() may return "true" if data is ready to be read from the device. A device
 * driver template and pre-packaged device drivers can be found in /drivers.
 *
 * @param uid_ The universal identifier of the driver
 * @return xBase If the device driver is "available", then "true" is returned. Otherwise "false" is returned.
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
 * @return xTaskState
 */
xTaskState xTaskGetTaskState(const xTask task_);

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
 * @return xSchedulerState
 */
xSchedulerState xTaskGetSchedulerState(void);

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