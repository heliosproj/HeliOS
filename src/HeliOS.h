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
 * @brief Enumerated data type for task states.
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
  TaskStateError,     /**< Returned by xTaskGetTaskState() when the task cannot be found. */
  TaskStateSuspended, /**< State a task is in when it is first created OR after calling xTaskSuspend() - tasks in the TaskStateSuspended state will not be scheduled for execution. */
  TaskStateRunning,   /**< State a task is in after calling xTaskResume() - tasks in the TaskStateRunning state will be scheduled co-operatively. */
  TaskStateWaiting    /**< State a task is in after calling xTaskWait() - tasks in the TaskStateWaiting state will be scheduled as event driven. */
} TaskState_t;

/**
 * @brief Enumerated data type for task states.
 *
 * @sa TaskState_t
 *
 */
typedef TaskState_t xTaskState;

/**
 * @brief Enumerated data type for scheduler state.
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
  SchedulerStateError,     /**< Not used - reserved for future use. */
  SchedulerStateSuspended, /**< State the scheduler is in after calling xTaskSuspendAll() - xTaskStartScheduler() will stop scheduling tasks for execution and relinquish control when xTaskSuspendAll() is called. */
  SchedulerStateRunning    /**< State the scheduler is in after calling xTaskResumeAll() - xTaskStartScheduler() will continue to schedule tasks for execution until xTaskSuspendAll() is called. */
} SchedulerState_t;

/**
 * @brief Enumerated data type for the scheduler state.
 *
 * @sa SchedulerState_t
 *
 */
typedef SchedulerState_t xSchedulerState;

/* START OF BASIC TYPES */

/**
 * @brief Data type for the task paramater.
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
 * @brief Data type for the task paramater.
 *
 * @sa TaskParm_t
 *
 */
typedef TaskParm_t *xTaskParm;

/**
 * @brief Data type for the base type.
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
 * @brief Data type for the base type.
 *
 * @sa Base_t
 *
 */
typedef Base_t xBase;

/**
 * @brief Data type for an 8-bit wide byte.
 *
 * The Byte_t type is an 8-bit wide data type and is guaranteed to always be
 * 8-bits wide. Byte_t should be declared as xByte.
 *
 * @sa xByte
 *
 */
typedef UINT8_TYPE Byte_t;

/**
 * @brief Data type for an 8-bit wide byte.
 *
 * @sa Byte_t
 *
 */
typedef Byte_t xByte;

/**
 * @brief Data type for a pointer to an address.
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
 * @brief Data type for a pointer to an address.
 *
 * @sa Addr_t
 *
 */
typedef Addr_t *xAddr;

/**
 * @brief Data type for the storage requirements of an object in memory.
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
 * @brief Data type for the storage requirements of an object in memory.
 *
 * @sa Size_t
 *
 */
typedef Size_t xSize;

/**
 * @brief Data type for a 16-bit half word.
 *
 * The HalfWord_t type is a 16-bit wide data type and is guaranteed to always be
 * 16-bits wide. HalfWord_t should be declared as xHalfWord.
 *
 * @sa xHalfWord
 *
 */
typedef UINT16_TYPE HalfWord_t;

/**
 * @brief Data type for a 16-bit half word.
 *
 * @sa HalfWord_t
 *
 */
typedef HalfWord_t xHalfWord;

/**
 * @brief Data type for a 32-bit word.
 *
 * The Word_t type is a 32-bit wide data type and is guaranteed to always be
 * 32-bits wide. Word_t should be declared as xWord.
 *
 * @sa xWord
 *
 */
typedef UINT32_TYPE Word_t;

/**
 * @brief Data type for a 32-bit word.
 *
 * @sa Word_t
 *
 */
typedef Word_t xWord;

/**
 * @brief Data type for system ticks.
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
 * @brief Data type for system ticks.
 *
 * @sa Ticks_t
 *
 */
typedef Ticks_t xTicks;

/**
 * @brief Data type for a character.
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
 * @brief Data type for a character.
 *
 * @sa Data_t
 *
 */
typedef Char_t xChar;

/* START OF HANDLE TYPES */

/**
 * @brief Data type for a device handle.
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
 * @brief Data type for a device handle.
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
 * @attention The memory referenced by the task handle must be freed by calling xTaskDelete().
 * @sa xTaskDelete()
 *
 */
typedef VOID_TYPE Task_t;

/**
 * @brief Data type for a task handle.
 *
 * @sa Task_t
 * @attention The memory referenced by the task handle must be freed by calling xTaskDelete().
 * @sa xTaskDelete()
 *
 */
typedef Task_t *xTask;

/**
 * @brief Data type for a stream buffer handle.
 *
 * The StreamBuffer_t data type is used as a stream buffer handle. The stream buffer
 * handle is created when xStreamCreate() is called. For more information about
 * stream buffers, see xStreamCreate(). Stream_t should be declared as xStream.
 *
 * @sa xStream
 * @sa xStreamCreate()
 * @attention The memory referenced by the stream buffer handle must be freed by calling xStreamDelete().
 * @sa xStreamDelete()
 *
 */
typedef VOID_TYPE StreamBuffer_t;

/**
 * @brief Data type for a stream buffer handle.
 *
 * @sa StreamBuffer_t
 * @attention The memory referenced by the stream buffer handle must be freed by calling xStreamDelete().
 * @sa xStreamDelete()
 *
 */
typedef StreamBuffer_t *xStreamBuffer;

/**
 * @brief Data type for a queue handle.
 *
 * The Queue_t data type is used as a queue handle. The queue handle is created
 * when xQueueCreate() is called. For more information about queues, see xQueueCreate().
 * Queue_t should be declared as xQueue.
 *
 * @sa xQueue
 * @sa xQueueCreate()
 * @attention The memory referenced by the queue handle must be freed by calling xQueueDelete().
 * @sa xQueueDelete()
 *
 */
typedef VOID_TYPE Queue_t;

/**
 * @brief Data type for a queue handle.
 *
 * @sa Queue_t
 * @attention The memory referenced by the queue handle must be freed by calling xQueueDelete().
 * @sa xQueueDelete()
 *
 */
typedef Queue_t *xQueue;

/**
 * @brief Data type for a timer handle.
 *
 * The Timer_t data type is used as a timer handle. The timer handle is created when xTimerCreate()
 * is called. For more information about timers, see xTimerCreate(). Timer_t should be declared
 * as xTimer.
 *
 * @sa xTimer
 * @sa xTimerCreate()
 * @attention The memory referenced by the timer handle must be freed by calling xTimerDelete().
 * @sa xTimerDelete()
 *
 */
typedef VOID_TYPE Timer_t;

/**
 * @brief Data type for a timer handle.
 *
 * @sa Timer_t
 * @attention The memory referenced by the timer handle must be freed by calling xTimerDelete().
 * @sa xTimerDelete()
 *
 */
typedef Timer_t *xTimer;

/* START OF STRUCT TYPES */

/**
 * @brief Data structure for a direct to task notification.
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
 * @attention The memory allocated for the data struture must be freed by calling xMemFree().
 * @sa xMemFree()
 * @sa xTaskNotifyGive()
 * @sa xTaskNotifyTake()
 * @sa xTaskWait()
 *
 * @attention The notification value is *NOT* null terminated and thus Standard C Library string functions
 * such as strcmp(), strcpy() and strlen(), which expect a null terminated char array, must not be used to
 * manipulate the notification value.
 *
 */
