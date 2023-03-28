/*UNCRUSTIFY-OFF*/
/**
 * @file HeliOS.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for user application header
 * @version 0.4.1
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#ifndef HELIOS_H_
  #define HELIOS_H_

  #include "posix.h"

  #include <stdint.h>

  #include "config.h"
  #include "defines.h"


/**
 * @brief Enumerated type for task states
 *
 * A task can be in one of four possible states as defined by the TaskState_t
 * enumerated data type. The state a task is in is changed by calling
 * xTaskResume(), xTaskSuspend() or xTaskWait(). The HeliOS scheduler will only
 * schedule, for execution, tasks in either the TaskStateRunning or
 * TaskStateWaiting state.
 *
 * @sa xTaskState
 * @sa xTaskResume()
 * @sa xTaskSuspend()
 * @sa xTaskWait()
 * @sa xTaskGetTaskState()
 *
 */
  typedef enum TaskState_e {
    TaskStateSuspended,


    /**< State a task is in after it is created OR after calling xTaskSuspend().
     * Tasks in the TaskStateSuspended state will not be scheduled for execution
     * by the scheduler.  */
    TaskStateRunning,


    /**< State a task is in after calling xTaskResume(). Tasks in the
     * TaskStateRunning state will be scheduled for execution by the scheduler.
     */
    TaskStateWaiting /**< State a task is in after calling xTaskWait(). Tasks in
                      * the TaskStateWaiting state will be scheduled for
                      * execution by the scheduler only when a task event has
                      * occurred. */
  } TaskState_t;


  /**
   * @brief Enumerated type for task states
   *
   * @sa TaskState_t
   *
   */
  typedef TaskState_t xTaskState;


  /**
   * @brief Enumerated type for scheduler state
   *
   * The scheduler can be in one of three possible states as defined by the
   * SchedulerState_t enumerated data type. The state the scheduler is in is
   * changed by calling xTaskSuspendAll() and xTaskResumeAll(). The state the
   * scheduler is in can be obtained by calling xTaskGetSchedulerState().
   *
   * @sa xSchedulerState
   * @sa xTaskSuspendAll()
   * @sa xTaskResumeAll()
   * @sa xTaskGetSchedulerState()
   * @sa xTaskStartScheduler()
   *
   */
  typedef enum SchedulerState_e {
    SchedulerStateSuspended,


    /**< State the scheduler is in after calling xTaskSuspendAll().
     * TaskStartScheduler() will stop scheduling tasks for execution and
     * relinquish control when xTaskSuspendAll() is called. */
    SchedulerStateRunning /**< State the scheduler is in after calling
                           * xTaskResumeAll(). xTaskStartScheduler() will
                           * continue to schedule tasks for execution until
                           * xTaskSuspendAll() is called. */
  } SchedulerState_t;


  /**
   * @brief Enumerated type for scheduler state
   *
   * @sa SchedulerState_t
   *
   */
  typedef SchedulerState_t xSchedulerState;


  /**
   * @brief Enumerated type for syscall return type
   *
   * All HeliOS syscalls return the Return_t type which can either be ReturnOK
   * or ReturnError. The C macros OK() and ERROR() can be used as a more concise
   * way of checking the return value of a syscall (e.g.,
   * if(OK(xMemGetUsed(&size))) {} or if(ERROR(xMemGetUsed(&size))) {}).
   *
   * @sa OK()
   * @sa ERROR()
   * @sa xReturn
   *
   */
  typedef enum Return_e {
    ReturnOK, /**< Return value if the syscall was successful. */
    ReturnError /**< Return value if the syscall failed. */
  } Return_t;


  /**
   * @brief Enumerated type for syscall return type
   *
   * @sa Return_t
   *
   */
  typedef Return_t xReturn;


  /**
   * @brief Data type for the task paramater
   *
   * The TaskParm_t type is used to pass a paramater to a task at the time of
   * task creation using xTaskCreate(). A task paramater is a pointer of type
   * void and can point to any number of types, arrays and/or data structures
   * that will be passed to the task. It is up to the end-user to manage,
   * allocate and free the memory related to these objects using xMemAlloc() and
   * xMemFree().
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
   * The Base_t type is a simple data type often used as an argument or result
   * type for syscalls when the value is known not to exceed its 8-bit width and
   * no data structure requirements exist. There are no guarantees the Base_t
   * will always be 8-bits wide. If an 8-bit data type is needed that is
   * guaranteed to remain 8-bits wide, the Byte_t data type should be used.
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
   * 8-bits wide.
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
   * @brief Data type for a pointer to a memory address
   *
   * The Addr_t type is a pointer of type void and is used to pass addresses
   * between the end-user application and syscalls. It is not necessary to use
   * the Addr_t type within the end-user application as long as the type is not
   * used to interact with the kernel through syscalls
   *
   * @sa xAddr
   *
   */
  typedef VOID_TYPE Addr_t;


  /**
   * @brief Data type for a pointer to a memory address
   *
   * @sa Addr_t
   *
   */
  typedef Addr_t *xAddr;


  /**
   * @brief Data type for the storage requirements of an object in memory
   *
   * The Size_t type is used for the storage requirements of an object in memory
   * and is always represented in bytes.
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
   * The HalfWord_t type is a 16-bit wide data type and is guaranteed to always
   * be 16-bits wide.
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
   * 32-bits wide.
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
   * The Ticks_t type is used to store ticks from the system clock. Ticks is not
   * bound to any one unit of measure for time though most systems are
   * configured for millisecond resolution, milliseconds is not guaranteed and
   * is dependent on the system clock frequency and prescaler.
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
   * @brief Data type for a task
   *
   * The Task_t data type is used as a task. The task is created when
   * xTaskCreate() is called. For more information about tasks, see
   * xTaskCreate().
   *
   * @sa xTask
   * @sa xTaskCreate()
   * @sa xTaskDelete()
   *
   */
  typedef VOID_TYPE Task_t;


  /**
   * @brief Data type for a task
   *
   * @sa Task_t
   *
   */
  typedef Task_t *xTask;


  /**
   * @brief Data type for a timer
   *
   * The Timer_t data type is used as a timer. The timer is created when
   * xTimerCreate() is called. For more information about timers, see
   * xTimerCreate().
   *
   * @sa xTimer
   * @sa xTimerCreate()
   * @sa xTimerDelete()
   *
   */
  typedef VOID_TYPE Timer_t;


  /**
   * @brief Data type for a timer
   *
   * @sa Timer_t
   *
   */
  typedef Timer_t *xTimer;


  /**
   * @brief Data type for a queue
   *
   * The Queue_t data type is used as a queue The queue is created when
   * xQueueCreate() is called. For more information about queues, see
   * xQueueCreate().
   *
   * @sa xQueue
   * @sa xQueueCreate()
   * @sa xQueueDelete()
   *
   */
  typedef VOID_TYPE Queue_t;


  /**
   * @brief Data type for a queue
   *
   * @sa Queue_t
   *
   */
  typedef Queue_t *xQueue;


  /**
   * @brief Data type for a stream buffer
   *
   * The StreamBuffer_t data type is used as a stream buffer. The stream buffer
   * is created when xStreamCreate() is called. For more information about
   * stream buffers, see xStreamCreate(). Stream_t should be declared as
   * xStream.
   *
   * @sa xStream
   * @sa xStreamCreate()
   * @sa xStreamDelete()
   *
   */
  typedef VOID_TYPE StreamBuffer_t;


  /**
   * @brief Data type for a stream buffer
   *
   * @sa StreamBuffer_t
   *
   */
  typedef StreamBuffer_t *xStreamBuffer;


  /**
   * @brief Data structure for a direct to task notification
   *
   * The TaskNotification_t data structure is used by xTaskNotifyGive() and
   * xTaskNotifyTake() to send and receive direct to task notifications. Direct
   * to task notifications are part of the event-driven multitasking model. A
   * direct to task notification may be received by event-driven and
   * co-operative tasks alike. However, the benefit of direct to task
   * notifications may only be realized by tasks scheduled as event-driven. In
   * order to wait for a direct to task notification, the task must be in a
   * "waiting" state which is set by xTaskWait().
   *
   * @sa xTaskNotification
   * @sa xMemFree()
   * @sa xTaskNotifyGive()
   * @sa xTaskNotifyTake()
   * @sa xTaskWait()
   *
   */
  typedef struct TaskNotification_s {
    Base_t notificationBytes; /**< The length in bytes of the notification value
                               * which cannot exceed
                               * CONFIG_NOTIFICATION_VALUE_BYTES. */
    Byte_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES]; /**< The
                                                                * notification
                                                                * value whose
                                                                * length is
                                                                * specified by
                                                                * the
                                                                * notification
                                                                * bytes member.
                                                                */
  } TaskNotification_t;


  /**
   * @brief Data structure for a direct to task notification
   *
   * @sa TaskNotification_t
   *
   */
  typedef TaskNotification_t *xTaskNotification;


  /**
   * @brief Data structure for task runtime statistics
   *
   * The TaskRunTimeStats_t data structure is used by xTaskGetTaskRunTimeStats()
   * and xTaskGetAllRuntimeStats() to obtain runtime statistics about a task.
   *
   * @sa xTaskRunTimeStats
   * @sa xTaskGetTaskRunTimeStats()
   * @sa xTaskGetAllRunTimeStats()
   * @sa xMemFree()
   *
   */
  typedef struct TaskRunTimeStats_s {
    Base_t id; /**< The ID of the task. */
    Ticks_t lastRunTime; /**< The duration in ticks of the task's last runtime.
                          */
    Ticks_t totalRunTime; /**< The duration in ticks of the task's total
                           * runtime. */
  } TaskRunTimeStats_t;


  /**
   * @brief Data structure for task runtime statistics
   *
   */
  typedef TaskRunTimeStats_t *xTaskRunTimeStats;


  /**
   * @brief Data structure for memory region statistics
   *
   * The MemoryRegionStats_t data structure is used by xMemGetHeapStats() and
   * xMemGetKernelStats() to obtain statistics about either memory region.
   *
   * @sa xMemoryRegionStats
   * @sa xMemGetHeapStats()
   * @sa xMemGetKernelStats()
   * @sa xMemFree()
   *
   */
  typedef struct MemoryRegionStats_s {
    Word_t largestFreeEntryInBytes; /**< The largest free entry in bytes. */
    Word_t smallestFreeEntryInBytes; /**< The smallest free entry in bytes. */
    Word_t numberOfFreeBlocks; /**< The number of free blocks. See
                                * CONFIG_MEMORY_REGION_BLOCK_SIZE for block size
                                * in bytes. */
    Word_t availableSpaceInBytes; /**< The amount of free memory in bytes (i.e.,
                                   * numberOfFreeBlocks *
                                   * CONFIG_MEMORY_REGION_BLOCK_SIZE). */
    Word_t successfulAllocations; /**< Number of successful memory allocations.
                                   */
    Word_t successfulFrees; /**< Number of successful memory "frees". */
    Word_t minimumEverFreeBytesRemaining; /**< Lowest water lever since system
                                           * initialization of free bytes of
                                           * memory. */
  } MemoryRegionStats_t;


  /**
   * @brief Data structure for memory region statistics
   *
   */
  typedef MemoryRegionStats_t *xMemoryRegionStats;


  /**
   * @brief Data structure for information about a task
   *
   * The TaskInfo_t structure is similar to xTaskRuntimeStats_t in that it
   * contains runtime statistics for a task. However, TaskInfo_t also contains
   * additional details about a task such as its name and state. The TaskInfo_t
   * structure is returned by xTaskGetTaskInfo() and xTaskGetAllTaskInfo(). If
   * only runtime statistics are needed, then TaskRunTimeStats_t should be used
   * because of its smaller memory footprint.
   *
   * @sa xTaskInfo
   * @sa xTaskGetTaskInfo()
   * @sa xTaskGetAllTaskInfo()
   * @sa CONFIG_TASK_NAME_BYTES
   * @sa xMemFree()
   *
   */
  typedef struct TaskInfo_s {
    Base_t id; /**< The ID of the task. */
    Byte_t name[CONFIG_TASK_NAME_BYTES]; /**< The name of the task which must be
                                          * exactly CONFIG_TASK_NAME_BYTES bytes
                                          * in length. Shorter task names must
                                          * be padded. */
    TaskState_t state; /**< The state the task is in which is one of four states
                        * specified in the TaskState_t enumerated data type. */
    Ticks_t lastRunTime; /**< The duration in ticks of the task's last runtime.
                          */
    Ticks_t totalRunTime; /**< The duration in ticks of the task's total
                           * runtime. */
  } TaskInfo_t;


  /**
   * @brief Data structure for information about a task
   *
   */
  typedef TaskInfo_t *xTaskInfo;


  /**
   * @brief Data structure for a queue message
   *
   * The QueueMessage_t stucture is used to store a queue message and is
   * returned by xQueueReceive() and xQueuePeek().
   *
   * @sa xQueueMessage
   * @sa xQueueReceive()
   * @sa xQueuePeek()
   * @sa CONFIG_MESSAGE_VALUE_BYTES
   * @sa xMemFree()
   *
   */
  typedef struct QueueMessage_s {
    Base_t messageBytes; /**< The number of bytes contained in the message value
                          * which cannot exceed CONFIG_MESSAGE_VALUE_BYTES. */
    Byte_t messageValue[CONFIG_MESSAGE_VALUE_BYTES]; /**< The queue message
                                                      * value. */
  } QueueMessage_t;


  /**
   * @brief Data structure for a queue message
   *
   */
  typedef QueueMessage_t *xQueueMessage;


  /**
   * @brief Data structure for information about the HeliOS system
   *
   * The SystemInfo_t data structure is used to store information about the
   * HeliOS system and is returned by xSystemGetSystemInfo().
   *
   * @sa xSystemInfo
   * @sa xSystemGetSystemInfo()
   * @sa OS_PRODUCT_NAME_SIZE
   * @sa xMemFree()
   *
   */
  typedef struct SystemInfo_s {
    Byte_t productName[OS_PRODUCT_NAME_SIZE]; /**< The product name of the
                                               * operating system (always
                                               * "HeliOS"). */
    Base_t majorVersion; /**< The SemVer major version number of HeliOS. */
    Base_t minorVersion; /**< The SemVer minor version number of HeliOS. */
    Base_t patchVersion; /**< The SemVer patch version number of HeliOS. */
    Base_t numberOfTasks; /**< The number of tasks regardless of their state. */
  } SystemInfo_t;


  /**
   * @brief Data structure for information about the HeliOS system
   *
   */
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
   * xDeviceWrite(), etc. Once a device is registered, it cannot be
   * un-registered - it can only be placed in a suspended state which is done by
   * calling xDeviceConfigDevice(). However, as with most aspects of the HeliOS
   * device driver model , it is important to note that the implementation of
   * and support for device state and mode is up to the device driver's author.
   *
   * @note A device driver's unique identifier ("UID") must be a globally unique
   * identifier. No two device drivers in the same application can share the
   * same UID. This is best achieved by ensuring the device driver author
   * selects a UID for his device driver that is not in use by other device
   * drivers. A device driver template and device drivers can be found in
   * /drivers.
   *
   * @sa CONFIG_DEVICE_NAME_BYTES
   * @sa xReturn
   *
   * @param  device_self_register_ The device driver's self registration
   *                               function, DRIVERNAME_self_register().
   * @return                       On success, the syscall returns ReturnOK. On
   *                               failure, the syscall returns ReturnError. A
   *                               failure is any condition in which the syscall
   *                               was unable to achieve its intended objective.
   *                               For example, if xTaskGetId() was unable to
   *                               locate the task by the task object (i.e.,
   *                               xTask) passed to the syscall, because either
   *                               the object was null or invalid (e.g., a
   *                               deleted task), xTaskGetId() would return
   *                               ReturnError. All HeliOS syscalls return the
   *                               xReturn (a.k.a., Return_t) type which can
   *                               either be ReturnOK or ReturnError. The C
   *                               macros OK() and ERROR() can be used as a more
   *                               concise way of checking the return value of a
   *                               syscall (e.g., if(OK(xMemGetUsed(&size))) {}
   *                               or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xDeviceRegisterDevice(xReturn (*device_self_register_)());


  /**
   * @brief Syscall to query the device driver about the availability of a
   * device
   *
   * The xDeviceIsAvailable() syscall queries the device driver about the
   * availability of a device. Generally "available" means the that the device
   * is available for read and/or write operations though the meaning is
   * implementation specific and left up to the device driver's author.
   *
   * @sa xReturn
   *
   * @param  uid_ The unique identifier ("UID") of the device driver to be
   *              operated on.
   * @param  res_ The result of the inquiry; here, taken to mean the
   *              availability of the device.
   * @return      On success, the syscall returns ReturnOK. On failure, the
   *              syscall returns ReturnError. A failure is any condition in
   *              which the syscall was unable to achieve its intended
   *              objective. For example, if xTaskGetId() was unable to locate
   *              the task by the task object (i.e., xTask) passed to the
   *              syscall, because either the object was null or invalid (e.g.,
   *              a deleted task), xTaskGetId() would return ReturnError. All
   *              HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *              which can either be ReturnOK or ReturnError. The C macros OK()
   *              and ERROR() can be used as a more concise way of checking the
   *              return value of a syscall (e.g., if(OK(xMemGetUsed(&size))) {}
   *              or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xDeviceIsAvailable(const xHalfWord uid_, xBase *res_);


  /**
   * @brief Syscall to write a byte of data to the device
   *
   * The xDeviceSimpleWrite() syscall will write a byte of data to a device.
   * Whether the data is written to the device is dependent on the device driver
   * mode, state and implementation of these features by the device driver's
   * author.
   *
   * @sa xReturn
   *
   * @param  uid_  The unique identifier ("UID") of the device driver to be
   *               operated on.
   * @param  data_ A byte of data to be written to the device.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xDeviceSimpleWrite(const xHalfWord uid_, xByte data_);


  /**
   * @brief Syscall to write multiple bytes of data to a device
   *
   * The xDeviceWrite() syscall will write multiple bytes of data contained in a
   * data buffer to a device. The data buffer must have been allocated by
   * xMemAlloc(). Whether the data is written to the device is dependent on the
   * device driver mode, state and implementation of these features by the
   * device driver's author.
   *
   * @sa xReturn
   * @sa xMemAlloc()
   * @sa xMemFree()
   *
   * @param  uid_  The unique identifier ("UID") of the device driver to be
   *               operated on.
   * @param  size_ The size of the data buffer, in bytes.
   * @param  data_ The data buffer containing the data to be written to the
   *               device. The data buffer must have been allocated by
   *               xMemAlloc().
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xDeviceWrite(const xHalfWord uid_, xSize *size_, xAddr data_);


  /**
   * @brief Syscall to read a byte of data from the device
   *
   * The xDeviceSimpleRead() syscall will read a byte of data from a device.
   * Whether the data is read from the device is dependent on the device driver
   * mode, state and implementation of these features by the device driver's
   * author.
   *
   * @sa xReturn
   *
   * @param  uid_  The unique identifier ("UID") of the device driver to be
   *               operated on.
   * @param  data_ The byte of data read from the device.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xDeviceSimpleRead(const xHalfWord uid_, xByte *data_);


  /**
   * @brief Syscall to read multiple bytes from a device
   *
   * The xDeviceRead() syscall will read multiple bytes of data from a device
   * into a data buffer. The data buffer must be freed by xMemFree(). Whether
   * the data is read from the device is dependent on the device driver mode,
   * state and implementation of these features by the device driver's author.
   *
   * @sa xReturn
   * @sa xMemFree()
   *
   * @param  uid_  The unique identifier ("UID") of the device driver to be
   *               operated on.
   * @param  size_ The number of bytes read from the device and contained in the
   *               data buffer.
   * @param  data_ The data buffer containing the data read from the device
   *               which must be freed by xMemFree().
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xDeviceRead(const xHalfWord uid_, xSize *size_, xAddr *data_);


  /**
   * @brief Syscall to initialize a device
   *
   * The xDeviceInitDevice() syscall will call the device driver's
   * DRIVERNAME_init() function to bootstrap the device. For example, setting
   * memory mapped registers to starting values or setting the device driver's
   * state and mode. This syscall is optional and is dependent on the specifics
   * of the device driver's implementation by its author.
   *
   * @sa xReturn
   *
   * @param  uid_ The unique identifier ("UID") of the device driver to be
   *              operated on.
   * @return      On success, the syscall returns ReturnOK. On failure, the
   *              syscall returns ReturnError. A failure is any condition in
   *              which the syscall was unable to achieve its intended
   *              objective. For example, if xTaskGetId() was unable to locate
   *              the task by the task object (i.e., xTask) passed to the
   *              syscall, because either the object was null or invalid (e.g.,
   *              a deleted task), xTaskGetId() would return ReturnError. All
   *              HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *              which can either be ReturnOK or ReturnError. The C macros OK()
   *              and ERROR() can be used as a more concise way of checking the
   *              return value of a syscall (e.g., if(OK(xMemGetUsed(&size))) {}
   *              or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xDeviceInitDevice(const xHalfWord uid_);


  /**
   * @brief Syscall to configure a device
   *
   * The xDeviceConfigDevice() will call the device driver's DEVICENAME_config()
   * function to configure the device. The syscall is bi-directional (i.e., it
   * will write configuration data to the device and read the same from the
   * device before returning). The purpose of the bi-directional functionality
   * is to allow the device's configuration to be set and queried using one
   * syscall. The structure of the configuration data is left to the device
   * driver's author. What is required is that the configuration data memory is
   * allocated using xMemAlloc() and that the "size_" parameter is set to the
   * size (i.e., amount) of the configuration data (e.g.,
   * sizeof(MyDeviceDriverConfig)) in bytes.
   *
   * @sa xReturn
   * @sa xMemAlloc()
   * @sa xMemFree()
   *
   * @param  uid_    The unique identifier ("UID") of the device driver to be
   *                 operated on.
   * @param  size_   The size (i.e., amount) of configuration data to bw written
   *                 and read to and from the device, in bytes.
   * @param  config_ The configuration data. The configuration data must have
   *                 been allocated by xMemAlloc().
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xDeviceConfigDevice(const xHalfWord uid_, xSize *size_, xAddr config_);


  /**
   * @brief Syscall to request memory from the heap
   *
   * The xMemAlloc() syscall allocates heap memory for user's application. The
   * amount of available heap memory is dependent on the
   * CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS and CONFIG_MEMORY_REGION_BLOCK_SIZE
   * settings. Similar to libc calloc(), xMemAlloc() clears (i.e., zeros out)
   * the allocated memory it allocates. Because the address of the newly
   * allocated heap memory is handed back through the "addr_" argument, the
   * argument must be cast to "volatile xAddr *" to avoid compiler warnings.
   *
   * @sa xReturn
   * @sa CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS
   * @sa CONFIG_MEMORY_REGION_BLOCK_SIZE
   * @sa xMemFree()
   *
   * @param  addr_ The address of the allocated memory. For example, if heap
   *               memory for a structure called mystruct (MyStruct *) needs to
   *               be allocated, the call to xMemAlloc() would be written as
   *               follows if(OK(xMemAlloc((volatile xAddr *) &mystruct,
   *               sizeof(MyStruct)))) {}.
   * @param  size_ The amount of heap memory, in bytes, being requested.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xMemAlloc(volatile xAddr *addr_, const xSize size_);


  /**
   * @brief Syscall to free heap memory allocated by xMemAlloc()
   *
   * The xMemFree() syscall frees (i.e., de-allocates) heap memory allocated by
   * xMemAlloc(). xMemFree() is also used to free heap memory allocated by
   * syscalls including xTaskGetAllRunTimeStats().
   *
   * @sa xReturn
   * @sa xMemAlloc()
   *
   * @param  addr_ The address of the allocated memory to be freed.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xMemFree(const volatile xAddr addr_);


  /**
   * @brief Syscall to obtain the amount of in-use heap memory
   *
   * The xMemGetUsed() syscall will update the "size_" argument with the amount,
   * in bytes, of in-use heap memory. If more memory statistics are needed,
   * xMemGetHeapStats() provides a more complete picture of the heap memory
   * region.
   *
   * @sa xReturn
   * @sa xMemGetHeapStats()
   *
   * @param  size_ The size (i.e., amount), in bytes, of in-use heap memory.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xMemGetUsed(xSize *size_);


  /**
   * @brief Syscall to obtain the amount of heap memory allocated at a specific
   * address
   *
   * The xMemGetSize() syscall can be used to obtain the amount, in bytes, of
   * heap memory allocated at a specific address. The address must be the
   * address obtained from xMemAlloc().
   *
   * @sa xReturn
   *
   * @param  addr_ The address of the heap memory for which the size (i.e.,
   *               amount) allocated, in bytes, is being sought.
   * @param  size_ The size (i.e., amount), in bytes, of heap memory allocated
   *               to the address.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xMemGetSize(const volatile xAddr addr_, xSize *size_);


  /**
   * @brief Syscall to get memory statistics on the heap memory region
   *
   * The xMemGetHeapStats() syscall is used to obtain detailed statistics about
   * the heap memory region which can be used by the application to monitor
   * memory utilization.
   *
   * @sa xReturn
   * @sa xMemoryRegionStats
   * @sa xMemFree()
   *
   * @param  stats_ The memory region statistics. The memory region statistics
   *                must be freed by xMemFree().
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xMemGetHeapStats(xMemoryRegionStats *stats_);


  /**
   * @brief Syscall to get memory statistics on the kernel memory region
   *
   * The xMemGetKernelStats() syscall is used to obtain detailed statistics
   * about the kernel memory region which can be used by the application to
   * monitor memory utilization.
   *
   * @sa xReturn
   * @sa xMemoryRegionStats
   * @sa xMemFree()
   *
   * @param  stats_ The memory region statistics. The memory region statistics
   *                must be freed by xMemFree().
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xMemGetKernelStats(xMemoryRegionStats *stats_);


  /**
   * @brief Syscall to create a message queue
   *
   * The xQueueCreate() syscall will create a new message queue for inter-task
   * communication.
   *
   * @sa xReturn
   * @sa xQueue
   * @sa CONFIG_QUEUE_MINIMUM_LIMIT
   * @sa xQueueDelete()
   *
   * @param  queue_ The message queue to be operated on.
   * @param  limit_ The message limit for the queue. When this value is reached,
   *                the message queue is considered to be full. The minimume
   *                message limit is configured using the
   *                CONFIG_QUEUE_MINIMUM_LIMIT (default is 5) setting.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueCreate(xQueue *queue_, const xBase limit_);


  /**
   * @brief Syscall to delete a message queue
   *
   * The xQueueDelete() syscall will delete a message queue used for inter-task
   * communication.
   *
   * @sa xReturn
   * @sa xQueue
   * @sa xQueueCreate()
   *
   * @param  queue_ The message queue to be operated on.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueDelete(xQueue queue_);


  /**
   * @brief Syscall to get the length of a message queue
   *
   * The xQueueGetLength() syscall is used to inquire about the length (i.e.,
   * the number of messages) of a message queue.
   *
   * @sa xReturn
   * @sa xQueue
   *
   * @param  queue_ The message queue to be operated on.
   * @param  res_   The result of the inquiry; taken here to mean the number of
   *                messages a message queue contains.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueGetLength(const xQueue queue_, xBase *res_);


  /**
   * @brief Syscall to inquire as to whether a message queue is empty
   *
   * The xQueueIsQueueEmpty() syscall is used to inquire as to whether a message
   * queue is empty. A message queue is considered empty if the length (i.e.,
   * number of messages) of a queue is zero.
   *
   * @sa xReturn
   * @sa xQueue
   *
   * @param  queue_ The message queue to be operated on.
   * @param  res_   The result of the inquiry; taken here to mean "true" if the
   *                queue is empty, "false" if it contains one or more messages.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueIsQueueEmpty(const xQueue queue_, xBase *res_);


  /**
   * @brief Syscall to inquire as to whether a message queue is full
   *
   * The xQueueIsQueueFull() syscall is used to inquire as to whether a message
   * queue is full. A message queue is considered full if the length (i.e.,
   * number of messages) of a queue has reached its message limit which is
   * configured using the CONFIG_QUEUE_MINIMUM_LIMIT (default is 5) setting.
   *
   * @sa xReturn
   * @sa xQueue
   * @sa CONFIG_QUEUE_MINIMUM_LIMIT
   *
   * @param  queue_ The message queue to be operated on.
   * @param  res_   The result of the inquiry; taken here to mean "true" if the
   *                queue is full, "false" if it contains less than "limit"
   *                messages.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueIsQueueFull(const xQueue queue_, xBase *res_);


  /**
   * @brief Syscall to inquire as to whether a message queue has one or more
   * messages waiting
   *
   * The xQueueMessagesWaiting() syscall is used to inquire as to whether a
   * message queue has one or more messages waiting.
   *
   * @sa xReturn
   * @sa xQueue
   *
   * @param  queue_ The message queue to be operated on.
   * @param  res_   The result of the inquiry; taken here to mean "true" if
   *                there is one or more messages waiting.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueMessagesWaiting(const xQueue queue_, xBase *res_);


  /**
   * @brief Syscall to send a message to a message queue.
   *
   * The xQueueSend() syscall is used to send a message to a message queue. The
   * message value is an array of bytes (i.e., xByte) and cannot exceed
   * CONFIG_MESSAGE_VALUE_BYTES (default is 8) bytes in size.
   *
   * @sa xReturn
   * @sa xQueue
   * @sa xByte
   * @sa CONFIG_MESSAGE_VALUE_BYTES
   *
   * @param  queue_ The message queue to be operated on.
   * @param  bytes_ The size, in bytes, of the message to send to the message
   *                queue. The size of the message cannot exceed the
   *                CONFIG_MESSAGE_VALUE_BYTES (default is 8) setting.
   * @param  value_ The message to be sent to the queue.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueSend(xQueue queue_, const xBase bytes_, const xByte *value_);


  /**
   * @brief Syscall to retrieve a message from a message queue without dropping
   * the message
   *
   * The xQueuePeek() syscall is used to retrieve the next message from a
   * message queue without dropping the message (i.e., peek at the message).
   *
   * @sa xReturn
   * @sa xQueue
   * @sa xQueueMessage
   * @sa xMemFree()
   *
   * @param  queue_   The message queue to be operated on.
   * @param  message_ The message retrieved from the message queue. The message
   *                  must be freed by xMemFree().
   * @return          On success, the syscall returns ReturnOK. On failure, the
   *                  syscall returns ReturnError. A failure is any condition in
   *                  which the syscall was unable to achieve its intended
   *                  objective. For example, if xTaskGetId() was unable to
   *                  locate the task by the task object (i.e., xTask) passed to
   *                  the syscall, because either the object was null or invalid
   *                  (e.g., a deleted task), xTaskGetId() would return
   *                  ReturnError. All HeliOS syscalls return the xReturn
   *                  (a.k.a., Return_t) type which can either be ReturnOK or
   *                  ReturnError. The C macros OK() and ERROR() can be used as
   *                  a more concise way of checking the return value of a
   *                  syscall (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                  if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueuePeek(const xQueue queue_, xQueueMessage *message_);


  /**
   * @brief Syscall to drop a message from a message queue without retrieving
   * the message
   *
   * The xQueueDropMessage() syscall is used to drop the next message from a
   * message queue without retrieving the message.
   *
   * @sa xReturn
   * @sa xQueue
   *
   * @param  queue_ The message queue to be operated on.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueDropMessage(xQueue queue_);


  /**
   * @brief Syscall to retrieve and drop the next message from a message queue
   *
   * The xQueueReceive() syscall has the effect of calling xQueuePeek() followed
   * by xQueueDropMessage(). The syscall will receive the next message from the
   * message queue if there is a waiting message.
   *
   * @sa xReturn
   * @sa xQueue
   * @sa xQueueMessage
   * @sa xMemFree()
   *
   * @param  queue_   The message queue to be operated on.
   * @param  message_ The message retrieved from the message queue. The message
   *                  must be freed by xMemFree().
   * @return          On success, the syscall returns ReturnOK. On failure, the
   *                  syscall returns ReturnError. A failure is any condition in
   *                  which the syscall was unable to achieve its intended
   *                  objective. For example, if xTaskGetId() was unable to
   *                  locate the task by the task object (i.e., xTask) passed to
   *                  the syscall, because either the object was null or invalid
   *                  (e.g., a deleted task), xTaskGetId() would return
   *                  ReturnError. All HeliOS syscalls return the xReturn
   *                  (a.k.a., Return_t) type which can either be ReturnOK or
   *                  ReturnError. The C macros OK() and ERROR() can be used as
   *                  a more concise way of checking the return value of a
   *                  syscall (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                  if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueReceive(xQueue queue_, xQueueMessage *message_);


  /**
   * @brief Syscall to lock a message queue
   *
   * The xQueueLockQueue() syscall is used to lock a message queue. Locking a
   * message queue prevents tasks from sending messages to the queue but does
   * not prevent tasks from peeking, receiving or dropping messages from a
   * message queue.
   *
   * @sa xReturn
   * @sa xQueue
   * @sa xQueueUnLockQueue()
   *
   * @param  queue_ The message queue to be operated on.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueLockQueue(xQueue queue_);


  /**
   * @brief Syscall to unlock a message queue
   *
   * The xQueueUnLockQueue() syscall is used to unlock a message queue that was
   * previously locked by xQueueLockQueue(). Once a message queue is unlocked,
   * tasks may resume sending messages to the message queue.
   *
   * @sa xReturn
   * @sa xQueue
   * @sa xQueueLockQueue()
   *
   * @param  queue_ The message queue to be operated on.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xQueueUnLockQueue(xQueue queue_);


  /**
   * @brief Syscall to create a stream buffer
   *
   * The xStreamCreate() syscall is used to create a stream buffer which is used
   * for inter-task communications. A stream buffer is similar to a message
   * queue, however, it operates only on one byte at a time.
   *
   * @sa xReturn
   * @sa xStreamBuffer
   * @sa xStreamDelete()
   *
   * @param  stream_ The stream buffer to be operated on.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xStreamCreate(xStreamBuffer *stream_);


  /**
   * @brief Syscall to delete a stream buffer
   *
   * The xStreamDelete() syscall is used to delete a stream buffer created by
   * xStreamCreate().
   *
   * @sa xReturn
   * @sa xStreamBuffer
   * @sa xStreamCreate()
   *
   * @param  stream_ The stream buffer to be operated on.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xStreamDelete(const xStreamBuffer stream_);


  /**
   * @brief Syscall to send a byte to a stream buffer
   *
   * The xStreamSend() syscall is used to send one byte to a stream buffer.
   *
   * @sa xReturn
   * @sa xByte
   * @sa xStreamBuffer
   *
   * @param  stream_ The stream buffer to be operated on.
   * @param  byte_   The byte to send to the stream buffer.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xStreamSend(xStreamBuffer stream_, const xByte byte_);


  /**
   * @brief Syscall to retrieve all waiting bytes from a stream buffer
   *
   * The xStreamReceive() syscall is used to retrieve all waiting bytes from a
   * stream buffer.
   *
   * @sa xReturn
   * @sa xByte
   * @sa xStreamBuffer
   * @sa xMemFree()
   *
   * @param  stream_ The stream buffer to be operated on.
   * @param  bytes_  The number of bytes retrieved from the stream buffer.
   * @param  data_   The bytes retrieved from the stream buffer. The data must
   *                 be freed by xMemFree().
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xStreamReceive(const xStreamBuffer stream_, xHalfWord *bytes_, xByte **data_);


  /**
   * @brief Syscall to inquire about the number of bytes waiting in a stream
   * buffer
   *
   * The xStreamBytesAvailable() syscall is used to obtain the number of waiting
   * (i.e., available) bytes in a stream buffer.
   *
   * @sa xReturn
   * @sa xStreamBuffer
   *
   * @param  stream_ The stream buffer to be operated on.
   * @param  bytes_  The number of available bytes in the stream buffer.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xStreamBytesAvailable(const xStreamBuffer stream_, xHalfWord *bytes_);


  /**
   * @brief Syscall to reset a stream buffer
   *
   * The xStreamReset() syscall is used to reset a stream buffer. Resetting a
   * stream buffer has the effect of clearing the stream buffer such that
   * xStreamBytesAvailable() would return zero bytes available.
   *
   * @sa xReturn
   * @sa xStreamBuffer
   *
   * @param  stream_ The stream buffer to be operated on.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xStreamReset(const xStreamBuffer stream_);


  /**
   * @brief Syscall to inquire as to whether a stream buffer is empty
   *
   * The xStreamIsEmpty() syscall is used to inquire as to whether a stream
   * buffer is empty. An empty stream buffer has zero waiting (i.e.,available)
   * bytes.
   *
   * @sa xReturn
   * @sa xStreamBuffer
   *
   * @param  stream_ The stream buffer to be operated on.
   * @param  res_    The result of the inquiry; taken here to mean "true" if the
   *                 length (i.e., number of waiting bytes) is zero.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xStreamIsEmpty(const xStreamBuffer stream_, xBase *res_);


  /**
   * @brief Syscall to inquire as to whether a stream buffer is full
   *
   * The xStreamIsFull() syscall is used to inquire as to whether a stream
   * buffer is full. An full stream buffer has CONFIG_STREAM_BUFFER_BYTES
   * (default is 32) bytes waiting.
   *
   * @sa xReturn
   * @sa xStreamBuffer
   * @sa CONFIG_STREAM_BUFFER_BYTES
   *
   * @param  stream_ The stream buffer to be operated on.
   * @param  res_    The result of the inquiry; taken here to mean "true" if the
   *                 length (i.e., number of waiting bytes) is
   *                 CONFIG_STREAM_BUFFER_BYTES bytes.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xStreamIsFull(const xStreamBuffer stream_, xBase *res_);


  /**
   * @brief Syscall to to raise a system assert
   *
   * The xSystemAssert() syscall is used to raise a system assert. In order fot
   * xSystemAssert() to have an effect the configuration setting
   * CONFIG_SYSTEM_ASSERT_BEHAVIOR must be defined. That said, it is recommended
   * that the ASSERT C macro be used in place of xSystemAssert(). In order for
   * the ASSERT C macro to have any effect, the configuration setting
   * CONFIG_ENABLE_SYSTEM_ASSERT must be defined.
   *
   * @sa xReturn
   * @sa CONFIG_SYSTEM_ASSERT_BEHAVIOR
   * @sa CONFIG_ENABLE_SYSTEM_ASSERT
   * @sa ASSERT
   *
   * @param  file_ The C file where the assert occurred. This will be set by the
   *               ASSERT C macro.
   * @param  line_ The C file line where the assert occurred. This will be set
   *               by the ASSERT C macro.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xSystemAssert(const char *file_, const int line_);


  /**
   * @brief Syscall to bootstrap HeliOS
   *
   * The xSystemInit() syscall is used to bootstrap HeliOS and must be the first
   * syscall made in the user's application. The xSystemInit() syscall
   * initializes memory and calls initialization functions through the port
   * layer.
   *
   * @sa xReturn
   *
   * @return On success, the syscall returns ReturnOK. On failure, the syscall
   *         returns ReturnError. A failure is any condition in which the
   *         syscall was unable to achieve its intended objective. For example,
   *         if xTaskGetId() was unable to locate the task by the task object
   *         (i.e., xTask) passed to the syscall, because either the object was
   *         null or invalid (e.g., a deleted task), xTaskGetId() would return
   *         ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *         Return_t) type which can either be ReturnOK or ReturnError. The C
   *         macros OK() and ERROR() can be used as a more concise way of
   *         checking the return value of a syscall (e.g.,
   *         if(OK(xMemGetUsed(&size))) {} or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xSystemInit(void);


  /**
   * @brief Syscall to halt HeliOS
   *
   * The xSystemHalt() syscall is used to halt HeliOS. Once called,
   * xSystemHalt() will disable all interrupts and stops the execution of
   * further statements. The system will have to be reset to recover.
   *
   * @sa xReturn
   *
   * @return On success, the syscall returns ReturnOK. On failure, the syscall
   *         returns ReturnError. A failure is any condition in which the
   *         syscall was unable to achieve its intended objective. For example,
   *         if xTaskGetId() was unable to locate the task by the task object
   *         (i.e., xTask) passed to the syscall, because either the object was
   *         null or invalid (e.g., a deleted task), xTaskGetId() would return
   *         ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *         Return_t) type which can either be ReturnOK or ReturnError. The C
   *         macros OK() and ERROR() can be used as a more concise way of
   *         checking the return value of a syscall (e.g.,
   *         if(OK(xMemGetUsed(&size))) {} or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xSystemHalt(void);


  /**
   * @brief Syscall to inquire about the system
   *
   * The xSystemGetSystemInfo() syscall is used to inquire about the system. The
   * information bout the system that may be obtained is the product (i.e., OS)
   * name, version and number of tasks.
   *
   * @sa xReturn
   * @sa xSystemInfo
   * @sa xMemFree()
   *
   * @param  info_ The system information. The system information must be freed
   *               by xMemFree().
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xSystemGetSystemInfo(xSystemInfo *info_);


  /**
   * @brief Syscall to create a new task
   *
   * The xTaskCreate() syscall is used to create a new task. Neither the
   * xTaskCreate() or xTaskDelete() syscalls can be called from within a task
   * (i.e., while the scheduler is running).
   *
   * @sa xReturn
   * @sa xTaskDelete()
   * @sa xTask
   * @sa xTaskParm
   * @sa CONFIG_TASK_NAME_BYTES
   *
   * @param  task_          The task to be operated on.
   * @param  name_          The name of the task which must be exactly
   *                        CONFIG_TASK_NAME_BYTES (default is 8) bytes in
   *                        length. Shorter task names must be padded.
   * @param  callback_      The task's main (i.e., entry point) function.
   * @param  taskParameter_ A parameter which is accessible from the task's main
   *                        function. If a task parameter is not needed, this
   *                        parameter may be set to null.
   * @return                On success, the syscall returns ReturnOK. On
   *                        failure, the syscall returns ReturnError. A failure
   *                        is any condition in which the syscall was unable to
   *                        achieve its intended objective. For example, if
   *                        xTaskGetId() was unable to locate the task by the
   *                        task object (i.e., xTask) passed to the syscall,
   *                        because either the object was null or invalid (e.g.,
   *                        a deleted task), xTaskGetId() would return
   *                        ReturnError. All HeliOS syscalls return the xReturn
   *                        (a.k.a., Return_t) type which can either be ReturnOK
   *                        or ReturnError. The C macros OK() and ERROR() can be
   *                        used as a more concise way of checking the return
   *                        value of a syscall (e.g., if(OK(xMemGetUsed(&size)))
   *                        {} or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskCreate(xTask *task_, const xByte *name_, void (*callback_)(xTask task_, xTaskParm parm_), xTaskParm taskParameter_);


  /**
   * @brief Syscall to delete a task.
   *
   * The xTaskDelete() syscall is used to delete an existing task. Neither the
   * xTaskCreate() or xTaskDelete() syscalls can be called from within a task
   * (i.e., while the scheduler is running).
   *
   * @sa xReturn
   * @sa xTask
   *
   * @param  task_ The task to be operated on.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskDelete(const xTask task_);


  /**
   * @brief Syscall to get the task handle by name
   *
   * The xTaskGetHandleByName() syscall will get the task handle using the task
   * name.
   *
   * @sa xReturn
   * @sa xTask
   * @sa CONFIG_TASK_NAME_BYTES
   *
   * @param  task_ The task to be operated on.
   * @param  name_ The name of the task which must be exactly
   *               CONFIG_TASK_NAME_BYTES (default is 8) bytes in length.
   *               Shorter task names must be padded.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetHandleByName(xTask *task_, const xByte *name_);


  /**
   * @brief Syscall to get the task handle by task id
   *
   * The xTaskGetHandleById() syscall will get the task handle using the task
   * id.
   *
   * @sa xReturn
   * @sa xTask
   *
   * @param  task_ The task to be operated on.
   * @param  id_   The task id.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetHandleById(xTask *task_, const xBase id_);


  /**
   * @brief Syscall to get obtain the runtime statistics of all tasks
   *
   * The xTaskGetAllRunTimeStats() syscall is used to obtain the runtime
   * statistics of all tasks.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTaskRunTimeStats
   * @sa xMemFree()
   *
   * @param  stats_ The runtime statistics. The runtime statics must be freed by
   *                xMemFree().
   * @param  tasks_ The number of tasks in the runtime statistics.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetAllRunTimeStats(xTaskRunTimeStats *stats_, xBase *tasks_);


  /**
   * @brief Syscall to get the runtime statistics for a single task.
   *
   * The xTaskGetTaskRunTimeStats() syscall is used to get the runtime
   * statistics for a single task.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTaskRunTimeStats
   * @sa xMemFree()
   *
   * @param  task_  The task to be operated on.
   * @param  stats_ The runtime statistics. The runtime statistics must be freed
   *                by xMemFree().
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetTaskRunTimeStats(const xTask task_, xTaskRunTimeStats *stats_);


  /**
   * @brief Syscall to get the number of tasks
   *
   * The xTaskGetNumberOfTasks() syscall is used to obtain the number of tasks
   * regardless of their state (i.e., suspended, running or waiting).
   *
   * @sa xReturn
   *
   * @param  tasks_ The number of tasks.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetNumberOfTasks(xBase *tasks_);


  /**
   * @brief Syscall to get info about a task
   *
   * The xTaskGetTaskInfo() syscall is used to get info about a single task.
   * xTaskGetTaskInfo() is similar to xTaskGetTaskRunTimeStats() with one
   * difference, xTaskGetTaskInfo() provides the state and name of the task
   * along with the task's runtime statistics.
   *
   * @sa xReturn
   * @sa xMemFree()
   * @sa xTask
   * @sa xTaskInfo
   *
   * @param  task_ The task to be operated on.
   * @param  info_ Information about the task. The task information must be
   *               freed by xMemFree().
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetTaskInfo(const xTask task_, xTaskInfo *info_);


  /**
   * @brief Syscall to get info about all tasks
   *
   * The xTaskGetAllTaskInfo() syscall is used to get info about all tasks.
   * xTaskGetAllTaskInfo() is similar to xTaskGetAllRunTimeStats() with one
   * difference, xTaskGetAllTaskInfo() provides the state and name of the task
   * along with the task's runtime statistics.
   *
   * @sa xReturn
   * @sa xTaskInfo
   * @sa xMemFree()
   *
   * @param  info_  Information about the tasks. The task information must be
   *                freed by xMemFree().
   * @param  tasks_ The number of tasks.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetAllTaskInfo(xTaskInfo *info_, xBase *tasks_);


  /**
   * @brief Syscall to get the state of a task
   *
   * The xTaskGetTaskState() syscall is used to obtain the state of a task
   * (i.e., suspended, running or waiting).
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTaskState
   *
   * @param  task_  The task to be operated on.
   * @param  state_ The state of the task.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetTaskState(const xTask task_, xTaskState *state_);


  /**
   * @brief Syscall to get the name of a task
   *
   * The xTaskGetName() syscall is used to get the ASCII name of a task. The
   * size of the task name is CONFIG_TASK_NAME_BYTES (default is 8) bytes in
   * length.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xMemFree()
   *
   * @param  task_ The task to be operated on.
   * @param  name_ The task name which must be precisely CONFIG_TASK_NAME_BYTES
   *               (default is 8) bytes in length. The task name must be freed
   *               by xMemFree().
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetName(const xTask task_, xByte **name_);


  /**
   * @brief Syscall to get the task id of a task
   *
   * The xTaskGetId() syscall is used to obtain the id of a task.
   *
   * @sa xReturn
   * @sa xTask
   *
   * @param  task_ The task to be operated on.
   * @param  id_   The id of the task.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetId(const xTask task_, xBase *id_);


  /**
   * @brief Syscall to clear a waiting direct-to-task notification
   *
   * The xTaskNotifyStateClear() syscall is used to clear a waiting
   * direct-to-task notification for the given task.
   *
   * @sa xReturn
   * @sa xTask
   *
   * @param  task_ The task to be operated on.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskNotifyStateClear(xTask task_);


  /**
   * @brief Syscall to inquire as to whether a direct-to-task notification is
   * waiting
   *
   * The xTaskNotificationIsWaiting() syscall is used to inquire as to whether a
   * direct-to-task notification is waiting for the given task.
   *
   * @sa xReturn
   * @sa xTask
   *
   * @param  task_ Task to be operated on.
   * @param  res_  The result of the inquiry; taken here to mean "true" if there
   *               is a waiting direct-to-task notification. Otherwise "false",
   *               if there is not a waiting direct-to-notification.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskNotificationIsWaiting(const xTask task_, xBase *res_);


  /**
   * @brief Syscall to give (i.e., send) a task a direct-to-task notification
   *
   * The xTaskNotifyGive() syscall is used to give (i.e., send) a direct-to-task
   * notification to the given task.
   *
   * @sa xReturn
   * @sa xTask
   * @sa CONFIG_NOTIFICATION_VALUE_BYTES
   *
   * @param  task_  The task to be operated on.
   * @param  bytes_ The number of bytes contained in the notification value. The
   *                number of bytes in the notification value cannot exceed
   *                CONFIG_NOTIFICATION_VALUE_BYTES (default is 8) bytes.
   * @param  value_ The notification value which is a byte array whose length is
   *                defined by "bytes_".
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskNotifyGive(xTask task_, const xBase bytes_, const xByte *value_);


  /**
   * @brief Syscall to take (i.e. receive) a waiting direct-to-task notification
   *
   * The xTaskNotifyTake() syscall is used to take (i.e., receive) a waiting
   * direct-to-task notification.
   *
   * @sa xReturn
   * @sa xTask
   * @sa CONFIG_NOTIFICATION_VALUE_BYTES
   * @sa xTaskNotification
   *
   * @param  task_         The task to be operated on.
   * @param  notification_ The direct-to-task notification.
   * @return               On success, the syscall returns ReturnOK. On failure,
   *                       the syscall returns ReturnError. A failure is any
   *                       condition in which the syscall was unable to achieve
   *                       its intended objective. For example, if xTaskGetId()
   *                       was unable to locate the task by the task object
   *                       (i.e., xTask) passed to the syscall, because either
   *                       the object was null or invalid (e.g., a deleted
   *                       task), xTaskGetId() would return ReturnError. All
   *                       HeliOS syscalls return the xReturn (a.k.a., Return_t)
   *                       type which can either be ReturnOK or ReturnError. The
   *                       C macros OK() and ERROR() can be used as a more
   *                       concise way of checking the return value of a syscall
   *                       (e.g., if(OK(xMemGetUsed(&size)))
   *                       {} or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskNotifyTake(xTask task_, xTaskNotification *notification_);


  /**
   * @brief Syscall to place a task in the "running" state
   *
   * The xTaskResume() syscall will place a task in the "running" state. A task
   * in this state will run continuously until suspended and is scheduled to run
   * cooperatively by the HeliOS scheduler.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTaskResume()
   * @sa xTaskSuspend()
   * @sa xTaskWait()
   *
   * @param  task_ The task to be operated on.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskResume(xTask task_);


  /**
   * @brief Syscall to place a task in the "suspended" state
   *
   * The xTaskSuspend() syscall will place a task in the "suspended" state. A
   * task in this state is not scheduled to run by the HeliOS scheduler and will
   * not run.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTaskResume()
   * @sa xTaskSuspend()
   * @sa xTaskWait()
   *
   * @param  task_ The task to be operated on.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskSuspend(xTask task_);


  /**
   * @brief Syscall to place a task in the "waiting" state
   *
   * The xTaskWait() syscall will place a task in the "waiting" state. A task in
   * this state is not scheduled to run by the HeliOS scheduler *UNTIL* an event
   * occurs. When an event occurs, the HeliOS will schedule the task to run
   * until the even has passed (e.g., the task either "takes" or "clears a
   * direct-to-task notification"). Tasks in the "waiting" state are tasks that
   * are using event-driven multitasking. HeliOS supports two types of events:
   * task timers and direct-to-task notifications.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTaskResume()
   * @sa xTaskSuspend()
   * @sa xTaskWait()
   *
   * @param  task_ The task to be operated on.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskWait(xTask task_);


  /**
   * @brief Syscall to change the interval period of a task timer
   *
   * The xTaskChangePeriod() is used to change the interval period of a task
   * timer. The period is measured in ticks. While architecture and/or platform
   * dependent, a tick is often one millisecond. In order for the task timer to
   * have an effect, the task must be in the "waiting" state which can be set
   * using xTaskWait().
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTicks
   * @sa xTaskWait()
   *
   * @param  task_   The task to be operated on.
   * @param  period_ The interval period in ticks.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskChangePeriod(xTask task_, const xTicks period_);


  /**
   * @brief Syscall to change the task watchdog timer period
   *
   * The xTaskChangeWDPeriod() syscall is used to change the task watchdog timer
   * period. This has no effect unless CONFIG_TASK_WD_TIMER_ENABLE is defined
   * and the watchdog timer period is greater than zero. The task watchdog timer
   * will place a task in a suspended state if a task's runtime exceeds the
   * watchdog timer period. The task watchdog timer period is set on a per task
   * basis.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTicks
   * @sa CONFIG_TASK_WD_TIMER_ENABLE
   *
   *
   * @param  task_   The task to be operated on.
   * @param  period_ The task watchdog timer period measured in ticks. Ticks is
   *                 platform and/or architecture dependent. However, most
   *                 platforms and/or architectures have a one millisecond tick
   *                 duration.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskChangeWDPeriod(xTask task_, const xTicks period_);


  /**
   * @brief Syscall to obtain the task timer period
   *
   * The xTaskGetPeriod() syscall is used to obtain the current task timer
   * period.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTicks
   *
   * @param  task_   The task to be operated on.
   * @param  period_ The task timer period in ticks. Ticks is platform and/or
   *                 architecture dependent. However, most platforms and/or
   *                 architect
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetPeriod(const xTask task_, xTicks *period_);


  /**
   * @brief Syscall to set the task timer elapsed time to zero
   *
   * The xTaskResetTimer() syscall is used to reset the task timer. In effect,
   * this sets the elapsed time, measured in ticks, back to zero.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTicks
   *
   * @param  task_ The task to be operated on.
   * @return       On success, the syscall returns ReturnOK. On failure, the
   *               syscall returns ReturnError. A failure is any condition in
   *               which the syscall was unable to achieve its intended
   *               objective. For example, if xTaskGetId() was unable to locate
   *               the task by the task object (i.e., xTask) passed to the
   *               syscall, because either the object was null or invalid (e.g.,
   *               a deleted task), xTaskGetId() would return ReturnError. All
   *               HeliOS syscalls return the xReturn (a.k.a., Return_t) type
   *               which can either be ReturnOK or ReturnError. The C macros
   *               OK() and ERROR() can be used as a more concise way of
   *               checking the return value of a syscall (e.g.,
   *               if(OK(xMemGetUsed(&size))) {} or
   *               if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskResetTimer(xTask task_);


  /**
   * @brief Syscall to start the HeliOS scheduler
   *
   * The xTaskStartScheduler() syscall is used to start the HeliOS task
   * scheduler. On this syscall is made, control is handed over to HeliOS. In
   * order to suspend the scheduler and return to the caller, the
   * xTaskSuspendAll() syscall will need to be made. Once a call to
   * xTaskSuspendAll() is made, xTaskResumeAll() must be called before calling
   * xTaskStartScheduler() again. If xTaskStartScheduler() is called while the
   * scheduler is in a suspended state, xTaskStartScheduler() will immediately
   * return.
   *
   * @sa xReturn
   * @sa xTaskResumeAll()
   * @sa xTaskSuspendAll()
   *
   *
   * @return On success, the syscall returns ReturnOK. On failure, the syscall
   *         returns ReturnError. A failure is any condition in which the
   *         syscall was unable to achieve its intended objective. For example,
   *         if xTaskGetId() was unable to locate the task by the task object
   *         (i.e., xTask) passed to the syscall, because either the object was
   *         null or invalid (e.g., a deleted task), xTaskGetId() would return
   *         ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *         Return_t) type which can either be ReturnOK or ReturnError. The C
   *         macros OK() and ERROR() can be used as a more concise way of
   *         checking the return value of a syscall (e.g.,
   *         if(OK(xMemGetUsed(&size))) {} or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskStartScheduler(void);


  /**
   * @brief Syscall to set the scheduler state to running
   *
   * The xTaskResumeAll() syscall is used to set the scheduler state to running.
   * xTaskStartScheduler() must still be called to pass control to the
   * scheduler. If the scheduler state is not running, then
   * xTaskStartScheduler() will simply return to the caller when called.
   *
   * @sa xReturn
   * @sa xTaskStartScheduler()
   * @sa xTaskResumeAll()
   * @sa xTaskSuspendAll()
   *
   * @return On success, the syscall returns ReturnOK. On failure, the syscall
   *         returns ReturnError. A failure is any condition in which the
   *         syscall was unable to achieve its intended objective. For example,
   *         if xTaskGetId() was unable to locate the task by the task object
   *         (i.e., xTask) passed to the syscall, because either the object was
   *         null or invalid (e.g., a deleted task), xTaskGetId() would return
   *         ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *         Return_t) type which can either be ReturnOK or ReturnError. The C
   *         macros OK() and ERROR() can be used as a more concise way of
   *         checking the return value of a syscall (e.g.,
   *         if(OK(xMemGetUsed(&size))) {} or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskResumeAll(void);


  /**
   * @brief Syscall to set the scheduler state to suspended
   *
   * The xTaskSuspendAll() syscall is used to set the scheduler state to
   * suspended. If called from a running task, the HeliOS scheduler will quit
   * and return control back to the caller. To set the scheduler state to
   * running, xTaskResumeAll() must be called followed by a call to
   * xTaskStartScheduler().
   *
   * @sa xReturn
   * @sa xTaskStartScheduler()
   * @sa xTaskResumeAll()
   * @sa xTaskSuspendAll()
   *
   * @return On success, the syscall returns ReturnOK. On failure, the syscall
   *         returns ReturnError. A failure is any condition in which the
   *         syscall was unable to achieve its intended objective. For example,
   *         if xTaskGetId() was unable to locate the task by the task object
   *         (i.e., xTask) passed to the syscall, because either the object was
   *         null or invalid (e.g., a deleted task), xTaskGetId() would return
   *         ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *         Return_t) type which can either be ReturnOK or ReturnError. The C
   *         macros OK() and ERROR() can be used as a more concise way of
   *         checking the return value of a syscall (e.g.,
   *         if(OK(xMemGetUsed(&size))) {} or if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskSuspendAll(void);


  /**
   * @brief Syscall to get the scheduler state
   *
   * The xTaskGetSchedulerState() is used to get the state of the scheduler.
   *
   * @sa xReturn
   * @sa xSchedulerState
   *
   * @param  state_ The state of the scheduler.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetSchedulerState(xSchedulerState *state_);


  /**
   * @brief Syscall to get the task watchdog timer period
   *
   * The xTaskGetWDPeriod() syscall is used to obtain the task watchdog timer
   * period.
   *
   * @sa xReturn
   * @sa xTask
   * @sa xTicks
   * @sa CONFIG_TASK_WD_TIMER_ENABLE
   *
   * @param  task_   The task to be operated on.
   * @param  period_ The task watchdog timer period, measured in ticks. Ticks
   *                 are platform and/or architecture dependent. However, on
   *                 must platforms and/or architectures the tick represents one
   *                 millisecond.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTaskGetWDPeriod(const xTask task_, xTicks *period_);


  /**
   * @brief Syscall to create an application timer
   *
   * The xTimerCreate() syscall is used to create a new application timer.
   * Application timers are not the same as task timers. Application timers are
   * not part of HeliOS's event-driven multitasking. Application timers are just
   * that, timers for use by the user's application for general purpose
   * timekeeping. Application timers can be started, stopped, reset and have
   * time period, measured in ticks, that elapses.
   *
   * @sa xReturn
   * @sa xTimer
   * @sa xTicks
   * @sa xTimerDelete()
   *
   * @param  timer_  The application timer to be operated on.
   * @param  period_ The application timer period, measured in ticks.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTimerCreate(xTimer *timer_, const xTicks period_);


  /**
   * @brief Syscall to delete an application timer
   *
   * The xTimerDelete() syscall is used to delete an application timer created
   * with xTimerCreate().
   *
   * @sa xReturn
   * @sa xTimer
   * @sa xTicks
   * @sa xTimerCreate()
   *
   * @param  timer_ The application timer to be operated on.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTimerDelete(const xTimer timer_);


  /**
   * @brief Syscall to change the period on an application timer
   *
   * The xTimerChangePeriod() syscall is used to change the time period on an
   * application timer. Once the period has elapsed, the application timer is
   * considered expired.
   *
   * @sa xReturn
   * @sa xTimer
   * @sa xTicks
   *
   * @param  timer_  The application timer to be operated on.
   * @param  period_ The application timer period, measured in ticks.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTimerChangePeriod(xTimer timer_, const xTicks period_);


  /**
   * @brief Syscall to get the current period for an application timer
   *
   * The xTimerGetPeriod() syscall is used to obtain the current period for an
   * application timer.
   *
   * @sa xReturn
   * @sa xTimer
   * @sa xTicks
   *
   * @param  timer_  The application timer to be operate don.
   * @param  period_ The application timer period, measured in ticks.
   * @return         On success, the syscall returns ReturnOK. On failure, the
   *                 syscall returns ReturnError. A failure is any condition in
   *                 which the syscall was unable to achieve its intended
   *                 objective. For example, if xTaskGetId() was unable to
   *                 locate the task by the task object (i.e., xTask) passed to
   *                 the syscall, because either the object was null or invalid
   *                 (e.g., a deleted task), xTaskGetId() would return
   *                 ReturnError. All HeliOS syscalls return the xReturn
   *                 (a.k.a., Return_t) type which can either be ReturnOK or
   *                 ReturnError. The C macros OK() and ERROR() can be used as a
   *                 more concise way of checking the return value of a syscall
   *                 (e.g., if(OK(xMemGetUsed(&size))) {} or
   *                 if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTimerGetPeriod(const xTimer timer_, xTicks *period_);


  /**
   * @brief Syscall to inquire as to whether an application timer is active
   *
   * The xTimerIsTimerActive() syscall is used to inquire as to whether an
   * application timer is active. An application timer is considered to be
   * active if the application timer has been started by xTimerStare().
   *
   * @sa xReturn
   * @sa xTimer
   * @sa xTimerStart()
   * @sa xTimerStop()
   *
   * @param  timer_ The application timer to be operated on.
   * @param  res_   The result of the inquiry; taken here to mean "true" if the
   *                application timer is running. "False" if the application
   *                timer is not running.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTimerIsTimerActive(const xTimer timer_, xBase *res_);


  /**
   * @brief Syscall to inquire as to whether an application timer has expired
   *
   * The xTimerHasTimerExpired() syscall is used to inquire as to whether an
   * application timer has expired. If the application timer has expired, it
   * must be reset with xTimerReset(). If a timer is not active (i.e., started),
   * it cannot expire even if the timer period has elapsed.
   *
   * @sa xReturn
   * @sa xTimer
   * @sa xTimerReset()
   *
   * @param  timer_ The application timer to be operated on.
   * @param  res_   The result of the inquiry; taken here to mean "true" if the
   *                application timer has elapsed (i.e., expired). "False" if
   *                the application timer has not expired
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTimerHasTimerExpired(const xTimer timer_, xBase *res_);


  /**
   * @brief Syscall to reset an application timer
   *
   * The xTimerReset() syscall is used to reset an application timer. Resetting
   * has the effect of setting the application timer's elapsed time to zero.
   *
   * @sa xReturn
   * @sa xTimer
   * @sa xTimerReset()
   * @sa xTimerStart()
   * @sa xTimerStop()
   *
   * @param  timer_ The application timer to be operated on.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTimerReset(xTimer timer_);


  /**
   * @brief Syscall to place an application timer in the running state
   *
   * The xTimerStart() syscall is used to place an application timer in the
   * running state.
   *
   * @sa xReturn
   * @sa xTimer
   * @sa xTimerReset()
   * @sa xTimerStart()
   * @sa xTimerStop()
   *
   * @param  timer_ The application timer to be operated on.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTimerStart(xTimer timer_);


  /**
   * @brief Syscall to place an application timer in the suspended state
   *
   * The xTimerStop() syscall is used to place an application timer in the
   * suspended state.
   *
   * @sa xReturn
   * @sa xTimer
   * @sa xTimerReset()
   * @sa xTimerStart()
   * @sa xTimerStop()
   *
   * @param  timer_ The application timer to be operated on.
   * @return        On success, the syscall returns ReturnOK. On failure, the
   *                syscall returns ReturnError. A failure is any condition in
   *                which the syscall was unable to achieve its intended
   *                objective. For example, if xTaskGetId() was unable to locate
   *                the task by the task object (i.e., xTask) passed to the
   *                syscall, because either the object was null or invalid
   *                (e.g., a deleted task), xTaskGetId() would return
   *                ReturnError. All HeliOS syscalls return the xReturn (a.k.a.,
   *                Return_t) type which can either be ReturnOK or ReturnError.
   *                The C macros OK() and ERROR() can be used as a more concise
   *                way of checking the return value of a syscall (e.g.,
   *                if(OK(xMemGetUsed(&size))) {} or
   *                if(ERROR(xMemGetUsed(&size))) {}).
   */
  xReturn xTimerStop(xTimer timer_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */


  #ifdef __cplusplus

    #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || \
    defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || \
    defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)
      String xByte2String(xSize size_, xByte *bytes_);


      /* This is here to give Arduino users an easy way to convert from the
       * HeliOS byte (xByte) array which is NOT null terminated to a String. */
      String xByte2String(xSize size_, xByte *bytes_) {
        String str = "";
        xSize i = 0;
        char buf[size_ + 1];


        if(NOTNULLPTR(bytes_) && (zero < size_)) {
          for(i = 0; i < size_; i++) {
            buf[i] = (char) bytes_[i];
          }

          buf[size_] = '\0';
          str = String(buf);
        }

        return(str);
      }


    #endif /* if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) ||
            * defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) ||
            * defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_TEENSY_MICROMOD) ||
            * defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) ||
            * defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) ||
            * defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) ||
            * defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC) */

  #endif /* ifdef __cplusplus */

#endif /* ifndef HELIOS_H_ */