typedef struct TaskNotification_s {
  Base_t notificationBytes;                                  /**< The length in bytes of the notification value which cannot exceed CONFIG_NOTIFICATION_VALUE_BYTES. */
  Char_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES]; /**< The notification value whose length is specified by the notification bytes member. */
} TaskNotification_t;

/**
 * @brief Data structure for a direct to task notification.
 *
 * @sa TaskNotification_t
 * @attention The memory allocated for the data struture must be freed by calling xMemFree().
 * @sa xMemFree()
 *
 */
typedef TaskNotification_t *xTaskNotification;

/**
 * @brief Data structure for task runtime statistics.
 *
 * The TaskRunTimeStats_t data structure is used by xTaskGetTaskRunTimeStats() and xTaskGetAllRuntimeStats() to obtain
 * runtime statistics about a task. The TaskRunTimeStats_t type should be declared as xTaskRunTimeStats.
 *
 * @sa xTaskRunTimeStats
 * @sa xTaskGetTaskRunTimeStats()
 * @sa xTaskGetAllRunTimeStats()
 * @attention The memory allocated for the data struture must be freed by calling xMemFree().
 * @sa xMemFree()
 *
 */
typedef struct TaskRunTimeStats_s {
  Base_t id;            /**< The ID of the task referenced by the task handle. */
  Ticks_t lastRunTime;  /**< The duration in ticks of the task's last runtime. */
  Ticks_t totalRunTime; /**< The duration in ticks of the task's total runtime. */
} TaskRunTimeStats_t;

/**
 * @brief Data structure for task runtime statistics.
 *
 * @sa TaskRunTimeStats_t
 * @attention The memory allocated for the data struture must be freed by calling xMemFree().
 * @sa xMemFree()
 *
 */
typedef TaskRunTimeStats_t *xTaskRunTimeStats;

/**
 * @brief Data structure for memory region statistics.
 *
 * The MemoryRegionStats_t data structure is used by xMemGetHeapStats() and xMemGetKernelStats()
 * to obtain statistics about either memory region. The MemoryRegionStats_t type should be declared
 * as xMemoryRegionStats.
 *
 * @sa xMemoryRegionStats
 * @sa xMemGetHeapStats()
 * @sa xMemGetKernelStats()
 * @attention The memory allocated for the data structure must be freed by calling xMemFree().
 * @sa xMemFree()
 *
 */
typedef struct MemoryRegionStats_s {
  Word_t largestFreeEntryInBytes;       /**< The largest free entry in bytes. */
  Word_t smallestFreeEntryInBytes;      /**< The smallest free entry in bytes. */
  Word_t numberOfFreeBlocks;            /**< The number of free blocks - see CONFIG_MEMORY_REGION_BLOCK_SIZE for block size in bytes. */
  Word_t availableSpaceInBytes;         /**< The amount of free memory in bytes (i.e., numberOfFreeBlocks * CONFIG_MEMORY_REGION_BLOCK_SIZE). */
  Word_t successfulAllocations;         /**< Number of successful memory allocations. */
  Word_t successfulFrees;               /**< Number of successful memory "frees". */
  Word_t minimumEverFreeBytesRemaining; /**< Lowest water lever since system initialization of free bytes of memory. */
} MemoryRegionStats_t;

/**
 * @brief Data structure for memory region statistics.
 *
 * @sa MemoryRegionStats_t
 * @attention The memory allocated for the data struture must be freed by calling xMemFree().
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
 * @attention The memory allocated for the data structure must be freed by calling xMemFree().
 *
 * @attention The task name is *NOT* null terminated and thus Standard C Library string functions
 * such as strcmp(), strcpy() and strlen(), which expect a null terminated char array, must not be used to
 * manipulate the task name.
 *
 */
typedef struct TaskInfo_s {
  Base_t id;                           /**< The task identifier which is used by xTaskGetHandleById() to return the task handle. */
  Char_t name[CONFIG_TASK_NAME_BYTES]; /**< The ASCII name of the task which is used by xTaskGetHandleByName() to return the task handle - this is *NOT* a null terminated char array. */
  TaskState_t state;                   /**< The state the task is in which is one of four states specified in the TaskState_t enumerated data type. */
  Ticks_t lastRunTime;                 /**< The duration in ticks of the task's last runtime. */
  Ticks_t totalRunTime;                /**< The duration in ticks of the task's total runtime. */
} TaskInfo_t;

/**
 * @brief Data structure for information about a task.
 *
 * @sa TaskInfo_t
 * @attention The memory allocated for the data struture must be freed by calling xMemFree().
 * @sa xMemFree()
 *
 */
typedef TaskInfo_t *xTaskInfo;

/**
 * @brief Data structure for a queue message.
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
 * @attention The memory allocated for the data structure must be freed by calling xMemFree().
 *
 * @attention The message value is *NOT* null terminated and thus Standard C Library string functions
 * such as strcmp(), strcpy() and strlen(), which expect a null terminated char array, must not be used to
 * manipulate the message value.
 *
 *
 */
typedef struct QueueMessage_s {
  Base_t messageBytes;                             /**< The number of bytes contained in the message value which cannot exceed CONFIG_MESSAGE_VALUE_BYTES. */
  Char_t messageValue[CONFIG_MESSAGE_VALUE_BYTES]; /**< The ASCII queue message value - this is *NOT* a null terminated character array. */
} QueueMessage_t;

/**
 * @brief Data structure for a queue message.
 *
 * @sa QueueMessage_t
 * @attention The memory allocated for the data structure must be freed by calling xMemFree().
 * @sa xMemFree()
 *
 */
typedef QueueMessage_t *xQueueMessage;

/**
 * @brief Data structure for information about the HeliOS system.
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
 * @attention The memory allocated for the data structure must be freed by calling xMemFree().
 *
 * @attention The product name is *NOT* null terminated and thus Standard C Library string functions
 * such as strcmp(), strcpy() and strlen(), which expect a null terminated char array, must not be used to
 * manipulate the product name.
 *
 */
typedef struct SystemInfo_s {
  Char_t productName[OS_PRODUCT_NAME_SIZE]; /**< The ASCII product name of the operating system (always "HeliOS"). */
  Base_t majorVersion;                      /**< The SemVer major version number of HeliOS. */
  Base_t minorVersion;                      /**< The SemVer minor version number of HeliOS. */
  Base_t patchVersion;                      /**< The SemVer patch version number of HeliOS. */
  Base_t numberOfTasks;                     /**< The number of tasks regardless of their state. */
} SystemInfo_t;

/**
 * @brief Data structure for information about the HeliOS system.
 *
 * @sa SystemInfo_t
 * @attention The memory allocated for the data structure must be freed by calling xMemFree().
 * @sa xMemFree()
 *
 */
typedef SystemInfo_t *xSystemInfo;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System call to register a device driver.
 *
 * The xDeviceRegisterDevice() system call, as part of the HeliOS device driver model, registers a
 * device driver with the HeliOS kernel. This system call must be made before a device driver can
 * be called by xDeviceRead(), xDeviceWrite(), etc. If the device driver is successfully registered
 * with the kernel, xDeviceRegisterDevice() will return RETURN_SUCCESS. Once a device is registered
 * it cannot be un-registered - it can only be placed in a suspended state which is done by calling
 * xDeviceConfigDevice(). However, as with most aspects of the device driver model in HeliOS, it is
 * important to note that the implementation of the device state and mode is up to the device driver
 * author. A quick word about device driver unique identifiers (uid). A device driver uid *MUST* be
 * a globally unique identifier. No two device drivers in the same application can share the same uid.
 * This is best achieved by ensuring the device driver author selects a uid for his device driver
 * that is not in use by another device driver. A device driver template and pre-packaged device drivers
 * can be found in /drivers.
 *
 * @param device_self_register_ A pointer to the self registration function for the device driver.
 * @return xBase If the device driver is successfully registered with the kernel, xDeviceRegisterDevice()
 * returns RETURN_SUCCESS. Otherwise RETURN_FAILURE is returned..
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
 * @param uid_ The unique identifier of the device driver.
 * @return xBase If the device driver is "available", then "true" is returned. Otherwise "false" is returned.
 */
xBase xDeviceIsAvailable(const xHalfWord uid_);

/**
 * @brief System call to write fixed length data to a device.
 *
 * The xDeviceSimpleWrite() system call will write fixed length (one word) data to a device driver. Whether the
 * data is written is dependent on the device driver mode, state and implementation of these features by the
 * device driver author. A device driver template and pre-packaged device drives can be found in /drivers.
 *
 * @param uid_ The unique identifier of the device driver
 * @param data_ A pointer to the data to be written to the device - because xDeviceSimpleWrite() uses fixed length data,
 * the pointer must reference a word of memory and the memory *MUST* be located in the heap memory region.
 * @return xBase If the write operation was successful, RETURN_SUCCESS is returned. Otherwise RETURN_FAILURE is returned.
 */
xBase xDeviceSimpleWrite(const xHalfWord uid_, xWord *data_);

/**
 * @brief System call to write variable length data to a device.
 *
 * The xDeviceWrite() system call will write variable length data to a device driver. Whether the data is written
 * is dependent on the device driver mode, state and implementation of these features by the device driver author. A
 * device driver template and pre-packaged drivers an be found in /drivers.
 *
 * @param uid_ The unique identifier of the device driver.
 * @param size_ The length (i.e., size) of the data to be written to the device.
 * @param data_ A pointer to the data to be written to the device - because xDeviceWrite uses variable length data,
 * the pointer must reference size_ bytes of memory and the memory *MUST* be located in the heap memory region.
 * @return xBase If the write operation was successful, RETURN_SUCCESS is returned. Otherwise RETURN_FAILURE is returned.
 */
xBase xDeviceWrite(const xHalfWord uid_, xSize *size_, xAddr data_);

/**
 * @brief System call to read fixed length data from a device.
 *
 * The xDeviceSimpleRead() system call will read fixed length (one word) data from a device driver. Whether the data
 * is read is dependent on the device driver mode, state and implementation of these features by the device driver
 * author. A device driver template and pre-packaged device drivers can be found in /drivers.
 *
 * @param uid_ The unique identifier of the device driver.
 * @param data_ A pointer to the data buffer to read the data into from the device - because xDeviceSimpleRead() uses fixed
 * length data, the pointer must reference a word of memory and the memory *MUST* be located in the heap memory region.
 * @return xBase If the read operation was successful, RETURN_SUCCESS is returned. Otherwise RETURN_FAILURE is returned.
 */
xBase xDeviceSimpleRead(const xHalfWord uid_, xWord *data_);

/**
 * @brief System call to read variable length data from a device.
 *
 * The xDeviceRead() system call will read variable length data from a device driver. Whether the data is read
 * is dependent on the device driver mode, state and implementation of these features by the device driver author. A
 * device driver template and pre-packaged drivers can be found in /drivers.
 *
 * @param uid_ The unique identifier of the device driver.
 * @param size_ The number of bytes read from the device.
 * @param data_ A pointer to the data buffer to read the data into from the device - because xDeviceRead() uses variable length data,
 * the pointer must reference size_ bytes of memory and the memory *MUST* be located in the heap memory region.
 * @return xBase If the read operation was successful, RETURN_SUCCESS is returned. Otherwise RETURN_FAILURE is returned.
 */
xBase xDeviceRead(const xHalfWord uid_, xSize *size_, xAddr data_);

/**
 * @brief System call to initialize a device driver and its device.
 *
 * The xDeviceInitDevice() system call will initialize the device driver and its device. Like most aspects of the HeliOS device driver model,
 * the implementation of this feature is dependent on the author of the device driver. Some device drivers may not implement this
 * feature of the device driver model at all. A device driver template and pre-packaged drivers can be found in /drivers.
 *
 * @param uid_ The unique identifier of the device driver.
 * @return xBase If the initialization of the device driver was successful, RETURN_SUCCESS is returned. Otherwise RETURN_FAILURE is returned.
 */
xBase xDeviceInitDevice(const xHalfWord uid_);

/**
 * @brief System call to configure a device driver and its device.
 *
 * The xDeviceConfigDevice() system call will configure the device driver and its device. Like most aspects of the HeliOS device driver model,
 * the implementation of this feature is dependent on the author of the device driver. Some device drivers may not implement this
 * feature of the device driver model at all. A device driver template and pre-packaged drivers can be found in /drivers.
 *
 * @param uid_ The unique identifier of the device driver.
 * @param size_ The number of bytes (i.e., size) of the device configuration data structure.
 * @param config_ A pointer to the size_ bytes of memory occupied by the configuration data structure - the memory *MUST* be located
 * in the heap memory region.
 * @return xBase If configuration of the device driver was successful, RETURN_SUCCESS is returned. Otherwise RETURN_FAILURE is returned.
 */
xBase xDeviceConfigDevice(const xHalfWord uid_, xSize *size_, xAddr config_);

/**
 * @brief System call to allocate memory from the heap.
 *
 * The xMemAlloc() system call allocates memory from the heap for HeliOS system
 * calls and end-user tasks. The size of the heap, in bytes, is dependent on the
 * CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS and CONFIG_MEMORY_REGION_BLOCK_SIZE settings. xMemAlloc()
 * functions similarly to calloc() in that it clears the memory it allocates.
 *
 * @sa CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS
 * @sa CONFIG_MEMORY_REGION_BLOCK_SIZE
 * @sa xMemFree()
 *
 * @param size_ The amount (size) of the memory to be allocated from the heap in bytes.
 * @return xAddr If successful, xMemAlloc() returns the address of the newly allocated memory.
 * If unsuccessful, the system call will return null.
 *
 * @note HeliOS technically does not allocate memory from what is traditionally heap memory.
 * HeliOS uses a private "heap" which is actually static memory allocated at compile time. This
 * is done to maintain MISRA C:2012 compliance since standard library functions like malloc(),
 * calloc() and free() are not permitted.
 *
 */
xAddr xMemAlloc(const xSize size_);

/**
 * @brief System call to free memory allocated from the heap.
 *
 * The xMemFree() system call will free heap memory allocated by
 * xMemAlloc() and other HeliOS system calls such as xSystemGetSystemInfo().
 *
 * @sa xMemAlloc()
 *
 * @param addr_ The address of the allocated heap memory to be freed.
 *
 * @warning xMemFree() cannot be used to free memory allocated for kernel objects.
 * Memory allocated by xTaskCreate(), xTimerCreate() or xQueueCreate() must
 * be freed by their respective delete system calls (e.g., xTaskDelete()).
 */
void xMemFree(const volatile xAddr addr_);

/**
 * @brief System call to return the amount of allocated heap memory.
 *
 * The xMemGetUsed() system call returns the amount of heap memory, in bytes,
 * that is currently allocated. Calls to xMemAlloc() increases and xMemFree()
 * decreases the amount of memory in use.
 *
 * @return xSize The amount of memory currently allocated in bytes. If no heap
 * memory is currently allocated, xMemGetUsed() will return zero.
 *
 * @note xMemGetUsed() returns the amount of heap memory that is currently
 * allocated to end-user objects AND kernel objects. However, only end-user
 * objects may be freed using xMemFree(). Kernel objects must be freed using
 * their respective delete system call (e.g., xTaskDelete()).
 */
xSize xMemGetUsed(void);

/**
 * @brief System call to return the amount of heap memory allcoated for a given address.
 *
 * The xMemGetSize() system call returns the amount of heap memory in bytes that
 * is currently allocated to a specific address. If the address is null or invalid,
 * xMemGetSize() will return zero bytes.
 *
 * @param addr_ The address of the allocated heap memory to obtain the size of the
 * memory, in bytes, that is allocated.
 * @return xSize The amount of memory currently allocated to the specific address in bytes. If
 * the address is invalid or null, xMemGetSize() will return zero.
 *
 * @note If the address addr_ points to a structure that, for example, is 48 bytes in size
 * base on sizeof(), xMemGetSize() will return the number of bytes allocated by the block(s)
 * that contain the structure. Assuming the default block size of 32, a 48 byte structure would require
 * TWO blocks so xMemGetSize() would return 64 - not 48. xMemGetSize() also checks the health of the
 * heap and will return zero if it detects a consistency issue with the heap. Thus, xMemGetSize()
 * can be used to validate addresses before the objects they reference are accessed.
 */
xSize xMemGetSize(const volatile xAddr addr_);

/**
 * @brief System call to obtain statistics on the heap.
 *
 * The xMemGetHeapStats() system call will return statistics about the heap
 * so the end-user can better understand the state of the heap.
 *
 * @sa xMemoryRegionStats
 *
 * @return xMemoryRegionStats Returns the xMemoryRegionStats structure or null
 * if unsuccessful.
 *
 * @warning The memory allocated by xMemGetHeapStats() must be freed by xMemFree().
 */
xMemoryRegionStats xMemGetHeapStats(void);

/**
 * @brief System call to obtain statistics on the kernel memory region.
 *
 * The xMemGetKernelStats() system call will return statistics about the kernel
 * memory region so the end-user can better understand the state of kernel
 * memory.
 *
 * @sa xMemoryRegionStats
 *
 * @return xMemoryRegionStats Returns the xMemoryRegionStats structure or null
 * if unsuccessful.
 *
 * @warning The memory allocated by xMemGetKernelStats() must be freed by xMemFree().
 */
xMemoryRegionStats xMemGetKernelStats(void);

/**
 * @brief System call to create a new message queue.
 *
 * The xQueueCreate() system call creates a message queue for inter-task
 * communication.
 *
 * @sa xQueue
 * @sa xQueueDelete()
 * @sa CONFIG_QUEUE_MINIMUM_LIMIT
 *
 * @param limit_ The message limit for the queue. When this number is reach, the queue
 * is considered full and xQueueSend() will fail. The minimum limit for queues is dependent
 * on the  setting CONFIG_QUEUE_MINIMUM_LIMIT.
 * @return xQueue A queue is returned if successful, otherwise null is returned if unsuccessful.
 *
 * @warning The message queue memory can only be freed by xQueueDelete().
 */
xQueue xQueueCreate(const xBase limit_);

/**
 * @brief System call to delete a message queue.
 *
 * The xQueueDelete() system call will delete a message queue created by xQueueCreate(). xQueueDelete()
 * will delete a queue regardless of how many messages the queue contains at the time xQueueDelete()
 * is called. Any messages the message queue contains will be deleted in the process of deleting the
 * message queue.
 *
 * @sa xQueueCreate()
 *
 * @param queue_ The queue to be deleted.
 */
void xQueueDelete(xQueue queue_);

/**
 * @brief System call to get the length of the message queue.
 *
 * The xQueueGetLength() system call returns the length of the queue (the number of messages
 * the queue currently contains).
 *
 * @param queue_ The queue to return the length of.
 * @return xBase The number of messages in the queue. If unsuccessful or if the queue is empty,
 * xQueueGetLength() returns zero.
 */
xBase xQueueGetLength(const xQueue queue_);

/**
 * @brief System call to check if the message queue is empty.
 *
 * The xQueueIsEmpty() system call will return a true or false dependent on whether the queue is
 * empty (message queue length is zero) or contains one or more messages.
 *
 * @param queue_ The queue to determine whether it is empty.
 * @return xBase True if the queue is empty. False if the queue has one or more messages. xQueueIsQueueEmpty()
 * will also return false if the queue parameter is invalid.
 */
xBase xQueueIsQueueEmpty(const xQueue queue_);

/**
 * @brief System call to check if the message queue is full.
 *
 * The xQueueIsFull() system call will return a true or false dependent on whether the queue is
 * full or contains zero messages. A queue is considered full if the number of messages in the queue
 * is equal to the queue's length limit.
 *
 * @param queue_ The queue to determine whether it is full.
 * @return xBase True if the queue is full. False if the queue has zero. xQueueIsQueueFull()
 * will also return false if the queue parameter is invalid.
 */
xBase xQueueIsQueueFull(const xQueue queue_);

/**
 * @brief System call to check if there are message queue messages waiting.
 *
 * The xQueueMessageWaiting() system call returns true or false dependent on whether
 * there is at least one message waiting. The message queue does not have to be full to return true.
 *
 * @param queue_ The queue to determine whether one or more messages are waiting.
 * @return xBase True if one or more messages are waiting. False if there are no
 * messages waiting of the queue parameter is invalid.
 */
xBase xQueueMessagesWaiting(const xQueue queue_);

/**
 * @brief System call to send a message using a message queue.
 *
 * The xQueueSend() system call will send a message using the specified message queue. The size of the message
 * value is passed in the message bytes parameter. The maximum message value size in bytes is dependent
 * on the CONFIG_MESSAGE_VALUE_BYTES setting.
 *
 * @sa CONFIG_MESSAGE_VALUE_BYTES
 * @sa xQueuePeek()
 * @sa xQueueReceive()
 *
 * @param queue_ The queue to send the message to.
 * @param messageBytes_ The number of bytes contained in the message value. The number of bytes must be greater than
 * zero and less than or equal to the setting CONFIG_MESSAGE_VALUE_BYTES.
 * @param messageValue_ The message value. If the message value is greater than defined in CONFIG_MESSAGE_VALUE_BYTES,
 * only the number of bytes defined in CONFIG_MESSAGE_VALUE_BYTES will be copied into the message value. The message
 * value is NOT a null terminated string.
 * @return xBase xQueueSend() returns RETURN_SUCCESS if the message was sent to the queue successfully. Otherwise
 * RETURN_FAILURE if unsuccessful.
 */
xBase xQueueSend(xQueue queue_, const xBase messageBytes_, const xChar *messageValue_);

/**
 * @brief System call to peek at the next message in a message queue.
 *
 * The xQueuePeek() system call will return the next message in the specified message queue without
 * dropping the message.
 *
 * @sa xQueueMessage
 * @sa xMemFree()
 *
 * @param queue_ The queue to return the next message from.
 * @return xQueueMessage The next message in the queue. If the queue is empty or the queue
 * parameter is invalid, xQueuePeek() will return null.
 *
 * @warning The memory allocated by xQueuePeek() must be freed by xMemFree().
 */
xQueueMessage xQueuePeek(const xQueue queue_);

/**
 * @brief System call to drop the next message in a message queue.
 *
 * The xQueueDropMessage() system call will drop the next message from the message queue without
 * returning the message.
 *
 * @param queue_ The queue to drop the next message from.
 */
void xQueueDropMessage(xQueue queue_);

/**
 * @brief System call to receive the next message in the message queue.
 *
 * The xQueueReceive() system call will return the next message in the message queue and drop
 * it from the message queue.
 *
 * @sa xQueueMessage
 * @sa xMemFree()
 *
 * @param queue_ The queue to return the next message from.
 * @return xQueueMessage The message returned from the queue. If the queue is empty
 * of the queue parameter is invalid, xQueueReceive() will return null.
 *
 * @warning The memory allocated by xQueueReceive() must be freed by xMemFree().
 */
xQueueMessage xQueueReceive(xQueue queue_);

/**
 * @brief System call to LOCK the message queue.
 *
 * The xQueueLockQueue() system call will lock the message queue. Locking a message queue
 * will prevent xQueueSend() from sending messages to the queue.
 *
 * @param queue_ The queue to lock.
 */
void xQueueLockQueue(xQueue queue_);

/**
 * @brief System call to UNLOCk the message queue.
 *
 * The xQueueUnLockQueue() system call will unlock the message queue. Unlocking a message queue
 * will allow xQueueSend() to send messages to the queue.
 *
 * @param queue_ The queue to unlock.
 */
void xQueueUnLockQueue(xQueue queue_);

/**
 * @brief The xStreamCreate() system call will create a new stream buffer.
 *
 * The xStreamCreate() system call will create a new stream buffer. The
 * memory for a stream buffer is allocated from kernel memory and therefor
 * cannot be freed by calling xMemFree().
 *
 * @return xStreamBuffer The newly created stream buffer.
 *
 * @warning The stream buffer created by xStreamCreate() must
 * be freed by calling xStreamDelete().
 */
xStreamBuffer xStreamCreate(void);

/**
 * @brief The xStreamDelete() system call will delete a stream buffer
 *
 * The xStreamDelete() system call will delete a stream buffer and
 * free its memory. Once a stream buffer is deleted, it can not
 * be written to or read from.
 *
 * @param stream_ The stream buffer to operate on.
 */
void xStreamDelete(const xStreamBuffer stream_);

/**
 * @brief The xStreamSend() system call will write one byte to the stream buffer
 *
 * The xStreamSend() system call will write one byte to the stream buffer. If the
 * stream buffer's length is equal to CONFIG_STREAM_BUFFER_BYTES (i.e., full) then
 * the byte will not be written to the stream buffer and xStreamSend() will return
 * RETURN_FAILURE.
 *
 * @param stream_ The stream buffer to operate on.
 * @param byte_ The byte to be sent to the stream buffer.
 * @return xBase Returns RETURN_SUCCESS if the byte was successfully written to the
 * stream buffer. Otherwise, returns RETURN_FAILURE.
 */
xBase xStreamSend(xStreamBuffer stream_, const xByte byte_);

/**
 * @brief The xStreamReceive() system call will return the contents of the stream buffer.
 *
 * The xStreamReceive() system call will return the contents of the stream buffer. The
 * contents are returned as a byte array whose length is known by the bytes_ paramater.
 * Because the byte array is stored in the heap, it must be freed by calling xMemFree().
 *
 * @param stream_ The stream to operate on.
 * @param bytes_ The number of bytes returned (i.e., length of the byte array) by xStreamReceive().
 * @return xByte* The byte array containing the contents of the stream buffer.
 *
 * @warning The byte array returned by xStreamReceive() must be freed by calling xMemFree().
 */
xByte *xStreamReceive(const xStreamBuffer stream_, xHalfWord *bytes_);

/**
 * @brief The xStreamBytesAvailable() system call returns the length of the stream buffer.
 *
 * The xStreamBytesAvailable() system call will return the length of the stream buffer
 * in bytes (i.e., bytes available to be received by xStreamReceive()).
 *
 * @param stream_ The stream to operate on.
 * @return xHalfWord The length of the stream buffer in bytes.
 */
xHalfWord xStreamBytesAvailable(const xStreamBuffer stream_);

/**
 * @brief The xStreamReset() system call will reset a stream buffer.
 *
 * The xStreamRest() system call will clear the contents of the stream buffer
 * and reset its length to zero.
 *
 * @param stream_ The stream buffer to operate on.
 */
void xStreamReset(const xStreamBuffer stream_);

/**
 * @brief The xStreamIsEmpty() system call returns true if the stream buffer is empty.
 *
 * The xStreamIsEmpty() system call is used to determine if the stream buffer is empty.
 * A stream buffer is considered empty when it's length is equal to zero. If the buffer
 * is greater than zero in length, xStreamIsEmpty() will return false.
 *
 * @param stream_ The stream buffer to operate on.
 * @return xBase Returns true if the stream buffer length is equal to zero
 * in length, otherwise xStreamIsEmpty() will return false.
 */
xBase xStreamIsEmpty(const xStreamBuffer stream_);

/**
 * @brief The xStreamIsFull() system call returns true if the stream buffer is full
 *
 * The xStreamIsFull() system call is used to determine if the stream buffer is full.
 * A stream buffer is considered full when it's length is equal to CONFIG_STREAM_BUFFER_BYTES.
 * If the buffer is less than CONFIG_STREAM_BUFFER_BYTES in length, xStreamIsFull()
 * will return false.
 *
 * @param steam_ The stream buffer to operate on.
 * @return xBase Returns true if the stream buffer is equal to CONFIG_STREAM_BUFFER_BYTES
 * in length, otherwise xStreamIsFull() will return false.
 */
xBase xStreamIsFull(const xStreamBuffer steam_);

/**
 * @brief System call to initialize the system.
 *
 * The xSystemInit() system call initializes the required interrupt handlers and
 * memory and must be called prior to calling any other system call.
 *
 */
void xSystemInit(void);

/**
 * @brief The xSystemHalt() system call will halt HeliOS.
 *
 * The xSystemHalt() system call will halt HeliOS. Once xSystemHalt() is called,
 * the system must be reset.
 *
 */
void xSystemHalt(void);

/**
 * @brief The xSystemGetSystemInfo() system call will return information about the running system.
 *
 * The xSystemGetSystemInfo() system call will return the type xSystemInfo containing
 * information about the system including the OS (product) name, its version and how many tasks
 * are currently in the running, suspended or waiting states.
 *
 * @return xSystemInfo The system info is returned if successful, otherwise null is
 * returned if unsuccessful.
 *
 * @sa xSystemInfo
 * @sa xMemFree()
 *
 * @warning The memory allocated by the xSystemGetSystemInfo() must be freed with xMemFree().
 */
xSystemInfo xSystemGetSystemInfo(void);

/**
 * @brief System call to create a new task.
 *
 * The xTaskCreate() system call will create a new task. The task will be created with its
 * state set to suspended. The xTaskCreate() and xTaskDelete() system calls cannot be called within
 * a task. They MUST be called outside of the scope of the HeliOS scheduler.
 *
 * @param name_ The ASCII name of the task which can be used by xTaskGetHandleByName() to obtain the task handle. The
 * length of the name is depended on the CONFIG_TASK_NAME_BYTES. The task name is NOT a null terminated char string.
 * @param callback_ The address of the task main function. This is the function that will be invoked
 * by the scheduler when a task is scheduled for execution.
 * @param taskParameter_ A pointer to any type or structure that the end-user wants to pass into the task as
 * a parameter. The task parameter is not required and may simply be set to null.
 * @return xTask A handle to the newly created task.
 *
 * @sa xTask
 * @sa xTaskParm
 * @sa xTaskDelete()
 * @sa xTaskState
 * @sa CONFIG_TASK_NAME_BYTES
 *
 * @warning xTaskCreate() MUST be called outside the scope of the HeliOS scheduler (i.e., not from a task's main).
 * The task memory can only be freed by xTaskDelete().
 */
xTask xTaskCreate(const xChar *name_, void (*callback_)(xTask task_, xTaskParm parm_), xTaskParm taskParameter_);

/**
 * @brief System call to delete a task.
 *
 * The xTaskDelete() system call will delete a task. The xTaskCreate() and xTaskDelete() system calls
 * cannot be called within a task. They MUST be called outside of the scope of the HeliOS scheduler.
 *
 * @param task_ The handle of the task to be deleted.
 *
 * @warning xTaskDelete() MUST be called outside the scope of the HeliOS scheduler (i.e., not from a task's main).
 */
void xTaskDelete(const xTask task_);

/**
 * @brief System call to get a task's handle by its ASCII name.
 *
 * The xTaskGetHandleByName() system call will return the task handle of the
 * task specified by its ASCII name. The length of the task name is dependent on the
 * CONFIG_TASK_NAME_BYTES setting. The name is compared byte-for-byte so the name is
 * case sensitive.
 *
 * @sa CONFIG_TASK_NAME_BYTES
 *
 * @param name_ The ASCII name of the task to return the handle of. The task name is NOT a null terminated string.
 * @return xTask The task handle. xTaskGetHandleByName() returns null if the name cannot be found.
 */
xTask xTaskGetHandleByName(const xChar *name_);

/**
 * @brief System call to get a task's handle by its task identifier.
 *
 * The xTaskGetHandleById() system call will return the task handle of the task specified
 * by identifier identifier.
 *
 * @sa xBase
 *
 * @param id_ The identifier of the task to return the handle of.
 * @return xTask The task handle. xTaskGetHandleById() returns null if the the task identifier
 * cannot be found.
 */
xTask xTaskGetHandleById(const xBase id_);

/**
 * @brief System call to return task runtime statistics for all tasks.
 *
 * The xTaskGetAllRunTimeStats() system call will return the runtime statistics for all
 * of the tasks regardless of their state. The xTaskGetAllRunTimeStats() system call returns
 * the xTaskRunTimeStats type. An xBase variable must be passed by reference to xTaskGetAllRunTimeStats()
 * which will be updated by xTaskGetAllRunTimeStats() to contain the number of tasks so the
 * end-user can iterate through the tasks. The xTaskRunTimeStats memory must be freed by xMemFree()
 * after it is no longer needed.
 *
 * @sa xTaskRunTimeStats
 * @sa xMemFree()
 *
 * @param tasks_ A variable of type xBase passed by reference which will contain the number of tasks
 * upon return. If no tasks currently exist, this variable will not be modified.
 * @return xTaskRunTimeStats The runtime stats returned by xTaskGetAllRunTimeStats(). If there are
 * currently no tasks then this will be null. This memory must be freed by xMemFree().
 *
 * @warning The memory allocated by xTaskGetAllRunTimeStats() must be freed by xMemFree().
 */
xTaskRunTimeStats xTaskGetAllRunTimeStats(xBase *tasks_);

/**
 * @brief System call to return task runtime statistics for the specified task.
 *
 * The xTaskGetTaskRunTimeStats() system call returns the task runtime statistics for
 * one task. The xTaskGetTaskRunTimeStats() system call returns the xTaskRunTimeStats type.
 * The memory must be freed by calling xMemFree() after it is no longer needed.
 *
 * @sa xTaskRunTimeStats
 * @sa xMemFree()
 *
 * @param task_ The task to get the runtime statistics for.
 * @return xTaskRunTimeStats The runtime stats returned by xTaskGetTaskRunTimeStats().
 * xTaskGetTaskRunTimeStats() will return null of the task cannot be found.
 *
 * @warning The memory allocated by xTaskGetTaskRunTimeStats() must be freed by xMemFree().
 */
xTaskRunTimeStats xTaskGetTaskRunTimeStats(const xTask task_);

/**
 * @brief System call to return the number of tasks regardless of their state.
 *
 * The xTaskGetNumberOfTasks() system call returns the current number of tasks
 * regardless of their state.
 *
 * @return xBase The number of tasks.
 */
xBase xTaskGetNumberOfTasks(void);

/**
 * @brief System call to return the details of a task.
 *
 * The xTaskGetTaskInfo() system call returns the xTaskInfo structure containing
 * the details of the task including its identifier, name, state and runtime statistics.
 *
 * @sa xTaskInfo
 *
 * @param task_ The task to return the details of.
 * @return xTaskInfo The xTaskInfo structure containing the task details. xTaskGetTaskInfo()
 * returns null if the task cannot be found.
 *
 * @warning The memory allocated by xTaskGetTaskInfo() must be freed by xMemFree().
 */
xTaskInfo xTaskGetTaskInfo(const xTask task_);

/**
 * @brief System call to return the details of all tasks.
 *
 * The xTaskGetAllTaskInfo() system call returns the xTaskInfo structure containing
 * the details of ALL tasks including their identifier, name, state and runtime statistics.
 *
 * @sa xTaskInfo
 *
 * @param tasks_ A variable of type xBase passed by reference which will contain the number of tasks
 * upon return. If no tasks currently exist, this variable will not be modified.
 * @return xTaskInfo The xTaskInfo structure containing the tasks details. xTaskGetAllTaskInfo()
 * returns null if there no tasks or if a consistency issue is detected.
 *
 * @warning The memory allocated by xTaskGetAllTaskInfo() must be freed by xMemFree().
 */
xTaskInfo xTaskGetAllTaskInfo(xBase *tasks_);

/**
 * @brief System call to return the state of a task.
 *
 * The xTaskGetTaskState() system call will return the state of the task.
 *
 * @sa xTaskState
 *
 * @param task_ The task to return the state of.
 * @return xTaskState The xTaskState of the task. If the task cannot be found, xTaskGetTaskState()
 * will return null.
 */
xTaskState xTaskGetTaskState(const xTask task_);

/**
 * @brief System call to return the ASCII name of a task.
 *
 * The xTaskGetName() system call returns the ASCII name of the task. The size of the
 * task is dependent on the setting CONFIG_TASK_NAME_BYTES. The task name is NOT a null
 * terminated char string. The memory allocated for the char array must be freed by
 * xMemFree() when no longer needed.
 *
 * @sa CONFIG_TASK_NAME_BYTES
 * @sa xMemFree()
 *
 * @param task_ The task to return the name of.
 * @return xChar* A pointer to the char array containing the ASCII name of the task. The task name
 * is NOT a null terminated char string. xTaskGetName() will return null if the task cannot be found.
 *
 * @warning The memory allocated by xTaskGetName() must be free by xMemFree().
 */
xChar *xTaskGetName(const xTask task_);

/**
 * @brief System call to return the task identifier for a task.
 *
 * The xTaskGetId() system call returns the task identifier for the task.
 *
 * @param task_ The task to return the identifier of.
 * @return xBase The identifier of the task. If the task cannot be found, xTaskGetId()
 * returns zero (all tasks identifiers are 1 or greater).
 */
xBase xTaskGetId(const xTask task_);

/**
 * @brief System call to clear a waiting direct to task notification.
 *
 * The xTaskNotifyStateClear() system call will clear a waiting direct to task notification if one
 * exists without returning the notification.
 *
 * @param task_ The task to clear the notification for.
 */
void xTaskNotifyStateClear(xTask task_);

/**
 * @brief System call to check if a direct to task notification is waiting.
 *
 * The xTaskNotificationIsWaiting() system call will return true or false depending
 * on whether there is a direct to task notification waiting for the task.
 *
 * @param task_ The task to check for a waiting task notification.
 * @return xBase Returns true if there is a task notification. False if there is no notification
 * or if the task could not be found.
 */
xBase xTaskNotificationIsWaiting(const xTask task_);

/**
 * @brief System call to give another task a direct to task notification.
 *
 * The xTaskNotifyGive() system call will give a direct to task notification to the specified task. The
 * task notification bytes is the number of bytes contained in the notification value. The number of
 * notification bytes must be between one and the CONFIG_NOTIFICATION_VALUE_BYTES setting. The notification
 * value must contain a pointer to a char array containing the notification value. If the task already
 * has a waiting task notification, xTaskNotifyGive() will NOT overwrite the waiting task notification.
 * xTaskNotifyGive() will return true if the direct to task notification was successfully given.
 *
 * @sa CONFIG_NOTIFICATION_VALUE_BYTES
 * @sa xTaskNotifyTake()
 *
 * @param task_ The task to send the task notification to.
 * @param notificationBytes_ The number of bytes contained in the notification value. The number must be
 * between one and the CONFIG_NOTIFICATION_VALUE_BYTES setting.
 * @param notificationValue_ A char array containing the notification value. The notification value is NOT a null terminated string.
 * @return xBase RETURN_SUCCESS if the direct to task notification was successfully given, RETURN_FAILURE if not.
 */
xBase xTaskNotifyGive(xTask task_, const xBase notificationBytes_, const xChar *notificationValue_);

/**
 * @brief System call to take a direct to task notification from another task.
 *
 * The xTaskNotifyTake() system call will return the waiting direct to task notification if there
 * is one. The xTaskNotifyTake() system call will return an xTaskNotification structure containing
 * the notification bytes and its value. The memory allocated by xTaskNotifyTake() must be freed
 * by xMemFree().
 *
 * @sa xTaskNotification
 * @sa xTaskNotifyGive()
 * @sa xMemFree()
 * @sa CONFIG_NOTIFICATION_VALUE_BYTES
 *
 * @param task_ The task to return a waiting task notification.
 * @return xTaskNotification The xTaskNotification structure containing the notification bytes
 * and value. xTaskNotifyTake() will return null if no waiting task notification exists or if
 * the task cannot be found.
 *
 * @warning The memory allocated by xTaskNotifyTake() must be freed by xMemFree().
 */
xTaskNotification xTaskNotifyTake(xTask task_);

/**
 * @brief System call to resume a task.
 *
 * The xTaskResume() system call will resume a suspended task. Tasks are suspended on creation
 * so either xTaskResume() or xTaskWait() must be called to place the task in a state that the scheduler
 * will execute.
 *
 * @sa xTaskState
 * @sa xTaskSuspend()
 * @sa xTaskWait()
 *
 * @param task_ The task to set its state to running.
 */
void xTaskResume(xTask task_);

/**
 * @brief System call to suspend a task.
 *
 * The xTaskSuspend() system call will suspend a task. A task that has been suspended
 * will not be executed by the scheduler until xTaskResume() or xTaskWait() is called.
 *
 * @sa xTaskState
 * @sa xTaskResume()
 * @sa xTaskWait()
 *
 * @param task_ The task to suspend.
 */
void xTaskSuspend(xTask task_);

/**
 * @brief System call to place a task in a waiting state.
 *
 * The xTaskWait() system call will place a task in the waiting state. A task must
 * be in the waiting state for event driven multitasking with either direct to task
 * notifications OR setting the period on the task timer with xTaskChangePeriod(). A task
 * in the waiting state will not be executed by the scheduler until an event has occurred.
 *
 * @sa xTaskState
 * @sa xTaskResume()
 * @sa xTaskSuspend()
 *
 * @param task_ The task to place in the waiting state.
 */
void xTaskWait(xTask task_);

/**
 * @brief System call to set the task timer period.
 *
 * The xTaskChangePeriod() system call will change the period (ticks) on the task timer
 * for the specified task. The timer period must be greater than zero. To have any effect, the task
 * must be in the waiting state set by calling xTaskWait() on the task. Once the timer period is set
 * and the task is in the waiting state, the task will be executed every timerPeriod_ ticks.
 * Changing the period to zero will prevent the task from being executed even if it is in the waiting state
 * unless it were to receive a direct to task notification.
 *
 * @sa xTaskWait()
 * @sa xTaskGetPeriod()
 * @sa xTaskResetTimer()
 *
 * @param task_ The task to change the timer period for.
 * @param timerPeriod_ The timer period in ticks.
 */
void xTaskChangePeriod(xTask task_, const xTicks timerPeriod_);

/**
 * @brief System call to get the task timer period.
 *
 * The xTaskGetPeriod() will return the period for the timer for the specified task. See
 * xTaskChangePeriod() for more information on how the task timer works.
 *
 * @sa xTaskWait()
 * @sa xTaskChangePeriod()
 * @sa xTaskResetTimer()
 *
 * @param task_ The task to return the timer period for.
 * @return xTicks The timer period in ticks. xTaskGetPeriod() will return zero
 * if the timer period is zero or if the task could not be found.
 */
xTicks xTaskGetPeriod(const xTask task_);

/**
 * @brief System call to reset the task timer.
 *
 * The xTaskResetTimer() system call will reset the task timer. xTaskResetTimer() does not change
 * the timer period or the task state when called. See xTaskChangePeriod() for more details on task timers.
 *
 * @sa xTaskWait()
 * @sa xTaskChangePeriod()
 * @sa xTaskGetPeriod()
 *
 * @param task_ The task to reset the task timer for.
 */
void xTaskResetTimer(xTask task_);

/**
 * @brief System call to pass control to the HeliOS scheduler.
 *
 * The xTaskStartScheduler() system call passes control to the HeliOS scheduler. This system
 * call will not return until xTaskSuspendAll() is called. If xTaskSuspendAll() is called, xTaskResumeAll()
 * must be called before xTaskStartScheduler() can be called again to continue executing tasks.
 *
 */
void xTaskStartScheduler(void);

/**
 * @brief System call to set scheduler state to running.
 *
 * The xTaskResumeAll() system call will set the scheduler state to running so the next
 * call to xTaskStartScheduler() will resume execute of all tasks. The state of each task
 * is not altered by xTaskSuspendAll() or xTaskResumeAll().
 *
 * @sa xTaskSuspendAll()
 *
 */
void xTaskResumeAll(void);

/**
 * @brief System call to set the scheduler state to suspended.
 *
 * The xTaskSuspendAll() system call will set the scheduler state to suspended
 * so the scheduler will stop and return. The state of each task is not altered by
 * xTaskSuspendAll() or xTaskResumeAll().
 *
 * @sa xTaskResumeAll()
 *
 */
void xTaskSuspendAll(void);

/**
 * @brief System call to get the state of the scheduler.
 *
 * The xTaskGetSchedulerState() system call will return the state of the scheduler. The
 * state of the scheduler can only be changed using xTaskSuspendAll() and xTaskResumeAll().
 *
 * @sa xSchedulerState
 * @sa xTaskSuspendAll()
 * @sa xTaskResumeAll()
 *
 * @return xSchedulerState The state of the scheduler.
 */
xSchedulerState xTaskGetSchedulerState(void);

/**
 * @brief The xTaskChangeWDPeriod() will change the period on the task watchdog timer.
 *
 * The xTaskChangeWDPeriod() system call will change the task watchdog timer period. The period,
 * measured in ticks, must be greater than zero to have any effect. If the tasks last runtime
 * exceeds the task watchdog timer period, the task will automatically be placed in a suspended
 * state.
 *
 * @sa xTaskGetWDPeriod()
 * @param task_ The task to change the task watchdog timer for.
 * @param wdTimerPeriod_  The task watchdog timer period which is measured in ticks. If zero, the task watchdog timer will not have any effect.
 */
void xTaskChangeWDPeriod(xTask task_, const xTicks wdTimerPeriod_);

/**
 * @brief The xTaskGetWDPeriod() return the current task watchdog timer.
 *
 * The xTaskGetWDPeriod() will return the current task watchdog timer for the task.
 *
 * @sa xTaskChangeWDPeriod()
 * @param task_ The task to get the task watchdog timer period for.
 * @return xTicks  The task watchdog timer period which is measured in ticks.
 */
xTicks xTaskGetWDPeriod(const xTask task_);

/**
 * @brief System call to create a new timer.
 *
 * The xTimerCreate() system call will create a new timer. Timers differ from
 * task timers in that they do not create events that effect the scheduling of a task.
 * Timers can be used by tasks to initiate various task activities based on a specified
 * time period represented in ticks. The memory allocated by xTimerCreate() must
 * be freed by xTimerDelete(). Unlike tasks, timers may be created and deleted within
 * tasks.
 *
 * @sa xTimer
 * @sa xTimerDelete()
 *
 * @param timerPeriod_ The number of ticks before the timer expires.
 * @return xTimer The newly created timer. If the timer period parameter is less than zero
 * or xTimerCreate() was unable to allocate the required memory, xTimerCreate() will return null.
 *
 * @warning The timer memory can only be freed by xTimerDelete().
 */
xTimer xTimerCreate(const xTicks timerPeriod_);

/**
 * @brief System call will delete a timer.
 *
 * The xTimerDelete() system call will delete a timer. For more information on timers see the
 * xTaskTimerCreate() system call.
 *
 * @sa xTimerCreate()
 *
 * @param timer_ The timer to be deleted.
 */
void xTimerDelete(const xTimer timer_);

/**
 * @brief System call to change the period of a timer.
 *
 * The xTimerChangePeriod() system call will change the period of the specified timer.
 * The timer period is measured in ticks. If the timer period is zero, the xTimerHasTimerExpired()
 * system call will always return false.
 *
 * @sa xTimerHasTimerExpired()
 *
 * @param timer_ The timer to change the period for.
 * @param timerPeriod_ The timer period in is ticks. Timer period must be zero or greater.
 */
void xTimerChangePeriod(xTimer timer_, const xTicks timerPeriod_);

/**
 * @brief System call to get the period of a timer.
 *
 * The xTimerGetPeriod() system call will return the current timer period
 * for the specified timer.
 *
 * @param timer_ The timer to get the timer period for.
 * @return xTicks The timer period. If the timer cannot be found, xTimerGetPeriod()
 * will return zero.
 */
xTicks xTimerGetPeriod(const xTimer timer_);

/**
 * @brief System call to check if a timer is active.
 *
 * The xTimerIsTimerActive() system call will return true of the timer has been
 * started with xTimerStart().
 *
 * @sa xTimerStart()
 *
 * @param timer_ The timer to check if active.
 * @return xBase True if active, false if not active or if the timer could not be found.
 */
xBase xTimerIsTimerActive(const xTimer timer_);

/**
 * @brief System call to check if a timer has expired.
 *
 * The xTimerHasTimerExpired() system call will return true or false dependent on whether
 * the timer period for the specified timer has elapsed. xTimerHasTimerExpired() will NOT
 * reset the timer. Timers will not automatically reset. Timers MUST be reset with xTimerReset().
 *
 * @sa xTimerReset()
 *
 * @param timer_ The timer to determine if the period has expired.
 * @return xBase True if the timer has expired, false if the timer has not expired or could not be found.
 */
xBase xTimerHasTimerExpired(const xTimer timer_);

/**
 * @brief System call to reset a timer.
 *
 * The xTimerReset() system call will reset the start time of the timer to zero.
 *
 * @param timer_ The timer to be reset.
 */
void xTimerReset(xTimer timer_);

/**
 * @brief System call to start a timer.
 *
 * The xTimerStart() system call will place the timer in the running (active) state. Neither xTimerStart() nor
 * xTimerStop() will reset the timer. Timers can only be reset with xTimerReset().
 *
 * @sa xTimerStop()
 * @sa xTimerReset()
 *
 * @param timer_ The timer to be started.
 */
void xTimerStart(xTimer timer_);

/**
 * @brief System call to stop a timer.
 * 
 * The xTimerStop() system call will place the timer in the stopped state. Neither xTimerStart() nor
 * xTimerStop() will reset the timer. Timers can only be reset with xTimerReset().
 *
 * @sa xTimerStart()
 * @sa xTimerReset()
 *
 * @param timer_ The timer to be stopped.
 */
void xTimerStop(xTimer timer_);

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/* DO NOT USE - USE THE SYSASSERT() C MACRO INSTEAD! */
void __SystemAssert__(const char *file_, int line_);

/* DO NOT USE - THESE ARE FOR UNIT TESTING! */
#if defined(POSIX_ARCH_OTHER)
void __MemoryClear__(void);
void __SysStateClear__(void);
void __TimerStateClear__(void);
void __TaskStateClear__(void);
void __MemoryRegionDumpKernel__(void);
void __MemoryRegionDumpHeap__(void);
void __DeviceStateClear__(void);
#endif

#endif

#ifdef __cplusplus
}
#endif
#endif