/*UNCRUSTIFY-OFF*/
/**
 * @file HeliOS.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Public API header for HeliOS embedded operating system applications
 * @version 0.5.0
 * @date 2023-03-19
 *
 * This header file provides the complete public API for HeliOS, a lightweight
 * embedded operating system designed for resource-constrained microcontrollers.
 * It includes all type definitions, enumerations, and system call (syscall)
 * declarations needed to develop applications on HeliOS.
 *
 * HeliOS provides the following subsystems:
 * - Task Management: Create and manage cooperative multitasking
 * - Memory Management: Dynamic heap allocation with safety checks
 * - Device I/O: Abstract device driver interface
 * - Timers: Software timers for periodic and one-shot events
 * - Queues: Inter-task message passing with FIFO semantics
 * - Streams: Byte-oriented data buffers for streaming I/O
 * - Filesystem: FAT32 filesystem support with block device abstraction
 *
 * All HeliOS system calls follow the naming convention xSubsystem*() where
 * x is the prefix for public APIs and Subsystem identifies the functional area
 * (e.g., Task_t *, xMem, Queue_t *).
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef HELIOS_H_
  #define HELIOS_H_

  #include "posix.h"

  #include <stdint.h>

  #include "config.h"
  #include "defines.h"

  #if defined(DEREF_TASKPARM)
    #undef DEREF_TASKPARM
  #endif /* if defined(DEREF_TASKPARM) */
  #define DEREF_TASKPARM(type_, ptr_) (*((type_ *) (ptr_)))


/**
 * @brief Enumerated type defining the possible states of a task
 *
 * Tasks in HeliOS transition between three distinct states that control their
 * execution behavior by the cooperative scheduler. The task state determines
 * whether the scheduler will invoke the task's function during scheduling
 * cycles.
 *
 * State Transitions:
 * - New tasks begin in TaskStateSuspended
 * - xTaskResume() transitions a task to TaskStateRunning
 * - xTaskSuspend() transitions a task to TaskStateSuspended
 * - xTaskWait() transitions a task to TaskStateWaiting
 *
 * Scheduling Behavior:
 * - TaskStateSuspended: Task will NOT be scheduled for execution
 * - TaskStateRunning: Task will be scheduled normally based on its period
 * - TaskStateWaiting: Task will be scheduled only after a task event occurs
 * (e.g., timer expiration, notification)
 *
 * @note HeliOS uses cooperative multitasking. Tasks must voluntarily yield
 * control to allow other tasks to execute.
 *
 * @sa TaskState_t
 * @sa xTaskResume()
 * @sa xTaskSuspend()
 * @sa xTaskWait()
 * @sa xTaskGetTaskState()
 *
 */
  typedef enum TaskState_e {
    TaskStateSuspended, /**< Task is inactive and will not be scheduled. This is
                         * the initial state after task creation and the state
                         * after calling xTaskSuspend(). */
    TaskStateRunning, /**< Task is active and will be scheduled for execution
                       * according to its period. Set by calling xTaskResume().
                       */
    TaskStateWaiting /**< Task is waiting for an event and will only be
                      * scheduled when that event occurs (timer, notification,
                      * etc.). Set by calling xTaskWait(). */
  } TaskState_t;


  /**
   * @brief Public API type alias for task states
   *
   * This is the user-facing type name for task states, providing a consistent
   * naming convention across the HeliOS public API where all types are prefixed
   * with 'x'.
   *
   * @sa TaskState_t
   *
   */
  /**
   * @brief Enumerated type for scheduler state
   *
   * The scheduler can be in one of three possible states as defined by the
   * SchedulerState_t enumerated data type. The state the scheduler is in is
   * changed by calling xTaskSuspendAll() and xTaskResumeAll(). The state the
   * scheduler is in can be obtained by calling xTaskGetSchedulerState().
   *
   * @sa SchedulerState_t
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
   * @sa Return_t
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
   * @sa TaskParm_t *
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
  /**
   * @brief Data type for the base type
   *
   * The Base_t type is a simple data type often used as an argument or result
   * type for syscalls when the value is known not to exceed its 8-bit width and
   * no data structure requirements exist. There are no guarantees the Base_t
   * will always be 8-bits wide. If an 8-bit data type is needed that is
   * guaranteed to remain 8-bits wide, the Byte_t data type should be used.
   *
   * @sa Base_t
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
  /**
   * @brief Data type for an 8-bit wide byte
   *
   * The Byte_t type is an 8-bit wide data type and is guaranteed to always be
   * 8-bits wide.
   *
   * @sa Byte_t
   *
   */
  typedef UINT8_TYPE Byte_t;


  /**
   * @brief Data type for an 8-bit wide byte
   *
   * @sa Byte_t
   *
   */
  /**
   * @brief Data type for a pointer to a memory address
   *
   * The Addr_t type is a pointer of type void and is used to pass addresses
   * between the end-user application and syscalls. It is not necessary to use
   * the Addr_t type within the end-user application as long as the type is not
   * used to interact with the kernel through syscalls
   *
   * @sa Addr_t *
   *
   */
  typedef VOID_TYPE Addr_t;


  /**
   * @brief Data type for a pointer to a memory address
   *
   * @sa Addr_t
   *
   */
  /**
   * @brief Data type for the storage requirements of an object in memory
   *
   * The Size_t type is used for the storage requirements of an object in memory
   * and is always represented in bytes.
   *
   * @sa Size_t
   *
   */
  typedef SIZE_TYPE Size_t;


  /**
   * @brief Data type for the storage requirements of an object in memory
   *
   * @sa Size_t
   *
   */
  /**
   * @brief Data type for a 16-bit half word
   *
   * The HalfWord_t type is a 16-bit wide data type and is guaranteed to always
   * be 16-bits wide.
   *
   * @sa HalfWord_t
   *
   */
  typedef UINT16_TYPE HalfWord_t;


  /**
   * @brief Data type for a 16-bit half word
   *
   * @sa HalfWord_t
   *
   */
  /**
   * @brief Data type for a 32-bit word
   *
   * The Word_t type is a 32-bit wide data type and is guaranteed to always be
   * 32-bits wide.
   *
   * @sa Word_t
   *
   */
  typedef UINT32_TYPE Word_t;


  /**
   * @brief Data type for a 32-bit word
   *
   * @sa Word_t
   *
   */
  /**
   * @brief Data type for system ticks
   *
   * The Ticks_t type is used to store ticks from the system clock. Ticks is not
   * bound to any one unit of measure for time though most systems are
   * configured for millisecond resolution, milliseconds is not guaranteed and
   * is dependent on the system clock frequency and prescaler.
   *
   * @sa Ticks_t
   *
   */
  typedef UINT32_TYPE Ticks_t;


  /**
   * @brief Data type for system ticks
   *
   * @sa Ticks_t
   *
   */
  /**
   * @brief Data type for a task
   *
   * The Task_t data type is used as a task. The task is created when
   * xTaskCreate() is called. For more information about tasks, see
   * xTaskCreate().
   *
   * @sa Task_t *
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
  /**
   * @brief Data type for a timer
   *
   * The Timer_t data type is used as a timer. The timer is created when
   * xTimerCreate() is called. For more information about timers, see
   * xTimerCreate().
   *
   * @sa Timer_t *
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
  /**
   * @brief Data type for a queue
   *
   * The Queue_t data type is used as a queue The queue is created when
   * xQueueCreate() is called. For more information about queues, see
   * xQueueCreate().
   *
   * @sa Queue_t *
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
  /**
   * @brief Data structure for FAT32 volume metadata
   *
   * @sa Volume_t *
   * @sa xFSMount()
   * @sa xFSUnmount()
   *
   */
  typedef struct Volume_s {
    HalfWord_t blockDeviceUID;
    Word_t fatStartSector;
    Word_t dataStartSector;
    Word_t rootDirCluster;
    Byte_t sectorsPerCluster;
    HalfWord_t bytesPerSector;
    HalfWord_t reservedSectors;
    Byte_t numFATs;
    Word_t sectorsPerFAT;
    Base_t mounted;
  } Volume_t;


  /**
   * @brief Data type for a FAT32 volume
   *
   * @sa Volume_t
   * @sa xFSMount()
   * @sa xFSUnmount()
   *
   */
  /**
   * @brief Data structure for file handle
   *
   * @sa File_t *
   * @sa xFileOpen()
   * @sa xFileClose()
   *
   */
  typedef struct File_s {
    struct Volume_s *volume;
    Word_t firstCluster;
    Word_t currentCluster;
    Word_t fileSize;
    Word_t position;
    Byte_t mode;
    Base_t isOpen;
    Base_t isDirty;
  } File_t;


  /**
   * @brief Data type for a file handle
   *
   * @sa File_t
   * @sa xFileOpen()
   * @sa xFileClose()
   *
   */
  /**
   * @brief Data structure for directory handle
   *
   * @sa Dir_t *
   * @sa xDirOpen()
   * @sa xDirClose()
   *
   */
  typedef struct Dir_s {
    struct Volume_s *volume;
    Word_t currentCluster;
    HalfWord_t entryIndex;
    Base_t isOpen;
  } Dir_t;


  /**
   * @brief Data type for a directory handle
   *
   * @sa Dir_t
   * @sa xDirOpen()
   * @sa xDirClose()
   *
   */
  /**
   * @brief Data structure for directory entry information
   *
   * @sa DirEntry_t *
   * @sa xDirRead()
   * @sa xFileGetInfo()
   *
   */
  typedef struct DirEntry_s {
    Byte_t name[256];
    Word_t size;
    Word_t firstCluster;
    Base_t isDirectory;
    Base_t isReadOnly;
    Base_t isHidden;
    Base_t isSystem;
  } DirEntry_t;


  /**
   * @brief Data type for a directory entry
   *
   * @sa DirEntry_t
   * @sa xDirRead()
   * @sa xFileGetInfo()
   *
   */
  /**
   * @brief Data structure for volume information
   *
   * @sa VolumeInfo_t *
   * @sa xFSGetVolumeInfo()
   *
   */
  typedef struct VolumeInfo_s {
    Word_t totalClusters;
    Word_t freeClusters;
    Word_t totalBytes;
    Word_t freeBytes;
    HalfWord_t bytesPerSector;
    Byte_t sectorsPerCluster;
    Word_t bytesPerCluster;
  } VolumeInfo_t;


  /**
   * @brief Data type for volume information
   *
   * @sa VolumeInfo_t
   * @sa xFSGetVolumeInfo()
   *
   */
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
   * @sa TaskNotification_t *
   * @sa xMemFree()
   * @sa xTaskNotifyGive()
   * @sa xTaskNotifyTake()
   * @sa xTaskWait()
   *
   */

  #ifndef HELIOS_PUBLIC_STRUCTURES_DEFINED_
    #define HELIOS_PUBLIC_STRUCTURES_DEFINED_
    typedef struct TaskNotification_s {
      Base_t notificationBytes; /**< The length in bytes of the notification
                                 * value which cannot exceed
                                 * CONFIG_NOTIFICATION_VALUE_BYTES. */
      Byte_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES]; /**< The
                                                                  * notification
                                                                  * value whose
                                                                  * length is
                                                                  * specified by
                                                                  * the
                                                                  * notification
                                                                  * bytes
                                                                  * member.
                                                                  */
    } TaskNotification_t;


    /**
     * @brief Data structure for a direct to task notification
     *
     * @sa TaskNotification_t
     *
     */
    /**
     * @brief Data structure for task runtime statistics
     *
     * The TaskRunTimeStats_t data structure is used by
     * xTaskGetTaskRunTimeStats() and xTaskGetAllRuntimeStats() to obtain
     * runtime statistics about a task.
     *
     * @sa TaskRunTimeStats_t *
     * @sa xTaskGetTaskRunTimeStats()
     * @sa xTaskGetAllRunTimeStats()
     * @sa xMemFree()
     *
     */
    typedef struct TaskRunTimeStats_s {
      Base_t id; /**< The ID of the task. */
      Ticks_t lastRunTime; /**< The duration in ticks of the task's last
                            * runtime.
                            */
      Ticks_t totalRunTime; /**< The duration in ticks of the task's total
                             * runtime. */
    } TaskRunTimeStats_t;


    /**
     * @brief Data structure for task runtime statistics
     *
     */
    /**
     * @brief Data structure for memory region statistics
     *
     * The MemoryRegionStats_t data structure is used by xMemGetHeapStats() and
     * xMemGetKernelStats() to obtain statistics about either memory region.
     *
     * @sa MemoryRegionStats_t *
     * @sa xMemGetHeapStats()
     * @sa xMemGetKernelStats()
     * @sa xMemFree()
     *
     */
    typedef struct MemoryRegionStats_s {
      Word_t largestFreeEntryInBytes; /**< The largest free entry in bytes. */
      Word_t smallestFreeEntryInBytes; /**< The smallest free entry in bytes. */
      Word_t numberOfFreeBlocks; /**< The number of free blocks. See
                                  * CONFIG_MEMORY_REGION_BLOCK_SIZE for block
                                  * size in bytes. */
      Word_t availableSpaceInBytes; /**< The amount of free memory in bytes
                                     * (i.e., numberOfFreeBlocks *
                                     * CONFIG_MEMORY_REGION_BLOCK_SIZE). */
      Word_t successfulAllocations; /**< Number of successful memory
                                     * allocations.
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
    /**
     * @brief Data structure for information about a task
     *
     * The TaskInfo_t structure is similar to xTaskRuntimeStats_t in that it
     * contains runtime statistics for a task. However, TaskInfo_t also contains
     * additional details about a task such as its name and state. The
     * TaskInfo_t structure is returned by xTaskGetTaskInfo() and
     * xTaskGetAllTaskInfo(). If only runtime statistics are needed, then
     * TaskRunTimeStats_t should be used because of its smaller memory
     * footprint.
     *
     * @sa TaskInfo_t *
     * @sa xTaskGetTaskInfo()
     * @sa xTaskGetAllTaskInfo()
     * @sa CONFIG_TASK_NAME_BYTES
     * @sa xMemFree()
     *
     */
    typedef struct TaskInfo_s {
      Base_t id; /**< The ID of the task. */
      Byte_t name[CONFIG_TASK_NAME_BYTES]; /**< The name of the task which must
                                            * be exactly CONFIG_TASK_NAME_BYTES
                                            * bytes in length. Shorter task
                                            * names must be padded. */
      TaskState_t state; /**< The state the task is in which is one of four
                          * states specified in the TaskState_t enumerated data
                          * type. */
      Ticks_t lastRunTime; /**< The duration in ticks of the task's last
                            * runtime.
                            */
      Ticks_t totalRunTime; /**< The duration in ticks of the task's total
                             * runtime. */
    } TaskInfo_t;


    /**
     * @brief Data structure for information about a task
     *
     */
    /**
     * @brief Data structure for a queue message
     *
     * The QueueMessage_t stucture is used to store a queue message and is
     * returned by xQueueReceive() and xQueuePeek().
     *
     * @sa QueueMessage_t *
     * @sa xQueueReceive()
     * @sa xQueuePeek()
     * @sa CONFIG_MESSAGE_VALUE_BYTES
     * @sa xMemFree()
     *
     */
    typedef struct QueueMessage_s {
      Base_t messageBytes; /**< The number of bytes contained in the message
                            * value which cannot exceed
                            * CONFIG_MESSAGE_VALUE_BYTES. */
      Byte_t messageValue[CONFIG_MESSAGE_VALUE_BYTES]; /**< The queue message
                                                        * value. */
    } QueueMessage_t;


    /**
     * @brief Data structure for a queue message
     *
     */
    /**
     * @brief Data structure for information about the HeliOS system
     *
     * The SystemInfo_t data structure is used to store information about the
     * HeliOS system and is returned by xSystemGetSystemInfo().
     *
     * @sa SystemInfo_t *
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
      Base_t numberOfTasks; /**< The number of tasks regardless of their state.
                             */
      Base_t littleEndian; /**< True if the system byte order is little endian.
                            */
    } SystemInfo_t;
  #endif /* ifndef HELIOS_PUBLIC_STRUCTURES_DEFINED_ */


  /**
   * @brief Data structure for information about the HeliOS system
   *
   */

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */


  /**
   * @brief Register a device driver with HeliOS
   *
   * Registers a device driver with the HeliOS kernel, making it available for
   * I/O operations through the device abstraction layer. Device registration
   * must occur before any device I/O functions (xDeviceRead(), xDeviceWrite(),
   * etc.) can be used with that device.
   *
   * HeliOS uses a self-registration pattern where each device driver provides
   * its own registration function. This function is passed to
   * xDeviceRegisterDevice(), which calls it to obtain the driver's callback
   * functions, configuration, and metadata.
   *
   * Device Registration Process:
   * 1. Device driver provides a DRIVERNAME_self_register() function 2.
   * Application calls xDeviceRegisterDevice() with this function pointer 3.
   * HeliOS calls the registration function to obtain driver information 4.
   * Driver is added to the internal device list with its unique ID (UID) 5.
   * Device becomes available for I/O operations
   *
   * Device Driver Requirements:
   * - Each driver must have a globally unique identifier (UID)
   * - Driver must provide callback functions for init, config, read, write
   * - Driver must specify its name, state, and access mode
   * - UID must not conflict with any other registered device
   *
   * @warning Device UIDs must be unique across all drivers in the application.
   * Registering multiple devices with the same UID will cause undefined
   * behavior. Common practice is to use high byte for driver type and low byte
   * for instance (e.g., 0x0100 for first UART, 0x0200 for first SPI).
   *
   * @note Once registered, a device cannot be unregistered. However, devices
   * can be placed in suspended state via xDeviceConfigDevice() to disable them.
   *
   * @note Device registration should occur during system initialization, before
   * xTaskStartScheduler() is called, to ensure devices are available when tasks
   * begin executing.
   *
   * @note The device driver model is extensible. Driver authors define their
   * own state machines, configuration structures, and operational modes within
   * the framework provided by the callback functions.
   *
   * Example Usage:
   * @code
   * // In device driver file (e.g., uart_driver.c) Return_t
   * UART0_self_register(void) {
   *   // Register driver with HeliOS return __RegisterDevice__(
   *     0x0100,              // Unique ID for UART0
   *     "UART0   ",          // 8-byte name DeviceStateRunning,  // Initial
   * state DeviceModeReadWrite, // Access mode UART0_init,          // Init
   * callback UART0_config,        // Config callback UART0_read,          //
   * Read callback UART0_write,         // Write callback UART0_simple_read,
   *   // Simple read callback UART0_simple_write   // Simple write callback
   *   );
   * }
   *
   * // In application code (main.c) int main(void) {
   *   // Register UART driver if
   * (OK(xDeviceRegisterDevice(UART0_self_register))) {
   *     // Initialize the device if (OK(xDeviceInitDevice(0x0100))) {
   *       // Device ready for I/O xDeviceWrite(0x0100, &size, data);
   *     }
   *   }
   *
   *   xTaskStartScheduler();
   * }
   * @endcode
   *
   * @param[in] device_self_register_ Pointer to the device driver's
   *                                  self-registration function. This function
   *                                  must return ReturnOK on successful
   *                                  registration. By convention, this function
   *                                  is named DRIVERNAME_self_register().
   *
   * @return                          ReturnOK if the device was successfully
   *                                  registered, ReturnError if registration
   *                                  failed (duplicate UID, invalid driver
   *                                  structure, out of memory, or driver
   *                                  registration function returned error).
   *
   * @sa xDeviceInitDevice() - Initialize a registered device
   * @sa xDeviceConfigDevice() - Configure device parameters or state
   * @sa xDeviceRead() - Read data from a device
   * @sa xDeviceWrite() - Write data to a device
   * @sa xDeviceIsAvailable() - Check if a device is registered
   * @sa CONFIG_DEVICE_NAME_BYTES - Device name length configuration
   */
  Return_t xDeviceRegisterDevice(Return_t (*device_self_register_)());


  /**
   * @brief Query device availability and readiness status
   *
   * Checks whether a device is available and ready for I/O operations by
   * invoking the device driver's availability callback. The meaning of
   * "available" is driver-specific but typically indicates the device is
   * initialized, operational, and ready to accept read/write requests without
   * errors.
   *
   * Device availability can vary based on hardware state, driver configuration,
   * and operational conditions. For example, a UART might be unavailable if not
   * initialized, a sensor might be unavailable during calibration, or a storage
   * device might be unavailable if busy with another operation.
   *
   * Typical availability semantics by device type:
   * - **Serial devices**: Ready to transmit/receive (not in error state)
   * - **Storage devices**: Not busy, no pending operations, hardware ready
   * - **Sensors**: Calibrated, powered up, and ready to sample
   * - **Network devices**: Link established, buffers available
   * - **Custom devices**: Driver-defined operational readiness
   *
   * Common use cases:
   * - **Pre-operation checks**: Verify device is ready before I/O
   * - **Error recovery**: Check availability after device errors
   * - **Initialization validation**: Confirm device initialized successfully
   * - **Polling loops**: Wait for device to become ready
   * - **Diagnostics**: Determine which devices are operational
   *
   * Example 1: Check device before write operation
   * @code
   * #define UART0_UID 0x0100
   *
   * Return_t sendData(const Byte_t *data, Size_t len) {
   *   Base_t isAvailable;
   *
   *   // Check if UART is available before writing if
   * (OK(xDeviceIsAvailable(UART0_UID, &isAvailable))) {
   *     if (isAvailable) {
   *       // Device ready - perform write return xDeviceWrite(UART0_UID, &len,
   * (Addr_t *)data);
   *     } else {
   *       logWarning("UART not available for write");
   *       return ReturnError;
   *     }
   *   }
   *
   *   logError("Failed to query UART availability");
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 2: Poll until device becomes available
   * @code
   * #define SENSOR_UID 0x0300
   * #define MAX_WAIT_CYCLES 100
   *
   * Return_t waitForSensorReady(void) {
   *   Base_t isAvailable;
   *   Base_t retries = 0;
   *
   *   while (retries < MAX_WAIT_CYCLES) {
   *     if (OK(xDeviceIsAvailable(SENSOR_UID, &isAvailable))) {
   *       if (isAvailable) {
   *         logInfo("Sensor ready after %u attempts", retries);
   *         return ReturnOK;
   *       }
   *     }
   *
   *     // Wait a bit before retrying xTaskDelayUntil(10);  // 10 ticks
   * retries++;
   *   }
   *
   *   logError("Sensor failed to become available");
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 3: System diagnostic - check all devices
   * @code typedef struct {
   *   HalfWord_t uid;
   *   const char *name;
   * } DeviceInfo_t;
   *
   * void checkSystemDevices(void) {
   *   DeviceInfo_t devices[] = {
   *     {0x0100, "UART0"},
   *     {0x0200, "SPI0"},
   *     {0x0300, "Sensor"},
   *     {0x0400, "Storage"}
   *   };
   *   Base_t deviceCount = 4;
   *   Base_t availableCount = 0;
   *
   *   printf("Device Availability Check:\n");
   *
   *   for (Base_t i = 0; i < deviceCount; i++) {
   *     Base_t isAvailable;
   *
   *     if (OK(xDeviceIsAvailable(devices[i].uid, &isAvailable))) {
   *       printf("  %-10s [0x%04X]: %s\n", devices[i].name, devices[i].uid,
   * isAvailable ? "AVAILABLE" : "UNAVAILABLE");
   *
   *       if (isAvailable) {
   *         availableCount++;
   *       }
   *     } else {
   *       printf("  %-10s [0x%04X]: ERROR (not registered?)\n",
   * devices[i].name, devices[i].uid);
   *     }
   *   }
   *
   *   printf("\nSummary: %u/%u devices available\n", availableCount,
   * deviceCount);
   * }
   * @endcode
   *
   * Example 4: Retry I/O with availability check
   * @code
   * #define FLASH_UID 0x1000
   * #define MAX_RETRIES 5
   *
   * Return_t writeFlashWithRetry(Byte_t *data, Size_t len) {
   *   Base_t retries = 0;
   *
   *   while (retries < MAX_RETRIES) {
   *     Base_t isAvailable;
   *
   *     // Check availability if (OK(xDeviceIsAvailable(FLASH_UID,
   * &isAvailable)) && isAvailable) {
   *       // Try write operation if (OK(xDeviceWrite(FLASH_UID, &len, (Addr_t
   * *)data))) {
   *         return ReturnOK;  // Success
   *       }
   *     }
   *
   *     // Write failed or device unavailable - wait and retry
   * logWarning("Flash write failed, retry %u/%u", retries + 1, MAX_RETRIES);
   *     xTaskDelayUntil(100);  // Wait longer for flash retries++;
   *   }
   *
   *   logError("Flash write failed after %u retries", MAX_RETRIES);
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[in]  uid_ Unique identifier of the device to query. Must match a UID
   *                  previously registered via xDeviceRegisterDevice().
   * @param[out] res_ Pointer to variable that receives the availability status.
   *                  Set to non-zero (true) if device is available, zero
   *                  (false) if unavailable.
   *
   * @return          ReturnOK if query succeeded (res_ contains valid result),
   *                  ReturnError if query failed (invalid UID, device not
   *                  registered, or driver does not implement availability
   *                  check).
   *
   * @warning A successful return (ReturnOK) only means the query was performed.
   * Check the res_ value to determine actual device availability. ReturnOK with
   * res_=0 means "successfully determined device is unavailable."
   *
   * @warning Device availability can change between checking and using. After
   * confirming availability, the device might become unavailable before your
   * I/O operation. Always check I/O function return values.
   *
   * @warning If the driver does not implement an availability callback, this
   * function may return ReturnError or always report available. Check your
   * driver documentation.
   *
   * @note The interpretation of "available" is driver-specific. Some drivers
   * may always return available after initialization, while others perform
   * detailed hardware checks.
   *
   * @note This is a lightweight query that typically does not perform I/O. It's
   * safe to call frequently in polling loops.
   *
   * @note Combining availability checks with actual I/O operations is a common
   * pattern for robust device communication in embedded systems.
   *
   * @sa xDeviceInitDevice() - Initialize device before checking availability
   * @sa xDeviceConfigDevice() - Configure device state
   * @sa xDeviceWrite() - Write to device (check availability first)
   * @sa xDeviceRead() - Read from device (check availability first)
   * @sa xDeviceRegisterDevice() - Register device driver
   */
  Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_);


  /**
   * @brief Write a single byte to a device
   *
   * Writes a single byte of data to the specified device through its registered
   * driver. This is a simplified write operation for single-byte transfers,
   * ideal for character output, simple control commands, or low-throughput data
   * transmission where heap allocation overhead is unnecessary.
   *
   * Unlike xDeviceWrite() which requires heap-allocated buffers for bulk data,
   * xDeviceSimpleWrite() accepts a single byte value directly. This makes it
   * more efficient for single-character operations and simpler to use when you
   * only need to send one byte at a time.
   *
   * Common use cases:
   * - **Character output**: Sending individual characters to a UART or terminal
   * - **Control commands**: Sending single-byte commands to peripherals
   * - **Status updates**: Writing status bytes to LED controllers or displays
   * - **Protocol framing**: Sending header bytes, delimiters, or checksums
   * - **Low-rate data**: Simple sensors or actuators requiring occasional
   * updates
   *
   * Operational behavior:
   * - The byte is passed directly to the device driver's write callback
   * - No heap allocation or memory validation is required
   * - Operation is synchronous (returns after driver completes the write)
   * - Device must be in writable state (DeviceModeWriteOnly or
   * DeviceModeReadWrite)
   * - Device must be running (DeviceStateRunning)
   *
   * Example 1: Send character to UART
   * @code
   * #define UART0_UID 0x0100
   *
   * void sendChar(char c) {
   *   if (OK(xDeviceSimpleWrite(UART0_UID, (Byte_t)c))) {
   *     // Character sent successfully
   *   } else {
   *     // Write failed - check device state
   *   }
   * }
   *
   * // Send a string character-by-character void sendString(const char *str) {
   *   while (*str) {
   *     xDeviceSimpleWrite(UART0_UID, (Byte_t)*str);
   *     str++;
   *   }
   * }
   * @endcode
   *
   * Example 2: Control LED via byte commands
   * @code
   * #define LED_CTRL_UID 0x0200
   * #define LED_ON  0x01
   * #define LED_OFF 0x00
   *
   * void setLED(Base_t state) {
   *   Byte_t command = state ? LED_ON : LED_OFF;
   *   if (ERROR(xDeviceSimpleWrite(LED_CTRL_UID, command))) {
   *     logError("Failed to control LED");
   *   }
   * }
   * @endcode
   *
   * Example 3: Send protocol framing bytes
   * @code
   * #define START_BYTE 0xAA
   * #define END_BYTE   0x55
   *
   * Return_t sendFrame(HalfWord_t deviceUID, Byte_t *payload, Size_t len) {
   *   // Send start byte if (ERROR(xDeviceSimpleWrite(deviceUID, START_BYTE)))
   * {
   *     return ReturnError;
   *   }
   *
   *   // Send payload (using bulk write) if (ERROR(xDeviceWrite(deviceUID,
   * &len, (Addr_t *)payload))) {
   *     return ReturnError;
   *   }
   *
   *   // Send end byte if (ERROR(xDeviceSimpleWrite(deviceUID, END_BYTE))) {
   *     return ReturnError;
   *   }
   *
   *   return ReturnOK;
   * }
   * @endcode
   *
   * @param[in] uid_  Unique identifier of the target device. Must match a UID
   *                  previously registered via xDeviceRegisterDevice().
   * @param[in] data_ Single byte value to write to the device. Accepts any
   *                  value from 0x00 to 0xFF.
   *
   * @return          ReturnOK if byte was written successfully, ReturnError if
   *                  the write failed (invalid UID, device not found, device in
   *                  wrong state/mode, or driver write operation failed).
   *
   * @warning The device must be in a writable state (DeviceModeWriteOnly or
   * DeviceModeReadWrite) for the write to succeed. Use xDeviceConfigDevice() to
   * check or change device mode if writes are failing.
   *
   * @warning The device must be in running state (DeviceStateRunning). Devices
   * in stopped or error states will reject write operations.
   *
   * @note Unlike xDeviceWrite(), this function does not require heap-allocated
   * memory, making it more efficient for single-byte operations.
   *
   * @note For writing multiple bytes, consider using xDeviceWrite() instead as
   * it's more efficient for bulk transfers and reduces driver overhead.
   *
   * @note Write behavior depends on the device driver implementation. Some
   * drivers may buffer writes, while others perform immediate hardware I/O.
   *
   * @sa xDeviceWrite() - Write multiple bytes to a device (requires heap
   * buffer)
   * @sa xDeviceSimpleRead() - Read a single byte from a device
   * @sa xDeviceRegisterDevice() - Register a device driver
   * @sa xDeviceInitDevice() - Initialize a device
   * @sa xDeviceConfigDevice() - Configure device state or mode
   */
  Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Byte_t data_);


  /**
   * @brief Write data to a device
   *
   * Writes a buffer of data to the specified device through its registered
   * driver. The data is transferred from user heap memory to the device via the
   * driver's write callback function. This provides an abstract interface for
   * device output, allowing applications to write to any registered device
   * using a consistent API.
   *
   * The write operation behavior depends on the device driver implementation:
   * - Serial devices typically write bytes to a transmit buffer or UART
   * - Storage devices write to specific sectors or blocks
   * - Network devices transmit packets
   * - Custom devices implement application-specific write semantics
   *
   * Data Flow:
   * 1. Application prepares data in heap-allocated buffer 2. xDeviceWrite()
   * validates device UID, state, and permissions 3. Data is copied from user
   * heap to kernel memory 4. Driver's write callback is invoked with kernel
   * memory 5. Driver performs hardware-specific write operations 6. Driver
   * returns number of bytes written via size_ parameter
   *
   * @warning The data buffer MUST be allocated from the user heap using
   * xMemAlloc(). Stack-allocated buffers or static data will cause memory
   * validation errors. HeliOS enforces this to maintain memory safety
   * boundaries between user space and kernel space.
   *
   * @note Device write operations are synchronous by default. The function does
   * not return until the driver completes the write operation. Drivers may
   * implement buffering or asynchronous operations internally.
   *
   * @note The size_ parameter is both input and output. On input, it specifies
   * the number of bytes to write. On output (if driver supports it), it may
   * reflect the actual number of bytes written, which can be less than
   * requested for devices with limited buffers.
   *
   * @note The device must be in a writable state (DeviceModeWriteOnly or
   * DeviceModeReadWrite) and running state (DeviceStateRunning) for the write
   * to succeed. Check device state via xDeviceConfigDevice() if needed.
   *
   * Example Usage:
   * @code
   * #define UART0_UID 0x0100
   *
   * // Write string to UART const char *message = "Hello, World!\n";
   * Size_t messageLen = strlen(message);
   * Byte_t *buffer = NULL;
   *
   * // Allocate buffer from heap (required!) if (OK(xMemAlloc((volatile Addr_t
   * *
   * *)&buffer, messageLen))) {
   *   // Copy data to heap buffer memcpy(buffer, message, messageLen);
   *
   *   // Write to device if (OK(xDeviceWrite(UART0_UID, &messageLen, (Addr_t
   * *)buffer))) {
   *     // Data written successfully
   *   }
   *
   *   // Free the buffer xMemFree((Addr_t *)buffer);
   * }
   *
   * // Writing to a block device Byte_t *sectorData = NULL;
   * Size_t sectorSize = 512;
   *
   * if (OK(xMemAlloc((volatile Addr_t **)&sectorData, sectorSize))) {
   *   // Fill sector data...
   *   prepareSectorData(sectorData, sectorSize);
   *
   *   // Write to block device if (OK(xDeviceWrite(0x1000, &sectorSize, (Addr_t
   * *)sectorData))) {
   *     // Sector written
   *   }
   *
   *   xMemFree((Addr_t *)sectorData);
   * }
   * @endcode
   *
   * @param[in]     uid_  Unique identifier of the target device. Must match a
   *                      UID previously registered via xDeviceRegisterDevice().
   * @param[in,out] size_ Pointer to size variable. On input: number of bytes to
   *                      write. On output: may be updated by driver to reflect
   *                      actual bytes written (driver-dependent).
   * @param[in]     data_ Pointer to data buffer allocated via xMemAlloc().
   *                      Contains the data to write to the device.
   *
   * @return              ReturnOK if data was written successfully, ReturnError
   *                      if the write failed (invalid UID, device not found,
   *                      device in wrong state/mode, data not from heap, or
   *                      driver write operation failed).
   *
   * @sa xDeviceRead() - Read data from a device
   * @sa xDeviceSimpleWrite() - Write a single byte to a device
   * @sa xDeviceRegisterDevice() - Register a device driver
   * @sa xDeviceInitDevice() - Initialize a device
   * @sa xDeviceConfigDevice() - Configure device state or parameters
   * @sa xMemAlloc() - Allocate heap memory for write buffer
   * @sa xMemFree() - Free the write buffer after use
   */
  Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);


  /**
   * @brief Read a single byte from a device
   *
   * Reads a single byte of data from the specified device through its
   * registered driver. This is a simplified read operation for single-byte
   * transfers, ideal for character input, polling device status, or reading
   * simple sensor values where the overhead of buffer management is
   * unnecessary.
   *
   * Unlike xDeviceRead() which allocates heap memory and returns
   * variable-length data, xDeviceSimpleRead() reads exactly one byte directly
   * into the provided variable. This makes it more efficient and simpler to use
   * when you only need to read one byte at a time.
   *
   * Common use cases:
   * - **Character input**: Reading individual characters from a UART or
   * keyboard
   * - **Status polling**: Reading device status registers or flags
   * - **Simple sensors**: Reading 8-bit sensor values (temperature, light
   * level)
   * - **Protocol parsing**: Reading header bytes, delimiters, or checksums
   * - **Command acknowledgment**: Reading single-byte responses from
   * peripherals
   *
   * Operational behavior:
   * - The byte is retrieved directly from the device driver's read callback
   * - No heap allocation or memory management is required
   * - Operation is synchronous (returns after driver completes the read)
   * - Device must be in readable state (DeviceModeReadOnly or
   * DeviceModeReadWrite)
   * - Device must be running (DeviceStateRunning)
   * - If no data is available, behavior depends on driver implementation (may
   * return error or block)
   *
   * Example 1: Read character from UART
   * @code
   * #define UART0_UID 0x0100
   *
   * char getChar(void) {
   *   Byte_t data;
   *   if (OK(xDeviceSimpleRead(UART0_UID, &data))) {
   *     return (char)data;
   *   } else {
   *     return '\0';  // No data available or error
   *   }
   * }
   *
   * // Read a line of text character-by-character Return_t readLine(char
   * buffer, Size_t maxLen) {
   *   Size_t idx = 0;
   *   Byte_t c;
   *
   *   while (idx < maxLen - 1) {
   *     if (OK(xDeviceSimpleRead(UART0_UID, &c))) {
   *       if (c == '\n' || c == '\r') {
   *         break;
   *       }
   *       buffer[idx++] = (char)c;
   *     }
   *   }
   *   buffer[idx] = '\0';
   *   return ReturnOK;
   * }
   * @endcode
   *
   * Example 2: Poll device status register
   * @code
   * #define SENSOR_UID 0x0300
   * #define STATUS_READY  0x01
   * #define STATUS_ERROR  0x80
   *
   * Base_t isSensorReady(void) {
   *   Byte_t status;
   *   if (OK(xDeviceSimpleRead(SENSOR_UID, &status))) {
   *     return (status & STATUS_READY) != 0;
   *   }
   *   return 0;
   * }
   *
   * Base_t checkSensorError(void) {
   *   Byte_t status;
   *   if (OK(xDeviceSimpleRead(SENSOR_UID, &status))) {
   *     return (status & STATUS_ERROR) != 0;
   *   }
   *   return 1;  // Assume error if can't read
   * }
   * @endcode
   *
   * Example 3: Read simple sensor value
   * @code
   * #define TEMP_SENSOR_UID 0x0400
   *
   * // Read temperature sensor (returns 0-255 representing 0-100°C) Base_t
   * readTemperature(Byte_t *tempOut) {
   *   if (OK(xDeviceSimpleRead(TEMP_SENSOR_UID, tempOut))) {
   *     // Convert to actual temperature (0-255 maps to 0-100°C)
   *     // Caller can do: actualTemp = (*tempOut * 100) / 255 return 1;  //
   * Success
   *   }
   *   return 0;  // Failed to read
   * }
   * @endcode
   *
   * @param[in]  uid_  Unique identifier of the target device. Must match a UID
   *                   previously registered via xDeviceRegisterDevice().
   * @param[out] data_ Pointer to byte variable to receive the read data. On
   *                   success, this variable is updated with the byte value
   *                   read from the device.
   *
   * @return           ReturnOK if byte was read successfully, ReturnError if
   *                   the read failed (invalid UID, device not found, device in
   *                   wrong state/mode, no data available, or driver read
   *                   operation failed).
   *
   * @warning The device must be in a readable state (DeviceModeReadOnly or
   * DeviceModeReadWrite) for the read to succeed. Use xDeviceConfigDevice() to
   * check or change device mode if reads are failing.
   *
   * @warning The device must be in running state (DeviceStateRunning). Devices
   * in stopped or error states will reject read operations.
   *
   * @warning If no data is available to read, driver behavior varies. Some
   * drivers return ReturnError immediately, while others may block waiting for
   * data. Check your device driver documentation for specific behavior.
   *
   * @note Unlike xDeviceRead(), this function does not allocate heap memory,
   * making it more efficient for single-byte operations.
   *
   * @note For reading multiple bytes, consider using xDeviceRead() instead as
   * it's more efficient for bulk transfers and reduces driver overhead.
   *
   * @note Read behavior depends on the device driver implementation. Some
   * drivers read from hardware registers directly, while others may maintain
   * receive buffers.
   *
   * @sa xDeviceRead() - Read multiple bytes from a device (allocates heap
   * buffer)
   * @sa xDeviceSimpleWrite() - Write a single byte to a device
   * @sa xDeviceRegisterDevice() - Register a device driver
   * @sa xDeviceInitDevice() - Initialize a device
   * @sa xDeviceConfigDevice() - Configure device state or mode
   */
  Return_t xDeviceSimpleRead(const HalfWord_t uid_, Byte_t *data_);


  /**
   * @brief Read data from a device
   *
   * Reads data from the specified device through its registered driver. The
   * device driver allocates a buffer from the user heap, fills it with data
   * from the device, and returns it to the caller. This provides an abstract
   * interface for device input, allowing applications to read from any
   * registered device using a consistent API.
   *
   * Unlike xDeviceWrite() where the caller allocates memory, xDeviceRead() has
   * the DEVICE DRIVER allocate the buffer. This is necessary because the driver
   * determines how much data is available to read. The caller receives both the
   * data buffer pointer and the size of data read.
   *
   * The read operation behavior depends on the device driver implementation:
   * - Serial devices typically read bytes from a receive buffer or UART
   * - Storage devices read from specific sectors or blocks
   * - Network devices receive packets
   * - Sensor devices read measurement data
   * - Custom devices implement application-specific read semantics
   *
   * Data Flow:
   * 1. Application calls xDeviceRead() with device UID 2. xDeviceRead()
   * validates device UID, state, and permissions 3. Driver's read callback is
   * invoked 4. Driver allocates buffer from heap (via xMemAlloc internally) 5.
   * Driver fills buffer with data from device 6. Driver returns buffer pointer
   * and size to HeliOS 7. HeliOS returns buffer to application 8. Application
   * processes data then MUST free buffer with xMemFree()
   *
   * @warning The caller is RESPONSIBLE FOR FREEING the returned buffer using
   * xMemFree(). Failing to free the buffer will cause memory leaks. The driver
   * allocates this memory specifically for this read operation.
   *
   * @note Device read operations are synchronous by default. The function does
   * not return until the driver completes the read operation and allocates the
   * buffer. Drivers may implement buffering or blocking behavior internally.
   *
   * @note The device must be in a readable state (DeviceModeReadOnly or
   * DeviceModeReadWrite) and running state (DeviceStateRunning) for the read to
   * succeed. Check device state via xDeviceConfigDevice() if needed.
   *
   * @note If no data is available, driver behavior varies. Some drivers may
   * return ReturnError, others may return an empty buffer (size=0), and others
   * may block waiting for data. Check specific driver documentation.
   *
   * Example Usage:
   * @code
   * #define UART0_UID 0x0100
   *
   * // Read data from UART Byte_t *rxBuffer = NULL;
   * Size_t rxSize = 0;
   *
   * if (OK(xDeviceRead(UART0_UID, &rxSize, (Addr_t **)&rxBuffer))) {
   *   // Data successfully read if (rxSize > 0) {
   *     // Process received data processUARTData(rxBuffer, rxSize);
   *   }
   *
   *   // IMPORTANT: Free the buffer allocated by driver xMemFree((Addr_t
   * *)rxBuffer);
   * }
   *
   * // Reading from a block device
   * #define BLOCKDEV_UID 0x1000 Byte_t *sectorData = NULL;
   * Size_t sectorSize = 0;
   *
   * if (OK(xDeviceRead(BLOCKDEV_UID, &sectorSize, (Addr_t **)&sectorData))) {
   *   // Sector data read (typically 512 or 4096 bytes) if (sectorSize > 0) {
   *     analyzeSectorData(sectorData, sectorSize);
   *   }
   *
   *   // Free the buffer xMemFree((Addr_t *)sectorData);
   * }
   *
   * // Reading in a loop (e.g., serial communication) void
   * serialReaderTask(Task_t *task, TaskParm_t *parm) {
   *   Byte_t *data = NULL;
   *   Size_t len = 0;
   *
   *   if (OK(xDeviceRead(UART0_UID, &len, (Addr_t **)&data))) {
   *     if (len > 0) {
   *       handleSerialData(data, len);
   *     }
   *     xMemFree((Addr_t *)data);
   *   }
   * }
   * @endcode
   *
   * @param[in]  uid_  Unique identifier of the target device. Must match a UID
   *                   previously registered via xDeviceRegisterDevice().
   * @param[out] size_ Pointer to size variable that receives the number of
   *                   bytes read from the device. Set to 0 if no data
   *                   available.
   * @param[out] data_ Pointer to buffer pointer variable. Receives address of
   *                   heap-allocated buffer containing the read data. Caller
   *                   MUST free this buffer with xMemFree() after use.
   *
   * @return           ReturnOK if data was read successfully (size may be 0),
   *                   ReturnError if the read failed (invalid UID, device not
   *                   found, device in wrong state/mode, memory allocation
   *                   failed, or driver read operation failed).
   *
   * @sa xDeviceWrite() - Write data to a device
   * @sa xDeviceSimpleRead() - Read a single byte from a device
   * @sa xDeviceRegisterDevice() - Register a device driver
   * @sa xDeviceInitDevice() - Initialize a device
   * @sa xDeviceConfigDevice() - Configure device state or parameters
   * @sa xMemFree() - Free the buffer returned by this function (REQUIRED!)
   */
  Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);


  /**
   * @brief Initialize a device driver
   *
   * Initializes the specified device by invoking its driver's initialization
   * callback function. This performs hardware and software setup required
   * before the device can be used for I/O operations. Initialization typically
   * configures hardware registers, sets initial device state and mode,
   * allocates driver-specific resources, and prepares the device for operation.
   *
   * Device initialization is a critical first step after registering a device
   * driver with xDeviceRegisterDevice(). Without proper initialization, devices
   * may not respond to I/O requests or may behave unpredictably. The specific
   * initialization actions depend entirely on the device driver implementation.
   *
   * Common initialization tasks performed by drivers:
   * - **Hardware configuration**: Setting memory-mapped registers, clock
   * speeds, baud rates
   * - **State initialization**: Setting device to DeviceStateRunning or initial
   * state
   * - **Mode configuration**: Configuring read/write mode, interrupts, DMA
   * - **Buffer allocation**: Creating internal receive/transmit buffers if
   * needed
   * - **Self-test**: Performing device self-checks or calibration
   * - **Feature enabling**: Activating device-specific features (e.g., UART
   * flow control)
   *
   * Typical device initialization sequence:
   * 1. Register device driver with xDeviceRegisterDevice() 2. Initialize device
   * with xDeviceInitDevice() 3. Optionally configure device with
   * xDeviceConfigDevice() 4. Begin I/O operations with xDeviceRead() /
   * xDeviceWrite()
   *
   * Example 1: Initialize UART device
   * @code
   * #define UART0_UID 0x0100
   *
   * // Register UART driver (done during system startup) extern DeviceDriver_t
   * uart0Driver;
   * if (OK(xDeviceRegisterDevice(UART0_UID, &uart0Driver))) {
   *   // Initialize the UART hardware if (OK(xDeviceInitDevice(UART0_UID))) {
   *     // UART ready for use
   *   } else {
   *     logError("UART0 initialization failed");
   *   }
   * }
   * @endcode
   *
   * Example 2: Initialize multiple devices in sequence
   * @code
   * #define UART0_UID  0x0100
   * #define SPI0_UID   0x0200
   * #define I2C0_UID   0x0300
   *
   * Return_t initPeripherals(void) {
   *   // Initialize UART if (ERROR(xDeviceInitDevice(UART0_UID))) {
   *     return ReturnError;
   *   }
   *
   *   // Initialize SPI if (ERROR(xDeviceInitDevice(SPI0_UID))) {
   *     return ReturnError;
   *   }
   *
   *   // Initialize I2C if (ERROR(xDeviceInitDevice(I2C0_UID))) {
   *     return ReturnError;
   *   }
   *
   *   return ReturnOK;  // All devices initialized
   * }
   * @endcode
   *
   * Example 3: Initialize with error recovery
   * @code
   * #define MAX_INIT_RETRIES 3
   *
   * Return_t initDeviceWithRetry(HalfWord_t deviceUID) {
   *   Base_t retries = 0;
   *
   *   while (retries < MAX_INIT_RETRIES) {
   *     if (OK(xDeviceInitDevice(deviceUID))) {
   *       logInfo("Device 0x%04X initialized", deviceUID);
   *       return ReturnOK;
   *     }
   *
   *     retries++;
   *     if (retries < MAX_INIT_RETRIES) {
   *       logWarning("Device init failed, retrying %u/%u", retries,
   * MAX_INIT_RETRIES);
   *       delayMs(100);  // Brief delay before retry
   *     }
   *   }
   *
   *   logError("Device 0x%04X failed to initialize after %u attempts",
   * deviceUID, MAX_INIT_RETRIES);
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[in] uid_ Unique identifier of the device to initialize. Must match a
   *                 UID previously registered via xDeviceRegisterDevice().
   *
   * @return         ReturnOK if device initialized successfully, ReturnError if
   *                 initialization failed (invalid UID, device not registered,
   *                 driver init callback failed, or hardware initialization
   *                 error).
   *
   * @warning Always check the return value. A failed initialization means the
   * device is not ready for use, and subsequent I/O operations will likely
   * fail.
   *
   * @warning Do not call xDeviceInitDevice() on an already-initialized device
   * unless the driver explicitly supports re-initialization. Some drivers may
   * leak resources or behave unpredictably if initialized multiple times.
   *
   * @note Not all device drivers require initialization. Some simple drivers
   * may be ready immediately after registration. However, calling
   * xDeviceInitDevice() on such devices is safe and recommended for
   * consistency.
   *
   * @note Initialization is typically performed during system startup, before
   * the scheduler starts. However, it can be called at any time (e.g., for
   * runtime device hotplug or power management).
   *
   * @note If initialization fails, check hardware connections, power supply,
   * clock configuration, and driver implementation for issues.
   *
   * @sa xDeviceRegisterDevice() - Register device driver before initialization
   * @sa xDeviceConfigDevice() - Configure device after initialization
   * @sa xDeviceRead() - Read from initialized device
   * @sa xDeviceWrite() - Write to initialized device
   * @sa xDeviceIsAvailable() - Check if device is ready for use
   */
  Return_t xDeviceInitDevice(const HalfWord_t uid_);


  /**
   * @brief Configure a device and retrieve its current configuration
   *
   * Configures the specified device and retrieves its current configuration in
   * a single bidirectional operation. This function writes configuration
   * parameters to the device driver and reads back the effective configuration,
   * allowing applications to both set and verify device settings atomically.
   *
   * The bidirectional nature serves two purposes: writing configuration to
   * apply new settings, and reading back the actual configuration to verify
   * successful application or to query current device state. This is
   * particularly useful for devices where the effective configuration may
   * differ from requested settings (due to hardware limitations or automatic
   * adjustments).
   *
   * Configuration structure and content are driver-specific. Each device driver
   * defines its own configuration data structure containing parameters like:
   * - Device operating mode (DeviceMode, DeviceState)
   * - Hardware settings (baud rate, clock speed, resolution)
   * - Feature enables (interrupts, DMA, flow control)
   * - Operational parameters (timeouts, buffer sizes, thresholds)
   *
   * Typical configuration workflow:
   * 1. Allocate configuration structure with xMemAlloc() 2. Populate structure
   * with desired settings 3. Call xDeviceConfigDevice() to apply and read back
   * 4. Verify returned configuration matches expectations 5. Free configuration
   * structure with xMemFree()
   *
   * @warning Configuration data MUST be allocated from user heap via
   * xMemAlloc(). Stack-allocated or static configuration structures will cause
   * memory validation errors. HeliOS enforces this to maintain memory safety.
   *
   * Example 1: Configure UART baud rate
   * @code
   * #define UART0_UID 0x0100
   *
   * typedef struct {
   *   Word_t baudRate;
   *   Byte_t dataBits;
   *   Byte_t stopBits;
   *   Byte_t parity;
   *   DeviceMode mode;
   * } UARTConfig_t;
   *
   * Return_t configureUART(Word_t baud) {
   *   UARTConfig_t *config = NULL;
   *   Size_t configSize = sizeof(UARTConfig_t);
   *
   *   // Allocate config from heap if (OK(xMemAlloc((volatile Addr_t
   * **)&config, configSize))) {
   *     // Set desired configuration config->baudRate = baud;
   *     config->dataBits = 8;
   *     config->stopBits = 1;
   *     config->parity = 0;  // No parity config->mode =
   * DeviceModeReadWrite;
   *
   *     // Apply configuration and read back if
   * (OK(xDeviceConfigDevice(UART0_UID,
   * &configSize, (Addr_t *)config))) {
   *       // Verify effective baud rate if (config->baudRate != baud) {
   *         logWarning("UART baud rate adjusted to %lu", config->baudRate);
   *       }
   *     }
   *
   *     xMemFree((Addr_t *)config);
   *     return ReturnOK;
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 2: Query current device configuration
   * @code
   * #define SENSOR_UID 0x0400
   *
   * typedef struct {
   *   HalfWord_t sampleRate;
   *   Byte_t resolution;
   *   Byte_t powerMode;
   *   DeviceState state;
   * } SensorConfig_t;
   *
   * Return_t querySensorConfig(SensorConfig_t *outConfig) {
   *   SensorConfig_t *config = NULL;
   *   Size_t configSize = sizeof(SensorConfig_t);
   *
   *   if (OK(xMemAlloc((volatile Addr_t **)&config, configSize))) {
   *     // Don't set any fields - just query current settings if
   * (OK(xDeviceConfigDevice(SENSOR_UID, &configSize, (Addr_t *)config))) {
   *       // Copy config to output parameter *outConfig = *config;
   *       xMemFree((Addr_t *)config);
   *       return ReturnOK;
   *     }
   *     xMemFree((Addr_t *)config);
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 3: Change device mode dynamically
   * @code typedef struct {
   *   DeviceMode mode;
   *   DeviceState state;
   * } GenericDeviceConfig_t;
   *
   * Return_t setDeviceMode(HalfWord_t deviceUID, DeviceMode newMode) {
   *   GenericDeviceConfig_t *config = NULL;
   *   Size_t configSize = sizeof(GenericDeviceConfig_t);
   *
   *   if (OK(xMemAlloc((volatile Addr_t **)&config, configSize))) {
   *     // First query current config if (OK(xDeviceConfigDevice(deviceUID,
   * &configSize, (Addr_t *)config))) {
   *       // Change only the mode config->mode = newMode;
   *
   *       // Apply updated config if (OK(xDeviceConfigDevice(deviceUID,
   * &configSize, (Addr_t *)config))) {
   *         // Verify mode change if (config->mode == newMode) {
   *           xMemFree((Addr_t *)config);
   *           return ReturnOK;
   *         }
   *       }
   *     }
   *     xMemFree((Addr_t *)config);
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[in]     uid_    Unique identifier of the device to configure. Must
   *                        match a UID previously registered via
   *                        xDeviceRegisterDevice().
   * @param[in,out] size_   Pointer to size variable. On input: size in bytes of
   *                        the configuration structure. On output: may be
   *                        updated by driver to reflect actual configuration
   *                        size returned.
   * @param[in,out] config_ Pointer to heap-allocated configuration structure.
   *                        On input: contains desired configuration parameters.
   *                        On output: contains actual effective configuration
   *                        as reported by driver. Must be allocated via
   *                        xMemAlloc().
   *
   * @return                ReturnOK if configuration successful, ReturnError if
   *                        operation failed (invalid UID, device not found,
   *                        config not from heap, invalid configuration
   *                        parameters, or driver config callback failed).
   *
   * @warning The config_ buffer MUST be allocated from user heap using
   * xMemAlloc(). Stack or static allocation will cause validation failure.
   *
   * @warning After successful return, always check the returned configuration
   * to verify the driver accepted your requested settings. Some devices may
   * adjust parameters to supported values.
   *
   * @warning Configuration structures are driver-specific. Ensure you're using
   * the correct structure type for the target device driver.
   *
   * @note The size_ parameter should initially contain the size of your
   * configuration structure (e.g., sizeof(MyConfigStruct)).
   *
   * @note This function is bidirectional: it both writes and reads
   * configuration in a single call. To query configuration without changing it,
   * simply pass an uninitialized structure.
   *
   * @note Not all device drivers support configuration. Drivers for simple
   * devices may implement a no-op configuration callback that always succeeds.
   *
   * @sa xDeviceInitDevice() - Initialize device before configuration
   * @sa xDeviceRegisterDevice() - Register device driver
   * @sa xMemAlloc() - Allocate configuration structure
   * @sa xMemFree() - Free configuration structure after use
   */
  Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);


  /**
   * @brief Allocate memory from the user heap
   *
   * Allocates a block of memory from the HeliOS user heap and returns a pointer
   * to the allocated memory. The allocated memory is automatically zeroed
   * (similar to calloc() in standard C), ensuring predictable initialization.
   *
   * HeliOS maintains separate user and kernel memory regions. This function
   * allocates from the user heap, which is intended for application data
   * structures, buffers, and general-purpose memory needs. The total heap size
   * is determined by CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS ×
   * CONFIG_MEMORY_REGION_BLOCK_SIZE.
   *
   * Memory Allocation Features:
   * - Automatic zero-initialization of allocated memory
   * - Best-fit allocation strategy to minimize fragmentation
   * - Memory safety checks to prevent heap corruption
   * - Tracking of allocation statistics (via xMemGetHeapStats())
   *
   * Memory allocated by this function must be freed using xMemFree() when no
   * longer needed to prevent memory leaks. HeliOS does not provide automatic
   * garbage collection.
   *
   * @warning The addr_ parameter must be cast to (volatile Addr_t **) to avoid
   * compiler warnings. This is because the function modifies the pointer
   * variable itself (by-reference parameter passing).
   *
   * @note Allocation may fail if insufficient contiguous memory is available,
   * even if the total free memory exceeds the requested size. This can occur
   * due to heap fragmentation.
   *
   * @note Several HeliOS functions allocate heap memory internally and return
   * it to the caller (e.g., xDeviceRead(), xMemGetHeapStats()). Memory returned
   * by these functions must also be freed with xMemFree().
   *
   * Example Usage:
   * @code
   * // Allocate memory for a structure typedef struct {
   *   int temperature;
   *   int humidity;
   * } SensorData_t;
   *
   * SensorData_t *data = NULL;
   *
   * if (OK(xMemAlloc((volatile Addr_t **)&data, sizeof(SensorData_t)))) {
   *   // Memory allocated successfully and zeroed data->temperature =
   * readTemperature();
   *   data->humidity = readHumidity();
   *
   *   // Use the data...
   *
   *   // Free when done xMemFree((Addr_t *)data);
   * } else {
   *   // Handle allocation failure reportError("Out of memory");
   * }
   *
   * // Allocate an array uint8_t *buffer = NULL;
   * if (OK(xMemAlloc((volatile Addr_t **)&buffer, 256))) {
   *   // Use buffer...
   *   xMemFree((Addr_t *)buffer);
   * }
   * @endcode
   *
   * @param[out] addr_ Pointer to a pointer variable that will receive the
   *                   address of the allocated memory. Must be cast to
   *                   (volatile Addr_t **). On failure, this pointer is not
   *                   modified.
   * @param[in]  size_ Number of bytes to allocate. Must be greater than zero.
   *
   * @return           ReturnOK if memory was successfully allocated,
   *                   ReturnError if allocation failed (insufficient memory,
   *                   invalid parameters, or memory system error).
   *
   * @sa xMemFree() - Free allocated memory
   * @sa xMemFreeAll() - Free all allocated memory
   * @sa xMemGetUsed() - Get total allocated memory
   * @sa xMemGetSize() - Get size of an allocation
   * @sa xMemGetHeapStats() - Get detailed heap statistics
   * @sa CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS - Heap size configuration
   * @sa CONFIG_MEMORY_REGION_BLOCK_SIZE - Memory block size configuration
   */
  Return_t xMemAlloc(volatile Addr_t **addr_, const Size_t size_);


  /**
   * @brief Free memory previously allocated from the user heap
   *
   * Deallocates a block of memory that was previously allocated by xMemAlloc()
   * or returned by HeliOS functions that allocate memory (such as
   * xDeviceRead(), xMemGetHeapStats(), or xTaskGetAllRunTimeStats()). The freed
   * memory becomes available for future allocations.
   *
   * After freeing memory, the pointer should be considered invalid and must not
   * be dereferenced. HeliOS does not automatically NULL the pointer; the caller
   * is responsible for proper pointer management.
   *
   * Memory Management Best Practices:
   * - Always free memory when it is no longer needed to prevent leaks
   * - Set pointers to NULL after freeing to avoid use-after-free bugs
   * - Never free the same memory twice (double-free)
   * - Never free memory that was not allocated by xMemAlloc() or HeliOS
   * - Never free stack-allocated variables or static memory
   *
   * @warning Freeing invalid memory addresses or freeing the same memory twice
   * will cause heap corruption and undefined behavior. HeliOS performs
   * validation checks, but cannot detect all misuse scenarios.
   *
   * @warning After calling xMemFree(), do not access the freed memory. Doing so
   * results in undefined behavior and may cause data corruption or system
   * crashes.
   *
   * @note Some HeliOS functions allocate memory and return it to the caller.
   * The caller is responsible for freeing this memory. Check function
   * documentation to determine if memory management is required.
   *
   * Example Usage:
   * @code
   * // Proper memory management uint8_t *buffer = NULL;
   *
   * if (OK(xMemAlloc((volatile Addr_t **)&buffer, 128))) {
   *   // Use the buffer processData(buffer, 128);
   *
   *   // Free when done if (OK(xMemFree((Addr_t *)buffer))) {
   *     buffer = NULL;  // Good practice: NULL the pointer
   *   }
   * }
   *
   * // Freeing memory returned by HeliOS functions MemoryRegionStats_t **stats
   * =
   * NULL;
   * if (OK(xMemGetHeapStats(&stats))) {
   *   // Use stats...
   *   printf("Free bytes: %lu\n", stats->availableSpaceInBytes);
   *
   *   // Must free the stats structure xMemFree((Addr_t *)stats);
   *   stats = NULL;
   * }
   * @endcode
   *
   * @param[in] addr_ Pointer to the memory block to free. Must be a valid
   *                  pointer previously returned by xMemAlloc() or a HeliOS
   *                  allocation function. Passing NULL is safe and results in
   *                  no operation.
   *
   * @return          ReturnOK if memory was successfully freed, ReturnError if
   *                  the operation failed (invalid address, double-free
   *                  attempt, or memory system error).
   *
   * @sa xMemAlloc() - Allocate memory from the heap
   * @sa xMemFreeAll() - Free all allocated memory at once
   * @sa xMemGetUsed() - Get total allocated memory
   * @sa xMemGetSize() - Get size of an allocation
   * @sa xMemGetHeapStats() - Get detailed heap statistics
   */
  Return_t xMemFree(const volatile Addr_t *addr_);


  /**
   * @brief Free all allocated memory from the user heap at once
   *
   * Deallocates all memory blocks currently allocated from the user heap,
   * effectively resetting the heap to its initial empty state. This is a
   * nuclear option that invalidates ALL heap pointers in one operation, making
   * it useful for complete system resets, test cleanup, or transitioning
   * between major application modes.
   *
   * After calling this function, every pointer previously returned by
   * xMemAlloc() or HeliOS allocation functions becomes invalid. Accessing any
   * of these pointers results in undefined behavior. This makes xMemFreeAll()
   * powerful but dangerous—use it only when you're certain all heap references
   * can be safely discarded.
   *
   * Common use cases:
   * - **System reset**: Preparing for a complete application restart
   * - **Mode transitions**: Switching between major operating modes that use
   * different data structures
   * - **Test cleanup**: Resetting memory state between unit tests
   * - **Error recovery**: Clearing all allocations after detecting heap
   * corruption
   * - **Initialization**: Ensuring clean slate during system startup
   *
   * Typical scenarios:
   * - Application has completed a major operation and needs to free all
   * associated data
   * - System is entering a low-power mode and needs to minimize memory usage
   * - Test harness needs to reset state between test cases
   * - Fatal error occurred and system is preparing to restart
   *
   * Example 1: Reset between application modes
   * @code typedef enum {
   *   MODE_INITIALIZATION, MODE_NORMAL_OPERATION, MODE_DIAGNOSTICS
   * } AppMode_t;
   *
   * void transitionToMode(AppMode_t newMode) {
   *   // Free all current mode's allocations xMemFreeAll();
   *
   *   // Initialize new mode switch (newMode) {
   *     case MODE_INITIALIZATION:
   *       initializeSystem();
   *       break;
   *     case MODE_NORMAL_OPERATION:
   *       startNormalOperation();
   *       break;
   *     case MODE_DIAGNOSTICS:
   *       startDiagnostics();
   *       break;
   *   }
   * }
   * @endcode
   *
   * Example 2: Unit test cleanup
   * @code void setUp(void) {
   *   // Start each test with clean heap xMemFreeAll();
   * }
   *
   * void tearDown(void) {
   *   // Clean up after test xMemFreeAll();
   * }
   *
   * void testMemoryAllocation(void) {
   *   Byte_t *buffer1 = NULL;
   *   Byte_t *buffer2 = NULL;
   *
   *   // Allocate test data xMemAlloc((volatile Addr_t **)&buffer1, 128);
   *   xMemAlloc((volatile Addr_t **)&buffer2, 256);
   *
   *   // Perform test...
   *
   *   // Cleanup happens automatically in tearDown()
   * }
   * @endcode
   *
   * Example 3: Error recovery
   * @code void handleFatalError(const char *errorMsg) {
   *   // Log the error logError("Fatal error: %s", errorMsg);
   *
   *   // Free all memory before restart xMemFreeAll();
   *
   *   // Reset system state resetSystemState();
   *
   *   // Restart application restartApplication();
   * }
   * @endcode
   *
   * Example 4: Periodic memory defragmentation
   * @code void performMaintenanceCycle(void) {
   *   // Save critical state to non-volatile storage saveCriticalState();
   *
   *   // Free all heap memory xMemFreeAll();
   *
   *   // Restore state with fresh allocations (reduces fragmentation)
   * restoreCriticalState();
   * }
   * @endcode
   *
   * @return ReturnOK if all memory successfully freed, ReturnError if the
   *         operation failed (rare, typically indicates heap corruption).
   *
   * @warning After calling xMemFreeAll(), ALL heap pointers become invalid.
   * This includes pointers to task parameters, queue data, stream buffers,
   * device configurations, and any application data structures. Accessing these
   * pointers will cause undefined behavior.
   *
   * @warning Do not call xMemFreeAll() while tasks are running that depend on
   * heap-allocated data. Suspend all tasks or ensure they can handle their data
   * being freed unexpectedly.
   *
   * @warning This function does NOT affect kernel memory allocations. Only user
   * heap allocations are freed. Kernel structures (tasks, timers, queues) are
   * NOT affected.
   *
   * @warning Be aware that some HeliOS internal structures may hold pointers to
   * heap memory (e.g., task parameters, queue messages). Freeing all heap
   * memory may cause these structures to contain dangling pointers.
   *
   * @note This is the fastest way to free large numbers of allocations, as it
   * doesn't need to process each allocation individually.
   *
   * @note After xMemFreeAll(), the heap is fully defragmented with no
   * fragmentation overhead, making it ideal for periodic memory maintenance.
   *
   * @note Consider suspending the scheduler with xTaskSuspendAll() before
   * calling xMemFreeAll() to prevent tasks from attempting to use freed memory.
   *
   * @sa xMemFree() - Free individual memory blocks
   * @sa xMemAlloc() - Allocate memory from heap
   * @sa xMemGetUsed() - Check memory usage before/after freeing
   * @sa xMemGetHeapStats() - Get detailed heap statistics
   * @sa xTaskSuspendAll() - Suspend scheduler during memory reset
   */
  Return_t xMemFreeAll(void);


  /**
   * @brief Query the total amount of allocated heap memory
   *
   * Returns the total number of bytes currently allocated from the user heap
   * across all active allocations. This provides a quick snapshot of heap
   * utilization without the overhead of detailed statistics, making it ideal
   * for runtime memory monitoring, leak detection, and capacity planning.
   *
   * The returned value represents the sum of all memory blocks allocated via
   * xMemAlloc() or HeliOS functions that allocate memory, excluding any
   * internal heap management overhead. This gives an accurate picture of actual
   * application memory consumption.
   *
   * Common use cases:
   * - **Memory monitoring**: Tracking heap usage over time to detect trends
   * - **Leak detection**: Comparing usage before and after operations to find
   * leaks
   * - **Capacity planning**: Determining if more heap space is needed
   * - **Threshold alerts**: Triggering warnings when usage exceeds limits
   * - **Performance tuning**: Identifying memory-intensive operations
   *
   * Example 1: Monitor heap usage in diagnostic task
   * @code void memoryMonitorTask(Task_t *task, TaskParm_t *parm) {
   *   Size_t usedBytes;
   *   Size_t totalHeap = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS *
   *                     CONFIG_MEMORY_REGION_BLOCK_SIZE;
   *
   *   if (OK(xMemGetUsed(&usedBytes))) {
   *     Byte_t percentUsed = (Byte_t)((usedBytes * 100) / totalHeap);
   *
   *     if (percentUsed > 90) {
   *       logWarning("Heap usage critical: %u%% (%lu / %lu bytes)",
   * percentUsed, usedBytes, totalHeap);
   *     } else if (percentUsed > 75) {
   *       logInfo("Heap usage high: %u%%", percentUsed);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Detect memory leaks
   * @code Return_t checkForLeaks(void) {
   *   Size_t beforeSize, afterSize;
   *
   *   // Record usage before operation if (ERROR(xMemGetUsed(&beforeSize))) {
   *     return ReturnError;
   *   }
   *
   *   // Perform operation that should clean up after itself
   * performOperation();
   *
   *   // Check usage after operation if (OK(xMemGetUsed(&afterSize))) {
   *     if (afterSize > beforeSize) {
   *       Size_t leaked = afterSize - beforeSize;
   *       logError("Memory leak detected: %lu bytes not freed", leaked);
   *       return ReturnError;
   *     }
   *   }
   *
   *   return ReturnOK;
   * }
   * @endcode
   *
   * Example 3: Pre-allocation size check
   * @code Return_t allocateBuffer(Byte_t **buffer, Size_t requestedSize) {
   *   Size_t currentUsage;
   *   Size_t totalHeap = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS *
   *                     CONFIG_MEMORY_REGION_BLOCK_SIZE;
   *
   *   // Check if allocation would exceed safe threshold if
   * (OK(xMemGetUsed(&currentUsage))) {
   *     if (currentUsage + requestedSize > (totalHeap * 85 / 100)) {
   *       logWarning("Allocation would exceed 85%% heap capacity");
   *       return ReturnError;
   *     }
   *   }
   *
   *   // Proceed with allocation return xMemAlloc((volatile Addr_t **)buffer,
   * requestedSize);
   * }
   * @endcode
   *
   * Example 4: Runtime memory statistics reporting
   * @code void reportMemoryStatus(void) {
   *   Size_t usedBytes;
   *   Size_t totalBytes = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS *
   *                      CONFIG_MEMORY_REGION_BLOCK_SIZE;
   *
   *   if (OK(xMemGetUsed(&usedBytes))) {
   *     Size_t freeBytes = totalBytes - usedBytes;
   *     Byte_t percentFree = (Byte_t)((freeBytes * 100) / totalBytes);
   *
   *     printf("Memory Status:\n");
   *     printf("  Total:     %lu bytes\n", totalBytes);
   *     printf("  Used:      %lu bytes\n", usedBytes);
   *     printf("  Free:      %lu bytes (%u%%)\n", freeBytes, percentFree);
   *   }
   * }
   * @endcode
   *
   * @param[out] size_ Pointer to variable that receives the total number of
   *                   bytes currently allocated from the heap.
   *
   * @return           ReturnOK if usage query succeeded, ReturnError if the
   *                   operation failed (invalid parameter or memory system
   *                   error).
   *
   * @note The returned value represents allocated memory only. It does not
   * include heap management overhead (block headers, alignment padding, etc.).
   *
   * @note This function is very fast as it simply returns a tracked counter.
   * Call it frequently for monitoring without performance concerns.
   *
   * @note For more detailed information including fragmentation statistics,
   * free space breakdown, and allocation counts, use xMemGetHeapStats()
   * instead.
   *
   * @note The value returned is a snapshot at the moment of the call.
   * Concurrent allocations or frees by other tasks may change the value
   * immediately after this function returns.
   *
   * @sa xMemGetHeapStats() - Get comprehensive heap statistics
   * @sa xMemGetSize() - Get size of a specific allocation
   * @sa xMemAlloc() - Allocate memory (increases used bytes)
   * @sa xMemFree() - Free memory (decreases used bytes)
   * @sa xMemFreeAll() - Free all memory (resets used bytes to zero)
   */
  Return_t xMemGetUsed(Size_t *size_);


  /**
   * @brief Query the size of a specific heap allocation
   *
   * Returns the size in bytes of a memory block at the specified address. The
   * address must be a valid pointer previously returned by xMemAlloc() or a
   * HeliOS function that allocates memory. This allows applications to
   * determine allocation sizes at runtime, useful for dynamic buffer
   * management, serialization, and memory accounting.
   *
   * The returned size is the exact number of bytes originally requested during
   * allocation, not including any internal heap management overhead. This
   * matches the size parameter originally passed to xMemAlloc().
   *
   * Common use cases:
   * - **Dynamic buffer handling**: Determining buffer capacity without tracking
   * size separately
   * - **Serialization**: Knowing data structure size for network transmission
   * or storage
   * - **Memory accounting**: Calculating per-object memory usage
   * - **Validation**: Verifying allocation size before operations
   * - **Debugging**: Inspecting allocation sizes during development
   *
   * Example 1: Generic buffer processing without size tracking
   * @code void processBuffer(Byte_t *buffer) {
   *   Size_t bufferSize;
   *
   *   // Discover buffer size dynamically if (OK(xMemGetSize((Addr_t *)buffer,
   * &bufferSize))) {
   *     // Process up to bufferSize bytes for (Size_t i = 0; i < bufferSize;
   * i++) {
   *       processData(buffer[i]);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Safe buffer copy with size verification
   * @code Return_t safeCopy(Byte_t *dest, const Byte_t *src, Size_t copySize) {
   *   Size_t destSize, srcSize;
   *
   *   // Verify destination has enough space if (ERROR(xMemGetSize((Addr_t
   * *)dest,
   * &destSize))) {
   *     return ReturnError;
   *   }
   *
   *   // Verify source has enough data if (ERROR(xMemGetSize((Addr_t *)src,
   * &srcSize))) {
   *     return ReturnError;
   *   }
   *
   *   // Check bounds if (copySize > destSize || copySize > srcSize) {
   *     logError("Copy size exceeds buffer capacity");
   *     return ReturnError;
   *   }
   *
   *   // Safe to copy memcpy(dest, src, copySize);
   *   return ReturnOK;
   * }
   * @endcode
   *
   * Example 3: Serialization with automatic size detection
   * @code Return_t serializeToStream(Byte_t *data, StreamBuffer_t *stream) {
   *   Size_t dataSize;
   *
   *   // Get actual data size if (ERROR(xMemGetSize((Addr_t *)data,
   * &dataSize))) {
   *     return ReturnError;
   *   }
   *
   *   // Send size header first xStreamSend(stream, (Byte_t)(dataSize >> 8));
   *   // High byte xStreamSend(stream, (Byte_t)(dataSize & 0xFF)); // Low byte
   *
   *   // Send data for (Size_t i = 0; i < dataSize; i++) {
   *     if (ERROR(xStreamSend(stream, data[i]))) {
   *       return ReturnError;
   *     }
   *   }
   *
   *   return ReturnOK;
   * }
   * @endcode
   *
   * Example 4: Memory usage reporting for debug
   * @code typedef struct {
   *   Byte_t *rxBuffer;
   *   Byte_t *txBuffer;
   *   Byte_t *workBuffer;
   * } CommBuffers_t;
   *
   * void reportBufferUsage(CommBuffers_t *buffers) {
   *   Size_t rxSize, txSize, workSize;
   *
   *   xMemGetSize((Addr_t *)buffers->rxBuffer, &rxSize);
   *   xMemGetSize((Addr_t *)buffers->txBuffer, &txSize);
   *   xMemGetSize((Addr_t *)buffers->workBuffer, &workSize);
   *
   *   Size_t totalBufferMemory = rxSize + txSize + workSize;
   *
   *   printf("Communication Buffer Memory:\n");
   *   printf("  RX Buffer:   %lu bytes\n", rxSize);
   *   printf("  TX Buffer:   %lu bytes\n", txSize);
   *   printf("  Work Buffer: %lu bytes\n", workSize);
   *   printf("  Total:       %lu bytes\n", totalBufferMemory);
   * }
   * @endcode
   *
   * @param[in]  addr_ Pointer to the memory block. Must be a valid address
   *                   previously returned by xMemAlloc() or a HeliOS allocation
   *                   function.
   * @param[out] size_ Pointer to variable that receives the size in bytes of
   *                   the allocation at addr_.
   *
   * @return           ReturnOK if size query succeeded, ReturnError if the
   *                   operation failed (invalid address, address not from heap,
   *                   or memory system error).
   *
   * @warning The address must be a valid heap pointer returned by xMemAlloc()
   * or a HeliOS allocation function. Passing stack addresses, static addresses,
   * or already-freed addresses will result in ReturnError.
   *
   * @warning Do not call xMemGetSize() on addresses that have been freed with
   * xMemFree() or invalidated by xMemFreeAll(). The result is undefined and
   * will likely return ReturnError.
   *
   * @note The returned size is the user-requested allocation size, not the
   * total memory consumed including heap overhead. Actual heap usage may be
   * slightly larger due to alignment and metadata.
   *
   * @note Passing NULL for addr_ is safe and will return ReturnError with size_
   * unmodified.
   *
   * @note This function is useful when working with buffers returned by HeliOS
   * functions (like xDeviceRead()) where you receive a pointer but need to know
   * its size.
   *
   * @sa xMemAlloc() - Allocate memory with known size
   * @sa xMemGetUsed() - Get total heap usage
   * @sa xMemGetHeapStats() - Get comprehensive heap statistics
   * @sa xMemFree() - Free memory block
   */
  Return_t xMemGetSize(const volatile Addr_t *addr_, Size_t *size_);


  /**
   * @brief Retrieve detailed statistics about the user heap memory region
   *
   * Returns comprehensive statistics about the user heap including total size,
   * used/free space, allocation counts, fragmentation metrics, and largest
   * available block. This provides deep insight into memory utilization,
   * allocation patterns, and fragmentation for optimization and diagnostics.
   *
   * The function allocates a MemoryRegionStats_t *structure from the heap and
   * populates it with current heap metrics. The caller MUST free this structure
   * with xMemFree() after use. The statistics represent a snapshot at the time
   * of the call and may become outdated as allocations/frees occur.
   *
   * Statistics provided in MemoryRegionStats_t *:
   * - **totalSize**: Total heap capacity in bytes
   * - **usedSpace**: Bytes currently allocated
   * - **freeSpace**: Bytes available for allocation
   * - **allocationCount**: Number of active allocations
   * - **largestFreeBlock**: Size of largest contiguous free block
   * - **fragmentationPercent**: Degree of memory fragmentation (0-100%)
   *
   * Common use cases:
   * - **Memory profiling**: Understanding application memory footprint
   * - **Fragmentation analysis**: Detecting and addressing heap fragmentation
   * - **Capacity planning**: Determining if heap size is adequate
   * - **Performance tuning**: Optimizing allocation patterns
   * - **Diagnostic logging**: Capturing memory state for troubleshooting
   * - **Runtime monitoring**: Tracking memory trends over time
   *
   * Example 1: Display heap utilization
   * @code void displayHeapStatus(void) {
   *   MemoryRegionStats_t **stats = NULL;
   *
   *   if (OK(xMemGetHeapStats(&stats))) {
   *     Byte_t usagePercent = (Byte_t)((stats->usedSpace * 100) /
   * stats->totalSize);
   *
   *     printf("Heap Memory Status:\n");
   *     printf("  Total:       %lu bytes\n", stats->totalSize);
   *     printf("  Used:        %lu bytes (%u%%)\n", stats->usedSpace,
   * usagePercent);
   *     printf("  Free:        %lu bytes\n", stats->freeSpace);
   *     printf("  Allocations: %lu\n", stats->allocationCount);
   *     printf("  Largest free block: %lu bytes\n", stats->largestFreeBlock);
   *     printf("  Fragmentation: %u%%\n", stats->fragmentationPercent);
   *
   *     // Always free the stats structure xMemFree((Addr_t *)stats);
   *   }
   * }
   * @endcode
   *
   * Example 2: Monitor for fragmentation issues
   * @code
   * #define FRAG_WARNING_THRESHOLD 30
   * #define FRAG_CRITICAL_THRESHOLD 50
   *
   * void checkHeapFragmentation(void) {
   *   MemoryRegionStats_t **stats = NULL;
   *
   *   if (OK(xMemGetHeapStats(&stats))) {
   *     if (stats->fragmentationPercent >= FRAG_CRITICAL_THRESHOLD) {
   *       logCritical("Heap fragmentation critical: %u%%",
   * stats->fragmentationPercent);
   *       // Consider xMemFreeAll() or defragmentation strategy
   *     } else if (stats->fragmentationPercent >= FRAG_WARNING_THRESHOLD) {
   *       logWarning("Heap fragmentation high: %u%%",
   * stats->fragmentationPercent);
   *     }
   *
   *     xMemFree((Addr_t *)stats);
   *   }
   * }
   * @endcode
   *
   * Example 3: Check if allocation will succeed
   * @code Return_t canAllocate(Size_t requestedSize) {
   *   MemoryRegionStats_t **stats = NULL;
   *
   *   if (OK(xMemGetHeapStats(&stats))) {
   *     // Check if largest free block can satisfy request if
   * (stats->largestFreeBlock >= requestedSize) {
   *       xMemFree((Addr_t *)stats);
   *       return ReturnOK;  // Allocation likely to succeed
   *     }
   *
   *     logWarning("Insufficient contiguous space: need %lu, have %lu",
   * requestedSize, stats->largestFreeBlock);
   *     xMemFree((Addr_t *)stats);
   *   }
   *
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 4: Periodic heap health monitoring
   * @code
   * #define HEAP_WARNING_THRESHOLD 85
   *
   * void heapMonitorTask(Task_t *task, TaskParm_t *parm) {
   *   static Word_t lastUsed = 0;
   *   MemoryRegionStats_t **stats = NULL;
   *
   *   if (OK(xMemGetHeapStats(&stats))) {
   *     Byte_t usagePercent = (Byte_t)((stats->usedSpace * 100) /
   * stats->totalSize);
   *
   *     // Check for high usage if (usagePercent >= HEAP_WARNING_THRESHOLD) {
   *       logWarning("Heap usage high: %u%% (%lu/%lu bytes)", usagePercent,
   * stats->usedSpace, stats->totalSize);
   *     }
   *
   *     // Check for memory leaks (continually increasing usage) if
   * (stats->usedSpace > lastUsed) {
   *       Word_t increase = stats->usedSpace - lastUsed;
   *       logInfo("Heap usage increased by %lu bytes", increase);
   *     }
   *     lastUsed = stats->usedSpace;
   *
   *     xMemFree((Addr_t *)stats);
   *   }
   * }
   * @endcode
   *
   * @param[out] stats_ Pointer to MemoryRegionStats_t *pointer that will
   *                    receive the allocated statistics structure. Caller MUST
   *                    free with xMemFree() after use.
   *
   * @return            ReturnOK if statistics retrieved successfully,
   *                    ReturnError if operation failed (memory allocation error
   *                    or invalid parameter).
   *
   * @warning The caller MUST free the returned statistics structure using
   * xMemFree(). Failing to do so will cause a memory leak.
   *
   * @warning The statistics are a snapshot at call time. Memory state changes
   * continuously, so values may be outdated immediately after return.
   *
   * @warning Calling this function allocates memory from the heap it's
   * measuring, slightly affecting the reported statistics. The stats structure
   * itself consumes heap space.
   *
   * @note For a quick usage check without detailed stats, use xMemGetUsed()
   * instead, which is faster and doesn't allocate memory.
   *
   * @note High fragmentation (>40%) indicates many small free blocks scattered
   * throughout the heap, which can prevent large allocations even when total
   * free space is adequate.
   *
   * @note The largestFreeBlock value indicates the maximum allocation size that
   * can succeed, regardless of total free space available.
   *
   * @note This function provides heap statistics only. For kernel memory stats,
   * use xMemGetKernelStats().
   *
   * @sa xMemGetKernelStats() - Get kernel memory region statistics
   * @sa xMemGetUsed() - Quick check of heap usage (no allocation)
   * @sa xMemFree() - Free the statistics structure
   * @sa xMemFreeAll() - Free all heap memory (may help with fragmentation)
   * @sa xMemAlloc() - Allocate heap memory
   */
  Return_t xMemGetHeapStats(MemoryRegionStats_t **stats_);


  /**
   * @brief Retrieve detailed statistics about the kernel memory region
   *
   * Returns comprehensive statistics about the kernel memory region including
   * total size, used/free space, and allocation metrics. Kernel memory is used
   * exclusively by HeliOS for internal data structures like tasks, timers,
   * queues, and streams, separate from user heap memory.
   *
   * The function allocates a MemoryRegionStats_t *structure from the user heap
   * (not kernel memory) and populates it with kernel memory metrics. The caller
   * MUST free this structure with xMemFree() after use. The statistics
   * represent a snapshot at call time.
   *
   * Kernel memory is consumed by:
   * - **Task control blocks**: One per task created
   * - **Timers**: Task timers and watchdog timers
   * - **Queues**: Message queue structures and buffers
   * - **Streams**: Stream buffer control structures
   * - **Notifications**: Task notification state
   * - **Internal structures**: Scheduler and system data
   *
   * Common use cases:
   * - **System capacity monitoring**: Tracking kernel resource usage
   * - **Leak detection**: Identifying kernel memory leaks
   * - **Capacity planning**: Determining if kernel memory is sufficient
   * - **Performance analysis**: Understanding kernel memory overhead
   * - **Diagnostics**: Troubleshooting kernel memory exhaustion
   *
   * Example 1: Display kernel memory status
   * @code void displayKernelMemoryStatus(void) {
   *   MemoryRegionStats_t **stats = NULL;
   *
   *   if (OK(xMemGetKernelStats(&stats))) {
   *     Byte_t usagePercent = (Byte_t)((stats->usedSpace * 100) /
   * stats->totalSize);
   *
   *     printf("Kernel Memory Status:\n");
   *     printf("  Total:       %lu bytes\n", stats->totalSize);
   *     printf("  Used:        %lu bytes (%u%%)\n", stats->usedSpace,
   * usagePercent);
   *     printf("  Free:        %lu bytes\n", stats->freeSpace);
   *     printf("  Allocations: %lu\n", stats->allocationCount);
   *
   *     // Always free (from user heap, not kernel) xMemFree((Addr_t *)stats);
   *   }
   * }
   * @endcode
   *
   * Example 2: Compare heap vs kernel memory usage
   * @code void compareMemoryRegions(void) {
   *   MemoryRegionStats_t **heapStats = NULL;
   *   MemoryRegionStats_t **kernelStats = NULL;
   *
   *   if (OK(xMemGetHeapStats(&heapStats)) &&
   *       OK(xMemGetKernelStats(&kernelStats))) {
   *
   *     printf("Memory Comparison:\n");
   *     printf("  Heap   - Used: %lu / Total: %lu (%u%%)\n",
   * heapStats->usedSpace, heapStats->totalSize, (Byte_t)((heapStats->usedSpace
   * *
   * 100) / heapStats->totalSize));
   *     printf("  Kernel - Used: %lu / Total: %lu (%u%%)\n",
   * kernelStats->usedSpace, kernelStats->totalSize,
   * (Byte_t)((kernelStats->usedSpace * 100) / kernelStats->totalSize));
   *
   *     xMemFree((Addr_t *)heapStats);
   *     xMemFree((Addr_t *)kernelStats);
   *   }
   * }
   * @endcode
   *
   * Example 3: Monitor kernel memory for leaks
   * @code void monitorKernelMemory(void) {
   *   static Word_t lastUsed = 0;
   *   static Base_t lastAllocCount = 0;
   *   MemoryRegionStats_t **stats = NULL;
   *
   *   if (OK(xMemGetKernelStats(&stats))) {
   *     // Check for increasing usage if (stats->usedSpace > lastUsed) {
   *       Word_t increase = stats->usedSpace - lastUsed;
   *       Base_t newAllocs = stats->allocationCount - lastAllocCount;
   *
   *       logInfo("Kernel memory increased: +%lu bytes, +%u allocations",
   * increase, newAllocs);
   *
   *       // Unexpected growth might indicate a leak if (newAllocs == 0 &&
   * increase > 0) {
   *         logWarning("Kernel memory grew without new allocations!");
   *       }
   *     }
   *
   *     lastUsed = stats->usedSpace;
   *     lastAllocCount = stats->allocationCount;
   *
   *     xMemFree((Addr_t *)stats);
   *   }
   * }
   * @endcode
   *
   * Example 4: Check if kernel memory available for new resources
   * @code
   * #define KERNEL_MIN_FREE_BYTES 512
   *
   * Return_t canCreateKernelResource(void) {
   *   MemoryRegionStats_t **stats = NULL;
   *
   *   if (OK(xMemGetKernelStats(&stats))) {
   *     if (stats->freeSpace < KERNEL_MIN_FREE_BYTES) {
   *       logWarning("Kernel memory low: %lu bytes free (min: %u)",
   * stats->freeSpace, KERNEL_MIN_FREE_BYTES);
   *       xMemFree((Addr_t *)stats);
   *       return ReturnError;
   *     }
   *
   *     xMemFree((Addr_t *)stats);
   *     return ReturnOK;
   *   }
   *
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[out] stats_ Pointer to MemoryRegionStats_t *pointer that will
   *                    receive the allocated statistics structure. Allocated
   *                    from user heap. Caller MUST free with xMemFree() after
   *                    use.
   *
   * @return            ReturnOK if statistics retrieved successfully,
   *                    ReturnError if operation failed (memory allocation error
   *                    or invalid parameter).
   *
   * @warning The caller MUST free the returned statistics structure using
   * xMemFree(). The structure is allocated from user heap, not kernel memory.
   *
   * @warning Kernel memory statistics are a snapshot. Values change as tasks,
   * queues, timers, and other kernel objects are created or destroyed.
   *
   * @warning Running out of kernel memory prevents creating new tasks, queues,
   * timers, and other kernel objects. Monitor kernel usage to avoid exhaustion.
   *
   * @note The statistics structure itself is allocated from user heap, NOT
   * kernel memory. Only the statistics content describes kernel memory.
   *
   * @note Kernel memory size is configured at compile time via
   * CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS. It cannot be changed at runtime.
   *
   * @note Unlike user heap, kernel memory is typically not fragmented because
   * kernel objects are usually longer-lived and not freed frequently.
   *
   * @note Kernel memory usage increases primarily when creating tasks, queues,
   * timers, or streams. Deleting these objects frees kernel memory.
   *
   * @sa xMemGetHeapStats() - Get user heap memory statistics
   * @sa xMemFree() - Free the statistics structure (from user heap)
   * @sa xTaskCreate() - Creates task (uses kernel memory)
   * @sa xQueueCreate() - Creates queue (uses kernel memory)
   * @sa CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS - Kernel memory size configuration
   */
  Return_t xMemGetKernelStats(MemoryRegionStats_t **stats_);


  /**
   * @brief Create a message queue for inter-task communication
   *
   * Creates a message queue that enables tasks to exchange data safely in a
   * producer-consumer pattern. Queues provide a First-In-First-Out (FIFO)
   * mechanism for passing messages between tasks, making them ideal for
   * decoupling tasks and implementing asynchronous communication.
   *
   * Message queues in HeliOS store variable-length byte arrays, allowing
   * flexible message formats. Each queue has a configurable maximum capacity
   * (limit), and operations fail gracefully when the queue is full or empty.
   *
   * Queue Characteristics:
   * - FIFO ordering: Messages are retrieved in the order they were sent
   * - Bounded capacity: Queue has a maximum message limit
   * - Non-blocking: Send and receive operations return immediately
   * - Thread-safe: Can be safely accessed from multiple tasks
   * - Variable message size: Each message can be a different size
   *
   * Common Use Cases:
   * - Passing sensor data from interrupt handlers to processing tasks
   * - Command queues for serial communication
   * - Event queues for state machines
   * - Data buffering between producer and consumer tasks
   * - Decoupling hardware drivers from application logic
   *
   * Queue Operation Flow:
   * 1. Producer task sends message with xQueueSend() 2. Message is stored in
   * queue if space available 3. Consumer task receives message with
   * xQueueReceive() 4. Message is removed from queue
   *
   * @note The limit parameter must be at least CONFIG_QUEUE_MINIMUM_LIMIT
   * (default 5). This ensures reasonable queue capacity and prevents overly
   * small queues that could cause frequent overflow conditions.
   *
   * @note Messages are copied into the queue, so the original data buffer can
   * be reused or freed after calling xQueueSend(). Similarly, xQueueReceive()
   * provides a copy of the message.
   *
   * @note Queues can be locked with xQueueLockQueue() to prevent modifications
   * during critical operations, then unlocked with xQueueUnLockQueue().
   *
   * Example Usage:
   * @code Queue_t *sensorDataQueue;
   * Queue_t *commandQueue;
   *
   * // Create a queue that can hold up to 10 messages if
   * (OK(xQueueCreate(&sensorDataQueue, 10))) {
   *   // Queue created successfully
   *
   *   // Producer task: Send sensor readings void sensorTask(Task_t *task,
   * TaskParm_t *parm) {
   *     uint8_t sensorData[4];
   *     readSensor(sensorData, sizeof(sensorData));
   *
   *     // Send to queue Size_t dataSize = sizeof(sensorData);
   *     if (OK(xQueueSend(sensorDataQueue, dataSize, sensorData))) {
   *       // Data queued successfully
   *     } else {
   *       // Queue full - handle overflow
   *     }
   *   }
   *
   *   // Consumer task: Process sensor readings void processingTask(Task_t
   * task, TaskParm_t *parm) {
   *     QueueMessage_t *message;
   *
   *     // Check if messages available Base_t messagesWaiting;
   *     if (OK(xQueueMessagesWaiting(sensorDataQueue, &messagesWaiting))) {
   *       if (messagesWaiting > 0) {
   *         // Receive message if (OK(xQueueReceive(sensorDataQueue,
   * &message))) {
   *           // Process the data processSensorData(message.message,
   * message.size);
   *         }
   *       }
   *     }
   *   }
   * }
   *
   * // Create a command queue if (OK(xQueueCreate(&commandQueue, 5))) {
   *   // Use queue for command processing...
   *
   *   // Clean up when done xQueueDelete(commandQueue);
   * }
   * @endcode
   *
   * @param[out] queue_ Pointer to Queue_t *variable that will receive the queue
   *                    handle. This handle is used in subsequent queue
   *                    operations.
   * @param[in]  limit_ Maximum number of messages the queue can hold. Must be
   *                    at least CONFIG_QUEUE_MINIMUM_LIMIT (default 5). When
   *                    this limit is reached, the queue is full and
   *                    xQueueSend() will fail.
   *
   * @return            ReturnOK if the queue was successfully created,
   *                    ReturnError if creation failed (out of memory, invalid
   *                    limit, or system error).
   *
   * @sa xQueueDelete() - Delete a queue and free its resources
   * @sa xQueueSend() - Send a message to a queue
   * @sa xQueueReceive() - Receive a message from a queue
   * @sa xQueuePeek() - Examine next message without removing it
   * @sa xQueueMessagesWaiting() - Get number of messages in queue
   * @sa xQueueIsQueueFull() - Check if queue is at capacity
   * @sa xQueueIsQueueEmpty() - Check if queue has no messages
   * @sa xQueueLockQueue() - Lock queue to prevent modifications
   * @sa CONFIG_QUEUE_MINIMUM_LIMIT - Minimum queue capacity configuration
   */
  Return_t xQueueCreate(Queue_t **queue_, const Base_t limit_);


  /**
   * @brief Delete a message queue and free its resources
   *
   * Permanently removes a message queue and releases all associated kernel
   * resources, including all queued messages. After deletion, the queue handle
   * becomes invalid and must not be used in any subsequent queue operations.
   * This operation is typically performed during cleanup, reconfiguration, or
   * when a communication channel is no longer needed.
   *
   * xQueueDelete() immediately removes the queue and all its messages from the
   * kernel, freeing both the queue structure and all message memory. Any
   * messages that were waiting in the queue are lost—there is no mechanism to
   * retrieve them after deletion. Tasks attempting to send or receive on a
   * deleted queue will receive ReturnError.
   *
   * Key characteristics:
   * - **Immediate deletion**: Queue and all messages removed immediately
   * - **Message loss**: All pending messages are discarded
   * - **Resource cleanup**: All kernel memory associated with queue is freed
   * - **Handle invalidation**: Queue handle cannot be reused after deletion
   * - **Non-blocking**: Returns immediately after cleanup completes
   *
   * Common deletion scenarios:
   * - **Task cleanup**: Remove queues during task shutdown
   * - **Dynamic reconfiguration**: Delete and recreate queues with different
   * limits
   * - **Resource reclamation**: Free unused queues to reduce memory usage
   * - **Error recovery**: Clean up queues after initialization failures
   *
   * Example 1: Queue lifecycle management
   * @code Queue_t *commandQueue;
   *
   * void initCommandQueue(void) {
   *   if (OK(xQueueCreate(&commandQueue, 10))) {
   *     // Use queue for communication
   *   }
   * }
   *
   * void shutdownCommandQueue(void) {
   *   // Delete queue when no longer needed if (OK(xQueueDelete(commandQueue)))
   * {
   *     // Queue and all messages freed
   *   }
   * }
   * @endcode
   *
   * Example 2: Dynamic queue reconfiguration
   * @code Queue_t *dataQueue;
   *
   * void resizeQueue(Base_t newLimit) {
   *   // Delete existing queue if (dataQueue != null) {
   *     xQueueDelete(dataQueue);
   *   }
   *
   *   // Create new queue with different limit if (OK(xQueueCreate(&dataQueue,
   * newLimit))) {
   *     // New queue ready
   *   }
   * }
   * @endcode
   *
   * Example 3: Multiple queue cleanup
   * @code Queue_t *queues[MAX_CHANNELS];
   * Base_t queueCount = 0;
   *
   * void initQueues(void) {
   *   for (Base_t i = 0; i < MAX_CHANNELS; i++) {
   *     if (OK(xQueueCreate(&queues[i], 5))) {
   *       queueCount++;
   *     }
   *   }
   * }
   *
   * void shutdownQueues(void) {
   *   for (Base_t i = 0; i < queueCount; i++) {
   *     xQueueDelete(queues[i]);
   *   }
   *   queueCount = 0;
   * }
   * @endcode
   *
   * Example 4: Conditional queue cleanup
   * @code Queue_t *eventQueue;
   * Base_t queueActive = 0;
   *
   * void disableEvents(void) {
   *   if (queueActive) {
   *     xQueueDelete(eventQueue);
   *     queueActive = 0;
   *   }
   * }
   *
   * void enableEvents(void) {
   *   if (!queueActive) {
   *     if (OK(xQueueCreate(&eventQueue, 8))) {
   *       queueActive = 1;
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in] queue_ Handle to the queue to delete. Must be a valid queue
   *                   created with xQueueCreate(). After deletion, this handle
   *                   becomes invalid.
   *
   * @return           ReturnOK if queue deleted successfully, ReturnError if
   *                   deletion failed due to invalid queue handle or queue not
   *                   found.
   *
   * @warning All messages pending in the queue are permanently lost when the
   * queue is deleted. If you need to preserve messages, drain the queue with
   * xQueueReceive() before calling xQueueDelete().
   *
   * @warning After xQueueDelete() returns successfully, the queue handle is
   * invalid and must not be used in any subsequent operations. Using a deleted
   * queue handle will result in ReturnError.
   *
   * @warning Deleting a queue that is actively used by multiple tasks can lead
   * to errors if those tasks attempt to access the queue after deletion.
   * Coordinate queue deletion across all tasks using the queue.
   *
   * @note xQueueDelete() frees all memory associated with the queue, including
   * the queue structure and all message memory. This is the only way to reclaim
   * kernel memory allocated for a queue.
   *
   * @note Unlike stopping a timer, which preserves the timer for reuse,
   * deleting a queue permanently removes it. To reuse queue functionality,
   * create a new queue with xQueueCreate().
   *
   * @sa xQueueCreate() - Create a message queue
   * @sa xQueueReceive() - Receive messages before deletion
   * @sa xQueueMessagesWaiting() - Check for pending messages
   * @sa xTaskDelete() - Delete a task (similar resource cleanup pattern)
   * @sa xTimerDelete() - Delete a timer (similar resource cleanup pattern)
   */
  Return_t xQueueDelete(Queue_t *queue_);


  /**
   * @brief Query the maximum message capacity of a queue
   *
   * Retrieves the maximum number of messages (limit) that a queue can hold, as
   * configured during queue creation with xQueueCreate(). This non-destructive
   * query returns the queue's capacity, not the number of messages currently
   * waiting. The limit represents the upper bound on how many messages can be
   * queued before xQueueSend() returns ReturnError due to queue full.
   *
   * Note: Despite the name "GetLength", this function returns the queue's
   * maximum capacity (limit), not the current message count. To get the number
   * of messages currently waiting, use xQueueMessagesWaiting().
   *
   * Common use cases:
   * - **Capacity validation**: Verify queue created with expected limit
   * - **Flow control**: Calculate available space before bulk sends
   * - **Diagnostics**: Report queue configuration for debugging
   * - **Dynamic adjustment**: Determine if queue needs resizing
   *
   * Example 1: Validate queue configuration
   * @code Queue_t *dataQueue;
   * Base_t expectedLimit = 10;
   *
   * if (OK(xQueueCreate(&dataQueue, expectedLimit))) {
   *   Base_t actualLimit;
   *   if (OK(xQueueGetLength(dataQueue, &actualLimit))) {
   *     if (actualLimit == expectedLimit) {
   *       // Queue configured correctly
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Calculate available queue space
   * @code Queue_t *commandQueue;
   *
   * Base_t getAvailableSpace(void) {
   *   Base_t limit, waiting;
   *
   *   if (OK(xQueueGetLength(commandQueue, &limit)) &&
   *       OK(xQueueMessagesWaiting(commandQueue, &waiting))) {
   *     return limit - waiting;
   *   }
   *   return 0;
   * }
   * @endcode
   *
   * Example 3: Queue utilization monitoring
   * @code Queue_t *eventQueue;
   *
   * void reportQueueStatus(void) {
   *   Base_t limit, waiting;
   *
   *   if (OK(xQueueGetLength(eventQueue, &limit)) &&
   *       OK(xQueueMessagesWaiting(eventQueue, &waiting))) {
   *     Base_t utilization = (waiting * 100) / limit;
   *     printf("Queue: %d/%d messages (%d%% full)\n", waiting, limit,
   * utilization);
   *   }
   * }
   * @endcode
   *
   * @param[in]  queue_ Handle to the queue to query. Must be a valid queue
   *                    created with xQueueCreate().
   * @param[out] res_   Pointer to variable receiving the queue limit (maximum
   *                    capacity). On success, contains the limit value
   *                    specified during xQueueCreate().
   *
   * @return            ReturnOK if query succeeded, ReturnError if query failed
   *                    due to invalid queue handle or queue not found.
   *
   * @warning This function returns the queue's maximum capacity (limit), NOT
   * the current number of messages. Use xQueueMessagesWaiting() to get the
   * current message count.
   *
   * @note The returned limit is constant for the lifetime of the queue—it
   * cannot be changed after creation. To change capacity, delete and recreate
   * the queue.
   *
   * @sa xQueueMessagesWaiting() - Get current message count
   * @sa xQueueIsQueueFull() - Check if queue is at capacity
   * @sa xQueueIsQueueEmpty() - Check if queue has no messages
   * @sa xQueueCreate() - Create queue with specified limit
   */
  Return_t xQueueGetLength(const Queue_t *queue_, Base_t *res_);


  /**
   * @brief Check if a message queue has no messages waiting
   *
   * Queries whether a message queue is empty (contains zero messages). This
   * non-destructive check allows tasks to determine if messages are available
   * before attempting to receive, enabling conditional receive logic and
   * avoiding unnecessary xQueueReceive() calls on empty queues. A queue is
   * considered empty when it contains no messages, regardless of its maximum
   * capacity.
   *
   * Common use cases:
   * - **Conditional receive**: Only call xQueueReceive() if messages available
   * - **Flow control**: Check for messages before processing
   * - **Diagnostics**: Monitor queue activity
   * - **Polling loops**: Detect when queue has been drained
   *
   * Example 1: Conditional message processing
   * @code Queue_t *eventQueue;
   *
   * void processEvents(void) {
   *   Base_t isEmpty;
   *
   *   if (OK(xQueueIsQueueEmpty(eventQueue, &isEmpty)) && !isEmpty) {
   *     // Queue has messages - process them QueueMessage_t *msg;
   *     while (OK(xQueueReceive(eventQueue, &msg))) {
   *       handleEvent(&msg);
   *       xMemFree((Addr_t *)msg.value);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Wait for messages
   * @code Queue_t *dataQueue;
   *
   * void waitForData(void) {
   *   Base_t isEmpty = 1;
   *
   *   // Poll until message arrives while (isEmpty) {
   *     if (OK(xQueueIsQueueEmpty(dataQueue, &isEmpty))) {
   *       if (isEmpty) {
   *         xTaskWait(10);  // Wait before checking again
   *       }
   *     }
   *   }
   *
   *   // Message available - process it QueueMessage_t *msg;
   *   if (OK(xQueueReceive(dataQueue, &msg))) {
   *     processData(&msg);
   *     xMemFree((Addr_t *)msg.value);
   *   }
   * }
   * @endcode
   *
   * Example 3: Drain queue completely
   * @code Queue_t *commandQueue;
   *
   * void drainQueue(void) {
   *   Base_t isEmpty;
   *
   *   if (OK(xQueueIsQueueEmpty(commandQueue, &isEmpty))) {
   *     while (!isEmpty) {
   *       QueueMessage_t *msg;
   *       if (OK(xQueueReceive(commandQueue, &msg))) {
   *         xMemFree((Addr_t *)msg.value);
   *       }
   *       xQueueIsQueueEmpty(commandQueue, &isEmpty);
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  queue_ Handle to the queue to query. Must be a valid queue
   *                    created with xQueueCreate().
   * @param[out] res_   Pointer to variable receiving the empty status. Set to
   *                    non-zero (true) if queue is empty, zero (false) if queue
   *                    contains one or more messages.
   *
   * @return            ReturnOK if query succeeded, ReturnError if query failed
   *                    due to invalid queue handle or queue not found.
   *
   * @note This is a snapshot query—in multitasking environments, the queue
   * state may change immediately after the check if other tasks send messages.
   *
   * @note xQueueIsQueueEmpty() is equivalent to checking if
   * xQueueMessagesWaiting() returns 0, but provides a more readable API for
   * empty checks.
   *
   * @sa xQueueMessagesWaiting() - Get exact message count
   * @sa xQueueIsQueueFull() - Check if queue is at capacity
   * @sa xQueueReceive() - Receive message from queue
   * @sa xQueuePeek() - Check message without removing it
   */
  Return_t xQueueIsQueueEmpty(const Queue_t *queue_, Base_t *res_);


  /**
   * @brief Check if a message queue is at maximum capacity
   *
   * Queries whether a message queue is full (at maximum capacity). This check
   * allows tasks to determine if there is space available before attempting to
   * send, enabling flow control and preventing message loss. A queue is
   * considered full when the number of waiting messages equals the limit
   * specified during xQueueCreate(). When full, xQueueSend() will return
   * ReturnError.
   *
   * Common use cases:
   * - **Conditional send**: Only send if space available
   * - **Flow control**: Back off when queue approaches capacity
   * - **Overflow prevention**: Detect and handle full queue conditions
   * - **Diagnostics**: Monitor queue utilization and congestion
   *
   * Example 1: Conditional message send
   * @code Queue_t *commandQueue;
   *
   * Return_t sendCommand(Byte_t *cmd, Base_t size) {
   *   Base_t isFull;
   *
   *   if (OK(xQueueIsQueueFull(commandQueue, &isFull))) {
   *     if (isFull) {
   *       logWarning("Command queue full - dropping command");
   *       return ReturnError;
   *     }
   *
   *     // Space available - send message return xQueueSend(commandQueue, size,
   * cmd);
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 2: Flow control with backoff
   * @code Queue_t *dataQueue;
   *
   * void sendDataWithBackoff(Byte_t *data, Base_t size) {
   *   Base_t isFull;
   *   Base_t retries = 0;
   *
   *   while (retries < 10) {
   *     if (OK(xQueueIsQueueFull(dataQueue, &isFull))) {
   *       if (!isFull) {
   *         // Space available - send now if (OK(xQueueSend(dataQueue, size,
   * data))) {
   *           return;
   *         }
   *       }
   *     }
   *
   *     // Queue full - wait and retry xTaskWait(10);
   *     retries++;
   *   }
   *
   *   logError("Failed to send data - queue full");
   * }
   * @endcode
   *
   * Example 3: Monitor queue pressure
   * @code Queue_t *eventQueue;
   *
   * void monitorQueuePressure(void) {
   *   Base_t isFull, waiting;
   *
   *   if (OK(xQueueIsQueueFull(eventQueue, &isFull)) && isFull) {
   *     logWarning("Event queue at capacity");
   *   } else if (OK(xQueueMessagesWaiting(eventQueue, &waiting))) {
   *     Base_t limit;
   *     if (OK(xQueueGetLength(eventQueue, &limit))) {
   *       if (waiting > (limit * 80) / 100) {
   *         logWarning("Event queue nearly full: %d/%d", waiting, limit);
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  queue_ Handle to the queue to query. Must be a valid queue
   *                    created with xQueueCreate().
   * @param[out] res_   Pointer to variable receiving the full status. Set to
   *                    non-zero (true) if queue is full, zero (false) if queue
   *                    has space for more messages.
   *
   * @return            ReturnOK if query succeeded, ReturnError if query failed
   *                    due to invalid queue handle or queue not found.
   *
   * @note This is a snapshot query—in multitasking environments, the queue
   * state may change immediately after the check if other tasks receive
   * messages.
   *
   * @note xQueueIsQueueFull() checks if message count equals the limit. To
   * check for nearly-full conditions, use xQueueMessagesWaiting() and compare
   * to xQueueGetLength().
   *
   * @sa xQueueMessagesWaiting() - Get exact message count
   * @sa xQueueIsQueueEmpty() - Check if queue has no messages
   * @sa xQueueGetLength() - Get queue capacity
   * @sa xQueueSend() - Send message (fails when queue is full)
   */
  Return_t xQueueIsQueueFull(const Queue_t *queue_, Base_t *res_);


  /**
   * @brief Query the number of messages currently in a queue
   *
   * Retrieves the count of messages currently waiting in a queue. This
   * non-destructive query provides precise queue occupancy information,
   * allowing tasks to make informed decisions about message processing, flow
   * control, and resource allocation. The count represents messages
   * successfully sent with xQueueSend() but not yet received with
   * xQueueReceive().
   *
   * Common use cases:
   * - **Batch processing**: Process multiple messages when threshold reached
   * - **Queue utilization**: Monitor queue load and congestion
   * - **Flow control**: Throttle senders based on queue depth
   * - **Diagnostics**: Report queue activity for debugging
   *
   * Example 1: Batch message processing
   * @code Queue_t *eventQueue;
   *
   * #define BATCH_THRESHOLD 5
   *
   * void processBatchEvents(void) {
   *   Base_t waiting;
   *
   *   if (OK(xQueueMessagesWaiting(eventQueue, &waiting))) {
   *     if (waiting >= BATCH_THRESHOLD) {
   *       // Process messages in batch for efficiency for (Base_t i = 0; i <
   * waiting; i++) {
   *         QueueMessage_t *msg;
   *         if (OK(xQueueReceive(eventQueue, &msg))) {
   *           handleEvent(&msg);
   *           xMemFree((Addr_t *)msg.value);
   *         }
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Queue utilization monitoring
   * @code Queue_t *commandQueue;
   *
   * void reportQueueStats(void) {
   *   Base_t waiting, limit;
   *
   *   if (OK(xQueueMessagesWaiting(commandQueue, &waiting)) &&
   *       OK(xQueueGetLength(commandQueue, &limit))) {
   *     Base_t utilization = (waiting * 100) / limit;
   *     printf("Queue: %d/%d messages (%d%% full)\n", waiting, limit,
   * utilization);
   *
   *     if (utilization > 90) {
   *       logWarning("Queue nearly full - consider increasing capacity");
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Producer flow control
   * @code Queue_t *dataQueue;
   *
   * #define HIGH_WATER_MARK 8
   * #define LOW_WATER_MARK 3
   *
   * static Base_t producerThrottled = 0;
   *
   * void produceData(Byte_t *data, Base_t size) {
   *   Base_t waiting;
   *
   *   if (OK(xQueueMessagesWaiting(dataQueue, &waiting))) {
   *     // Implement hysteresis for flow control if (waiting >=
   * HIGH_WATER_MARK) {
   *       producerThrottled = 1;
   *       logInfo("Producer throttled - queue depth: %d", waiting);
   *     } else if (waiting <= LOW_WATER_MARK) {
   *       producerThrottled = 0;
   *     }
   *
   *     if (!producerThrottled) {
   *       xQueueSend(dataQueue, size, data);
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  queue_ Handle to the queue to query. Must be a valid queue
   *                    created with xQueueCreate().
   * @param[out] res_   Pointer to variable receiving the message count. On
   *                    success, contains the number of messages currently
   *                    waiting (0 to limit).
   *
   * @return            ReturnOK if query succeeded, ReturnError if query failed
   *                    due to invalid queue handle or queue not found.
   *
   * @note This is a snapshot query—in multitasking environments, the message
   * count may change immediately after the query if other tasks send or receive
   * messages.
   *
   * @note The count includes all messages from the head to the tail of the
   * queue. Messages accessed with xQueuePeek() are still counted as waiting.
   *
   * @sa xQueueIsQueueEmpty() - Check if count is zero (no messages)
   * @sa xQueueIsQueueFull() - Check if count equals limit (at capacity)
   * @sa xQueueGetLength() - Get queue maximum capacity
   * @sa xQueueReceive() - Remove and receive next message
   * @sa xQueueSend() - Add message to queue
   */
  Return_t xQueueMessagesWaiting(const Queue_t *queue_, Base_t *res_);


  /**
   * @brief Send a message to a queue (producer operation)
   *
   * Adds a message to the specified queue, copying the provided data into the
   * queue's internal storage. This is the primary mechanism for tasks to send
   * data to other tasks in a producer-consumer pattern. The operation is
   * non-blocking and returns immediately whether successful or not.
   *
   * Messages are stored in FIFO order and will be retrieved by xQueueReceive()
   * in the same order they were sent. Each message is a byte array up to
   * CONFIG_MESSAGE_VALUE_BYTES (default 8) bytes, allowing flexible message
   * formats including structures, commands, sensor readings, or any other data
   * that fits within the size limit.
   *
   * Message Sending Process:
   * 1. Check if queue has space (not full) 2. Copy message data into queue
   * storage 3. Increment queue message count 4. Return success or failure
   * immediately (non-blocking)
   *
   * Common Message Patterns:
   * - **Commands**: Single byte command codes (e.g., 0x01 = START, 0x02 = STOP)
   * - **Sensor data**: Multi-byte readings packed into message
   * - **Events**: Event type and optional data
   * - **Pointers**: Address of larger data structure (use with caution)
   * - **Structures**: Small structs that fit within byte limit
   *
   * @warning Messages are limited to CONFIG_MESSAGE_VALUE_BYTES bytes (default
   * 8). Attempting to send larger messages will fail. For larger data, consider
   * sending a pointer to heap-allocated data or using multiple messages.
   *
   * @warning The message data is COPIED into the queue. The original buffer can
   * be reused or freed immediately after xQueueSend() returns. The queue
   * maintains its own copy.
   *
   * @note If the queue is full, xQueueSend() returns ReturnError immediately
   * without blocking. Check queue status with xQueueIsQueueFull() before
   * sending if needed, or handle send failures appropriately.
   *
   * @note Messages are copied, not moved. This ensures the sender retains
   * control of its data and prevents aliasing issues.
   *
   * Example Usage:
   * @code Queue_t *dataQueue;
   * xQueueCreate(&dataQueue, 10);
   *
   * // Send a simple command byte uint8_t command = 0x42;
   * if (OK(xQueueSend(dataQueue, 1, &command))) {
   *   // Command queued successfully
   * }
   *
   * // Send sensor reading (multi-byte) typedef struct {
   *   uint16_t temperature;
   *   uint16_t humidity;
   * } SensorReading_t;
   *
   * SensorReading_t reading;
   * reading.temperature = 2350;  // 23.50°C reading.humidity = 6500;     //
   * 65.00%
   *
   * if (OK(xQueueSend(dataQueue, sizeof(SensorReading_t), (Byte_t *)&reading)))
   * {
   *   // Sensor data queued
   * } else {
   *   // Queue full - handle overflow handleQueueOverflow();
   * }
   *
   * // Producer task example void sensorTask(Task_t *task, TaskParm_t *parm) {
   *   uint8_t sensorData[4];
   *
   *   readSensors(sensorData);
   *
   *   // Try to send, handle failure if (ERROR(xQueueSend(dataQueue,
   * sizeof(sensorData), sensorData))) {
   *     // Queue full - either drop data or wait dataSamplesDropped++;
   *   }
   * }
   *
   * // Check queue space before sending Base_t isFull;
   * if (OK(xQueueIsQueueFull(dataQueue, &isFull)) && !isFull) {
   *   // Safe to send xQueueSend(dataQueue, msgSize, msgData);
   * }
   * @endcode
   *
   * @param[in] queue_ Handle of the queue to send to. Must be a valid queue
   *                   handle previously returned by xQueueCreate().
   * @param[in] bytes_ Size of the message in bytes. Must be greater than 0 and
   *                   not exceed CONFIG_MESSAGE_VALUE_BYTES (default 8).
   * @param[in] value_ Pointer to the message data to send. The data will be
   *                   copied into the queue, so this buffer can be reused after
   *                   the call.
   *
   * @return           ReturnOK if the message was successfully added to the
   *                   queue, ReturnError if the send failed (queue full,
   *                   invalid queue handle, invalid size, or null value
   *                   pointer).
   *
   * @sa xQueueReceive() - Receive a message from the queue (consumer operation)
   * @sa xQueuePeek() - Examine next message without removing it
   * @sa xQueueIsQueueFull() - Check if queue is at capacity
   * @sa xQueueMessagesWaiting() - Get number of messages in queue
   * @sa xQueueCreate() - Create a new message queue
   * @sa xQueueLockQueue() - Lock queue during critical operations
   * @sa CONFIG_MESSAGE_VALUE_BYTES - Maximum message size configuration
   */
  Return_t xQueueSend(Queue_t *queue_, const Base_t bytes_, const Byte_t *value_);


  /**
   * @brief Examine the next queue message without removing it
   *
   * Retrieves a copy of the oldest message from the queue without removing it,
   * allowing inspection of queue contents without consuming messages. Unlike
   * xQueueReceive(), which removes the message, xQueuePeek() leaves the queue
   * unchanged—the message remains available for subsequent peek or receive
   * operations. This enables conditional message processing and queue
   * inspection.
   *
   * The peeked message is allocated from the heap as a copy—the original
   * message stays in the queue. The caller must free the peeked message copy
   * with xMemFree() after use. Multiple peek calls return the same message
   * until it is removed with xQueueReceive() or xQueueDropMessage().
   *
   * Common use cases:
   * - **Conditional processing**: Inspect message before deciding to receive
   * - **Message filtering**: Check message type/priority before consuming
   * - **Queue monitoring**: Examine next message without affecting queue state
   * - **Priority handling**: Process high-priority messages first
   *
   * Example 1: Priority message handling
   * @code Queue_t *eventQueue;
   *
   * #define MSG_PRIORITY_HIGH 1
   * #define MSG_PRIORITY_NORMAL 0
   *
   * void processEvents(void) {
   *   QueueMessage_t *msg;
   *
   *   // Peek at next message if (OK(xQueuePeek(eventQueue, &msg))) {
   *     Byte_t priority = msg.value[0];  // Assume first byte is priority
   *
   *     if (priority == MSG_PRIORITY_HIGH) {
   *       // High priority - receive and process immediately xMemFree((Addr_t
   * *)msg.value);  // Free peek copy xQueueReceive(eventQueue,
   * &msg);
   *       handleHighPriority(&msg);
   *       xMemFree((Addr_t *)msg.value);
   *     } else {
   *       // Normal priority - skip for now xMemFree((Addr_t *)msg.value);  //
   * Free peek copy
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Message type filtering
   * @code Queue_t *commandQueue;
   *
   * Base_t hasCommandType(Byte_t type) {
   *   QueueMessage_t *msg;
   *
   *   // Peek without consuming if (OK(xQueuePeek(commandQueue, &msg))) {
   *     Byte_t cmdType = msg.value[0];
   *     xMemFree((Addr_t *)msg.value);
   *     return (cmdType == type);
   *   }
   *   return 0;
   * }
   * @endcode
   *
   * @param[in]  queue_   Handle to the queue to peek. Must be a valid queue
   *                      created with xQueueCreate().
   * @param[out] message_ Pointer to QueueMessage_t *structure receiving a copy
   *                      of the message. Memory is allocated for message->value
   *                      and must be freed with xMemFree() after use.
   *
   * @return              ReturnOK if message peeked successfully, ReturnError
   *                      if peek failed due to empty queue, invalid queue
   *                      handle, or memory allocation failure.
   *
   * @warning The peeked message is a heap-allocated COPY. You must free it with
   * xMemFree() even though the original message remains in the queue.
   *
   * @warning Multiple peeks return the SAME message until it is removed. If you
   * peek, then another task receives the message, subsequent peeks will return
   * the next message in the queue.
   *
   * @note xQueuePeek() does not modify the queue state—message count and queue
   * position remain unchanged.
   *
   * @sa xQueueReceive() - Receive and remove message
   * @sa xQueueDropMessage() - Remove message without retrieving
   * @sa xQueueMessagesWaiting() - Check if messages available before peeking
   * @sa xMemFree() - Free peeked message copy
   */
  Return_t xQueuePeek(const Queue_t *queue_, QueueMessage_t **message_);


  /**
   * @brief Remove the next message from queue without retrieving it
   *
   * Removes and discards the oldest message from the queue without retrieving
   * its contents. This operation decrements the message count and frees the
   * message memory, but does not return the message data to the caller. Use
   * this when you want to skip or discard messages without processing them.
   *
   * Unlike xQueueReceive() which allocates and returns the message,
   * xQueueDropMessage() simply removes the message and frees its memory
   * immediately. This is more efficient when message content is not needed—for
   * example, when clearing stale messages, handling overflow conditions, or
   * implementing message filtering.
   *
   * Common use cases:
   * - **Queue clearing**: Discard all messages when resetting state
   * - **Message filtering**: Skip unwanted messages after peeking
   * - **Overflow handling**: Drop old messages when queue backs up
   * - **Selective processing**: Discard messages based on peek inspection
   *
   * Example 1: Clear all messages from queue
   * @code Queue_t *eventQueue;
   *
   * void clearQueue(void) {
   *   Base_t isEmpty;
   *
   *   if (OK(xQueueIsQueueEmpty(eventQueue, &isEmpty))) {
   *     while (!isEmpty) {
   *       xQueueDropMessage(eventQueue);
   *       xQueueIsQueueEmpty(eventQueue, &isEmpty);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Selective message processing with peek and drop
   * @code Queue_t *dataQueue;
   *
   * void processOnlyValidMessages(void) {
   *   Base_t waiting;
   *
   *   if (OK(xQueueMessagesWaiting(dataQueue, &waiting))) {
   *     for (Base_t i = 0; i < waiting; i++) {
   *       QueueMessage_t *msg;
   *
   *       if (OK(xQueuePeek(dataQueue, &msg))) {
   *         Byte_t msgType = msg.value[0];
   *         xMemFree((Addr_t *)msg.value);  // Free peek copy
   *
   *         if (isValidMessageType(msgType)) {
   *           // Valid - receive and process if (OK(xQueueReceive(dataQueue,
   * &msg))) {
   *             processMessage(&msg);
   *             xMemFree((Addr_t *)msg.value);
   *           }
   *         } else {
   *           // Invalid - drop without retrieving
   * xQueueDropMessage(dataQueue);
   *         }
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Drop stale messages on timeout
   * @code Queue_t *timeStampedQueue;
   * Timer_t *timeoutTimer;
   *
   * void dropStaleMessages(void) {
   *   Base_t expired;
   *
   *   if (OK(xTimerHasTimerExpired(timeoutTimer, &expired)) && expired) {
   *     // Timeout elapsed - drop oldest message
   * xQueueDropMessage(timeStampedQueue);
   *     xTimerReset(timeoutTimer);
   *   }
   * }
   * @endcode
   *
   * @param[in] queue_ Handle to the queue from which to drop a message. Must be
   *                   a valid queue created with xQueueCreate().
   *
   * @return           ReturnOK if message dropped successfully, ReturnError if
   *                   operation failed due to empty queue or invalid queue
   *                   handle.
   *
   * @warning The dropped message is permanently lost—there is no way to
   * retrieve it after calling xQueueDropMessage(). If you need the message
   * content, use xQueueReceive() instead.
   *
   * @note xQueueDropMessage() is more efficient than xQueueReceive() followed
   * by xMemFree() because it avoids allocating memory for the message copy.
   *
   * @note If the queue is empty, xQueueDropMessage() returns ReturnError
   * immediately.
   *
   * @sa xQueueReceive() - Receive and retrieve message
   * @sa xQueuePeek() - Examine message without removing it
   * @sa xQueueMessagesWaiting() - Check message count before dropping
   * @sa xQueueIsQueueEmpty() - Check if queue has messages
   */
  Return_t xQueueDropMessage(Queue_t *queue_);


  /**
   * @brief Receive and remove a message from a queue (consumer operation)
   *
   * Retrieves the oldest message from the specified queue and removes it,
   * making space for new messages. This is the primary mechanism for tasks to
   * receive data from other tasks in a producer-consumer pattern. The operation
   * is non-blocking and returns immediately whether a message is available or
   * not.
   *
   * This function combines the operations of xQueuePeek() (examine message) and
   * xQueueDropMessage() (remove message) into a single atomic operation,
   * ensuring thread-safe message retrieval even with multiple consumers.
   *
   * The received message is allocated from the heap and must be freed by the
   * caller using xMemFree() after processing. This heap allocation is necessary
   * because message size is variable.
   *
   * Message Receiving Process:
   * 1. Check if queue has messages (not empty) 2. Allocate QueueMessage_t *
   * structure from heap 3. Copy oldest message data into structure 4. Remove
   * message from queue 5. Return message structure to caller 6. Caller
   * processes message then frees with xMemFree()
   *
   * Typical Consumer Pattern:
   * 1. Check if messages are waiting (optional optimization) 2. Call
   * xQueueReceive() to get message 3. Check return value - success means
   * message received 4. Process message->message bytes (size is in
   * message->size) 5. Free message structure with xMemFree()
   *
   * @warning The returned message structure is allocated from the heap and MUST
   * be freed by the caller using xMemFree(). Failing to free messages will
   * cause memory leaks and eventually exhaust available heap memory.
   *
   * @note If the queue is empty, xQueueReceive() returns ReturnError
   * immediately without blocking. Check queue status with xQueueIsQueueEmpty()
   * or xQueueMessagesWaiting() before receiving if needed.
   *
   * @note The QueueMessage_t *structure contains:
   *       - message->size: Number of bytes in the message
   *       - message->message: Pointer to the message data bytes
   *
   * @note Unlike xQueuePeek() which leaves the message in the queue,
   * xQueueReceive() removes the message, allowing the next message to be
   * retrieved on the next call.
   *
   * Example Usage:
   * @code Queue_t *commandQueue;
   * xQueueCreate(&commandQueue, 10);
   *
   * // Consumer task: Process commands from queue void
   * commandProcessorTask(Task_t *task, TaskParm_t *parm) {
   *   QueueMessage_t **msg = NULL;
   *
   *   // Try to receive a message if (OK(xQueueReceive(commandQueue, &msg))) {
   *     // Message received successfully
   *
   *     // Process based on message size and content if (msg->size == 1) {
   *       // Single byte command uint8_t cmd = msg->message[0];
   *       handleCommand(cmd);
   *     } else if (msg->size == sizeof(SensorData_t)) {
   *       // Structure message SensorData_t *data = (SensorData_t
   * *)msg->message;
   *       processSensorData(data);
   *     }
   *
   *     // IMPORTANT: Free the message structure xMemFree((Addr_t *)msg);
   *   } else {
   *     // No messages available - queue empty
   *   }
   * }
   *
   * // Complete producer-consumer example Queue_t *dataQueue;
   *
   * // Producer void producerTask(Task_t *task, TaskParm_t *parm) {
   *   uint8_t data[4] = {0x01, 0x02, 0x03, 0x04};
   *   xQueueSend(dataQueue, sizeof(data), data);
   * }
   *
   * // Consumer void consumerTask(Task_t *task, TaskParm_t *parm) {
   *   QueueMessage_t **msg = NULL;
   *
   *   // Check for messages first (optional optimization) Base_t numMessages;
   *   if (OK(xQueueMessagesWaiting(dataQueue, &numMessages))) {
   *     if (numMessages > 0) {
   *       // Messages available, receive one if (OK(xQueueReceive(dataQueue,
   * &msg))) {
   *         // Process msg->message[0] through msg->message[msg->size-1]
   * processData(msg->message, msg->size);
   *         xMemFree((Addr_t *)msg);
   *       }
   *     }
   *   }
   * }
   *
   * // Process all pending messages Base_t isEmpty;
   * while (OK(xQueueIsQueueEmpty(dataQueue, &isEmpty)) && !isEmpty) {
   *   QueueMessage_t **msg = NULL;
   *   if (OK(xQueueReceive(dataQueue, &msg))) {
   *     processMessage(msg);
   *     xMemFree((Addr_t *)msg);
   *   }
   * }
   * @endcode
   *
   * @param[in]  queue_   Handle of the queue to receive from. Must be a valid
   *                      queue handle previously returned by xQueueCreate().
   * @param[out] message_ Pointer to QueueMessage_t *pointer variable. On
   *                      success, receives a pointer to the message structure
   *                      containing the message data and size. Caller MUST free
   *                      this with xMemFree().
   *
   * @return              ReturnOK if a message was successfully received and
   *                      removed from the queue, ReturnError if the receive
   *                      failed (queue empty, invalid queue handle, memory
   *                      allocation failed, or null message pointer).
   *
   * @sa xQueueSend() - Send a message to the queue (producer operation)
   * @sa xQueuePeek() - Examine next message without removing it
   * @sa xQueueDropMessage() - Remove message after peeking
   * @sa xQueueIsQueueEmpty() - Check if queue has no messages
   * @sa xQueueMessagesWaiting() - Get number of messages in queue
   * @sa xQueueCreate() - Create a new message queue
   * @sa xMemFree() - Free the message structure (REQUIRED!)
   */
  Return_t xQueueReceive(Queue_t *queue_, QueueMessage_t **message_);


  /**
   * @brief Lock a queue to prevent new messages from being sent
   *
   * Places a queue in locked state, preventing tasks from sending new messages
   * with xQueueSend(). While locked, send operations return ReturnError
   * immediately. However, locked queues still allow receiving, peeking, and
   * dropping messages—
   * only send operations are blocked. This enables controlled queue drainage
   * and prevents queue overflow during critical processing.
   *
   * Queue locking is useful for implementing flow control, preventing message
   * accumulation during batch processing, or ensuring a queue is drained before
   * reconfiguration. Locks must be explicitly released with xQueueUnLockQueue()
   * to resume normal operation—they do not timeout or automatically unlock.
   *
   * Common use cases:
   * - **Batch processing**: Lock queue while processing all pending messages
   * - **Flow control**: Prevent producer overrun during high processing load
   * - **Queue drainage**: Ensure queue is empty before shutdown or
   * reconfiguration
   * - **Critical sections**: Prevent message accumulation during critical
   * operations
   *
   * Example 1: Batch processing with lock
   * @code Queue_t *eventQueue;
   *
   * void processBatch(void) {
   *   // Lock to prevent new messages during batch xQueueLockQueue(eventQueue);
   *
   *   // Process all current messages Base_t isEmpty;
   *   if (OK(xQueueIsQueueEmpty(eventQueue, &isEmpty))) {
   *     while (!isEmpty) {
   *       QueueMessage_t *msg;
   *       if (OK(xQueueReceive(eventQueue, &msg))) {
   *         handleEvent(&msg);
   *         xMemFree((Addr_t *)msg.value);
   *       }
   *       xQueueIsQueueEmpty(eventQueue, &isEmpty);
   *     }
   *   }
   *
   *   // Unlock to resume message sending xQueueUnLockQueue(eventQueue);
   * }
   * @endcode
   *
   * Example 2: Controlled shutdown
   * @code Queue_t *commandQueue;
   *
   * void shutdownCommandProcessor(void) {
   *   // Lock queue to prevent new commands xQueueLockQueue(commandQueue);
   *
   *   // Process remaining commands QueueMessage_t *msg;
   *   while (OK(xQueueReceive(commandQueue, &msg))) {
   *     executeCommand(&msg);
   *     xMemFree((Addr_t *)msg.value);
   *   }
   *
   *   // Delete queue (no need to unlock) xQueueDelete(commandQueue);
   * }
   * @endcode
   *
   * @param[in] queue_ Handle to the queue to lock. Must be a valid queue
   *                   created with xQueueCreate().
   *
   * @return           ReturnOK if queue locked successfully, ReturnError if
   *                   operation failed due to invalid queue handle or queue not
   *                   found.
   *
   * @warning Locking a queue does NOT prevent receiving, peeking, or dropping
   * messages—only xQueueSend() is blocked. Consumers can still drain the queue
   * while it is locked.
   *
   * @warning Locks must be explicitly released with xQueueUnLockQueue(). There
   * is no automatic timeout or unlock mechanism.
   *
   * @note Locking an already-locked queue is safe and has no effect.
   *
   * @note xQueueSend() on a locked queue returns ReturnError immediately
   * without blocking or queuing the message.
   *
   * @sa xQueueUnLockQueue() - Unlock queue to resume sending
   * @sa xQueueSend() - Send message (fails on locked queue)
   * @sa xQueueReceive() - Receive message (works on locked queue)
   */
  Return_t xQueueLockQueue(Queue_t *queue_);


  /**
   * @brief Unlock a queue to resume accepting new messages
   *
   * Removes the locked state from a queue, allowing tasks to resume sending
   * messages with xQueueSend(). After unlocking, the queue returns to normal
   * operation where send operations succeed (if queue is not full). This
   * function must be called to restore normal queue behavior after locking with
   * xQueueLockQueue().
   *
   * Unlocking affects only send operations—receive, peek, and drop operations
   * were unaffected by the lock and continue to work normally after unlock.
   *
   * Common use cases:
   * - **Resume normal operation**: Re-enable message sending after batch
   * processing
   * - **End critical section**: Allow producers to continue after critical
   * operations
   * - **Flow control**: Resume accepting messages after backpressure relieved
   *
   * Example 1: Lock/unlock pattern for batch processing
   * @code Queue_t *dataQueue;
   *
   * void batchProcessor(void) {
   *   // Lock to get consistent snapshot xQueueLockQueue(dataQueue);
   *
   *   Base_t count;
   *   if (OK(xQueueMessagesWaiting(dataQueue, &count))) {
   *     // Process exactly this many messages for (Base_t i = 0; i < count;
   * i++)
   * {
   *       QueueMessage_t *msg;
   *       if (OK(xQueueReceive(dataQueue, &msg))) {
   *         processBatchItem(&msg);
   *         xMemFree((Addr_t *)msg.value);
   *       }
   *     }
   *   }
   *
   *   // Unlock to allow new messages xQueueUnLockQueue(dataQueue);
   * }
   * @endcode
   *
   * Example 2: Conditional unlock based on processing result
   * @code Queue_t *eventQueue;
   *
   * Base_t processCriticalEvents(void) {
   *   xQueueLockQueue(eventQueue);
   *
   *   Base_t success = 1;
   *   QueueMessage_t *msg;
   *
   *   while (OK(xQueueReceive(eventQueue, &msg)) && success) {
   *     if (ERROR(handleCriticalEvent(&msg))) {
   *       success = 0;  // Processing failed
   *     }
   *     xMemFree((Addr_t *)msg.value);
   *   }
   *
   *   if (success) {
   *     // Success - resume normal operation xQueueUnLockQueue(eventQueue);
   *   } else {
   *     // Failure - leave locked for manual intervention logError("Critical
   * event processing failed - queue locked");
   *   }
   *
   *   return success;
   * }
   * @endcode
   *
   * @param[in] queue_ Handle to the queue to unlock. Must be a valid queue
   *                   created with xQueueCreate().
   *
   * @return           ReturnOK if queue unlocked successfully, ReturnError if
   *                   operation failed due to invalid queue handle or queue not
   *                   found.
   *
   * @warning Unlocking an already-unlocked queue is safe and has no effect.
   *
   * @note Always pair xQueueLockQueue() with xQueueUnLockQueue() to avoid
   * leaving queues permanently locked, which would prevent all future message
   * sending.
   *
   * @note If you delete a locked queue with xQueueDelete(), you do not need to
   * unlock it first—the deletion frees all queue resources including lock
   * state.
   *
   * @sa xQueueLockQueue() - Lock queue to prevent sending
   * @sa xQueueSend() - Send message (enabled after unlock)
   * @sa xQueueReceive() - Receive message (always works)
   */
  Return_t xQueueUnLockQueue(Queue_t *queue_);


  /**
   * @brief Create a byte-oriented stream buffer for inter-task communication
   *
   * Creates a new stream buffer for byte-by-byte data transfer between tasks.
   * Stream buffers provide a lightweight alternative to message queues when
   * communicating variable-length byte sequences or continuous data streams.
   * Unlike queues which handle discrete messages, streams accept individual
   * bytes that accumulate in a circular buffer until consumed by a receiver.
   *
   * Stream buffers are ideal for:
   * - **Serial/UART data**: Buffering incoming bytes from serial ports
   * - **Protocol parsing**: Accumulating bytes until a complete frame is ready
   * - **Sensor streaming**: Continuous data collection from sensors
   * - **Character-based I/O**: Terminal or text-based interfaces
   * - **Variable-length data**: When message size isn't known in advance
   *
   * Key characteristics:
   * - Fixed capacity: CONFIG_STREAM_BUFFER_BYTES (default 32 bytes)
   * - FIFO ordering: Bytes are retrieved in the order they were sent
   * - Byte-level granularity: Send and receive one byte at a time
   * - No message boundaries: Unlike queues, streams don't preserve message
   * structure
   * - Lightweight: Lower overhead than message queues for byte-oriented data
   *
   * Stream Lifecycle:
   * 1. Create stream with xStreamCreate() (allocates kernel resources) 2.
   * Producer tasks send bytes with xStreamSend() 3. Consumer tasks receive
   * bytes with xStreamReceive() 4. Optionally query status with
   * xStreamBytesAvailable(), xStreamIsEmpty(), xStreamIsFull() 5. Optionally
   * clear stream with xStreamReset() 6. Delete stream with xStreamDelete() when
   * no longer needed
   *
   * Example 1: UART receive buffer
   * @code StreamBuffer_t *uartRxStream;
   *
   * // Create stream during initialization if
   * (OK(xStreamCreate(&uartRxStream))) {
   *   // Stream ready for use
   * }
   *
   * // ISR or receive task sends bytes as they arrive void uartISR(void) {
   *   Byte_t rxByte = UART_DATA_REG;
   *   xStreamSend(uartRxStream, rxByte);
   * }
   *
   * // Processing task receives accumulated data void processUART(Task_t *task,
   * TaskParm_t *parm) {
   *   HalfWord_t count;
   *   Byte_t *data;
   *
   *   if (OK(xStreamReceive(uartRxStream, &count, &data))) {
   *     if (count > 0) {
   *       processReceivedData(data, count);
   *       xMemFree((Addr_t *)data);  // Always free received data
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Protocol parser with framing
   * @code StreamBuffer_t *protocolStream;
   *
   * void setupProtocol(void) {
   *   xStreamCreate(&protocolStream);
   * }
   *
   * // Producer accumulates bytes void receiveTask(Task_t *task, TaskParm_t
   * parm)
   * {
   *   Byte_t receivedByte = getByteFromSource();
   *   xStreamSend(protocolStream, receivedByte);
   *
   *   // Check if frame delimiter received if (receivedByte ==
   * FRAME_END_MARKER) {
   *     xTaskNotifyGive(parserTask);  // Signal parser
   *   }
   * }
   *
   * // Consumer processes complete frames void parserTask(Task_t *task,
   * TaskParm_t *
   * parm) {
   *   HalfWord_t frameSize;
   *   Byte_t *frame;
   *
   *   xTaskWait(1);  // Wait for notification
   *
   *   if (OK(xStreamReceive(protocolStream, &frameSize, &frame))) {
   *     parseProtocolFrame(frame, frameSize);
   *     xMemFree((Addr_t *)frame);
   *   }
   * }
   * @endcode
   *
   * Example 3: Sensor data streaming
   * @code StreamBuffer_t *sensorStream;
   *
   * void initSensors(void) {
   *   xStreamCreate(&sensorStream);
   * }
   *
   * // High-frequency sensor sampling void sampleSensor(Task_t *task,
   * TaskParm_t *
   * parm) {
   *   Byte_t sample = readADC();
   *
   *   Base_t isFull;
   *   if (OK(xStreamIsFull(sensorStream, &isFull)) && isFull) {
   *     // Stream full - handle overflow xStreamReset(sensorStream);  //
   * Discard old data
   *   }
   *
   *   xStreamSend(sensorStream, sample);
   * }
   *
   * // Lower-frequency processing void processSamples(Task_t *task, TaskParm_t
   * *
   * parm) {
   *   HalfWord_t sampleCount;
   *   Byte_t *samples;
   *
   *   if (OK(xStreamReceive(sensorStream, &sampleCount, &samples))) {
   *     if (sampleCount >= MIN_SAMPLES_FOR_PROCESSING) {
   *       computeStatistics(samples, sampleCount);
   *     }
   *     xMemFree((Addr_t *)samples);
   *   }
   * }
   * @endcode
   *
   * @param[out] stream_ Pointer to StreamBuffer_t *handle to be initialized.
   *                     After successful creation, this handle is used in all
   *                     subsequent stream operations. The handle remains valid
   *                     until xStreamDelete() is called.
   *
   * @return             ReturnOK if stream created successfully, ReturnError if
   *                     creation failed due to insufficient memory or invalid
   *                     parameters.
   *
   * @warning Stream buffers have a fixed capacity defined by
   * CONFIG_STREAM_BUFFER_BYTES (typically 32 bytes). Sending to a full stream
   * will fail with ReturnError. Use xStreamIsFull() to check capacity before
   * sending critical data.
   *
   * @warning The stream_ parameter must point to valid memory. Passing null
   * will result in ReturnError.
   *
   * @note Stream buffers are allocated from kernel memory and persist until
   * explicitly deleted with xStreamDelete(). Always delete streams when no
   * longer needed to prevent memory leaks.
   *
   * @note Unlike message queues, streams do not preserve message boundaries. If
   * you send bytes "ABC" and "DEF" separately, the receiver sees "ABCDEF" as a
   * continuous byte sequence. Implement your own framing if you need to
   * distinguish between separate transmissions.
   *
   * @note Streams operate on individual bytes (Byte_t/Byte_t). To send
   * multi-byte values, send each byte separately or use a message queue
   * instead.
   *
   * @sa xStreamDelete() - Delete stream and free resources
   * @sa xStreamSend() - Send byte to stream
   * @sa xStreamReceive() - Receive all waiting bytes from stream
   * @sa xStreamBytesAvailable() - Query number of bytes waiting in stream
   * @sa xStreamReset() - Clear all bytes from stream
   * @sa xStreamIsEmpty() - Check if stream has no waiting bytes
   * @sa xStreamIsFull() - Check if stream is at capacity
   * @sa xQueueCreate() - Alternative for message-oriented communication
   */
  Return_t xStreamCreate(StreamBuffer_t **stream_);


  /**
   * @brief Delete a stream buffer and free its resources
   *
   * Deletes a stream buffer created by xStreamCreate(), freeing all associated
   * kernel memory and resources. After deletion, the stream handle becomes
   * invalid and must not be used in any subsequent stream operations. Any bytes
   * waiting in the stream at the time of deletion are discarded.
   *
   * This function should be called when a stream buffer is no longer needed to
   * prevent memory leaks in long-running embedded systems. Proper resource
   * cleanup is essential for system reliability, especially in applications
   * that dynamically create and destroy communication channels.
   *
   * Deletion scenarios:
   * - **Application shutdown**: Clean up resources during de-initialization
   * - **Dynamic reconfiguration**: Remove old streams when changing
   * communication topology
   * - **Error recovery**: Delete and recreate streams after communication
   * failures
   * - **Resource management**: Free streams in resource-constrained systems
   *
   * Example 1: Basic stream cleanup
   * @code StreamBuffer_t *tempStream;
   *
   * // Create stream for temporary operation if
   * (OK(xStreamCreate(&tempStream))) {
   *   // Use stream for data transfer xStreamSend(tempStream, 0x42);
   *   // ... perform operations ...
   *
   *   // Clean up when done xStreamDelete(tempStream);
   * }
   * @endcode
   *
   * Example 2: Communication channel lifecycle
   * @code StreamBuffer_t *channelStream = null;
   *
   * void openChannel(void) {
   *   if (OK(xStreamCreate(&channelStream))) {
   *     // Channel ready for use
   *   }
   * }
   *
   * void closeChannel(void) {
   *   if (channelStream != null) {
   *     xStreamDelete(channelStream);
   *     channelStream = null;  // Prevent use-after-delete
   *   }
   * }
   * @endcode
   *
   * Example 3: Error recovery with stream recreation
   * @code StreamBuffer_t *dataStream;
   *
   * void setupDataStream(void) {
   *   xStreamCreate(&dataStream);
   * }
   *
   * void resetCommunication(void) {
   *   // Delete corrupted stream xStreamDelete(dataStream);
   *
   *   // Wait briefly for cleanup delayMs(10);
   *
   *   // Create fresh stream if (OK(xStreamCreate(&dataStream))) {
   *     // Stream reset successful
   *   }
   * }
   * @endcode
   *
   * @param[in] stream_ Handle to the stream buffer to delete. Must be a valid
   *                    handle obtained from a previous successful call to
   *                    xStreamCreate(). After deletion, this handle becomes
   *                    invalid.
   *
   * @return            ReturnOK if stream deleted successfully, ReturnError if
   *                    deletion failed due to invalid handle or stream not
   *                    found.
   *
   * @warning After calling xStreamDelete(), the stream handle becomes invalid
   * and must not be used in any subsequent stream operations. Attempting to use
   * a deleted stream will result in ReturnError.
   *
   * @warning If other tasks hold references to the stream buffer, ensure they
   * stop using the stream before deletion. Deleting a stream while other tasks
   * are actively sending or receiving data may cause those operations to fail
   * with ReturnError.
   *
   * @warning Any bytes waiting in the stream buffer at the time of deletion are
   * permanently lost. If you need to preserve data, call xStreamReceive() to
   * retrieve all pending bytes before deleting the stream.
   *
   * @note It is good practice to set the stream handle to null after deletion
   * to prevent accidental use of an invalid handle (use-after-delete bugs).
   *
   * @note xStreamDelete() only affects the stream buffer itself. Any data
   * previously received with xStreamReceive() and stored in heap memory remains
   * allocated until explicitly freed with xMemFree().
   *
   * @sa xStreamCreate() - Create a new stream buffer
   * @sa xStreamReset() - Clear stream contents without deleting the stream
   * @sa xStreamReceive() - Retrieve pending bytes before deletion
   * @sa xMemFree() - Free memory allocated by xStreamReceive()
   */
  Return_t xStreamDelete(const StreamBuffer_t *stream_);


  /**
   * @brief Send a single byte to a stream buffer (producer operation)
   *
   * Sends one byte to the specified stream buffer, adding it to the end of the
   * FIFO byte sequence. This is the producer-side operation for stream-based
   * inter-task communication. Bytes are stored in the stream until consumed by
   * a receiver task using xStreamReceive().
   *
   * Stream buffers provide byte-level granularity for communication, making
   * them ideal for character-oriented protocols, serial data buffering, and
   * continuous data streaming. Unlike message queues, streams don't preserve
   * message boundaries—
   * bytes are retrieved in the exact order they were sent, but without any
   * structure or framing.
   *
   * Operational characteristics:
   * - **FIFO ordering**: Bytes are retrieved in the order they were sent
   * - **Fixed capacity**: Stream holds up to CONFIG_STREAM_BUFFER_BYTES
   * (default 32)
   * - **Blocking behavior**: Returns ReturnError immediately if stream is full
   * - **Atomic operation**: Single byte send is atomic (safe from interrupts)
   * - **Low overhead**: Minimal processing compared to message queue operations
   *
   * Common usage patterns:
   * - **Serial/UART buffering**: Send received bytes from ISR to stream for
   * processing
   * - **Character output**: Build strings character-by-character for display
   * - **Protocol assembly**: Accumulate protocol bytes until complete frame
   * ready
   * - **Sensor sampling**: Stream continuous ADC or sensor readings
   * - **Event logging**: Record byte-oriented event codes or timestamps
   *
   * Example 1: UART transmit buffering
   * @code StreamBuffer_t *uartTxStream;
   *
   * void setupUART(void) {
   *   xStreamCreate(&uartTxStream);
   * }
   *
   * // Application sends string to UART void sendString(const char *str) {
   *   while (*str) {
   *     if (ERROR(xStreamSend(uartTxStream, (Byte_t)*str))) {
   *       // Stream full - wait for transmitter to drain delayMs(1);
   *     } else {
   *       str++;
   *     }
   *   }
   * }
   *
   * // UART task transmits buffered bytes void uartTransmitTask(Task_t *task,
   * TaskParm_t *parm) {
   *   HalfWord_t count;
   *   Byte_t *data;
   *
   *   if (OK(xStreamReceive(uartTxStream, &count, &data))) {
   *     if (count > 0) {
   *       uartWriteBytes(data, count);
   *       xMemFree((Addr_t *)data);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Protocol frame assembly
   * @code StreamBuffer_t *protocolStream;
   *
   * // Send protocol header, payload, and checksum Return_t
   * sendProtocolFrame(Byte_t command, Byte_t *payload, HalfWord_t len) {
   *   // Send start byte if (ERROR(xStreamSend(protocolStream, 0xAA))) {
   *     return ReturnError;
   *   }
   *
   *   // Send command if (ERROR(xStreamSend(protocolStream, command))) {
   *     return ReturnError;
   *   }
   *
   *   // Send length if (ERROR(xStreamSend(protocolStream, (Byte_t)len))) {
   *     return ReturnError;
   *   }
   *
   *   // Send payload bytes for (HalfWord_t i = 0; i < len; i++) {
   *     if (ERROR(xStreamSend(protocolStream, payload[i]))) {
   *       return ReturnError;
   *     }
   *   }
   *
   *   // Send checksum Byte_t checksum = calculateChecksum(command, payload,
   * len);
   *   return xStreamSend(protocolStream, checksum);
   * }
   * @endcode
   *
   * Example 3: Sensor data streaming with overflow handling
   * @code StreamBuffer_t *sensorStream;
   *
   * void sampleSensorTask(Task_t *task, TaskParm_t *parm) {
   *   Byte_t sample = readADC();
   *
   *   // Try to send sample if (ERROR(xStreamSend(sensorStream, sample))) {
   *     // Stream full - check how to handle overflow Base_t isFull;
   *     if (OK(xStreamIsFull(sensorStream, &isFull)) && isFull) {
   *       // Option 1: Drop oldest data and reset xStreamReset(sensorStream);
   *       xStreamSend(sensorStream, sample);
   *
   *       // Option 2: Notify error handler
   *       // logOverflowError();
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in] stream_ Handle to the stream buffer to send data to. Must be a
   *                    valid stream created with xStreamCreate().
   * @param[in] byte_   The byte value to send to the stream. Accepts any value
   *                    from 0x00 to 0xFF.
   *
   * @return            ReturnOK if byte sent successfully, ReturnError if send
   *                    failed due to stream full, invalid stream handle, or
   *                    stream not found.
   *
   * @warning If the stream is full (contains CONFIG_STREAM_BUFFER_BYTES bytes),
   * xStreamSend() returns ReturnError immediately without blocking. Check
   * stream capacity with xStreamIsFull() or xStreamBytesAvailable() before
   * sending critical data, or implement error handling for full conditions.
   *
   * @warning Sending to an invalid or deleted stream handle will return
   * ReturnError. Always verify stream creation succeeded before calling
   * xStreamSend().
   *
   * @note xStreamSend() operates on individual bytes only. To send multi-byte
   * values (integers, floats, structures), send each byte separately in the
   * appropriate order (considering endianness if necessary).
   *
   * @note Stream buffers do not preserve message boundaries. Bytes sent in
   * separate xStreamSend() calls are concatenated into a continuous sequence.
   * If you need message framing, implement your own protocol with
   * headers/delimiters.
   *
   * @note For high-throughput applications, consider checking xStreamIsFull()
   * before sending large amounts of data to avoid repeated ReturnError
   * conditions.
   *
   * @sa xStreamReceive() - Receive all waiting bytes from stream (consumer
   * operation)
   * @sa xStreamCreate() - Create a stream buffer
   * @sa xStreamBytesAvailable() - Query number of bytes waiting in stream
   * @sa xStreamIsFull() - Check if stream is at capacity
   * @sa xStreamReset() - Clear all bytes from stream
   * @sa xQueueSend() - Alternative for message-oriented communication
   */
  Return_t xStreamSend(StreamBuffer_t *stream_, const Byte_t byte_);


  /**
   * @brief Receive all waiting bytes from a stream buffer (consumer operation)
   *
   * Retrieves all bytes currently waiting in the specified stream buffer and
   * returns them in a newly allocated memory buffer. This is the consumer-side
   * operation for stream-based inter-task communication. After retrieval, the
   * bytes are removed from the stream, freeing space for new data.
   *
   * xStreamReceive() allocates memory from the user heap to hold the retrieved
   * bytes. The caller is responsible for freeing this memory with xMemFree()
   * after processing the data. This memory management pattern ensures safe data
   * transfer between tasks without buffer ownership conflicts.
   *
   * Key characteristics:
   * - **Retrieves all waiting bytes**: Returns complete contents of stream in
   * one call
   * - **Allocates memory**: Creates new buffer in user heap for received data
   * - **Clears stream**: Removes received bytes from stream buffer
   * - **Returns byte count**: Indicates number of bytes retrieved via bytes_
   * parameter
   * - **Non-blocking**: Returns immediately even if stream is empty (bytes_ =
   * 0)
   * - **FIFO ordering**: Bytes returned in the order they were sent
   *
   * Typical consumer workflow:
   * 1. Call xStreamReceive() to get all waiting bytes 2. Check if any bytes
   * were retrieved (bytes_ > 0) 3. Process the received data 4. Free the
   * allocated buffer with xMemFree() 5. Repeat as needed for continuous data
   * flow
   *
   * Example 1: UART receive processing
   * @code StreamBuffer_t *uartRxStream;
   *
   * // Producer (ISR or task) sends bytes to stream void uartISR(void) {
   *   if (UART_RX_READY) {
   *     Byte_t rxByte = UART_DATA_REG;
   *     xStreamSend(uartRxStream, rxByte);
   *   }
   * }
   *
   * // Consumer task processes received data void processUARTTask(Task_t *task,
   * TaskParm_t *parm) {
   *   HalfWord_t byteCount;
   *   Byte_t *rxData;
   *
   *   if (OK(xStreamReceive(uartRxStream, &byteCount, &rxData))) {
   *     if (byteCount > 0) {
   *       // Process received bytes for (HalfWord_t i = 0; i < byteCount; i++)
   * {
   *         processCharacter(rxData[i]);
   *       }
   *
   *       // Always free allocated memory xMemFree((Addr_t *)rxData);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Protocol frame parsing
   * @code StreamBuffer_t *protocolStream;
   *
   * void parseProtocolTask(Task_t *task, TaskParm_t *parm) {
   *   HalfWord_t frameSize;
   *   Byte_t *frame;
   *
   *   if (OK(xStreamReceive(protocolStream, &frameSize, &frame))) {
   *     if (frameSize > 0) {
   *       // Look for start byte (0xAA) for (HalfWord_t i = 0; i < frameSize;
   * i++) {
   *         if (frame[i] == 0xAA && (i + 3) < frameSize) {
   *           Byte_t command = frame[i + 1];
   *           Byte_t length = frame[i + 2];
   *
   *           // Verify complete frame available if ((i + 3 + length) <
   * frameSize) {
   *             Byte_t *payload = &frame[i + 3];
   *             processCommand(command, payload, length);
   *             i += 3 + length;  // Skip past this frame
   *           }
   *         }
   *       }
   *
   *       xMemFree((Addr_t *)frame);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Sensor data batch processing
   * @code StreamBuffer_t *sensorStream;
   *
   * #define MIN_SAMPLES 10
   *
   * void analyzeSensorTask(Task_t *task, TaskParm_t *parm) {
   *   HalfWord_t sampleCount;
   *   Byte_t *samples;
   *
   *   if (OK(xStreamReceive(sensorStream, &sampleCount, &samples))) {
   *     // Only process if we have enough samples if (sampleCount >=
   * MIN_SAMPLES) {
   *       // Calculate statistics Word_t sum = 0;
   *       Byte_t min = 255, max = 0;
   *
   *       for (HalfWord_t i = 0; i < sampleCount; i++) {
   *         sum += samples[i];
   *         if (samples[i] < min) min = samples[i];
   *         if (samples[i] > max) max = samples[i];
   *       }
   *
   *       Byte_t average = (Byte_t)(sum / sampleCount);
   *       reportStatistics(average, min, max);
   *     } else if (sampleCount > 0) {
   *       // Put samples back by re-sending them for (HalfWord_t i = 0; i <
   * sampleCount; i++) {
   *         xStreamSend(sensorStream, samples[i]);
   *       }
   *     }
   *
   *     // Always free even if we didn't process if (sampleCount > 0) {
   *       xMemFree((Addr_t *)samples);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 4: Complete producer-consumer pattern
   * @code StreamBuffer_t *dataStream;
   * Task_t *producerTask, consumerTask;
   *
   * // Producer accumulates data void producer(Task_t *task, TaskParm_t *parm)
   * {
   *   Byte_t dataPoint = collectData();
   *
   *   if (OK(xStreamSend(dataStream, dataPoint))) {
   *     // Check if buffer is getting full HalfWord_t available;
   *     if (OK(xStreamBytesAvailable(dataStream, &available))) {
   *       if (available >= 16) {
   *         // Signal consumer to process data xTaskNotifyGive(consumerTask);
   *       }
   *     }
   *   }
   * }
   *
   * // Consumer processes batches void consumer(Task_t *task, TaskParm_t *parm)
   * {
   *   xTaskWait(1);  // Wait for notification from producer
   *
   *   HalfWord_t count;
   *   Byte_t *data;
   *
   *   if (OK(xStreamReceive(dataStream, &count, &data))) {
   *     if (count > 0) {
   *       processBatch(data, count);
   *       xMemFree((Addr_t *)data);
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  stream_ Handle to the stream buffer to receive from. Must be a
   *                     valid stream created with xStreamCreate().
   * @param[out] bytes_  Pointer to variable receiving the number of bytes
   *                     retrieved. Set to 0 if stream is empty. The caller
   *                     should check this value before processing data_.
   * @param[out] data_   Pointer to variable receiving address of newly
   *                     allocated buffer containing the retrieved bytes. Memory
   *                     is allocated from user heap and must be freed with
   *                     xMemFree() after use. If no bytes are available, this
   *                     may point to null or uninitialized memory.
   *
   * @return             ReturnOK if receive operation completed (even if 0
   *                     bytes received), ReturnError if operation failed due to
   *                     invalid stream handle, memory allocation failure, or
   *                     stream not found.
   *
   * @warning The memory allocated for data_ MUST be freed by the caller using
   * xMemFree() after processing. Failure to free this memory will cause a
   * memory leak. Always pair xStreamReceive() with xMemFree() in your code.
   *
   * @warning Always check bytes_ before accessing data_. If bytes_ is 0, the
   * stream was empty and data_ should not be accessed.
   *
   * @warning xStreamReceive() retrieves ALL waiting bytes in a single call. If
   * you need to process bytes one at a time or in smaller chunks, you must
   * implement your own buffering or use the retrieved data array with indexing.
   *
   * @warning Receiving from an invalid or deleted stream handle will return
   * ReturnError.
   *
   * @note xStreamReceive() is non-blocking. If the stream is empty, it returns
   * immediately with bytes_ set to 0. This differs from some RTOS queue
   * implementations that support blocking receives with timeouts.
   *
   * @note After xStreamReceive() completes, the retrieved bytes are removed
   * from the stream buffer, freeing space for new data from producers.
   *
   * @note Memory allocation failures are rare but possible in heap-constrained
   * systems. If xStreamReceive() returns ReturnError and bytes_ indicates data
   * was available, suspect memory allocation failure and consider increasing
   * heap size.
   *
   * @sa xStreamSend() - Send bytes to stream (producer operation)
   * @sa xMemFree() - Free memory allocated by xStreamReceive()
   * @sa xStreamCreate() - Create a stream buffer
   * @sa xStreamBytesAvailable() - Query number of bytes waiting without
   * receiving
   * @sa xStreamIsEmpty() - Check if stream has any waiting bytes
   * @sa xStreamReset() - Clear stream without retrieving bytes
   * @sa xQueueReceive() - Alternative for message-oriented communication
   */
  Return_t xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_, Byte_t **data_);


  /**
   * @brief Query the number of bytes waiting in a stream buffer
   *
   * Returns the count of bytes currently available for retrieval in the
   * specified stream buffer without removing them. This non-destructive query
   * allows tasks to check data availability before committing to receive
   * operations, enabling more sophisticated flow control and buffering
   * strategies.
   *
   * Unlike xStreamReceive() which retrieves and removes bytes from the stream,
   * xStreamBytesAvailable() is a read-only query that leaves the stream
   * contents unchanged. This makes it useful for making decisions about when to
   * process data, whether to wait for more data, or how to handle buffer
   * capacity.
   *
   * Common use cases:
   * - **Threshold-based processing**: Wait until minimum amount of data
   * available
   * - **Flow control**: Monitor buffer levels to regulate producer rate
   * - **Batch optimization**: Collect data until optimal batch size reached
   * - **Buffer monitoring**: Check capacity before sending more data
   * - **Conditional receive**: Decide whether to retrieve data based on
   * quantity
   *
   * Example 1: Threshold-based data processing
   * @code StreamBuffer_t *dataStream;
   *
   * #define MIN_DATA_SIZE 16
   *
   * void processDataTask(Task_t *task, TaskParm_t *parm) {
   *   HalfWord_t available;
   *
   *   // Check if enough data accumulated if
   * (OK(xStreamBytesAvailable(dataStream, &available))) {
   *     if (available >= MIN_DATA_SIZE) {
   *       // Enough data - retrieve and process HalfWord_t count;
   *       Byte_t *data;
   *       if (OK(xStreamReceive(dataStream, &count, &data))) {
   *         processBatch(data, count);
   *         xMemFree((Addr_t *)data);
   *       }
   *     }
   *     // Otherwise wait for more data
   *   }
   * }
   * @endcode
   *
   * Example 2: Producer flow control
   * @code StreamBuffer_t *txStream;
   *
   * #define HIGH_WATER_MARK 28
   * #define LOW_WATER_MARK 8
   *
   * void sendDataTask(Task_t *task, TaskParm_t *parm) {
   *   static Base_t throttled = 0;
   *   HalfWord_t buffered;
   *
   *   if (OK(xStreamBytesAvailable(txStream, &buffered))) {
   *     // Implement hysteresis for flow control if (buffered >=
   * HIGH_WATER_MARK) {
   *       throttled = 1;  // Stop sending
   *     } else if (buffered <= LOW_WATER_MARK) {
   *       throttled = 0;  // Resume sending
   *     }
   *
   *     if (!throttled) {
   *       Byte_t data = getNextByte();
   *       xStreamSend(txStream, data);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Capacity checking before bulk send
   * @code StreamBuffer_t *protocolStream;
   *
   * Return_t sendFrame(Byte_t *frame, HalfWord_t frameLen) {
   *   HalfWord_t available;
   *   HalfWord_t capacity;
   *
   *   // Check current buffer usage if
   * (OK(xStreamBytesAvailable(protocolStream, &available))) {
   *     capacity = CONFIG_STREAM_BUFFER_BYTES - available;
   *
   *     // Verify enough space for entire frame if (capacity >= frameLen) {
   *       // Send all bytes for (HalfWord_t i = 0; i < frameLen; i++) {
   *         if (ERROR(xStreamSend(protocolStream, frame[i]))) {
   *           return ReturnError;  // Unexpected failure
   *         }
   *       }
   *       return ReturnOK;
   *     } else {
   *       // Not enough space - return error or wait return ReturnError;
   *     }
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 4: Monitoring with status reporting
   * @code StreamBuffer_t *sensorStream;
   *
   * void monitorTask(Task_t *task, TaskParm_t *parm) {
   *   static HalfWord_t maxObserved = 0;
   *   HalfWord_t current;
   *
   *   if (OK(xStreamBytesAvailable(sensorStream, &current))) {
   *     // Track high-water mark if (current > maxObserved) {
   *       maxObserved = current;
   *     }
   *
   *     // Calculate utilization percentage Byte_t utilization =
   * (Byte_t)((current * 100) / CONFIG_STREAM_BUFFER_BYTES);
   *
   *     // Report if approaching capacity if (utilization > 90) {
   *       logWarning("Stream buffer near capacity", utilization);
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  stream_ Handle to the stream buffer to query. Must be a valid
   *                     stream created with xStreamCreate().
   * @param[out] bytes_  Pointer to variable receiving the byte count. On
   *                     success, contains the number of bytes currently waiting
   *                     in the stream (0 to CONFIG_STREAM_BUFFER_BYTES).
   *
   * @return             ReturnOK if query succeeded, ReturnError if query
   *                     failed due to invalid stream handle or stream not
   *                     found.
   *
   * @warning This function only reports the current state of the stream buffer.
   * In multitasking environments, the byte count may change immediately after
   * the query if other tasks send or receive data. Use appropriate
   * synchronization if you need atomic check-and-receive operations.
   *
   * @note xStreamBytesAvailable() is a non-destructive query—it does not modify
   * the stream contents or affect subsequent operations.
   *
   * @note The returned byte count includes all bytes sent with xStreamSend()
   * that have not yet been retrieved with xStreamReceive().
   *
   * @note To calculate free space in the stream, subtract bytes_ from
   * CONFIG_STREAM_BUFFER_BYTES: freeSpace = CONFIG_STREAM_BUFFER_BYTES - bytes_
   *
   * @sa xStreamReceive() - Retrieve and remove bytes from stream
   * @sa xStreamSend() - Send byte to stream
   * @sa xStreamIsEmpty() - Check if stream has no waiting bytes
   * @sa xStreamIsFull() - Check if stream is at capacity
   * @sa xStreamReset() - Clear all bytes from stream
   * @sa xStreamCreate() - Create a stream buffer
   */
  Return_t xStreamBytesAvailable(const StreamBuffer_t *stream_, HalfWord_t *bytes_);


  /**
   * @brief Clear all bytes from a stream buffer
   *
   * Resets the specified stream buffer to an empty state by discarding all
   * waiting bytes. After reset, the stream behaves as if newly created—
   * xStreamBytesAvailable() returns 0 and xStreamIsEmpty() returns true. This
   * operation is useful for error recovery, protocol resyncs, or discarding
   * stale data.
   *
   * Unlike xStreamDelete() which destroys the stream entirely, xStreamReset()
   * preserves the stream structure and handle while only clearing its contents.
   * The stream remains fully functional and can immediately accept new data via
   * xStreamSend().
   *
   * Common scenarios for stream reset:
   * - **Error recovery**: Clear corrupted or incomplete protocol data
   * - **Protocol resync**: Discard partial frames when reestablishing sync
   * - **Buffer overflow handling**: Clear old data when capacity exceeded
   * - **Timeout handling**: Discard stale data after communication timeout
   * - **State machine reset**: Clear buffered data when returning to idle state
   * - **Channel cleanup**: Prepare stream for reuse without deallocating
   *
   * Example 1: Protocol error recovery
   * @code StreamBuffer_t *protocolStream;
   *
   * void handleProtocolError(void) {
   *   // Error detected - discard partial/corrupted data
   * xStreamReset(protocolStream);
   *
   *   // Send resync request to peer sendResyncCommand();
   *
   *   // Stream now empty and ready for fresh data
   * }
   * @endcode
   *
   * Example 2: Timeout-based data discard
   * @code StreamBuffer_t *rxStream;
   * Timer_t *rxTimeout;
   *
   * void receiveTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t timerExpired;
   *
   *   // Check for receive timeout if (OK(xTimerHasTimerExpired(rxTimeout,
   * &timerExpired)) && timerExpired) {
   *     HalfWord_t staleBytes;
   *
   *     // Check if incomplete data waiting if
   * (OK(xStreamBytesAvailable(rxStream, &staleBytes)) && staleBytes > 0) {
   *       // Data incomplete after timeout - discard it xStreamReset(rxStream);
   *       logWarning("Receive timeout - data discarded");
   *     }
   *
   *     // Reset timeout for next receive xTimerReset(rxTimeout);
   *   }
   * }
   * @endcode
   *
   * Example 3: Overflow handling with reset
   * @code StreamBuffer_t *sensorStream;
   *
   * void sampleSensor(Task_t *task, TaskParm_t *parm) {
   *   Byte_t sample = readADC();
   *   Base_t isFull;
   *
   *   // Check if buffer full if (OK(xStreamIsFull(sensorStream, &isFull)) &&
   * isFull) {
   *     // Option 1: Drop oldest data, keep newest xStreamReset(sensorStream);
   *     xStreamSend(sensorStream, sample);
   *
   *     // Log overflow event overflowCount++;
   *   } else {
   *     // Normal send xStreamSend(sensorStream, sample);
   *   }
   * }
   * @endcode
   *
   * Example 4: State machine with stream cleanup
   * @code typedef enum {
   *   STATE_IDLE, STATE_RECEIVING, STATE_PROCESSING
   * } CommState_t;
   *
   * StreamBuffer_t *commStream;
   * CommState_t commState = STATE_IDLE;
   *
   * void communicationTask(Task_t *task, TaskParm_t *parm) {
   *   switch (commState) {
   *     case STATE_IDLE:
   *       // Ensure stream clean when entering new transaction
   * xStreamReset(commStream);
   *       commState = STATE_RECEIVING;
   *       break;
   *
   *     case STATE_RECEIVING:
   *       // Accumulate data...
   *       if (frameComplete()) {
   *         commState = STATE_PROCESSING;
   *       }
   *       break;
   *
   *     case STATE_PROCESSING:
   *       // Process received frame processFrame();
   *       commState = STATE_IDLE;  // Will clear on next iteration break;
   *   }
   * }
   * @endcode
   *
   * @param[in] stream_ Handle to the stream buffer to reset. Must be a valid
   *                    stream created with xStreamCreate().
   *
   * @return            ReturnOK if stream reset successfully, ReturnError if
   *                    reset failed due to invalid stream handle or stream not
   *                    found.
   *
   * @warning All bytes waiting in the stream are permanently discarded. If you
   * need to preserve data, call xStreamReceive() before calling xStreamReset().
   *
   * @warning In multitasking environments, ensure no other task is actively
   * sending to or receiving from the stream during reset. Resetting a stream
   * while another task is mid-operation may lead to unexpected behavior or data
   * loss.
   *
   * @note xStreamReset() does not delete or invalidate the stream handle. After
   * reset, the stream remains fully functional and ready for new send/receive
   * operations.
   *
   * @note This operation is typically faster than deleting and recreating a
   * stream, making it preferable for error recovery scenarios where the stream
   * structure should be reused.
   *
   * @note After reset, xStreamBytesAvailable() returns 0, xStreamIsEmpty()
   * returns true, and xStreamIsFull() returns false.
   *
   * @sa xStreamCreate() - Create a stream buffer
   * @sa xStreamDelete() - Delete stream entirely (vs. just clearing contents)
   * @sa xStreamReceive() - Retrieve data before reset if needed
   * @sa xStreamBytesAvailable() - Check byte count before reset
   * @sa xStreamIsEmpty() - Verify stream empty after reset
   */
  Return_t xStreamReset(const StreamBuffer_t *stream_);


  /**
   * @brief Check if a stream buffer contains no waiting bytes
   *
   * Queries whether the specified stream buffer is empty (contains zero bytes).
   * This is a boolean status check that provides a simple, readable way to test
   * for data availability without needing to interpret byte counts. An empty
   * stream has no data waiting to be retrieved.
   *
   * This function is logically equivalent to checking if
   * xStreamBytesAvailable() returns 0, but offers more expressive code when you
   * only need a boolean empty/not-empty status rather than the exact byte
   * count.
   *
   * Common use cases:
   * - **Conditional processing**: Skip receive operations when no data
   * available
   * - **Stream state verification**: Confirm stream cleared after reset
   * - **Idle detection**: Determine if communication channel is idle
   * - **Polling optimization**: Avoid unnecessary memory allocations for empty
   * receives
   *
   * Example 1: Conditional receive
   * @code StreamBuffer_t *dataStream;
   *
   * void processTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t isEmpty;
   *
   *   // Check before attempting receive if (OK(xStreamIsEmpty(dataStream,
   * &isEmpty)) && !isEmpty) {
   *     // Data available - retrieve and process HalfWord_t count;
   *     Byte_t *data;
   *     if (OK(xStreamReceive(dataStream, &count, &data))) {
   *       processData(data, count);
   *       xMemFree((Addr_t *)data);
   *     }
   *   }
   *   // Otherwise skip processing this cycle
   * }
   * @endcode
   *
   * Example 2: Verify reset completion
   * @code StreamBuffer_t *protocolStream;
   *
   * void resetProtocol(void) {
   *   xStreamReset(protocolStream);
   *
   *   // Verify stream actually empty Base_t isEmpty;
   *   if (OK(xStreamIsEmpty(protocolStream, &isEmpty))) {
   *     if (isEmpty) {
   *       // Reset confirmed - ready for new data protocolState =
   * PROTOCOL_IDLE;
   *     } else {
   *       // Unexpected - reset failed?
   *       handleError();
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Communication idle detection
   * @code StreamBuffer_t *rxStream;
   * Timer_t *idleTimer;
   *
   * void monitorActivity(Task_t *task, TaskParm_t *parm) {
   *   Base_t isEmpty;
   *   Base_t timerExpired;
   *
   *   if (OK(xStreamIsEmpty(rxStream, &isEmpty)) && isEmpty) {
   *     // No data waiting - check how long idle if
   * (OK(xTimerHasTimerExpired(idleTimer, &timerExpired)) && timerExpired) {
   *       // Idle timeout - enter power-saving mode enterLowPowerMode();
   *     }
   *   } else {
   *     // Activity detected - reset idle timer xTimerReset(idleTimer);
   *   }
   * }
   * @endcode
   *
   * @param[in]  stream_ Handle to the stream buffer to query. Must be a valid
   *                     stream created with xStreamCreate().
   * @param[out] res_    Pointer to variable receiving the empty status. Set to
   *                     non-zero (true) if stream is empty (0 bytes waiting),
   *                     zero (false) if stream contains data.
   *
   * @return             ReturnOK if query succeeded, ReturnError if query
   *                     failed due to invalid stream handle or stream not
   *                     found.
   *
   * @note This is a non-destructive query that does not modify stream contents.
   *
   * @note For more detailed information about stream state, use
   * xStreamBytesAvailable() to get the exact byte count instead of just
   * empty/not-empty status.
   *
   * @sa xStreamBytesAvailable() - Get exact byte count
   * @sa xStreamIsFull() - Check if stream is at capacity
   * @sa xStreamReceive() - Retrieve bytes from stream
   * @sa xStreamReset() - Clear stream to empty state
   * @sa xStreamCreate() - Create a stream buffer
   */
  Return_t xStreamIsEmpty(const StreamBuffer_t *stream_, Base_t *res_);


  /**
   * @brief Check if a stream buffer is at full capacity
   *
   * Queries whether the specified stream buffer is full (contains the maximum
   * number of bytes defined by CONFIG_STREAM_BUFFER_BYTES, typically 32). This
   * boolean status check provides a simple way to test for buffer saturation
   * before attempting send operations or to implement overflow handling
   * strategies.
   *
   * A full stream cannot accept additional bytes via xStreamSend() until space
   * is freed by xStreamReceive() or xStreamReset(). This function is logically
   * equivalent to checking if xStreamBytesAvailable() equals
   * CONFIG_STREAM_BUFFER_BYTES, but offers more expressive code for
   * capacity-related logic.
   *
   * Common use cases:
   * - **Pre-send validation**: Check capacity before attempting xStreamSend()
   * - **Overflow prevention**: Detect full condition to trigger consumer
   * notification
   * - **Flow control**: Throttle producers when buffer reaches capacity
   * - **Overflow strategies**: Decide whether to drop data, reset buffer, or
   * block
   * - **Buffer health monitoring**: Track how often buffer reaches capacity
   *
   * Example 1: Pre-send capacity check
   * @code StreamBuffer_t *txStream;
   *
   * Return_t sendByte(Byte_t data) {
   *   Base_t isFull;
   *
   *   // Check capacity before sending if (OK(xStreamIsFull(txStream,
   * &isFull))) {
   *     if (isFull) {
   *       // Buffer full - return error or wait return ReturnError;
   *     }
   *
   *     // Space available - send byte return xStreamSend(txStream, data);
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 2: Overflow handling with notification
   * @code StreamBuffer_t *dataStream;
   * Task_t *consumerTask;
   *
   * void producerTask(Task_t *task, TaskParm_t *parm) {
   *   Byte_t data = generateData();
   *   Base_t isFull;
   *
   *   // Check if buffer full if (OK(xStreamIsFull(dataStream, &isFull)) &&
   * isFull) {
   *     // Wake up consumer to drain buffer xTaskNotifyGive(consumerTask);
   *
   *     // Wait briefly for consumer delayMs(5);
   *   }
   *
   *   // Attempt send if (ERROR(xStreamSend(dataStream, data))) {
   *     // Still full - data lost logDataLoss();
   *   }
   * }
   * @endcode
   *
   * Example 3: Overflow strategy selection
   * @code typedef enum {
   *   OVERFLOW_DROP_OLDEST, OVERFLOW_DROP_NEWEST, OVERFLOW_ERROR
   * } OverflowStrategy_t;
   *
   * StreamBuffer_t *sensorStream;
   * OverflowStrategy_t strategy = OVERFLOW_DROP_OLDEST;
   *
   * void recordSample(Byte_t sample) {
   *   Base_t isFull;
   *
   *   if (OK(xStreamIsFull(sensorStream, &isFull)) && isFull) {
   *     switch (strategy) {
   *       case OVERFLOW_DROP_OLDEST:
   *         // Clear buffer and add new sample xStreamReset(sensorStream);
   *         xStreamSend(sensorStream, sample);
   *         break;
   *
   *       case OVERFLOW_DROP_NEWEST:
   *         // Discard new sample, keep old data logDroppedSample();
   *         break;
   *
   *       case OVERFLOW_ERROR:
   *         // Report error condition handleBufferOverflow();
   *         break;
   *     }
   *   } else {
   *     // Normal send xStreamSend(sensorStream, sample);
   *   }
   * }
   * @endcode
   *
   * Example 4: Buffer utilization monitoring
   * @code StreamBuffer_t *commStream;
   *
   * typedef struct {
   *   Word_t fullCount;
   *   Word_t totalChecks;
   *   Byte_t peakUtilization;
   * } BufferStats_t;
   *
   * BufferStats_t stats = {0, 0, 0};
   *
   * void monitorBuffer(Task_t *task, TaskParm_t *parm) {
   *   Base_t isFull;
   *   HalfWord_t available;
   *
   *   stats.totalChecks++;
   *
   *   if (OK(xStreamIsFull(commStream, &isFull)) && isFull) {
   *     stats.fullCount++;
   *   }
   *
   *   // Track peak utilization if (OK(xStreamBytesAvailable(commStream,
   * &available))) {
   *     Byte_t utilization = (Byte_t)((available * 100) /
   * CONFIG_STREAM_BUFFER_BYTES);
   *     if (utilization > stats.peakUtilization) {
   *       stats.peakUtilization = utilization;
   *     }
   *   }
   *
   *   // Report statistics periodically if (stats.totalChecks % 1000 == 0) {
   *     Byte_t fullPercent = (Byte_t)((stats.fullCount * 100) /
   * stats.totalChecks);
   *     reportStats(fullPercent, stats.peakUtilization);
   *   }
   * }
   * @endcode
   *
   * @param[in]  stream_ Handle to the stream buffer to query. Must be a valid
   *                     stream created with xStreamCreate().
   * @param[out] res_    Pointer to variable receiving the full status. Set to
   *                     non-zero (true) if stream is full
   *                     (CONFIG_STREAM_BUFFER_BYTES bytes waiting), zero
   *                     (false) if stream has available space.
   *
   * @return             ReturnOK if query succeeded, ReturnError if query
   *                     failed due to invalid stream handle or stream not
   *                     found.
   *
   * @warning A full stream will cause xStreamSend() to fail with ReturnError.
   * Always check for full condition before critical send operations, or
   * implement appropriate error handling for send failures.
   *
   * @note This is a non-destructive query that does not modify stream contents.
   *
   * @note The buffer capacity is defined by CONFIG_STREAM_BUFFER_BYTES at
   * compile time (default 32 bytes). To calculate free space: freeSpace =
   * CONFIG_STREAM_BUFFER_BYTES - currentBytes
   *
   * @note In multitasking environments, the full status may change immediately
   * after the query if another task performs a receive operation. Use
   * appropriate synchronization for atomic check-and-send operations if needed.
   *
   * @sa xStreamBytesAvailable() - Get exact byte count and calculate free space
   * @sa xStreamIsEmpty() - Check if stream has no waiting bytes
   * @sa xStreamSend() - Send byte to stream (fails if full)
   * @sa xStreamReset() - Clear stream to free all space
   * @sa xStreamReceive() - Retrieve bytes to free space
   * @sa xStreamCreate() - Create a stream buffer
   */
  Return_t xStreamIsFull(const StreamBuffer_t *stream_, Base_t *res_);


  /**
   * @brief Trigger a system assertion failure for critical error handling
   *
   * Raises a system assertion to handle critical error conditions that violate
   * fundamental assumptions or invariants in the code. Assertions are defensive
   * programming constructs that catch bugs during development and provide
   * configurable responses in production builds.
   *
   * The behavior when an assertion triggers is determined by
   * CONFIG_SYSTEM_ASSERT_BEHAVIOR:
   * - **Halt**: Stop system execution completely (safest for critical systems)
   * - **Log**: Record assertion and continue (for debugging)
   * - **Callback**: Invoke custom handler function
   * - **Disabled**: Assertions compiled out entirely (production optimization)
   *
   * It is strongly recommended to use the __AssertOnElse__() macro rather than
   * calling xSystemAssert() directly. The macro automatically captures file
   * name and line number, making debugging significantly easier.
   *
   * Common use cases:
   * - **Precondition checking**: Validating function parameters and state
   * - **Postcondition verification**: Ensuring expected results after
   * operations
   * - **Invariant enforcement**: Checking critical system state consistency
   * - **Null pointer detection**: Catching unexpected null dereferences
   * - **Range validation**: Detecting out-of-bounds array/buffer access
   * - **Resource validation**: Ensuring handles and resources are valid
   *
   * Example 1: Parameter validation with __AssertOnElse__()
   * @code void processData(Byte_t *buffer, Size_t size) {
   *   // Assert buffer is not null __AssertOnElse__(buffer != NULL, return);
   *
   *   // Assert size is reasonable __AssertOnElse__(size > 0 && size <=
   * MAX_BUFFER_SIZE, return);
   *
   *   // Proceed with processing - preconditions verified for (Size_t i = 0; i
   * <
   * size; i++) {
   *     processBuffer(buffer[i]);
   *   }
   * }
   * @endcode
   *
   * Example 2: State invariant checking
   * @code typedef enum {
   *   STATE_IDLE, STATE_ACTIVE, STATE_ERROR
   * } SystemState_t;
   *
   * SystemState_t systemState = STATE_IDLE;
   *
   * void startOperation(void) {
   *   // Assert we're in correct state to start __AssertOnElse__(systemState ==
   * STATE_IDLE, return);
   *
   *   systemState = STATE_ACTIVE;
   *   performOperation();
   *
   *   // Assert valid ending state __AssertOnElse__(systemState == STATE_ACTIVE
   * || systemState == STATE_ERROR, systemState = STATE_ERROR);
   * }
   * @endcode
   *
   * Example 3: Handle validation
   * @code Return_t sendToQueue(Queue_t *queue, Byte_t *data, Size_t len) {
   *   // Assert queue handle is valid __AssertOnElse__(queue != NULL, return
   * ReturnError);
   *
   *   // Assert data parameters are reasonable __AssertOnElse__(data != NULL,
   * return ReturnError);
   *   __AssertOnElse__(len > 0, return ReturnError);
   *
   *   return xQueueSend(queue, len, data);
   * }
   * @endcode
   *
   * Example 4: Array bounds checking
   * @code
   * #define SENSOR_COUNT 8 Byte_t sensorReadings[SENSOR_COUNT];
   *
   * Byte_t getSensorReading(Base_t sensorId) {
   *   // Assert index is in valid range __AssertOnElse__(sensorId <
   * SENSOR_COUNT, return 0);
   *
   *   return sensorReadings[sensorId];
   * }
   * @endcode
   *
   * @param[in] file_ Source file name where assertion occurred. Automatically
   *                  provided by __AssertOnElse__() macro using __FILE__.
   * @param[in] line_ Line number where assertion occurred. Automatically
   *                  provided by __AssertOnElse__() macro using __LINE__.
   *
   * @return          ReturnOK if assertion handling completed (when configured
   *                  to continue), ReturnError if assertion handling failed.
   *                  Note: May not return if CONFIG_SYSTEM_ASSERT_BEHAVIOR
   *                  halts the system.
   *
   * @warning Assertions are for catching programming errors, NOT for runtime
   * error handling. Use normal error checking (if/return) for expected error
   * conditions like invalid user input or communication failures.
   *
   * @warning If CONFIG_SYSTEM_ASSERT_BEHAVIOR is set to halt, calling this
   * function will stop system execution permanently. The system must be reset
   * to recover.
   *
   * @warning Assertions enabled in production builds add code size and
   * execution overhead. Consider CONFIG_ENABLE_SYSTEM_ASSERT carefully based on
   * safety vs performance requirements.
   *
   * @note Use __AssertOnElse__(condition, action) macro instead of calling this
   * function directly. The macro provides automatic file/line capture and
   * better readability.
   *
   * @note Assertions can be completely disabled by not defining
   * CONFIG_ENABLE_SYSTEM_ASSERT, causing __AssertOnElse__() to compile to
   * nothing for zero overhead in production.
   *
   * @note The file_ and line_ parameters help identify exactly where the
   * assertion failed, making debugging much faster than generic error messages.
   *
   * @note Consider different assert behaviors for development vs production:
   * development might halt immediately, while production might log and attempt
   * graceful degradation.
   *
   * @sa __AssertOnElse__() - Recommended macro for triggering assertions
   * @sa CONFIG_SYSTEM_ASSERT_BEHAVIOR - Controls assertion response
   * @sa CONFIG_ENABLE_SYSTEM_ASSERT - Enables/disables assertion system
   * @sa xSystemHalt() - May be called by assertion handler
   */
  Return_t xSystemAssert(const char *file_, const int line_);


  /**
   * @brief Bootstrap HeliOS kernel and initialize all system resources
   *
   * Performs initial system bootstrap and initialization of the HeliOS
   * real-time kernel. This function MUST be the very first HeliOS function
   * called in any application, as it establishes the fundamental runtime
   * environment for all subsequent operations.
   *
   * The initialization process performs several critical operations in
   * sequence:
   * - Initializes the kernel memory management subsystem (both user heap and
   * kernel memory pools)
   * - Establishes internal kernel data structures for task management,
   * scheduling, and device tracking
   * - Calls port-specific initialization functions to configure
   * hardware-dependent features (timers, interrupts, system clock)
   * - Sets up the system state machine to prepare for task creation and
   * scheduler startup
   * - Validates configuration parameters defined at compile time
   *
   * After successful initialization, the system is ready to accept task
   * creation requests (xTaskCreate()) and device registrations
   * (xDeviceConfig()). The scheduler itself is not started until
   * xTaskStartScheduler() is called, allowing the application to set up all
   * tasks and devices before cooperative multitasking begins.
   *
   * Initialization is a one-time operation that cannot be reversed without a
   * system reset. The function maintains internal state to detect and reject
   * multiple initialization attempts, returning ReturnError if called more than
   * once.
   *
   * **Common use cases:**
   * - Standard application startup: Initialize HeliOS before any other system
   * calls
   * - Robust initialization: Check return value to detect initialization
   * failures
   * - Configuration validation: Verify system configuration before proceeding
   * - Resource preparation: Ensure kernel resources are ready before task
   * creation
   * - Hardware setup: Trigger port-specific hardware initialization sequences
   * - Memory pool establishment: Set up heap and kernel memory allocators
   *
   * Example 1: Basic application initialization
   * @code int main(void) {
   *   // First thing: initialize HeliOS if (ERROR(xSystemInit())) {
   *     // Initialization failed - cannot proceed
   *     // On embedded systems, might enter error loop while (1) {
   *       // Flash error LED or output diagnostic
   *     }
   *   }
   *
   *   // System ready - create tasks Task_t *taskHandle = NULL;
   *   xTaskCreate(&taskHandle, "MainTask", sensorTask, NULL);
   *   xTaskResume(taskHandle);
   *
   *   // Start scheduler xTaskStartScheduler();
   *
   *   // Never reached return 0;
   * }
   * @endcode
   *
   * Example 2: Initialization with system validation
   * @code
   * #include <stdio.h>
   *
   * Return_t initializeSystem(void) {
   *   Return_t res = xSystemInit();
   *
   *   if (OK(res)) {
   *     // Get and validate system information SystemInfo_t **sysInfo = NULL;
   *     if (OK(xSystemGetSystemInfo(&sysInfo))) {
   *       printf("Initialized %s v%s\n", sysInfo->productName,
   * sysInfo->productVersion);
   *       xMemFree(sysInfo);
   *     }
   *   }
   *
   *   return res;
   * }
   *
   * int main(void) {
   *   if (ERROR(initializeSystem())) {
   *     return -1;
   *   }
   *
   *   // Continue with application setup return 0;
   * }
   * @endcode
   *
   * Example 3: Memory configuration validation after init
   * @code int main(void) {
   *   // Initialize system if (ERROR(xSystemInit())) {
   *     return -1;
   *   }
   *
   *   // Verify memory configuration is adequate xMemStats *heapStats = NULL;
   *   if (OK(xMemGetHeapStats(&heapStats))) {
   *     if (heapStats->regionSizeInBytes < 4096) {
   *       // Warning: limited memory available
   *     }
   *     xMemFree(heapStats);
   *   }
   *
   *   // Proceed with task creation
   *   // ...
   *
   *   return 0;
   * }
   * @endcode
   *
   * Example 4: Multiple subsystem initialization
   * @code
   * // Application-specific initialization sequence Return_t appInit(void) {
   *   // Step 1: Initialize HeliOS (MUST be first) if (ERROR(xSystemInit())) {
   *     return ReturnError;
   *   }
   *
   *   // Step 2: Initialize device drivers xDevice uartDev = NULL;
   *   if (ERROR(xDeviceInitDevice("UART0   ", &uartDev, myUARTDriver))) {
   *     return ReturnError;
   *   }
   *
   *   // Step 3: Create application tasks Task_t *commTask = NULL;
   *   if (ERROR(xTaskCreate(&commTask, "CommTask", commHandler, uartDev))) {
   *     return ReturnError;
   *   }
   *   xTaskResume(commTask);
   *
   *   return ReturnOK;
   * }
   *
   * int main(void) {
   *   if (OK(appInit())) {
   *     xTaskStartScheduler();  // Start cooperative multitasking
   *   }
   *   return 0;
   * }
   * @endcode
   *
   * @return ReturnOK if initialization completed successfully and the system is
   *         ready for task creation and device registration. ReturnError if
   *         initialization failed (e.g., already initialized, memory pool
   *         configuration invalid, port layer initialization failure).
   *
   * @warning This function MUST be called before any other HeliOS functions.
   * Calling any HeliOS function before xSystemInit() results in undefined
   * behavior and likely system crashes.
   *
   * @warning This function can only be called ONCE. Subsequent calls will
   * return ReturnError. The system cannot be re-initialized without a hardware
   * reset.
   *
   * @warning Initialization failure typically indicates a critical system
   * problem (invalid configuration, hardware failure, etc.). If this function
   * returns ReturnError, the application should not attempt to use any HeliOS
   * features.
   *
   * @warning On embedded systems without an operating system, initialization
   * failure often requires entering an infinite error loop or triggering a
   * watchdog reset, as there is no graceful way to recover.
   *
   * @note This function does NOT start the scheduler. After initialization,
   * tasks must be created with xTaskCreate() and the scheduler started with
   * xTaskStartScheduler().
   *
   * @note Port-specific initialization (xPortSystemInit()) is called internally
   * and varies by platform. This may include timer setup, interrupt
   * configuration, or other hardware-dependent operations.
   *
   * @note The memory configuration (CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS) is
   * validated during initialization. Invalid configurations will cause
   * initialization to fail.
   *
   * @note Memory allocations (xMemAlloc, xMemFree) can be performed after
   * successful initialization but before the scheduler starts. This is useful
   * for pre-allocating shared resources.
   *
   * @sa xTaskCreate() - Create tasks after initialization
   * @sa xTaskStartScheduler() - Start scheduler after task setup
   * @sa xSystemGetSystemInfo() - Query system information post-init
   * @sa xSystemHalt() - Halt system in case of critical errors
   * @sa CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS - Memory configuration
   * @sa xPortSystemInit() - Port-specific initialization (internal)
   */
  Return_t xSystemInit(void);


  /**
   * @brief Immediately halt HeliOS and stop all system execution
   *
   * Triggers an immediate, unrecoverable system halt that stops all task
   * execution, disables interrupts, and freezes the processor. This function is
   * used for critical error conditions where continued operation would be
   * unsafe or impossible. Once halted, the system requires a hardware reset to
   * recover.
   *
   * The halt operation performs the following sequence:
   * - Disables all interrupts globally to prevent further interrupt processing
   * - Stops the scheduler to prevent task switching
   * - Calls port-specific halt functions that may include watchdog disabling,
   * peripheral shutdown, or entry into low-power sleep modes
   * - Enters an infinite loop or processor halt instruction
   * - May optionally flash an LED or output diagnostic information if
   * configured at the port layer
   *
   * This function is typically called in response to unrecoverable errors such
   * as assertion failures, stack overflow detection, memory corruption, or
   * critical hardware faults. It provides a safe shutdown mechanism that
   * prevents erratic behavior or dangerous actions from a malfunctioning
   * system.
   *
   * The system halt is permanent and irreversible without physical intervention
   * (reset button, power cycle, watchdog timer, or external reset signal). This
   * makes it suitable for fail-safe scenarios where the system must stop
   * completely rather than risk continued operation in an unknown state.
   *
   * **Common use cases:**
   * - Assertion failure handling: Stop execution when invariants are violated
   * - Critical error response: Halt on unrecoverable errors like stack overflow
   * - Safety-critical shutdown: Stop dangerous operations immediately
   * - Debug breakpoints: Halt for debugging when critical conditions occur
   * - Watchdog trigger: Halt before external watchdog forces reset
   * - Memory corruption detection: Stop when heap or kernel memory is damaged
   *
   * Example 1: Assertion failure handler
   * @code
   * // Custom assertion behavior using halt void assertionHandler(const char
   * file, int line) {
   *   // Log assertion information if possible
   *   #ifdef DEBUG_UART printf("ASSERTION FAILED: %s:%d\n", file, line);
   *   #endif
   *
   *   // Halt system - no recovery possible xSystemHalt();
   *
   *   // Never reached
   * }
   * @endcode
   *
   * Example 2: Critical error handling
   * @code void criticalErrorHandler(ErrorCode error) {
   *   // Flash error code on LED if available for (int i = 0; i < error; i++) {
   *     toggleErrorLED();
   *     delayMs(200);
   *   }
   *
   *   // Critical error - cannot continue xSystemHalt();
   * }
   *
   * void sensorTask(Task_t *task, TaskParm_t *parm) {
   *   Byte_t buffer[128];
   *
   *   if (ERROR(xDeviceSimpleRead(sensorDevice, 128, buffer))) {
   *     // Sensor communication critical for safety
   * criticalErrorHandler(ERROR_SENSOR_COMM);
   *   }
   *
   *   // Process sensor data
   *   // ...
   * }
   * @endcode
   *
   * Example 3: Stack overflow detection
   * @code
   * #define STACK_CANARY 0xDEADBEEF
   *
   * void checkStackIntegrity(void) {
   *   volatile uint32_t *stackCanary = (uint32_t*)STACK_CANARY_ADDRESS;
   *
   *   if (*stackCanary != STACK_CANARY) {
   *     // Stack overflow detected!
   *     #ifdef DEBUG_OUTPUT printf("FATAL: Stack overflow detected\n");
   *     #endif
   *
   *     xSystemHalt();  // Stop immediately
   *   }
   * }
   *
   * void taskWithStackCheck(Task_t *task, TaskParm_t *parm) {
   *   checkStackIntegrity();
   *   // Perform task operations
   *   // ...
   * }
   * @endcode
   *
   * Example 4: Watchdog-aware halt
   * @code void safeHalt(void) {
   *   // Disable watchdog before halting to prevent continuous resets
   *   #ifdef WATCHDOG_ENABLED disableWatchdog();
   *   #endif
   *
   *   // Output diagnostic information
   *   #ifdef DEBUG_MODE SystemInfo_t **info = NULL;
   *   if (OK(xSystemGetSystemInfo(&info))) {
   *     printf("System halted. Tasks: %d\n", info->numberOfTasks);
   *     xMemFree(info);
   *   }
   *   #endif
   *
   *   // Halt system xSystemHalt();
   * }
   * @endcode
   *
   * @return This function typically does NOT return. If it does return
   *         ReturnOK, the halt operation completed but the port layer
   *         implementation allowed return (unusual). ReturnError if halt could
   *         not be performed (rare - usually indicates port layer issue).
   *
   * @warning This function NEVER returns under normal circumstances. Any code
   * following xSystemHalt() will not execute. Always treat this as a terminal
   * function call.
   *
   * @warning Once halted, the system cannot be recovered without a hardware
   * reset. All task state, memory contents, and device configurations are lost
   * on reset.
   *
   * @warning Interrupts are disabled before halting. This means any pending or
   * future interrupt requests will not be serviced. External hardware expecting
   * interrupt acknowledgment may time out or enter error states.
   *
   * @warning On battery-powered systems, halting may not fully power down the
   * device. Consider entering a low-power sleep mode instead if power
   * conservation is important.
   *
   * @warning Do NOT use this function for normal shutdown or task termination.
   * It is only for critical, unrecoverable error conditions. For normal
   * operation control, use task management functions.
   *
   * @note Some port implementations may flash an LED or output a diagnostic
   * pattern before entering the halt loop, which can aid in debugging.
   *
   * @note The port layer implementation (xPortSystemHalt()) determines the
   * exact behavior. Common implementations include infinite loops, WFI (Wait
   * For Interrupt) instructions, or entering deep sleep modes.
   *
   * @note In some debugging environments (JTAG, debugger attached), the halt
   * may trigger a breakpoint that allows the developer to inspect system state.
   *
   * @note If a watchdog timer is enabled and not disabled before halting, the
   * watchdog will eventually reset the system, which may or may not be desired
   * depending on the application.
   *
   * @sa xSystemAssert() - May call this function when assertions are configured
   * to halt
   * @sa xSystemInit() - Must be called before system is operational
   * @sa CONFIG_SYSTEM_ASSERT_BEHAVIOR - May configure assertions to halt
   * @sa xPortSystemHalt() - Port-specific halt implementation (internal)
   */
  Return_t xSystemHalt(void);


  /**
   * @brief Retrieve comprehensive system information and runtime statistics
   *
   * Obtains detailed information about the HeliOS system configuration and
   * current runtime state. The function allocates and populates a SystemInfo_t
   * *
   * structure containing the operating system name, version string, and task
   * count. This information is useful for diagnostics, logging, runtime
   * monitoring, and version verification.
   *
   * The returned SystemInfo_t *structure contains:
   * - **productName**: String identifying the operating system (e.g., "HeliOS")
   * - **productVersion**: Version string in semantic versioning format (e.g.,
   *   "2.1.0")
   * - **numberOfTasks**: Current count of tasks registered with the scheduler,
   * including both running and suspended tasks
   *
   * Memory for the SystemInfo_t *structure is allocated from the user heap
   * using xMemAlloc(). The caller is responsible for freeing this memory using
   * xMemFree() when the information is no longer needed. Failure to free the
   * structure will result in a memory leak.
   *
   * This function can be called at any time after xSystemInit() has completed
   * successfully. It works both before and after the scheduler has started,
   * making it useful for initialization-time validation as well as runtime
   * monitoring.
   *
   * The task count reflects the instantaneous number of tasks at the moment of
   * the call. In a running system, this count may change as tasks are created
   * or deleted by other tasks.
   *
   * **Common use cases:**
   * - System diagnostics: Log version information at startup for debugging
   * - Version verification: Ensure correct OS version is deployed
   * - Runtime monitoring: Track task count during system operation
   * - Status reporting: Send system information over communication interfaces
   * - Compatibility checking: Verify application compatibility with OS version
   * - Debug output: Include system details in error reports or crash dumps
   *
   * Example 1: Basic system information logging
   * @code void logSystemInfo(void) {
   *   SystemInfo_t **sysInfo = NULL;
   *
   *   if (OK(xSystemGetSystemInfo(&sysInfo))) {
   *     printf("System: %s v%s\n", sysInfo->productName,
   * sysInfo->productVersion);
   *     printf("Active tasks: %d\n", sysInfo->numberOfTasks);
   *
   *     // Clean up allocated memory xMemFree(sysInfo);
   *   } else {
   *     printf("Failed to get system info\n");
   *   }
   * }
   *
   * int main(void) {
   *   xSystemInit();
   *   logSystemInfo();  // Log at startup
   *
   *   // Create tasks and start scheduler
   *   // ...
   * }
   * @endcode
   *
   * Example 2: Version compatibility check
   * @code
   * #define REQUIRED_MAJOR_VERSION 2
   * #define REQUIRED_MINOR_VERSION 1
   *
   * Return_t checkSystemVersion(void) {
   *   SystemInfo_t **sysInfo = NULL;
   *
   *   if (ERROR(xSystemGetSystemInfo(&sysInfo))) {
   *     return ReturnError;
   *   }
   *
   *   // Parse version string (assumes "X.Y.Z" format) int major, minor, patch;
   *   sscanf(sysInfo->productVersion, "%d.%d.%d", &major, &minor, &patch);
   *
   *   Return_t result = ReturnOK;
   *   if (major < REQUIRED_MAJOR_VERSION ||
   *       (major == REQUIRED_MAJOR_VERSION && minor < REQUIRED_MINOR_VERSION))
   * {
   *     printf("ERROR: HeliOS version %s too old (need %d.%d+)\n",
   * sysInfo->productVersion, REQUIRED_MAJOR_VERSION, REQUIRED_MINOR_VERSION);
   *     result = ReturnError;
   *   }
   *
   *   xMemFree(sysInfo);
   *   return result;
   * }
   * @endcode
   *
   * Example 3: Runtime task monitoring
   * @code void monitoringTask(Task_t *task, TaskParm_t *parm) {
   *   static Base_t lastTaskCount = 0;
   *   SystemInfo_t **sysInfo = NULL;
   *
   *   if (OK(xSystemGetSystemInfo(&sysInfo))) {
   *     if (sysInfo->numberOfTasks != lastTaskCount) {
   *       printf("Task count changed: %d -> %d\n", lastTaskCount,
   * sysInfo->numberOfTasks);
   *       lastTaskCount = sysInfo->numberOfTasks;
   *     }
   *
   *     xMemFree(sysInfo);
   *   }
   * }
   * @endcode
   *
   * Example 4: Sending system info over UART
   * @code void sendSystemInfoToHost(xDevice uart) {
   *   SystemInfo_t **sysInfo = NULL;
   *
   *   if (OK(xSystemGetSystemInfo(&sysInfo))) {
   *     // Format as JSON for easy parsing by host char buffer[128];
   *     snprintf(buffer, sizeof(buffer),
   *              "{\"os\":\"%s\",\"version\":\"%s\",\"tasks\":%d}\n",
   * sysInfo->productName, sysInfo->productVersion, sysInfo->numberOfTasks);
   *
   *     // Send over UART xDeviceSimpleWrite(uart, strlen(buffer),
   * (Byte_t*)buffer);
   *
   *     xMemFree(sysInfo);
   *   }
   * }
   * @endcode
   *
   * @param[out] info_ Pointer to SystemInfo_t *pointer that will receive the
   *                   allocated system information structure. Must not be NULL.
   *                   On success, points to allocated structure that must be
   *                   freed with xMemFree(). On failure, remains unchanged.
   *
   * @return           ReturnOK if system information was successfully retrieved
   *                   and the structure allocated. ReturnError if the operation
   *                   failed (e.g., info_ is NULL, memory allocation failed,
   *                   system not initialized).
   *
   * @warning The caller MUST free the returned SystemInfo_t *structure using
   * xMemFree(). Failure to do so will leak memory from the user heap.
   *
   * @warning Do NOT free the structure using standard free() or other
   * allocators. Only xMemFree() is compatible with HeliOS memory management.
   *
   * @warning The strings within the SystemInfo_t *structure (productName,
   * productVersion) are part of the allocated structure and should NOT be freed
   * separately. xMemFree() on the structure handles all cleanup.
   *
   * @warning The numberOfTasks value is a snapshot at the moment of the call.
   * It may change immediately afterward if tasks are created or deleted. Do not
   * rely on this value remaining constant.
   *
   * @note This function allocates memory using xMemAlloc() from the user heap.
   * Ensure adequate heap space is available, especially if called frequently.
   *
   * @note The function can be called before the scheduler starts, making it
   * useful for startup diagnostics and initialization-time verification.
   *
   * @note Task count includes all tasks in any state (running, suspended,
   * waiting). It does not distinguish between active and inactive tasks.
   *
   * @note The version string format follows semantic versioning
   * (major.minor.patch) but the exact format is determined by the HeliOS build
   * configuration.
   *
   * @sa SystemInfo_t * - Structure containing system information
   * @sa xMemFree() - Must be used to free the returned structure
   * @sa xSystemInit() - Must be called before this function
   * @sa xTaskGetNumberOfTasks() - Alternative way to get task count
   * @sa xMemGetHeapStats() - For detailed memory usage information
   */
  Return_t xSystemGetSystemInfo(SystemInfo_t **info_);


  /**
   * @brief Create a new task for cooperative multitasking
   *
   * Creates a new task and registers it with the HeliOS scheduler. Tasks are
   * the fundamental unit of execution in HeliOS and represent functions that
   * execute cooperatively under scheduler control. Each task has a name,
   * callback function, optional parameters, and scheduling properties.
   *
   * When created, tasks begin in TaskStateSuspended state and will not execute
   * until xTaskResume() is called. Tasks can have an associated period that
   * controls how frequently they are scheduled for execution.
   *
   * Task Lifecycle:
   * 1. Create task with xTaskCreate() (starts in TaskStateSuspended) 2.
   * Optionally set task period with xTaskChangePeriod() 3. Resume task with
   * xTaskResume() (transitions to TaskStateRunning) 4. Task executes when
   * scheduler runs 5. Delete task with xTaskDelete() when no longer needed
   *
   * @warning This function must be called BEFORE xTaskStartScheduler() is
   * invoked. It cannot be called from within a task while the scheduler is
   * running. Creating or deleting tasks during scheduler execution will result
   * in undefined behavior.
   *
   * @note Task names are used for identification and debugging. They must be
   * exactly CONFIG_TASK_NAME_BYTES characters (default 8). Shorter names should
   * be space-padded; longer names will be truncated.
   *
   * @note The taskParameter_ allows passing data to the task. This is commonly
   * used to pass pointers to configuration structures or shared data. The
   * memory management of this parameter is the responsibility of the caller.
   *
   * Example Usage:
   * @code void blinkLED(Task_t *task, TaskParm_t *parm) {
   *   // Access parameter if needed int *ledPin = (int*)parm;
   *
   *   // Task logic here toggleLED(*ledPin);
   *
   *   // Task yields when function returns
   * }
   *
   * int main(void) {
   *   Task_t *ledTask;
   *   static int pin = 13;  // Must persist beyond task creation
   *
   *   // Create task (starts in suspended state) if (OK(xTaskCreate(&ledTask,
   * "LED     ", blinkLED, (TaskParm_t *)&pin))) {
   *     // Set task to run every 100ms xTaskChangePeriod(ledTask, 100);
   *
   *     // Activate the task xTaskResume(ledTask);
   *
   *     // Start scheduler xTaskStartScheduler();
   *   }
   * }
   * @endcode
   *
   * @param[out] task_          Pointer to Task_t *variable that will receive
   *                            the task handle. This handle is used in
   *                            subsequent operations on the task.
   * @param[in]  name_          Task name string, exactly CONFIG_TASK_NAME_BYTES
   *                            bytes. Common pattern: "TaskName" (8 bytes with
   *                            space padding).
   * @param[in]  callback_      Pointer to the task function that will be
   *                            executed. Function signature: void func(Task_t *
   *                            task, TaskParm_t *parm).
   * @param[in]  taskParameter_ Optional parameter passed to the task function.
   *                            Use NULL if no parameter is needed. Can be a
   *                            pointer to any data type.
   *
   * @return                    ReturnOK if task was successfully created,
   *                            ReturnError if creation failed (e.g., out of
   *                            memory, invalid parameters, or scheduler is
   *                            already running).
   *
   * @sa xTaskDelete() - Remove a task from the scheduler
   * @sa xTaskResume() - Activate a task for scheduling
   * @sa xTaskSuspend() - Deactivate a task
   * @sa xTaskChangePeriod() - Set task execution frequency
   * @sa xTaskStartScheduler() - Begin executing tasks
   * @sa CONFIG_TASK_NAME_BYTES - Configuration for task name length
   */
  Return_t xTaskCreate(Task_t **task_, const Byte_t *name_, void (*callback_)(Task_t *task_, TaskParm_t *parm_), TaskParm_t *taskParameter_);


  /**
   * @brief Delete a task and free its resources
   *
   * Removes a task from the HeliOS scheduler and frees all memory associated
   * with the task. Once deleted, the task will no longer be scheduled for
   * execution and its task handle becomes invalid.
   *
   * This function performs cleanup including:
   * - Removing the task from the scheduler's task list
   * - Freeing internal task control structures
   * - Invalidating the task handle
   *
   * @warning This function must be called BEFORE xTaskStartScheduler() starts
   * or AFTER the scheduler has been suspended with xTaskSuspendAll(). It cannot
   * be called from within a running task while the scheduler is active.
   * Attempting to delete a task during scheduler execution will result in
   * undefined behavior.
   *
   * @warning After deleting a task, the task handle becomes invalid and must
   * not be used in any subsequent HeliOS function calls. Using a deleted task
   * handle will result in undefined behavior.
   *
   * @note If a task needs to be temporarily deactivated rather than permanently
   * removed, use xTaskSuspend() instead. Suspended tasks can be resumed later
   * without recreating them.
   *
   * @note Memory allocated by the task (via xMemAlloc()) is NOT automatically
   * freed when the task is deleted. The application is responsible for managing
   * any dynamically allocated memory used by the task.
   *
   * Example Usage:
   * @code Task_t *temporaryTask;
   *
   * // Create a task for one-time initialization if
   * (OK(xTaskCreate(&temporaryTask, "InitTask", initFunction, NULL))) {
   *   xTaskResume(temporaryTask);
   *   xTaskStartScheduler();
   *
   *   // Later, after initialization is complete (scheduler suspended)
   * xTaskSuspendAll();
   *
   *   // Delete the task as it's no longer needed if
   * (OK(xTaskDelete(temporaryTask))) {
   *     // Task successfully deleted
   *   }
   *
   *   xTaskResumeAll();
   *   xTaskStartScheduler();
   * }
   * @endcode
   *
   * @param[in] task_ Handle of the task to delete. Must be a valid task handle
   *                  previously returned by xTaskCreate().
   *
   * @return          ReturnOK if the task was successfully deleted, ReturnError
   *                  if deletion failed (invalid task handle, scheduler is
   *                  running, or system error).
   *
   * @sa xTaskCreate() - Create a new task
   * @sa xTaskSuspend() - Temporarily deactivate a task (without deleting it)
   * @sa xTaskSuspendAll() - Suspend scheduler to allow task deletion
   * @sa xTaskStartScheduler() - Start the scheduler
   */
  Return_t xTaskDelete(const Task_t *task_);


  /**
   * @brief Retrieve a task handle by its name
   *
   * Searches for a task by its string name and returns the corresponding task
   * handle. This enables runtime task lookup when you know the task name but
   * don't have a direct reference to its handle, useful for inter-task
   * communication, dynamic task management, and debugging scenarios.
   *
   * Task names are assigned during task creation via xTaskCreate() and must be
   * exactly CONFIG_TASK_NAME_BYTES (default 8) bytes in length. Names shorter
   * than this must be null-padded. The search is case-sensitive and performs an
   * exact byte-for-byte comparison.
   *
   * Common use cases:
   * - **Inter-task communication**: Finding a target task to send notifications
   * - **Dynamic task control**: Suspending/resuming tasks by name at runtime
   * - **Debugging and diagnostics**: Inspecting task state by name
   * - **Configuration-driven systems**: Task names from config files or
   * commands
   * - **Task coordination**: One task finding and controlling related tasks
   *
   * Example 1: Find and notify a task by name
   * @code Return_t notifyTaskByName(const char *taskName) {
   *   Task_t *targetTask;
   *   Byte_t paddedName[CONFIG_TASK_NAME_BYTES];
   *
   *   // Prepare padded name (CONFIG_TASK_NAME_BYTES = 8) memset(paddedName, 0,
   * sizeof(paddedName));
   *   strncpy((char*)paddedName, taskName, sizeof(paddedName));
   *
   *   // Find the task if (OK(xTaskGetHandleByName(&targetTask, paddedName))) {
   *     // Notify the task if (OK(xTaskNotifyGive(targetTask))) {
   *       return ReturnOK;
   *     }
   *   }
   *
   *   logError("Failed to find or notify task: %s", taskName);
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 2: Suspend task by name from command handler
   * @code void handleCommand(const char *cmd) {
   *   if (strncmp(cmd, "suspend ", 8) == 0) {
   *     Task_t *task;
   *     Byte_t taskName[CONFIG_TASK_NAME_BYTES];
   *
   *     // Extract and pad task name memset(taskName, 0, sizeof(taskName));
   *     strncpy((char*)taskName, cmd + 8, sizeof(taskName));
   *
   *     // Find and suspend the task if (OK(xTaskGetHandleByName(&task,
   * taskName))) {
   *       if (OK(xTaskSuspend(task))) {
   *         printf("Task '%s' suspended\n", taskName);
   *       }
   *     } else {
   *       printf("Task '%s' not found\n", taskName);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Check if specific tasks are running
   * @code Base_t areSystemTasksRunning(void) {
   *   const char *requiredTasks[] = {
   *     "Monitor",
   *     "Logger",
   *     "Network",
   *     "Storage"
   *   };
   *   Base_t numTasks = 4;
   *
   *   for (Base_t i = 0; i < numTasks; i++) {
   *     Task_t *task;
   *     Byte_t paddedName[CONFIG_TASK_NAME_BYTES];
   *     TaskState_t state;
   *
   *     // Prepare name memset(paddedName, 0, sizeof(paddedName));
   *     strncpy((char*)paddedName, requiredTasks[i], sizeof(paddedName));
   *
   *     // Check if task exists and is running if
   * (ERROR(xTaskGetHandleByName(&task, paddedName))) {
   *       logError("Required task not found: %s", requiredTasks[i]);
   *       return 0;
   *     }
   *
   *     if (OK(xTaskGetTaskState(task, &state))) {
   *       if (state != TaskStateRunning) {
   *         logWarning("Task not running: %s", requiredTasks[i]);
   *         return 0;
   *       }
   *     }
   *   }
   *
   *   return 1;  // All required tasks running
   * }
   * @endcode
   *
   * Example 4: Build task name from pattern
   * @code Return_t getSensorTask(Base_t sensorId, Task_t **task) {
   *   Byte_t taskName[CONFIG_TASK_NAME_BYTES];
   *
   *   // Build task name: "Sensor0", "Sensor1", etc.
   *   memset(taskName, 0, sizeof(taskName));
   *   snprintf((char*)taskName, sizeof(taskName), "Sensor%u", sensorId);
   *
   *   // Find the sensor task if (OK(xTaskGetHandleByName(task, taskName))) {
   *     return ReturnOK;
   *   }
   *
   *   logError("Sensor task %u not found", sensorId);
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[out] task_ Pointer to Task_t *variable that will receive the task
   *                   handle if found.
   * @param[in]  name_ Pointer to task name buffer. Must be exactly
   *                   CONFIG_TASK_NAME_BYTES bytes long (default 8). Names
   *                   shorter than this must be null-padded to the full length.
   *
   * @return           ReturnOK if task found, ReturnError if task not found or
   *                   invalid parameters.
   *
   * @warning Task names must be exactly CONFIG_TASK_NAME_BYTES bytes. Shorter
   * names must be null-padded. Use memset() or strncpy() to ensure proper
   * padding.
   *
   * @warning The search is case-sensitive. "MyTask" and "mytask" are different
   * names.
   *
   * @warning If multiple tasks have identical names (which shouldn't happen but
   * is technically possible), this function returns the first match found. Use
   * unique task names to avoid ambiguity.
   *
   * @note Task names are assigned during xTaskCreate(). Ensure tasks are
   * created with meaningful, unique names for reliable lookup.
   *
   * @note This function searches through all tasks, which may take time if many
   * tasks exist. Consider caching task handles if lookups are frequent.
   *
   * @note Task names cannot be changed after creation. If you need dynamic task
   * identification, consider using task IDs with xTaskGetHandleById() instead.
   *
   * @sa xTaskGetHandleById() - Get task handle by numeric ID
   * @sa xTaskCreate() - Create task with name
   * @sa xTaskGetTaskInfo() - Get task information including name
   * @sa xTaskGetId() - Get numeric ID from task handle
   * @sa CONFIG_TASK_NAME_BYTES - Task name length configuration
   */
  Return_t xTaskGetHandleByName(Task_t **task_, const Byte_t *name_);


  /**
   * @brief Retrieve a task handle by its numeric ID
   *
   * Searches for a task by its unique numeric identifier and returns the
   * corresponding task handle. Each task is automatically assigned a sequential
   * ID starting from 0 when created, providing a lightweight alternative to
   * name-based lookup. This is faster than xTaskGetHandleByName() and useful
   * when working with task arrays or numeric references.
   *
   * Task IDs are assigned sequentially in the order tasks are created: the
   * first task gets ID 0, the second gets ID 1, and so on. IDs are never reused
   * even after a task is deleted, so they remain unique throughout system
   * lifetime.
   *
   * Common use cases:
   * - **Task arrays**: Managing tasks indexed by ID for efficient lookup
   * - **Numeric configuration**: Task references stored as numbers in config
   * - **Performance-critical code**: Faster lookup than string-based search
   * - **Task iteration**: Enumerating all tasks by ID from 0 to N-1
   * - **Compact references**: Storing task references as small integers
   *
   * Example 1: Iterate through all tasks
   * @code void inspectAllTasks(void) {
   *   Base_t numTasks;
   *   Task_t *task;
   *
   *   // Get total task count if (ERROR(xTaskGetNumberOfTasks(&numTasks))) {
   *     return;
   *   }
   *
   *   printf("Inspecting %u tasks:\n", numTasks);
   *
   *   // Iterate through each task by ID for (Base_t id = 0; id < numTasks;
   * id++) {
   *     if (OK(xTaskGetHandleById(&task, id))) {
   *       TaskInfo_t **info = NULL;
   *
   *       if (OK(xTaskGetTaskInfo(task, &info))) {
   *         printf("  Task %u: %s (state: %d)\n", id, info->name, info->state);
   *         xMemFree((Addr_t *)info);
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Task array management
   * @code
   * #define MAX_WORKER_TASKS 8 Task_t *workerTasks[MAX_WORKER_TASKS];
   * Base_t workerTaskIds[MAX_WORKER_TASKS];
   * Base_t workerCount = 0;
   *
   * void createWorkerTask(const char *name) {
   *   Task_t *newTask;
   *   Base_t taskId;
   *
   *   // Create the task if (OK(xTaskCreate(&newTask, name, workerFunction,
   * NULL))) {
   *     // Get its ID if (OK(xTaskGetId(newTask, &taskId))) {
   *       workerTasks[workerCount] = newTask;
   *       workerTaskIds[workerCount] = taskId;
   *       workerCount++;
   *     }
   *   }
   * }
   *
   * void notifyWorker(Base_t workerIndex) {
   *   Task_t *task;
   *
   *   // Look up task by stored ID if (OK(xTaskGetHandleById(&task,
   * workerTaskIds[workerIndex]))) {
   *     xTaskNotifyGive(task);
   *   }
   * }
   * @endcode
   *
   * Example 3: Configuration-driven task control
   * @code
   * // Configuration might specify task IDs to enable/disable typedef struct {
   *   Base_t taskId;
   *   Base_t enabled;
   * } TaskConfig_t;
   *
   * void applyTaskConfiguration(TaskConfig_t *config, Base_t count) {
   *   for (Base_t i = 0; i < count; i++) {
   *     Task_t *task;
   *
   *     if (OK(xTaskGetHandleById(&task, config[i].taskId))) {
   *       if (config[i].enabled) {
   *         xTaskResume(task);
   *         logInfo("Enabled task ID %u", config[i].taskId);
   *       } else {
   *         xTaskSuspend(task);
   *         logInfo("Disabled task ID %u", config[i].taskId);
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 4: Find task by ID with error handling
   * @code Return_t processTaskById(Base_t taskId) {
   *   Task_t *task;
   *   TaskState_t state;
   *
   *   // Verify task exists if (ERROR(xTaskGetHandleById(&task, taskId))) {
   *     logError("Task ID %u not found or deleted", taskId);
   *     return ReturnError;
   *   }
   *
   *   // Check task is in valid state if (OK(xTaskGetTaskState(task, &state)))
   * {
   *     if (state == TaskStateRunning) {
   *       // Perform operation on running task return
   * performTaskOperation(task);
   *     } else {
   *       logWarning("Task ID %u not running (state: %d)", taskId, state);
   *     }
   *   }
   *
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[out] task_ Pointer to Task_t *variable that will receive the task
   *                   handle if found.
   * @param[in]  id_   Numeric task ID to search for. Valid IDs range from 0 to
   *                   (number of tasks - 1).
   *
   * @return           ReturnOK if task found, ReturnError if task ID invalid,
   *                   task deleted, or invalid parameters.
   *
   * @warning Task IDs are never reused after task deletion. If a task is
   * deleted, its ID becomes permanently invalid even if new tasks are created.
   *
   * @warning Valid task IDs range from 0 to N-1 where N is the number of tasks
   * ever created. However, some IDs in this range may be invalid if those tasks
   * were deleted.
   *
   * @note This function is faster than xTaskGetHandleByName() because it uses
   * direct numeric comparison instead of string comparison.
   *
   * @note Task IDs are assigned sequentially based on creation order, starting
   * from 0. The first task created has ID 0, second has ID 1, etc.
   *
   * @note To enumerate all valid tasks, use xTaskGetNumberOfTasks() to get the
   * count, then iterate through IDs checking for ReturnOK.
   *
   * @note Task IDs are stable for the lifetime of the task but become invalid
   * permanently after xTaskDelete() is called on that task.
   *
   * @sa xTaskGetHandleByName() - Get task handle by name (slower but more
   * readable)
   * @sa xTaskGetId() - Get numeric ID from task handle
   * @sa xTaskGetNumberOfTasks() - Get total task count for iteration
   * @sa xTaskGetTaskInfo() - Get comprehensive task information
   * @sa xTaskCreate() - Create task (assigns automatic ID)
   */
  Return_t xTaskGetHandleById(Task_t **task_, const Base_t id_);


  /**
   * @brief Retrieve runtime execution statistics for all tasks in the system
   *
   * Collects and returns runtime performance statistics for every task
   * currently registered with the HeliOS scheduler. This function provides
   * comprehensive visibility into task execution patterns, CPU utilization, and
   * system workload distribution. The statistics are essential for performance
   * analysis, optimization, and debugging.
   *
   * The function allocates an array of TaskRunTimeStats_t *structures, one
   * entry for each task in the system. Each structure contains detailed timing
   * and execution metrics:
   * - **totalRunTime**: Total accumulated execution time in system ticks
   * - **lastRunTime**: Duration of the most recent execution in ticks
   * - **taskId**: Unique identifier for the task
   * - Additional port-specific timing information
   *
   * The statistics array is dynamically allocated from the user heap and must
   * be freed by the caller using xMemFree() to prevent memory leaks. The number
   * of entries in the array is returned through the tasks_ parameter, allowing
   * the caller to iterate through all task statistics.
   *
   * Runtime statistics tracking must be enabled at compile time with
   * CONFIG_ENABLE_RUNTIME_STATS for this function to return meaningful data. If
   * runtime stats are disabled, the function may return empty or zero-valued
   * statistics.
   *
   * The statistics represent a snapshot of task execution at the moment of the
   * call. In a running system, these values continuously change as tasks
   * execute. For accurate profiling, consider calling this function
   * periodically and analyzing trends over time.
   *
   * **Common use cases:**
   * - Performance profiling: Identify CPU-intensive tasks consuming excessive
   * time
   * - Load balancing: Analyze workload distribution across tasks
   * - Optimization targets: Find tasks that are candidates for optimization
   * - System monitoring: Track overall system execution patterns
   * - Debug analysis: Investigate scheduling anomalies or starvation issues
   * - Capacity planning: Determine if system can handle additional tasks
   *
   * Example 1: Basic runtime statistics display
   * @code void displayTaskStats(void) {
   *   TaskRunTimeStats_t **stats = NULL;
   *   Base_t taskCount = 0;
   *
   *   if (OK(xTaskGetAllRunTimeStats(&stats, &taskCount))) {
   *     printf("Runtime Statistics for %d tasks:\n", taskCount);
   *
   *     for (Base_t i = 0; i < taskCount; i++) {
   *       printf("  Task ID %d: Total=%lu ticks, Last=%lu ticks\n",
   * stats[i].taskId, stats[i].totalRunTime, stats[i].lastRunTime);
   *     }
   *
   *     xMemFree(stats);
   *   } else {
   *     printf("Failed to get runtime statistics\n");
   *   }
   * }
   * @endcode
   *
   * Example 2: CPU utilization percentage calculation
   * @code void showCPUUtilization(void) {
   *   TaskRunTimeStats_t **stats = NULL;
   *   Base_t taskCount = 0;
   *
   *   if (OK(xTaskGetAllRunTimeStats(&stats, &taskCount))) {
   *     // Calculate total system runtime unsigned long totalTime = 0;
   *     for (Base_t i = 0; i < taskCount; i++) {
   *       totalTime += stats[i].totalRunTime;
   *     }
   *
   *     // Display percentage utilization per task printf("CPU
   * Utilization:\n");
   *     for (Base_t i = 0; i < taskCount; i++) {
   *       float percent = (stats[i].totalRunTime * 100.0f) / totalTime;
   *       printf("  Task %d: %.2f%%\n", stats[i].taskId, percent);
   *     }
   *
   *     xMemFree(stats);
   *   }
   * }
   * @endcode
   *
   * Example 3: Identifying overloaded tasks
   * @code
   * #define CPU_THRESHOLD_PERCENT 30.0f
   *
   * void detectOverloadedTasks(void) {
   *   TaskRunTimeStats_t **stats = NULL;
   *   Base_t taskCount = 0;
   *
   *   if (ERROR(xTaskGetAllRunTimeStats(&stats, &taskCount))) {
   *     return;
   *   }
   *
   *   // Calculate total runtime unsigned long totalTime = 0;
   *   for (Base_t i = 0; i < taskCount; i++) {
   *     totalTime += stats[i].totalRunTime;
   *   }
   *
   *   // Identify tasks exceeding threshold printf("Tasks exceeding %0.f%%
   * CPU:\n", CPU_THRESHOLD_PERCENT);
   *   for (Base_t i = 0; i < taskCount; i++) {
   *     float percent = (stats[i].totalRunTime * 100.0f) / totalTime;
   *     if (percent > CPU_THRESHOLD_PERCENT) {
   *       printf("  WARNING: Task %d using %.2f%% CPU\n", stats[i].taskId,
   * percent);
   *     }
   *   }
   *
   *   xMemFree(stats);
   * }
   * @endcode
   *
   * Example 4: Periodic monitoring with trend detection
   * @code void monitoringTask(Task_t *task, TaskParm_t *parm) {
   *   static unsigned long lastTotalRuntime[32] = {0};
   *   TaskRunTimeStats_t **stats = NULL;
   *   Base_t taskCount = 0;
   *
   *   if (OK(xTaskGetAllRunTimeStats(&stats, &taskCount))) {
   *     for (Base_t i = 0; i < taskCount && i < 32; i++) {
   *       unsigned long delta = stats[i].totalRunTime - lastTotalRuntime[i];
   *
   *       if (delta > 10000) {  // More than 10000 ticks since last check
   * printf("Task %d execution increased significantly: +%lu ticks\n",
   * stats[i].taskId, delta);
   *       }
   *
   *       lastTotalRuntime[i] = stats[i].totalRunTime;
   *     }
   *
   *     xMemFree(stats);
   *   }
   * }
   * @endcode
   *
   * @param[out] stats_ Pointer to TaskRunTimeStats_t *pointer that will receive
   *                    the allocated array of runtime statistics. One entry per
   *                    task. Must not be NULL. On success, points to allocated
   *                    array that must be freed with xMemFree(). On failure,
   *                    remains unchanged.
   * @param[out] tasks_ Pointer to Base_t variable that will receive the number
   *                    of tasks (and array elements) in stats_. Must not be
   *                    NULL. On success, contains the task count. On failure,
   *                    remains unchanged.
   *
   * @return            ReturnOK if statistics were successfully retrieved and
   *                    allocated. ReturnError if the operation failed (e.g.,
   *                    stats_ or tasks_ is NULL, memory allocation failed, no
   *                    tasks exist, runtime stats not enabled).
   *
   * @warning The caller MUST free the returned array using xMemFree(). Failure
   * to do so will leak memory from the user heap.
   *
   * @warning Runtime statistics tracking must be enabled with
   * CONFIG_ENABLE_RUNTIME_STATS at compile time. If disabled, this function may
   * return zero values or return an error.
   *
   * @warning The statistics are a snapshot in time. Values change continuously
   * as tasks execute. Do not rely on absolute values remaining constant between
   * calls.
   *
   * @warning Large systems with many tasks may allocate significant memory for
   * the statistics array. Ensure adequate heap space is available, especially
   * if calling frequently.
   *
   * @note The array is allocated using xMemAlloc() from the user heap. Each
   * element corresponds to one task in the system.
   *
   * @note The order of tasks in the array is not guaranteed and may change
   * between calls. Use the taskId field to identify specific tasks.
   *
   * @note Runtime values are typically in system tick units. The exact time
   * resolution depends on the port configuration and system tick frequency.
   *
   * @note For single-task statistics, use xTaskGetTaskRunTimeStats() to avoid
   * allocating memory for all tasks.
   *
   * @sa TaskRunTimeStats_t * - Structure containing runtime statistics
   * @sa xTaskGetTaskRunTimeStats() - Get statistics for a single task
   * @sa xMemFree() - Must be used to free the returned array
   * @sa xTaskGetNumberOfTasks() - Get task count without statistics
   * @sa CONFIG_ENABLE_RUNTIME_STATS - Must be enabled for statistics tracking
   */
  Return_t xTaskGetAllRunTimeStats(TaskRunTimeStats_t **stats_, Base_t *tasks_);


  /**
   * @brief Retrieve runtime execution statistics for a specific task
   *
   * Obtains detailed runtime performance statistics for a single task
   * identified by its task handle. This function provides targeted performance
   * data for individual task analysis without the overhead of retrieving
   * statistics for all tasks in the system. It's ideal for monitoring specific
   * critical tasks or profiling individual task behavior.
   *
   * The function allocates and populates a TaskRunTimeStats_t *structure
   * containing comprehensive timing metrics for the specified task:
   * - **totalRunTime**: Cumulative execution time since task creation, measured
   * in system ticks
   * - **lastRunTime**: Duration of the most recent task execution in ticks
   * - **taskId**: Unique numeric identifier for the task
   * - Additional port-specific timing information that may include execution
   * counts, context switch counts, or other performance metrics
   *
   * The statistics structure is allocated from the user heap and must be freed
   * by the caller using xMemFree() to prevent memory leaks. This allocation is
   * significantly smaller than xTaskGetAllRunTimeStats() which allocates an
   * array for all tasks.
   *
   * Runtime statistics tracking must be enabled at compile time using
   * CONFIG_ENABLE_RUNTIME_STATS. If disabled, this function may return
   * zero-valued statistics or an error depending on the configuration.
   *
   * The returned statistics are a point-in-time snapshot. For a running task,
   * the totalRunTime value increases continuously as the task executes. The
   * lastRunTime value reflects the duration of the most recent complete
   * execution cycle.
   *
   * **Common use cases:**
   * - Critical task monitoring: Track execution time of safety-critical or
   * high-priority tasks
   * - Performance debugging: Profile specific tasks suspected of performance
   * issues
   * - Periodic sampling: Monitor individual task behavior over time
   * - Watchdog implementation: Verify tasks are executing within expected time
   * bounds
   * - Resource optimization: Identify if specific tasks need optimization
   * - SLA verification: Ensure tasks meet service level agreements for
   * execution time
   *
   * Example 1: Monitor specific critical task
   * @code void checkSensorTaskPerformance(Task_t *sensorTask) {
   *   TaskRunTimeStats_t **stats = NULL;
   *
   *   if (OK(xTaskGetTaskRunTimeStats(sensorTask, &stats))) {
   *     printf("Sensor Task (ID %d):\n", stats->taskId);
   *     printf("  Total runtime: %lu ticks\n", stats->totalRunTime);
   *     printf("  Last execution: %lu ticks\n", stats->lastRunTime);
   *
   *     xMemFree(stats);
   *   } else {
   *     printf("ERROR: Failed to get sensor task stats\n");
   *   }
   * }
   * @endcode
   *
   * Example 2: Watchdog for task execution time
   * @code
   * #define MAX_EXECUTION_TIME_TICKS 1000
   *
   * Return_t validateTaskExecution(Task_t *task) {
   *   TaskRunTimeStats_t **stats = NULL;
   *
   *   if (ERROR(xTaskGetTaskRunTimeStats(task, &stats))) {
   *     return ReturnError;
   *   }
   *
   *   Return_t result = ReturnOK;
   *   if (stats->lastRunTime > MAX_EXECUTION_TIME_TICKS) {
   *     printf("WARNING: Task %d exceeded max execution time: %lu > %d\n",
   * stats->taskId, stats->lastRunTime, MAX_EXECUTION_TIME_TICKS);
   *     result = ReturnError;
   *   }
   *
   *   xMemFree(stats);
   *   return result;
   * }
   * @endcode
   *
   * Example 3: Periodic task profiling
   * @code void profileTask(Task_t *task, TaskParm_t *parm) {
   *   static unsigned long lastTotalRuntime = 0;
   *   static unsigned long callCount = 0;
   *   TaskRunTimeStats_t **stats = NULL;
   *
   *   // Get stats for the monitored task (passed as parameter) Task_t *
   * monitoredTask = (Task_t *)parm;
   *
   *   if (OK(xTaskGetTaskRunTimeStats(monitoredTask, &stats))) {
   *     unsigned long delta = stats->totalRunTime - lastTotalRuntime;
   *     callCount++;
   *
   *     if (callCount % 100 == 0) {  // Report every 100 samples printf("Task
   * %d: Average execution per sample: %lu ticks\n", stats->taskId, delta /
   * 100);
   *       lastTotalRuntime = stats->totalRunTime;
   *     }
   *
   *     xMemFree(stats);
   *   }
   * }
   * @endcode
   *
   * Example 4: Compare task execution before and after optimization
   * @code typedef struct {
   *   unsigned long totalBefore;
   *   unsigned long totalAfter;
   * } OptimizationResults;
   *
   * void measureOptimizationImpact(Task_t *task, OptimizationResults *results)
   * {
   *   TaskRunTimeStats_t **stats = NULL;
   *
   *   // Measure before optimization if (OK(xTaskGetTaskRunTimeStats(task,
   * &stats))) {
   *     results->totalBefore = stats->totalRunTime;
   *     xMemFree(stats);
   *   }
   *
   *   // ... perform optimization ...
   *   // ... let task run for measurement period ...
   *
   *   // Measure after optimization if (OK(xTaskGetTaskRunTimeStats(task,
   * &stats))) {
   *     results->totalAfter = stats->totalRunTime;
   *     xMemFree(stats);
   *
   *     unsigned long improvement = results->totalBefore - results->totalAfter;
   *     float percent = (improvement * 100.0f) / results->totalBefore;
   *     printf("Optimization reduced runtime by %.2f%%\n", percent);
   *   }
   * }
   * @endcode
   *
   * @param[in]  task_  Task handle for the task to query. Must be a valid task
   *                    handle obtained from xTaskCreate() or
   *                    xTaskGetHandleByName(). Must not be NULL or refer to a
   *                    deleted task.
   * @param[out] stats_ Pointer to TaskRunTimeStats_t *pointer that will receive
   *                    the allocated statistics structure. Must not be NULL. On
   *                    success, points to allocated structure that must be
   *                    freed with xMemFree(). On failure, remains unchanged.
   *
   * @return            ReturnOK if statistics were successfully retrieved and
   *                    allocated. ReturnError if the operation failed (e.g.,
   *                    task_ is NULL or invalid, stats_ is NULL, memory
   *                    allocation failed, task no longer exists, runtime stats
   *                    not enabled).
   *
   * @warning The caller MUST free the returned structure using xMemFree().
   * Failure to do so will leak memory from the user heap.
   *
   * @warning The task handle must be valid. If the task has been deleted or the
   * handle is corrupted, this function returns ReturnError. Always verify task
   * existence before calling.
   *
   * @warning Runtime statistics tracking must be enabled with
   * CONFIG_ENABLE_RUNTIME_STATS at compile time. If disabled, this function may
   * return zero values or an error.
   *
   * @warning The statistics are a snapshot. For a currently executing task, the
   * values may be outdated immediately after return. Use for trend analysis
   * rather than instant-precise measurements.
   *
   * @note This function is more efficient than xTaskGetAllRunTimeStats() when
   * you only need information about a single task, as it allocates less memory
   * and performs less processing.
   *
   * @note The structure is allocated using xMemAlloc() from the user heap.
   * Ensure adequate heap space is available.
   *
   * @note Runtime values are in system tick units. Convert to real time using
   * the system tick frequency (typically configured in port layer).
   *
   * @note The taskId in the returned structure matches the ID returned by
   * xTaskGetId() for the same task.
   *
   * @sa TaskRunTimeStats_t * - Structure containing runtime statistics
   * @sa xTaskGetAllRunTimeStats() - Get statistics for all tasks at once
   * @sa xMemFree() - Must be used to free the returned structure
   * @sa xTaskGetId() - Get task ID for comparison
   * @sa CONFIG_ENABLE_RUNTIME_STATS - Must be enabled for statistics tracking
   */
  Return_t xTaskGetTaskRunTimeStats(const Task_t *task_, TaskRunTimeStats_t **stats_);


  /**
   * @brief Query the total number of tasks in the system
   *
   * Returns the total count of all tasks currently managed by HeliOS,
   * regardless of their state (running, suspended, or waiting). This count
   * includes tasks in all states and is useful for system monitoring, task
   * enumeration, and diagnostic reporting.
   *
   * The returned count reflects the current number of active task control
   * blocks in the system. This number changes dynamically as tasks are created
   * with xTaskCreate() and deleted with xTaskDelete(). The count includes tasks
   * in all states: running, waiting, and suspended.
   *
   * Common use cases:
   * - **Task iteration**: Determining loop bounds for enumerating all tasks
   * - **System monitoring**: Tracking task count for capacity management
   * - **Diagnostics**: Reporting system resource utilization
   * - **Validation**: Verifying expected number of tasks are active
   * - **Dynamic arrays**: Allocating arrays sized to hold all task data
   *
   * Example 1: Enumerate and display all tasks
   * @code void listAllTasks(void) {
   *   Base_t taskCount;
   *
   *   if (OK(xTaskGetNumberOfTasks(&taskCount))) {
   *     printf("System has %u tasks:\n", taskCount);
   *
   *     for (Base_t id = 0; id < taskCount; id++) {
   *       Task_t *task;
   *       TaskInfo_t **info = NULL;
   *
   *       if (OK(xTaskGetHandleById(&task, id))) {
   *         if (OK(xTaskGetTaskInfo(task, &info))) {
   *           printf("  %u: %s [%s]\n", id, info->name, info->state ==
   * TaskStateRunning ? "Running" :
   *                  info->state == TaskStateSuspended ? "Suspended" :
   *  "Waiting");
   *           xMemFree((Addr_t *)info);
   *         }
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Monitor task count for system health
   * @code
   * #define EXPECTED_TASK_COUNT 10
   * #define MAX_TASK_COUNT 15
   *
   * void monitorSystemHealth(void) {
   *   Base_t taskCount;
   *
   *   if (OK(xTaskGetNumberOfTasks(&taskCount))) {
   *     if (taskCount < EXPECTED_TASK_COUNT) {
   *       logWarning("Task count below expected: %u (expected %u)", taskCount,
   * EXPECTED_TASK_COUNT);
   *     } else if (taskCount > MAX_TASK_COUNT) {
   *       logError("Task count exceeds maximum: %u (max %u)", taskCount,
   * MAX_TASK_COUNT);
   *     } else {
   *       logInfo("Task count normal: %u tasks", taskCount);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Allocate array for all task info
   * @code Return_t analyzeAllTasks(void) {
   *   Base_t taskCount;
   *   TaskInfo_t **allInfo = NULL;
   *
   *   // Get task count if (ERROR(xTaskGetNumberOfTasks(&taskCount))) {
   *     return ReturnError;
   *   }
   *
   *   // Allocate array to hold all task info Size_t arraySize =
   * sizeof(TaskInfo_t *) * taskCount;
   *   if (ERROR(xMemAlloc((volatile Addr_t **)&allInfo, arraySize))) {
   *     return ReturnError;
   *   }
   *
   *   // Collect info for each task for (Base_t i = 0; i < taskCount; i++) {
   *     Task_t *task;
   *     TaskInfo_t **info = NULL;
   *
   *     if (OK(xTaskGetHandleById(&task, i))) {
   *       if (OK(xTaskGetTaskInfo(task, &info))) {
   *         allInfo[i] = *info;  // Copy to array xMemFree((Addr_t *)info);
   *       }
   *     }
   *   }
   *
   *   // Analyze tasks... analyzeTaskArray(allInfo, taskCount);
   *
   *   xMemFree((Addr_t *)allInfo);
   *   return ReturnOK;
   * }
   * @endcode
   *
   * Example 4: Wait for all worker tasks to be created
   * @code
   * #define EXPECTED_WORKERS 5
   *
   * Return_t waitForWorkersReady(void) {
   *   Base_t retries = 0;
   *   Base_t maxRetries = 100;
   *
   *   while (retries < maxRetries) {
   *     Base_t taskCount;
   *
   *     if (OK(xTaskGetNumberOfTasks(&taskCount))) {
   *       // Assuming 1 main task + EXPECTED_WORKERS if (taskCount >= (1 +
   * EXPECTED_WORKERS)) {
   *         logInfo("All worker tasks ready");
   *         return ReturnOK;
   *       }
   *     }
   *
   *     // Wait a bit for tasks to be created xTaskDelayUntil(10);  // 10 ticks
   * retries++;
   *   }
   *
   *   logError("Timeout waiting for worker tasks");
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[out] tasks_ Pointer to variable that receives the total number of
   *                    tasks currently in the system.
   *
   * @return            ReturnOK if count retrieved successfully, ReturnError if
   *                    operation failed (invalid parameter).
   *
   * @note The count includes ALL tasks regardless of state: running, suspended,
   * and waiting. It does not distinguish between active and inactive tasks.
   *
   * @note The returned count is a snapshot at the time of the call. Other tasks
   * may create or delete tasks immediately after this function returns,
   * changing the count.
   *
   * @note This function is very lightweight and can be called frequently for
   * monitoring without performance concerns.
   *
   * @note Task IDs range from 0 to N-1, but if tasks have been deleted, some
   * IDs in this range may be invalid. Use xTaskGetHandleById() and check its
   * return value when iterating.
   *
   * @sa xTaskGetHandleById() - Get task handle for iteration (use with task
   * count)
   * @sa xTaskGetAllTaskInfo() - Get comprehensive info for all tasks at once
   * @sa xTaskCreate() - Create task (increases task count)
   * @sa xTaskDelete() - Delete task (decreases task count)
   * @sa xTaskGetTaskInfo() - Get info about individual task
   */
  Return_t xTaskGetNumberOfTasks(Base_t *tasks_);


  /**
   * @brief Retrieve comprehensive information about a task
   *
   * Returns detailed information about a specific task including its name,
   * state, and runtime statistics. This provides a complete snapshot of a
   * task's identity and performance in a single call, allocating and returning
   * a TaskInfo_t *structure that must be freed by the caller.
   *
   * The returned TaskInfo_t *structure contains:
   * - **name**: Task name (CONFIG_TASK_NAME_BYTES bytes)
   * - **state**: Current task state (TaskStateRunning, TaskStateSuspended,
   * TaskStateWaiting)
   * - **Runtime statistics**: Execution time, run count, and performance
   * metrics
   *
   * This function allocates memory from the user heap for the info structure
   * and returns it to the caller. The caller MUST free this memory with
   * xMemFree() after use to prevent memory leaks.
   *
   * Common use cases:
   * - **Task monitoring**: Inspecting individual task state and performance
   * - **Debugging**: Examining specific task behavior during development
   * - **Health checks**: Verifying critical tasks are in expected states
   * - **Performance profiling**: Analyzing task execution patterns
   * - **Diagnostic logging**: Recording task status for troubleshooting
   *
   * Example 1: Check task state and log status
   * @code void checkTaskHealth(Task_t *task, const char *expectedName) {
   *   TaskInfo_t **info = NULL;
   *
   *   if (OK(xTaskGetTaskInfo(task, &info))) {
   *     logInfo("Task: %s", info->name);
   *     logInfo("  State: %s", info->state == TaskStateRunning ? "Running" :
   *             info->state == TaskStateSuspended ? "Suspended" : "Waiting");
   *
   *     // Verify task is as expected if (strncmp((char*)info->name,
   * expectedName, CONFIG_TASK_NAME_BYTES) != 0) {
   *       logWarning("Task name mismatch!");
   *     }
   *
   *     if (info->state != TaskStateRunning) {
   *       logWarning("Task not running!");
   *     }
   *
   *     // Always free the allocated info structure xMemFree((Addr_t *)info);
   *   }
   * }
   * @endcode
   *
   * Example 2: Monitor task runtime statistics
   * @code void profileTask(Task_t *task) {
   *   TaskInfo_t **info = NULL;
   *
   *   if (OK(xTaskGetTaskInfo(task, &info))) {
   *     printf("Task Profile: %s\n", info->name);
   *     printf("  Total Runtime: %lu ticks\n", info->totalRunTime);
   *     printf("  Run Count: %lu\n", info->runCount);
   *
   *     // Calculate average runtime per execution if (info->runCount > 0) {
   *       Word_t avgRuntime = info->totalRunTime / info->runCount;
   *       printf("  Avg Runtime: %lu ticks/run\n", avgRuntime);
   *     }
   *
   *     xMemFree((Addr_t *)info);
   *   }
   * }
   * @endcode
   *
   * Example 3: Find and inspect task by name
   * @code Return_t inspectTaskByName(const char *taskName) {
   *   Task_t *task;
   *   TaskInfo_t **info = NULL;
   *   Byte_t paddedName[CONFIG_TASK_NAME_BYTES];
   *
   *   // Prepare padded name memset(paddedName, 0, sizeof(paddedName));
   *   strncpy((char*)paddedName, taskName, sizeof(paddedName));
   *
   *   // Find task if (ERROR(xTaskGetHandleByName(&task, paddedName))) {
   *     logError("Task '%s' not found", taskName);
   *     return ReturnError;
   *   }
   *
   *   // Get detailed info if (OK(xTaskGetTaskInfo(task, &info))) {
   *     printf("Task Information:\n");
   *     printf("  Name: %s\n", info->name);
   *     printf("  State: %d\n", info->state);
   *     printf("  Statistics available\n");
   *
   *     xMemFree((Addr_t *)info);
   *     return ReturnOK;
   *   }
   *
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 4: Periodic task health monitoring
   * @code void monitorCriticalTasks(void) {
   *   Task_t *watchdogTask;
   *   Byte_t taskName[CONFIG_TASK_NAME_BYTES] = "Watchdog";
   *
   *   if (OK(xTaskGetHandleByName(&watchdogTask, taskName))) {
   *     TaskInfo_t **info = NULL;
   *
   *     if (OK(xTaskGetTaskInfo(watchdogTask, &info))) {
   *       // Check if watchdog is running if (info->state != TaskStateRunning)
   * {
   *         logCritical("Watchdog task not running! State: %d", info->state);
   *         // Attempt to resume xTaskResume(watchdogTask);
   *       }
   *
   *       // Check if watchdog is executing regularly static Word_t
   * lastRunCount
   * = 0;
   *       if (info->runCount == lastRunCount) {
   *         logWarning("Watchdog task may be stuck");
   *       }
   *       lastRunCount = info->runCount;
   *
   *       xMemFree((Addr_t *)info);
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  task_ Task handle to query. Must be a valid task handle from
   *                   xTaskCreate(), xTaskGetHandleByName(), or
   *                   xTaskGetHandleById().
   * @param[out] info_ Pointer to TaskInfo_t *pointer that will receive the
   *                   allocated task information structure. Caller MUST free
   *                   this with xMemFree() after use.
   *
   * @return           ReturnOK if information retrieved successfully,
   *                   ReturnError if operation failed (invalid task handle,
   *                   task deleted, or memory allocation failed).
   *
   * @warning The caller MUST free the returned info structure using xMemFree().
   * Failing to do so will cause memory leaks. The structure is allocated by
   * this function specifically for the caller.
   *
   * @warning Do not access the info structure after freeing it with xMemFree().
   * Set the pointer to NULL after freeing to prevent accidental use.
   *
   * @warning The returned information is a snapshot at the time of the call.
   * Task state and statistics may change immediately after this function
   * returns.
   *
   * @note This function combines task identity (name, state) with runtime
   * statistics in a single structure, making it more convenient than calling
   * multiple separate functions.
   *
   * @note For bulk operations on all tasks, use xTaskGetAllTaskInfo() instead,
   * which is more efficient than calling this function in a loop.
   *
   * @note The TaskInfo_t *structure size depends on configuration. The function
   * allocates the exact size needed from the heap.
   *
   * @sa xTaskGetAllTaskInfo() - Get info for all tasks at once
   * @sa xTaskGetTaskRunTimeStats() - Get runtime stats only (without
   * name/state)
   * @sa xTaskGetTaskState() - Get just the task state
   * @sa xTaskGetName() - Get just the task name
   * @sa xMemFree() - Free the allocated info structure
   * @sa xTaskGetHandleByName() - Find task by name before getting info
   * @sa xTaskGetHandleById() - Find task by ID before getting info
   */
  Return_t xTaskGetTaskInfo(const Task_t *task_, TaskInfo_t **info_);


  /**
   * @brief Retrieve comprehensive information about all tasks
   *
   * Returns detailed information for all tasks in the system in a single
   * efficient call. Allocates an array of TaskInfo_t *structures containing
   * name, state, and runtime statistics for every task, providing a complete
   * system snapshot for monitoring, diagnostics, or reporting.
   *
   * This function allocates a single contiguous array from the user heap
   * containing one TaskInfo_t *structure per task. The array is indexed by task
   * ID (0 to N-1), making it easy to access information for any specific task.
   * The caller MUST free this array with xMemFree() after use.
   *
   * Each TaskInfo_t *in the array contains:
   * - **name**: Task name (CONFIG_TASK_NAME_BYTES bytes)
   * - **state**: Current state (Running, Suspended, or Waiting)
   * - **Runtime statistics**: Execution time, run counts, performance metrics
   *
   * Common use cases:
   * - **System dashboards**: Display all task statuses and performance
   * - **Performance analysis**: Compare execution times across all tasks
   * - **System health checks**: Verify all expected tasks are present and
   * running
   * - **Diagnostic dumps**: Capture complete system state for troubleshooting
   * - **Load balancing**: Analyze task distribution and CPU utilization
   *
   * Example 1: Display complete system task status
   * @code void displaySystemStatus(void) {
   *   TaskInfo_t **allInfo = NULL;
   *   Base_t taskCount;
   *
   *   if (OK(xTaskGetAllTaskInfo(&allInfo, &taskCount))) {
   *     printf("System Tasks (%u total):\n", taskCount);
   *     printf("%-10s %-12s %-10s %-15s\n",
   *            "ID", "Name", "State", "Runtime");
   *     printf("----------------------------------------------------\n");
   *
   *     for (Base_t i = 0; i < taskCount; i++) {
   *       const char *stateStr =
   *         allInfo[i].state == TaskStateRunning ? "Running" :
   *         allInfo[i].state == TaskStateSuspended ? "Suspended" : "Waiting";
   *
   *       printf("%-10u %-12s %-12s %-15lu\n", i, allInfo[i].name, stateStr,
   * allInfo[i].totalRunTime);
   *     }
   *
   *     // Always free the array xMemFree((Addr_t *)allInfo);
   *   }
   * }
   * @endcode
   *
   * Example 2: Find tasks in specific states
   * @code void findSuspendedTasks(void) {
   *   TaskInfo_t **allInfo = NULL;
   *   Base_t taskCount;
   *   Base_t suspendedCount = 0;
   *
   *   if (OK(xTaskGetAllTaskInfo(&allInfo, &taskCount))) {
   *     printf("Suspended tasks:\n");
   *
   *     for (Base_t i = 0; i < taskCount; i++) {
   *       if (allInfo[i].state == TaskStateSuspended) {
   *         printf("  %s (ID: %u)\n", allInfo[i].name, i);
   *         suspendedCount++;
   *       }
   *     }
   *
   *     if (suspendedCount == 0) {
   *       printf("  (none)\n");
   *     }
   *
   *     xMemFree((Addr_t *)allInfo);
   *   }
   * }
   * @endcode
   *
   * Example 3: Analyze CPU utilization across tasks
   * @code void analyzeCPUUsage(void) {
   *   TaskInfo_t **allInfo = NULL;
   *   Base_t taskCount;
   *
   *   if (OK(xTaskGetAllTaskInfo(&allInfo, &taskCount))) {
   *     Word_t totalRuntime = 0;
   *
   *     // Calculate total runtime for (Base_t i = 0; i < taskCount; i++) {
   *       totalRuntime += allInfo[i].totalRunTime;
   *     }
   *
   *     if (totalRuntime > 0) {
   *       printf("CPU Usage by Task:\n");
   *
   *       // Calculate and display percentage for each task for (Base_t i = 0;
   * i
   * < taskCount; i++) {
   *         Byte_t percentage = (Byte_t)((allInfo[i].totalRunTime * 100) /
   *                                     totalRuntime);
   *         printf("  %-12s: %3u%%\n", allInfo[i].name, percentage);
   *       }
   *     }
   *
   *     xMemFree((Addr_t *)allInfo);
   *   }
   * }
   * @endcode
   *
   * Example 4: Periodic system health check
   * @code
   * #define EXPECTED_TASKS 8
   *
   * Return_t performHealthCheck(void) {
   *   TaskInfo_t **allInfo = NULL;
   *   Base_t taskCount;
   *   Base_t runningCount = 0;
   *   Base_t suspendedCount = 0;
   *
   *   if (ERROR(xTaskGetAllTaskInfo(&allInfo, &taskCount))) {
   *     logError("Failed to get task info");
   *     return ReturnError;
   *   }
   *
   *   // Verify expected task count if (taskCount != EXPECTED_TASKS) {
   *     logWarning("Task count mismatch: expected %u, got %u", EXPECTED_TASKS,
   * taskCount);
   *   }
   *
   *   // Count tasks by state for (Base_t i = 0; i < taskCount; i++) {
   *     if (allInfo[i].state == TaskStateRunning) {
   *       runningCount++;
   *     } else if (allInfo[i].state == TaskStateSuspended) {
   *       suspendedCount++;
   *       logWarning("Task %s is suspended", allInfo[i].name);
   *     }
   *   }
   *
   *   logInfo("Health check: %u running, %u suspended", runningCount,
   * suspendedCount);
   *
   *   xMemFree((Addr_t *)allInfo);
   *   return ReturnOK;
   * }
   * @endcode
   *
   * @param[out] info_  Pointer to TaskInfo_t *pointer that will receive the
   *                    allocated array of task information structures. Array
   *                    contains taskCount elements indexed by task ID. Caller
   *                    MUST free with xMemFree() after use.
   * @param[out] tasks_ Pointer to variable that receives the number of tasks
   *                    (and array elements) returned.
   *
   * @return            ReturnOK if information retrieved successfully,
   *                    ReturnError if operation failed (memory allocation error
   *                    or invalid parameters).
   *
   * @warning The caller MUST free the returned array using xMemFree(). Failing
   * to do so will cause significant memory leaks since the array size is
   * proportional to the number of tasks.
   *
   * @warning Do not access the array after freeing it. Set the pointer to NULL
   * after calling xMemFree() to prevent accidental use.
   *
   * @warning The returned information is a snapshot at the time of the call.
   * Task states and statistics change continuously, so the data may be stale
   * immediately after return.
   *
   * @note This function is more efficient than calling xTaskGetTaskInfo() in a
   * loop, as it allocates memory once and captures all task data atomically.
   *
   * @note The array is indexed by task ID (0 to taskCount-1), providing direct
   * access to any task's information.
   *
   * @note For systems with many tasks, this function allocates significant
   * memory. Ensure sufficient heap space is available before calling.
   *
   * @note The array size is (sizeof(TaskInfo_t *) * taskCount) bytes. On
   * memory-constrained systems, consider iterating through tasks individually
   * with xTaskGetTaskInfo() instead.
   *
   * @sa xTaskGetTaskInfo() - Get info for single task (uses less memory)
   * @sa xTaskGetNumberOfTasks() - Get task count (for array sizing)
   * @sa xTaskGetAllRunTimeStats() - Get runtime stats only (smaller structures)
   * @sa xMemFree() - Free the allocated array
   * @sa xTaskGetHandleById() - Access individual tasks by ID
   */
  Return_t xTaskGetAllTaskInfo(TaskInfo_t **info_, Base_t *tasks_);


  /**
   * @brief Query the current execution state of a task
   *
   * Retrieves the current state of a task, which indicates whether the task is
   * actively running, suspended awaiting resume, or waiting for a notification.
   * Understanding task state is essential for debugging scheduling behavior,
   * implementing state machines, and monitoring system health.
   *
   * HeliOS tasks exist in one of three states at any given time:
   * - **TaskStateRunning**: Task is actively scheduled and will execute when
   * the scheduler selects it. This is the normal operational state for tasks
   * that should be executing their callback functions periodically.
   * - **TaskStateSuspended**: Task has been suspended via xTaskSuspend() and
   * will not execute until explicitly resumed with xTaskResume(). Newly created
   * tasks start in this state.
   * - **TaskStateWaiting**: Task is blocked waiting for a direct-to-task
   * notification. It will not execute until a notification is sent via
   * xTaskNotify() or xTaskNotifyGive(). This state is used for event-driven
   * task synchronization.
   *
   * The state query is instantaneous and reflects the task state at the moment
   * of the call. In a running system, states can change rapidly as tasks are
   * suspended, resumed, and notified. This function does not affect the task's
   * state - it is purely a query operation.
   *
   * State information is useful for debugging race conditions, verifying that
   * tasks transition properly through state machines, and ensuring that
   * synchronization mechanisms are working correctly. It's also valuable for
   * system health monitoring and diagnostic reporting.
   *
   * **Common use cases:**
   * - State verification: Confirm task is in expected state before performing
   * operations
   * - Debug output: Include task state in diagnostic messages
   * - Health monitoring: Detect tasks stuck in unexpected states
   * - State machine implementation: Query state to determine next action
   * - Synchronization debugging: Verify tasks are waiting as expected
   * - System visualization: Display task states in monitoring tools
   *
   * Example 1: Verify task state before operation
   * @code Return_t safeTaskOperation(Task_t *task) {
   *   TaskState_t state;
   *
   *   if (ERROR(xTaskGetTaskState(task, &state))) {
   *     return ReturnError;  // Invalid task handle
   *   }
   *
   *   if (state != TaskStateRunning) {
   *     printf("WARNING: Task not in running state (state=%d)\n", state);
   *     return ReturnError;
   *   }
   *
   *   // Task is running - safe to proceed return ReturnOK;
   * }
   * @endcode
   *
   * Example 2: Debug output with state information
   * @code void debugPrintTaskState(Task_t *task, const char *taskName) {
   *   TaskState_t state;
   *
   *   if (OK(xTaskGetTaskState(task, &state))) {
   *     const char *stateName;
   *     switch (state) {
   *       case TaskStateRunning:
   *         stateName = "Running";
   *         break;
   *       case TaskStateSuspended:
   *         stateName = "Suspended";
   *         break;
   *       case TaskStateWaiting:
   *         stateName = "Waiting";
   *         break;
   *       default:
   *         stateName = "Unknown";
   *     }
   *     printf("%s: %s\n", taskName, stateName);
   *   } else {
   *     printf("%s: Error querying state\n", taskName);
   *   }
   * }
   * @endcode
   *
   * Example 3: Monitoring task for stuck state
   * @code void monitorTask(Task_t *task, TaskParm_t *parm) {
   *   static TaskState_t lastState = TaskStateRunning;
   *   static int sameStateCount = 0;
   *   Task_t *monitoredTask = (Task_t *)parm;
   *   TaskState_t currentState;
   *
   *   if (OK(xTaskGetTaskState(monitoredTask, &currentState))) {
   *     if (currentState == lastState) {
   *       sameStateCount++;
   *       if (sameStateCount > 1000 && currentState == TaskStateWaiting) {
   *         printf("WARNING: Task stuck in Waiting state for %d checks\n",
   * sameStateCount);
   *       }
   *     } else {
   *       sameStateCount = 0;
   *       lastState = currentState;
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 4: Conditional task resume based on state
   * @code Return_t ensureTaskRunning(Task_t *task) {
   *   TaskState_t state;
   *
   *   if (ERROR(xTaskGetTaskState(task, &state))) {
   *     return ReturnError;
   *   }
   *
   *   // If suspended, resume it if (state == TaskStateSuspended) {
   *     printf("Task was suspended - resuming\n");
   *     return xTaskResume(task);
   *   }
   *
   *   // If waiting, send notification to unblock if (state ==
   * TaskStateWaiting) {
   *     printf("Task was waiting - sending notification\n");
   *     return xTaskNotifyGive(task);
   *   }
   *
   *   // Already running return ReturnOK;
   * }
   * @endcode
   *
   * @param[in]  task_  Task handle for the task to query. Must be a valid task
   *                    handle obtained from xTaskCreate() or
   *                    xTaskGetHandleByName(). Must not be NULL or refer to a
   *                    deleted task.
   * @param[out] state_ Pointer to TaskState_t variable that will receive the
   *                    task's current state. Must not be NULL. On success,
   *                    contains one of: TaskStateRunning, TaskStateSuspended,
   *                    or TaskStateWaiting. On failure, remains unchanged.
   *
   * @return            ReturnOK if state was successfully retrieved.
   *                    ReturnError if the operation failed (e.g., task_ is NULL
   *                    or invalid, state_ is NULL, task has been deleted).
   *
   * @warning The task handle must be valid. If the task has been deleted or the
   * handle is corrupted, this function returns ReturnError.
   *
   * @warning The returned state is a snapshot and may change immediately after
   * return, especially in systems with multiple tasks or interrupt handlers
   * that modify task states.
   *
   * @warning Do not assume state persistence across function calls. Always
   * re-query state if making decisions based on it, as another task or
   * interrupt may have changed it.
   *
   * @warning Repeatedly polling task state in a tight loop wastes CPU cycles.
   * Use notifications or timers for event-driven task coordination instead.
   *
   * @note This function does not change the task's state - it only queries it.
   * To change state, use xTaskSuspend(), xTaskResume(), or notification
   * functions.
   *
   * @note The state query is very lightweight and can be called frequently
   * without significant performance impact.
   *
   * @note Newly created tasks start in TaskStateSuspended state until
   * xTaskResume() is called.
   *
   * @note Tasks transition to TaskStateWaiting only when explicitly waiting for
   * notifications via xTaskNotifyTake() or xTaskNotifyWait().
   *
   * @sa TaskState_t - Enumeration of possible task states
   * @sa xTaskSuspend() - Transition task to suspended state
   * @sa xTaskResume() - Transition task to running state
   * @sa xTaskNotifyTake() - Wait for notification (transitions to waiting
   * state)
   * @sa xTaskNotifyGive() - Send notification (may exit waiting state)
   * @sa xTaskGetTaskInfo() - Get comprehensive task information including state
   */
  Return_t xTaskGetTaskState(const Task_t *task_, TaskState_t *state_);


  /**
   * @brief Retrieve the ASCII name string of a task
   *
   * Obtains the human-readable ASCII name assigned to a task when it was
   * created with xTaskCreate(). Task names are essential for debugging,
   * logging, and identifying tasks in system diagnostics. This function
   * allocates and returns a copy of the task's name string.
   *
   * Task names in HeliOS have a fixed length defined by CONFIG_TASK_NAME_BYTES
   * (default is 8 bytes). When tasks are created, names shorter than this
   * length should be space-padded, and names longer than this length are
   * truncated. The returned name buffer is exactly CONFIG_TASK_NAME_BYTES in
   * size.
   *
   * The name buffer is allocated from the user heap using xMemAlloc() and must
   * be freed by the caller using xMemFree() to prevent memory leaks. This is a
   * common pattern in HeliOS for returning variable-length or dynamically
   * allocated data to the caller.
   *
   * Task names are particularly useful for:
   * - **Debug output**: Including readable task names in printf/logging
   * statements
   * - **Task identification**: Finding specific tasks without hard-coding task
   * IDs
   * - **Error reporting**: Identifying which task encountered an error
   * - **System monitoring**: Displaying human-readable task information in
   * monitoring interfaces
   * - **Reverse lookup**: Verifying you have the correct task handle
   *
   * The name string is a direct copy of what was provided to xTaskCreate(), so
   * any padding or truncation that occurred during creation is preserved in the
   * returned string.
   *
   * **Common use cases:**
   * - Debug logging: Include task names in diagnostic output
   * - Error reporting: Identify tasks in error messages
   * - Task verification: Confirm you have the correct task handle
   * - System monitoring: Display task names in status reports
   * - Task enumeration: Build lists of task names for user interfaces
   * - Reverse lookup: Verify task handle corresponds to expected name
   *
   * Example 1: Basic task name retrieval for debugging
   * @code void debugTask(Task_t *task) {
   *   Byte_t *name = NULL;
   *
   *   if (OK(xTaskGetName(task, &name))) {
   *     printf("Task name: %.*s\n", CONFIG_TASK_NAME_BYTES, name);
   *     xMemFree(name);
   *   } else {
   *     printf("ERROR: Failed to get task name\n");
   *   }
   * }
   * @endcode
   *
   * Example 2: Error reporting with task name
   * @code void reportTaskError(Task_t *task, const char *errorMsg) {
   *   Byte_t *taskName = NULL;
   *
   *   if (OK(xTaskGetName(task, &taskName))) {
   *     printf("ERROR in task '%.*s': %s\n", CONFIG_TASK_NAME_BYTES, taskName,
   * errorMsg);
   *     xMemFree(taskName);
   *   } else {
   *     printf("ERROR in unknown task: %s\n", errorMsg);
   *   }
   * }
   * @endcode
   *
   * Example 3: Verify task handle corresponds to expected name
   * @code Return_t verifyTaskHandle(Task_t *task, const char *expectedName) {
   *   Byte_t *actualName = NULL;
   *
   *   if (ERROR(xTaskGetName(task, &actualName))) {
   *     return ReturnError;  // Invalid handle
   *   }
   *
   *   // Compare names (using strncmp for safety) Return_t result = ReturnOK;
   *   if (strncmp((char*)actualName, expectedName, CONFIG_TASK_NAME_BYTES) !=
   * 0) {
   *     printf("WARNING: Expected task '%s' but got '%.8s'\n", expectedName,
   * actualName);
   *     result = ReturnError;
   *   }
   *
   *   xMemFree(actualName);
   *   return result;
   * }
   * @endcode
   *
   * Example 4: Build list of all task names
   * @code void listAllTaskNames(void) {
   *   Base_t taskCount = 0;
   *
   *   if (ERROR(xTaskGetNumberOfTasks(&taskCount))) {
   *     return;
   *   }
   *
   *   printf("System Tasks (%d total):\n", taskCount);
   *
   *   // Get all task info TaskInfo_t **taskInfo = NULL;
   *   if (OK(xTaskGetAllTaskInfo(&taskInfo, &taskCount))) {
   *     for (Base_t i = 0; i < taskCount; i++) {
   *       printf("  %d. %.*s (ID: %d)\n", i + 1, CONFIG_TASK_NAME_BYTES,
   * taskInfo[i].name, taskInfo[i].id);
   *     }
   *     xMemFree(taskInfo);
   *   }
   * }
   * @endcode
   *
   * @param[in]  task_ Task handle for the task to query. Must be a valid task
   *                   handle obtained from xTaskCreate() or
   *                   xTaskGetHandleByName(). Must not be NULL or refer to a
   *                   deleted task.
   * @param[out] name_ Pointer to Byte_t pointer that will receive the allocated
   *                   name buffer. Must not be NULL. On success, points to
   *                   allocated buffer of exactly CONFIG_TASK_NAME_BYTES size
   *                   that must be freed with xMemFree(). On failure, remains
   *                   unchanged.
   *
   * @return           ReturnOK if name was successfully retrieved and
   *                   allocated. ReturnError if the operation failed (e.g.,
   *                   task_ is NULL or invalid, name_ is NULL, memory
   *                   allocation failed, task has been deleted).
   *
   * @warning The caller MUST free the returned name buffer using xMemFree().
   * Failure to do so will leak memory from the user heap.
   *
   * @warning The returned name is exactly CONFIG_TASK_NAME_BYTES in length. It
   * may NOT be null-terminated if the original name was exactly that length.
   * Always use length-limited string functions (strncmp, snprintf with width)
   * when working with task names.
   *
   * @warning The task handle must be valid. If the task has been deleted or the
   * handle is corrupted, this function returns ReturnError.
   *
   * @warning Do not modify the task's name after creation. Task names are
   * immutable in HeliOS. This function returns a copy - modifying the copy does
   * not affect the task's actual name.
   *
   * @note The name buffer is allocated using xMemAlloc() from the user heap.
   * Ensure adequate heap space is available.
   *
   * @note Task names are set during xTaskCreate() and cannot be changed
   * afterward. This function only retrieves the existing name.
   *
   * @note The CONFIG_TASK_NAME_BYTES configuration (default 8) determines the
   * exact size of all task names. This is a compile-time constant.
   *
   * @note For efficient lookup by name without allocating memory, use
   * xTaskGetHandleByName() which searches for tasks by name string.
   *
   * @note The returned name may contain spaces if the original name was shorter
   * than CONFIG_TASK_NAME_BYTES and was space-padded during creation.
   *
   * @sa xTaskCreate() - Assigns the task name during creation
   * @sa xTaskGetHandleByName() - Find task by name (reverse operation)
   * @sa xMemFree() - Must be used to free the returned buffer
   * @sa xTaskGetTaskInfo() - Get comprehensive info including name
   * @sa CONFIG_TASK_NAME_BYTES - Defines task name length
   */
  Return_t xTaskGetName(const Task_t *task_, Byte_t **name_);


  /**
   * @brief Retrieve the unique numeric identifier of a task
   *
   * Obtains the unique integer ID assigned to a task when it was created by
   * xTaskCreate(). Each task in HeliOS has a unique numeric identifier that
   * remains constant throughout the task's lifetime. Task IDs are useful for
   * compact task identification, array indexing, and logging where numeric
   * values are more efficient than name strings.
   *
   * Task IDs are automatically assigned by HeliOS during task creation and
   * cannot be changed. The ID assignment is typically sequential, starting from
   * 1 (or 0 depending on implementation), but the exact assignment mechanism
   * should be treated as opaque. Task IDs are never reused during a single boot
   * cycle, even if tasks are deleted.
   *
   * Unlike task names (which are strings of fixed length
   * CONFIG_TASK_NAME_BYTES), task IDs are simple integers (Base_t type) that
   * require minimal storage and can be efficiently compared, logged, or used as
   * array indices. This makes IDs ideal for performance-critical code or
   * space-constrained logging.
   *
   * Task IDs complement task names and handles:
   * - **Task Handle (Task_t *)**: Opaque pointer used for all task operations
   * - **Task Name (string)**: Human-readable identifier for debugging
   * - **Task ID (integer)**: Compact numeric identifier for efficient
   * processing
   *
   * The ID can be used with xTaskGetHandleById() to perform reverse lookup,
   * converting from numeric ID back to task handle. This is useful when task
   * IDs are stored in data structures or received over communication channels.
   *
   * **Common use cases:**
   * - Compact logging: Store task IDs in logs instead of full names
   * - Array indexing: Use task IDs as indices into task-specific data arrays
   * - Efficient comparison: Compare integers instead of string names
   * - Cross-reference: Store task IDs in data structures for later lookup
   * - Performance tracking: Associate numeric IDs with performance metrics
   * - Communication protocols: Send task IDs over serial/network interfaces
   *
   * Example 1: Basic task ID retrieval
   * @code void printTaskId(Task_t *task) {
   *   Base_t taskId;
   *
   *   if (OK(xTaskGetId(task, &taskId))) {
   *     printf("Task ID: %d\n", taskId);
   *   } else {
   *     printf("ERROR: Invalid task handle\n");
   *   }
   * }
   * @endcode
   *
   * Example 2: Task-specific data array using IDs as indices
   * @code
   * #define MAX_TASKS 16 typedef struct {
   *   unsigned long executionCount;
   *   unsigned long errorCount;
   * } TaskMetrics;
   *
   * TaskMetrics taskMetrics[MAX_TASKS] = {0};
   *
   * void recordTaskExecution(Task_t *task) {
   *   Base_t taskId;
   *
   *   if (OK(xTaskGetId(task, &taskId)) && taskId < MAX_TASKS) {
   *     taskMetrics[taskId].executionCount++;
   *   }
   * }
   *
   * void recordTaskError(Task_t *task) {
   *   Base_t taskId;
   *
   *   if (OK(xTaskGetId(task, &taskId)) && taskId < MAX_TASKS) {
   *     taskMetrics[taskId].errorCount++;
   *   }
   * }
   * @endcode
   *
   * Example 3: Compact logging with task IDs
   * @code typedef struct {
   *   Base_t taskId;
   *   unsigned long timestamp;
   *   Byte_t eventCode;
   * } LogEntry;
   *
   * #define LOG_SIZE 128 LogEntry eventLog[LOG_SIZE];
   * int logIndex = 0;
   *
   * void logEvent(Task_t *task, Byte_t eventCode) {
   *   Base_t taskId;
   *
   *   if (OK(xTaskGetId(task, &taskId))) {
   *     eventLog[logIndex % LOG_SIZE].taskId = taskId;
   *     eventLog[logIndex % LOG_SIZE].timestamp = getSystemTime();
   *     eventLog[logIndex % LOG_SIZE].eventCode = eventCode;
   *     logIndex++;
   *   }
   * }
   *
   * void printLog(void) {
   *   for (int i = 0; i < LOG_SIZE; i++) {
   *     printf("Task %d: Event 0x%02X at %lu\n", eventLog[i].taskId,
   * eventLog[i].eventCode, eventLog[i].timestamp);
   *   }
   * }
   * @endcode
   *
   * Example 4: ID-based task lookup and verification
   * @code Return_t verifyTaskById(Base_t expectedId) {
   *   // Get handle by ID Task_t *task = NULL;
   *   if (ERROR(xTaskGetHandleById(&task, expectedId))) {
   *     printf("ERROR: No task with ID %d\n", expectedId);
   *     return ReturnError;
   *   }
   *
   *   // Verify ID matches (round-trip test) Base_t retrievedId;
   *   if (OK(xTaskGetId(task, &retrievedId))) {
   *     if (retrievedId == expectedId) {
   *       printf("Task ID %d verified\n", expectedId);
   *       return ReturnOK;
   *     }
   *   }
   *
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[in]  task_ Task handle for the task to query. Must be a valid task
   *                   handle obtained from xTaskCreate() or
   *                   xTaskGetHandleByName(). Must not be NULL or refer to a
   *                   deleted task.
   * @param[out] id_   Pointer to Base_t variable that will receive the task's
   *                   unique numeric identifier. Must not be NULL. On success,
   *                   contains the task ID. On failure, remains unchanged.
   *
   * @return           ReturnOK if ID was successfully retrieved. ReturnError if
   *                   the operation failed (e.g., task_ is NULL or invalid, id_
   *                   is NULL, task has been deleted).
   *
   * @warning The task handle must be valid. If the task has been deleted or the
   * handle is corrupted, this function returns ReturnError.
   *
   * @warning Task IDs should not be assumed to be sequential or start from any
   * specific value. Always treat IDs as opaque identifiers.
   *
   * @warning Task IDs are not reused within a single boot cycle, but may be
   * reused after a system reset. Do not persist task IDs across reboots.
   *
   * @warning When using task IDs as array indices, always validate the ID is
   * within array bounds before indexing. IDs may be larger than expected.
   *
   * @note Task IDs are assigned automatically during xTaskCreate() and cannot
   * be changed. They remain constant for the task's lifetime.
   *
   * @note Task IDs are lightweight integers (Base_t type) that require minimal
   * storage compared to name strings or handles.
   *
   * @note The ID is particularly useful in logging and telemetry where
   * bandwidth or storage is limited.
   *
   * @note For reverse lookup (ID to handle), use xTaskGetHandleById(). For name
   * lookup, use xTaskGetHandleByName().
   *
   * @note The Base_t type is typically a 16-bit or 32-bit integer depending on
   * platform. Check your platform's type definitions.
   *
   * @sa xTaskGetHandleById() - Get task handle from ID (reverse operation)
   * @sa xTaskGetName() - Get task name string
   * @sa xTaskGetTaskInfo() - Get comprehensive info including ID
   * @sa xTaskCreate() - Assigns task ID during creation
   */
  Return_t xTaskGetId(const Task_t *task_, Base_t *id_);


  /**
   * @brief Clear any pending direct-to-task notification for a task
   *
   * Clears (discards) any pending direct-to-task notification that is waiting
   * for the specified task. If a notification is waiting, it is removed and its
   * value is discarded. The task's notification state transitions from
   * "waiting"
   * to "no notification pending". This function is useful for resetting
   * notification state or discarding stale notifications.
   *
   * HeliOS implements a lightweight direct-to-task notification mechanism that
   * allows one task to signal another with an optional data payload. Each task
   * has a single notification "slot" that can hold one pending notification at
   * a time. When a notification is sent via xTaskNotifyGive(), it remains
   * pending until consumed via xTaskNotifyTake() or cleared with this function.
   *
   * Clearing notifications is useful in several scenarios:
   * - **State reset**: Clearing stale notifications before starting a new
   * operation
   * - **Error recovery**: Discarding notifications that arrived during error
   * conditions
   * - **Synchronization reset**: Clearing notifications to ensure clean
   * starting state
   * - **Spurious notification handling**: Discarding unexpected notifications
   *
   * If no notification is pending when this function is called, it has no
   * effect and still returns ReturnOK. The operation is idempotent - calling it
   * multiple times produces the same result as calling it once.
   *
   * The cleared notification's value (if any) is discarded and cannot be
   * retrieved. If you need the notification value, use xTaskNotifyTake()
   * instead.
   *
   * **Common use cases:**
   * - Initialization: Clear any stale notifications before task begins
   * operation
   * - Error recovery: Discard notifications from failed operations
   * - State machine reset: Clear notifications when resetting to initial state
   * - Timeout handling: Clear notifications after waiting period expires
   * - Synchronization cleanup: Remove notifications during shutdown sequences
   * - Protocol reset: Clear notifications when restarting communication
   * protocol
   *
   * Example 1: Clear notification before starting operation
   * @code void processData(Task_t *task, TaskParm_t *parm) {
   *   // Clear any stale notifications from previous iterations
   * xTaskNotifyStateClear(task);
   *
   *   // Perform data processing doWork();
   *
   *   // Now wait for fresh notification TaskNotification_t *notification;
   *   if (OK(xTaskNotifyTake(task, &notification))) {
   *     // Process notification value
   *   }
   * }
   * @endcode
   *
   * Example 2: Error recovery with notification cleanup
   * @code Return_t recoverFromError(Task_t *task) {
   *   // Clear any notifications that arrived during error state if
   * (ERROR(xTaskNotifyStateClear(task))) {
   *     return ReturnError;
   *   }
   *
   *   // Reset task state resetTaskState();
   *
   *   // Task ready for fresh operation return ReturnOK;
   * }
   * @endcode
   *
   * Example 3: State machine reset
   * @code typedef enum {
   *   STATE_IDLE, STATE_PROCESSING, STATE_ERROR
   * } TaskState;
   *
   * TaskState currentState = STATE_IDLE;
   *
   * void resetStateMachine(Task_t *task) {
   *   // Clear any pending notifications xTaskNotifyStateClear(task);
   *
   *   // Reset state to idle currentState = STATE_IDLE;
   *
   *   printf("State machine reset to IDLE\n");
   * }
   * @endcode
   *
   * Example 4: Timeout with notification discard
   * @code void taskWithTimeout(Task_t *task, TaskParm_t *parm) {
   *   static int waitCounter = 0;
   *   TaskNotification_t *notification;
   *   Base_t isWaiting;
   *
   *   // Check if notification is pending if
   * (OK(xTaskNotificationIsWaiting(task, &isWaiting)) && isWaiting) {
   *     // Notification available - process it if (OK(xTaskNotifyTake(task,
   * &notification))) {
   *       processNotification(&notification);
   *       waitCounter = 0;
   *     }
   *   } else {
   *     // No notification - increment timeout counter waitCounter++;
   *     if (waitCounter > 100) {
   *       printf("Timeout waiting for notification\n");
   *       // Clear any notifications and reset xTaskNotifyStateClear(task);
   *       waitCounter = 0;
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in] task_ Task handle for the task whose notification state should
   *                  be cleared. Must be a valid task handle obtained from
   *                  xTaskCreate() or xTaskGetHandleByName(). Must not be NULL
   *                  or refer to a deleted task.
   *
   * @return          ReturnOK if the notification state was successfully
   *                  cleared (or if no notification was pending). ReturnError
   *                  if the operation failed (e.g., task_ is NULL or invalid,
   *                  task has been deleted).
   *
   * @warning The task handle must be valid. If the task has been deleted or the
   * handle is corrupted, this function returns ReturnError.
   *
   * @warning Any pending notification value is permanently discarded and cannot
   * be retrieved. Use xTaskNotifyTake() if you need to access the notification
   * value before clearing it.
   *
   * @warning This function only clears the notification state. It does NOT
   * change the task's execution state (running/suspended/waiting). A task in
   * TaskStateWaiting remains in that state after clearing notifications.
   *
   * @warning Calling this on a task that is currently blocked in
   * xTaskNotifyTake() does not unblock the task. It only clears pending
   * notifications, not the waiting state itself.
   *
   * @note This function is idempotent - calling it multiple times has the same
   * effect as calling it once. It always returns ReturnOK for valid tasks,
   * whether or not a notification was actually pending.
   *
   * @note Each task has only one notification slot. There is no queue - only
   * the most recent notification is stored. Clearing removes this single
   * pending notification.
   *
   * @note This is a lightweight operation with minimal overhead. It can be
   * called frequently without performance concerns.
   *
   * @note To check if a notification is pending before clearing, use
   * xTaskNotificationIsWaiting().
   *
   * @sa xTaskNotifyGive() - Send a notification to a task
   * @sa xTaskNotifyTake() - Receive and consume a notification
   * @sa xTaskNotificationIsWaiting() - Check if notification is pending
   * @sa TaskNotification_t * - Structure containing notification data
   */
  Return_t xTaskNotifyStateClear(Task_t *task_);


  /**
   * @brief Check if a direct-to-task notification is pending for a task
   *
   * Queries whether a task has a pending direct-to-task notification waiting to
   * be consumed. Returns a boolean result indicating notification presence
   * without actually consuming the notification. This non-destructive query is
   * useful for polling notification status or implementing conditional logic
   * based on notification availability.
   *
   * HeliOS's direct-to-task notification system provides a lightweight
   * mechanism for task-to-task signaling. Each task has a single notification
   * slot that can hold one pending notification sent via xTaskNotifyGive().
   * This function checks whether that slot currently contains a pending
   * notification without removing it or accessing its value.
   *
   * The result is a simple boolean indicator (non-zero = notification waiting,
   * zero = no notification). Unlike xTaskNotifyTake() which blocks or returns
   * the notification value, this function only queries the notification state
   * and always returns immediately.
   *
   * Common scenarios for checking notification status:
   * - **Polling patterns**: Periodically check for notifications without
   * blocking
   * - **Conditional processing**: Execute different code paths based on
   * notification presence
   * - **Non-blocking checks**: Determine if notification is available before
   * calling xTaskNotifyTake()
   * - **Diagnostic monitoring**: Track notification patterns for debugging
   * - **Priority handling**: Process notifications only when available
   *
   * **Common use cases:**
   * - Poll for notifications: Check periodically without blocking
   * - Conditional logic: Branch based on notification availability
   * - Diagnostic monitoring: Log notification arrival patterns
   * - Performance optimization: Avoid blocking on xTaskNotifyTake() when no
   * notification exists
   * - Multi-source handling: Check multiple tasks for pending notifications
   * - Timeout implementation: Combine with counters for custom timeout behavior
   *
   * Example 1: Poll for notification without blocking
   * @code void pollingTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t hasNotification;
   *
   *   // Check if notification is waiting if
   * (OK(xTaskNotificationIsWaiting(task, &hasNotification))) {
   *     if (hasNotification) {
   *       // Notification present - process it TaskNotification_t notification;
   *       if (OK(xTaskNotifyTake(task, &notification))) {
   *         processNotification(&notification);
   *       }
   *     } else {
   *       // No notification - do other work performBackgroundTasks();
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Conditional processing based on notification
   * @code void smartTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t hasWork;
   *
   *   if (OK(xTaskNotificationIsWaiting(task, &hasWork)) && hasWork) {
   *     // High-priority: Process notification first TaskNotification_t *notif;
   *     xTaskNotifyTake(task, &notif);
   *     handleUrgentWork(&notif);
   *   } else {
   *     // Low-priority: Do regular housekeeping performMaintenanceTasks();
   *   }
   * }
   * @endcode
   *
   * Example 3: Multi-task notification monitoring
   * @code
   * // Global task handles Task_t *sensorTask, commTask, storageTask;
   *
   * void monitorNotifications(void) {
   *   Base_t pending;
   *   int totalPending = 0;
   *
   *   if (OK(xTaskNotificationIsWaiting(sensorTask, &pending)) && pending) {
   *     printf("Sensor task has pending notification\n");
   *     totalPending++;
   *   }
   *
   *   if (OK(xTaskNotificationIsWaiting(commTask, &pending)) && pending) {
   *     printf("Comm task has pending notification\n");
   *     totalPending++;
   *   }
   *
   *   if (OK(xTaskNotificationIsWaiting(storageTask, &pending)) && pending) {
   *     printf("Storage task has pending notification\n");
   *     totalPending++;
   *   }
   *
   *   printf("Total pending notifications: %d\n", totalPending);
   * }
   * @endcode
   *
   * Example 4: Custom timeout with notification check
   * @code
   * #define NOTIFICATION_TIMEOUT 1000  // 1000 task iterations
   *
   * void taskWithCustomTimeout(Task_t *task, TaskParm_t *parm) {
   *   static int waitCount = 0;
   *   Base_t hasNotification;
   *
   *   if (OK(xTaskNotificationIsWaiting(task, &hasNotification))) {
   *     if (hasNotification) {
   *       // Notification arrived - process it TaskNotification_t *notif;
   *       xTaskNotifyTake(task, &notif);
   *       handleNotification(&notif);
   *       waitCount = 0;  // Reset timeout
   *     } else {
   *       // No notification yet - check timeout waitCount++;
   *       if (waitCount >= NOTIFICATION_TIMEOUT) {
   *         printf("Timeout: No notification received in %d cycles\n",
   * waitCount);
   *         handleTimeout();
   *         waitCount = 0;
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  task_ Task handle for the task to query. Must be a valid task
   *                   handle obtained from xTaskCreate() or
   *                   xTaskGetHandleByName(). Must not be NULL or refer to a
   *                   deleted task.
   * @param[out] res_  Pointer to Base_t variable that will receive the result.
   *                   Must not be NULL. On success, set to non-zero if a
   *                   notification is pending, or zero if no notification is
   *                   pending. On failure, remains unchanged.
   *
   * @return           ReturnOK if the query was successful and res_ was
   *                   updated. ReturnError if the operation failed (e.g., task_
   *                   is NULL or invalid, res_ is NULL, task has been deleted).
   *
   * @warning The task handle must be valid. If the task has been deleted or the
   * handle is corrupted, this function returns ReturnError.
   *
   * @warning The result is a snapshot in time. The notification state may
   * change immediately after this function returns if another task sends or
   * clears a notification.
   *
   * @warning This function only checks for notification presence. It does NOT
   * retrieve the notification value or remove the notification. Use
   * xTaskNotifyTake() to actually consume the notification.
   *
   * @warning Do not use tight polling loops based solely on this function -
   * this wastes CPU cycles. Consider using event-driven patterns or periodic
   * checks within a task's normal execution cycle.
   *
   * @note This is a non-blocking, non-destructive query. The notification (if
   * present) remains pending after this call.
   *
   * @note The returned value is boolean: non-zero = notification waiting, zero
   * =
   * no notification. The exact non-zero value should not be relied upon.
   *
   * @note This function is lightweight and can be called frequently without
   * significant performance impact.
   *
   * @note Each task has only one notification slot. Multiple notifications sent
   * before consumption will overwrite each other - only the most recent is
   * available.
   *
   * @sa xTaskNotifyGive() - Send a notification to a task
   * @sa xTaskNotifyTake() - Receive and consume a notification
   * @sa xTaskNotifyStateClear() - Clear a pending notification without reading
   * it
   * @sa TaskNotification_t * - Structure containing notification data
   */
  Return_t xTaskNotificationIsWaiting(const Task_t *task_, Base_t *res_);


  /**
   * @brief Send a direct-to-task notification with optional data payload
   *
   * Sends a lightweight direct-to-task notification to the specified task,
   * optionally including a small data payload of up to
   * CONFIG_NOTIFICATION_VALUE_BYTES (default 8 bytes). This provides efficient
   * task-to-task signaling without the overhead of message queues. If the task
   * is waiting for a notification (TaskStateWaiting), this unblocks it and
   * transitions it to TaskStateRunning.
   *
   * HeliOS's notification mechanism is optimized for simple signaling and small
   * data transfers between tasks. Each task has a single notification slot that
   * stores one pending notification. If a notification is sent while one is
   * already pending, the old notification is overwritten with the new one -
   * there is no queuing.
   *
   * The notification value is optional - you can send a zero-length
   * notification (bytes_=0, value_=NULL) purely as a wake-up signal, or include
   * up to CONFIG_NOTIFICATION_VALUE_BYTES of data. Common uses include sending
   * status codes, event flags, sensor readings, or pointers to shared data
   * structures.
   *
   * **Notification semantics:**
   * - **Overwrite behavior**: New notifications overwrite pending ones
   * - **Lightweight**: No memory allocation, minimal overhead
   * - **Task wakeup**: Transitions waiting tasks to running state
   * - **Data payload**: Optional byte array up to
   * CONFIG_NOTIFICATION_VALUE_BYTES
   *
   * **Common use cases:**
   * - Event signaling: Notify tasks of events without data transfer
   * - Semaphore emulation: Use zero-length notifications as binary semaphores
   * - Status updates: Send small status codes or flags between tasks
   * - Data ready signals: Notify when shared data is ready for processing
   * - Synchronization: Coordinate task execution sequences
   * - Interrupt-to-task: Signal tasks from ISRs (if port supports it)
   *
   * Example 1: Simple event notification (zero-length)
   * @code
   * // Producer task signals consumer when data is ready Task_t *consumerTask =
   * NULL;  // Obtained during initialization
   *
   * void producerTask(Task_t *task, TaskParm_t *parm) {
   *   // Produce data prepareData();
   *
   *   // Signal consumer (no data payload needed) if
   * (OK(xTaskNotifyGive(consumerTask, 0, NULL))) {
   *     printf("Consumer notified\n");
   *   }
   * }
   * @endcode
   *
   * Example 2: Notification with status code
   * @code typedef enum {
   *   STATUS_SUCCESS = 0, STATUS_WARNING = 1, STATUS_ERROR = 2
   * } StatusCode;
   *
   * void sendStatus(Task_t *targetTask, StatusCode status) {
   *   Byte_t statusByte = (Byte_t)status;
   *
   *   if (ERROR(xTaskNotifyGive(targetTask, 1, &statusByte))) {
   *     printf("ERROR: Failed to send status notification\n");
   *   }
   * }
   * @endcode
   *
   * Example 3: Send sensor reading
   * @code void sensorTask(Task_t *task, TaskParm_t *parm) {
   *   Task_t **displayTask = (Task_t **)parm;
   *
   *   // Read temperature sensor (16-bit value) uint16_t temperature =
   * readTemperatureSensor();
   *
   *   // Send as notification (2 bytes) xTaskNotifyGive(*displayTask,
   * sizeof(temperature), (Byte_t*)&temperature);
   * }
   * @endcode
   *
   * Example 4: Producer-consumer pattern
   * @code
   * #define BUFFER_SIZE 128 Byte_t sharedBuffer[BUFFER_SIZE];
   * Task_t *processorTask;
   *
   * void dataCollectorTask(Task_t *task, TaskParm_t *parm) {
   *   // Fill buffer with data Base_t bytesCollected =
   * collectDataIntoBuffer(sharedBuffer, BUFFER_SIZE);
   *
   *   if (bytesCollected > 0) {
   *     // Notify processor with byte count xTaskNotifyGive(processorTask,
   * sizeof(bytesCollected), (Byte_t*)&bytesCollected);
   *   }
   * }
   *
   * void dataProcessorTask(Task_t *task, TaskParm_t *parm) {
   *   TaskNotification_t *notif;
   *
   *   if (OK(xTaskNotifyTake(task, &notif))) {
   *     // Extract byte count from notification Base_t byteCount =
   * *((Base_t*)notif.value);
   *
   *     // Process that many bytes from shared buffer processData(sharedBuffer,
   * byteCount);
   *   }
   * }
   * @endcode
   *
   * @param[in] task_  Task handle for the task to notify. Must be a valid task
   *                   handle obtained from xTaskCreate() or
   *                   xTaskGetHandleByName(). Must not be NULL or refer to a
   *                   deleted task.
   * @param[in] bytes_ Number of bytes in the notification value payload. Must
   *                   be 0 to CONFIG_NOTIFICATION_VALUE_BYTES (default 8). Use
   *                   0 for simple event notifications without data.
   * @param[in] value_ Pointer to byte array containing the notification
   *                   payload, or NULL if bytes_=0. If non-NULL, must point to
   *                   valid memory of at least bytes_ length. The data is
   *                   copied - the caller retains ownership of this memory.
   *
   * @return           ReturnOK if notification was successfully sent.
   *                   ReturnError if the operation failed (e.g., task_ is NULL
   *                   or invalid, bytes_ exceeds
   *                   CONFIG_NOTIFICATION_VALUE_BYTES, task has been deleted).
   *
   * @warning If a notification is already pending for the task, it will be
   * overwritten by the new notification. There is no queuing - only the most
   * recent notification is stored.
   *
   * @warning The bytes_ parameter must not exceed
   * CONFIG_NOTIFICATION_VALUE_BYTES. Exceeding this limit results in
   * ReturnError.
   *
   * @warning The task handle must be valid. If the task has been deleted or the
   * handle is corrupted, this function returns ReturnError.
   *
   * @warning The notification value is copied into the task's notification
   * slot. Changes to the source buffer after this call do not affect the
   * notification.
   *
   * @note This function can be called from any task or (if port supports)
   * interrupt service routine to signal other tasks.
   *
   * @note If the target task is in TaskStateWaiting (blocked in
   * xTaskNotifyTake()), this notification immediately unblocks it and
   * transitions it to TaskStateRunning.
   *
   * @note Zero-length notifications (bytes_=0, value_=NULL) are valid and
   * useful for simple event signaling without data transfer.
   *
   * @note The CONFIG_NOTIFICATION_VALUE_BYTES configuration (default 8)
   * determines the maximum payload size. This is a compile-time constant.
   *
   * @sa xTaskNotifyTake() - Receive and consume a notification
   * @sa xTaskNotificationIsWaiting() - Check if notification is pending
   * @sa xTaskNotifyStateClear() - Clear a notification without reading it
   * @sa TaskNotification_t * - Structure containing notification data
   * @sa CONFIG_NOTIFICATION_VALUE_BYTES - Maximum notification payload size
   */
  Return_t xTaskNotifyGive(Task_t *task_, const Base_t bytes_, const Byte_t *value_);


  /**
   * @brief Receive and consume a pending direct-to-task notification
   *
   * Retrieves and consumes a pending direct-to-task notification sent via
   * xTaskNotifyGive(). If a notification is waiting, it is removed from the
   * task's notification slot and its value is returned. If no notification is
   * pending, the function returns ReturnError immediately (non-blocking
   * behavior in HeliOS cooperative scheduler).
   *
   * The received notification is packaged in a TaskNotification_t *structure
   * containing the notification value (byte array) and the number of bytes.
   * This structure must be examined to extract the notification data sent by
   * the notifying task.
   *
   * Taking a notification is a destructive operation - the notification is
   * removed from the task's notification slot after being retrieved. If no new
   * notifications arrive, subsequent calls to this function will return
   * ReturnError until another notification is sent.
   *
   * In HeliOS's cooperative multitasking model, tasks are not pre-emptively
   * interrupted. Therefore, this function does not block waiting for
   * notifications - it either returns the pending notification immediately or
   * returns ReturnError if none is available. Tasks typically poll for
   * notifications during their execution cycle or use
   * xTaskNotificationIsWaiting() to check availability first.
   *
   * **Notification lifecycle:**
   * 1. Sender calls xTaskNotifyGive() - notification becomes pending 2.
   * Receiver calls xTaskNotifyTake() - notification is consumed and removed 3.
   * If no notification pending, xTaskNotifyTake() returns ReturnError
   *
   * **Common use cases:**
   * - Event-driven processing: Wait for and process notifications from other
   * tasks
   * - Producer-consumer: Receive data-ready signals from producer tasks
   * - Status reception: Receive status codes or flags from other tasks
   * - Synchronization: Coordinate task execution with notification handshakes
   * - ISR-to-task: Receive signals from interrupt handlers (port-dependent)
   * - Command reception: Receive small command codes or parameters
   *
   * Example 1: Simple notification reception
   * @code void consumerTask(Task_t *task, TaskParm_t *parm) {
   *   TaskNotification_t *notification;
   *
   *   // Try to receive notification if (OK(xTaskNotifyTake(task,
   * &notification))) {
   *     printf("Received notification with %d bytes\n", notification.bytes);
   *     // Process notification
   *   } else {
   *     // No notification - do other work performBackgroundWork();
   *   }
   * }
   * @endcode
   *
   * Example 2: Extract status code from notification
   * @code typedef enum {
   *   CMD_START = 1, CMD_STOP = 2, CMD_RESET = 3
   * } CommandCode;
   *
   * void commandTask(Task_t *task, TaskParm_t *parm) {
   *   TaskNotification_t *notif;
   *
   *   if (OK(xTaskNotifyTake(task, &notif)) && notif.bytes >= 1) {
   *     CommandCode cmd = (CommandCode)notif.value[0];
   *
   *     switch (cmd) {
   *       case CMD_START:
   *         startOperation();
   *         break;
   *       case CMD_STOP:
   *         stopOperation();
   *         break;
   *       case CMD_RESET:
   *         resetOperation();
   *         break;
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Producer-consumer with byte count
   * @code extern Byte_t sharedBuffer[256];
   *
   * void processorTask(Task_t *task, TaskParm_t *parm) {
   *   TaskNotification_t *notif;
   *
   *   if (OK(xTaskNotifyTake(task, &notif))) {
   *     // Notification contains number of valid bytes in shared buffer if
   * (notif.bytes == sizeof(Base_t)) {
   *       Base_t byteCount = *((Base_t*)notif.value);
   *       printf("Processing %d bytes from buffer\n", byteCount);
   *
   *       // Process that many bytes for (Base_t i = 0; i < byteCount; i++) {
   *         processBytes(sharedBuffer[i]);
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 4: Multi-value notification with structured data
   * @code typedef struct {
   *   uint8_t sensorId;
   *   uint16_t value;
   *   uint8_t status;
   * } __attribute__((packed)) SensorData;  // 4 bytes total
   *
   * void monitorTask(Task_t *task, TaskParm_t *parm) {
   *   TaskNotification_t *notif;
   *
   *   if (OK(xTaskNotifyTake(task, &notif))) {
   *     if (notif.bytes == sizeof(SensorData)) {
   *       SensorData *data = (SensorData*)notif.value;
   *       printf("Sensor %d: value=%u, status=%u\n", data->sensorId,
   * data->value, data->status);
   *
   *       // Take action based on sensor reading if (data->value > THRESHOLD) {
   *         triggerAlarm();
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  task_         Task handle for the task receiving the
   *                           notification. Must be a valid task handle
   *                           obtained from xTaskCreate() or
   *                           xTaskGetHandleByName(). Must not be NULL or refer
   *                           to a deleted task.
   * @param[out] notification_ Pointer to TaskNotification_t *structure that
   *                           will receive the notification data. Must not be
   *                           NULL. On success, contains the notification value
   *                           and byte count. On failure, remains unchanged.
   *
   * @return                   ReturnOK if a notification was pending and has
   *                           been retrieved. ReturnError if no notification
   *                           was pending, or task_ is NULL/invalid, or
   *                           notification_ is NULL, or task has been deleted.
   *
   * @warning This function is non-blocking. If no notification is pending, it
   * returns ReturnError immediately. There is no blocking/waiting behavior in
   * HeliOS's cooperative model.
   *
   * @warning The task handle must be valid. If the task has been deleted or the
   * handle is corrupted, this function returns ReturnError.
   *
   * @warning Taking a notification is destructive - it removes the notification
   * from the task's slot. You cannot retrieve the same notification twice.
   *
   * @warning The notification value is embedded in the TaskNotification_t *
   * structure. Examine notification_.bytes to determine how many bytes of
   * notification_.value are valid.
   *
   * @warning The notification value array has a maximum size of
   * CONFIG_NOTIFICATION_VALUE_BYTES (default 8). Ensure your code handles
   * notifications of varying lengths correctly.
   *
   * @note HeliOS uses cooperative multitasking - tasks are not preempted. This
   * function returns immediately whether or not a notification is available.
   *
   * @note Each task has only one notification slot. Multiple pending
   * notifications are not queued - only the most recent notification is
   * available.
   *
   * @note Zero-length notifications (bytes=0) are valid. Check
   * notification_.bytes to determine if a value payload is included.
   *
   * @note The TaskNotification_t *structure is not dynamically allocated - it's
   * filled in by this function. No memory management is required.
   *
   * @note To check for notification availability before taking, use
   * xTaskNotificationIsWaiting().
   *
   * @sa xTaskNotifyGive() - Send a notification to a task
   * @sa xTaskNotificationIsWaiting() - Check if notification is pending
   * @sa xTaskNotifyStateClear() - Clear a notification without reading it
   * @sa TaskNotification_t * - Structure containing notification data
   * @sa CONFIG_NOTIFICATION_VALUE_BYTES - Maximum notification payload size
   */
  Return_t xTaskNotifyTake(Task_t *task_, TaskNotification_t **notification_);


  /**
   * @brief Activate a task for scheduler execution
   *
   * Transitions a task to the TaskStateRunning state, making it eligible for
   * execution by the cooperative scheduler. Tasks in the running state are
   * scheduled according to their configured period and will execute
   * continuously until suspended, deleted, or transitioned to waiting state.
   *
   * When a task is created with xTaskCreate(), it begins in TaskStateSuspended
   * and will not execute until this function is called. This allows tasks to be
   * fully configured (period, parameters, etc.) before activation.
   *
   * State Transition:
   * - TaskStateSuspended → TaskStateRunning: Task becomes active
   * - TaskStateWaiting → TaskStateRunning: Task resumes normal scheduling
   * - TaskStateRunning → TaskStateRunning: No effect (already running)
   *
   * Scheduling Behavior:
   * Once resumed, the task will be scheduled for execution based on its period
   * setting. Tasks with period 0 execute on every scheduler cycle. Tasks with
   * non-zero periods execute when their elapsed time exceeds the period.
   *
   * @note This function can be called before or during scheduler execution. It
   * safely transitions the task state and the change takes effect on the next
   * scheduling cycle.
   *
   * @note A task in the running state will continue executing until explicitly
   * suspended with xTaskSuspend(), transitioned to waiting with xTaskWait(), or
   * deleted with xTaskDelete().
   *
   * Example Usage:
   * @code Task_t *sensorTask;
   * Task_t *displayTask;
   *
   * // Create tasks (both start in suspended state) xTaskCreate(&sensorTask,
   * "Sensor  ", readSensor, NULL);
   * xTaskCreate(&displayTask, "Display ", updateDisplay, NULL);
   *
   * // Configure task periods xTaskChangePeriod(sensorTask, 100);   // Run
   * every 100ms xTaskChangePeriod(displayTask, 500);  // Run every 500ms
   *
   * // Activate tasks xTaskResume(sensorTask);   // Now scheduled for execution
   * xTaskResume(displayTask);  // Now scheduled for execution
   *
   * // Start scheduler - both tasks will now execute xTaskStartScheduler();
   *
   * // Can also resume tasks while scheduler is running void controlTask(Task_t
   * *
   * task, TaskParm_t *parm) {
   *   if (systemReady) {
   *     xTaskResume(sensorTask);  // Activate sensor readings
   *   }
   * }
   * @endcode
   *
   * @param[in] task_ Handle of the task to resume. Must be a valid task handle
   *                  previously returned by xTaskCreate().
   *
   * @return          ReturnOK if the task state was successfully changed to
   *                  running, ReturnError if the operation failed (invalid task
   *                  handle or system error).
   *
   * @sa xTaskCreate() - Create a new task (starts in suspended state)
   * @sa xTaskSuspend() - Deactivate a task
   * @sa xTaskWait() - Place a task in event-waiting state
   * @sa xTaskGetTaskState() - Query current task state
   * @sa xTaskChangePeriod() - Set task execution frequency
   * @sa TaskState_t - Task state enumeration
   */
  Return_t xTaskResume(Task_t *task_);


  /**
   * @brief Deactivate a task to prevent scheduler execution
   *
   * Transitions a task to the TaskStateSuspended state, preventing it from
   * being scheduled for execution. The task remains registered with the
   * scheduler and retains all its configuration (period, parameters, etc.), but
   * will not execute until reactivated with xTaskResume().
   *
   * Suspending a task is useful for temporarily disabling functionality without
   * the overhead of deleting and recreating the task. Unlike xTaskWait(), which
   * waits for specific events, suspended tasks remain inactive indefinitely
   * until explicitly resumed.
   *
   * State Transition:
   * - TaskStateRunning → TaskStateSuspended: Task becomes inactive
   * - TaskStateWaiting → TaskStateSuspended: Task becomes inactive
   * - TaskStateSuspended → TaskStateSuspended: No effect (already suspended)
   *
   * Use Cases:
   * - Temporarily disable a task based on system state or mode
   * - Reduce CPU usage by deactivating unused functionality
   * - Disable tasks during power-saving modes
   * - Pause task execution during system reconfiguration
   *
   * @note This function can be called before or during scheduler execution.
   * When called on an active task, the task will not be scheduled on subsequent
   * scheduler cycles until resumed.
   *
   * @note Suspending a task does NOT free its resources or invalidate its
   * handle. The task can be resumed at any time with xTaskResume(). To
   * permanently remove a task, use xTaskDelete() instead.
   *
   * @note This function (xTaskSuspend) operates on individual tasks. For
   * suspending the entire scheduler, use xTaskSuspendAll() instead.
   *
   * Example Usage:
   * @code Task_t *bluetoothTask;
   * Task_t *wifiTask;
   *
   * xTaskCreate(&bluetoothTask, "BT Task", bluetoothHandler, NULL);
   * xTaskCreate(&wifiTask, "WiFi    ", wifiHandler, NULL);
   *
   * xTaskResume(bluetoothTask);
   * xTaskResume(wifiTask);
   * xTaskStartScheduler();
   *
   * // In a control task, disable wireless when not needed void
   * powerManagementTask(Task_t *task, TaskParm_t *parm) {
   *   if (lowPowerMode) {
   *     // Suspend wireless tasks to save power xTaskSuspend(bluetoothTask);
   *     xTaskSuspend(wifiTask);
   *   } else {
   *     // Resume wireless tasks when needed xTaskResume(bluetoothTask);
   *     xTaskResume(wifiTask);
   *   }
   * }
   * @endcode
   *
   * @param[in] task_ Handle of the task to suspend. Must be a valid task handle
   *                  previously returned by xTaskCreate().
   *
   * @return          ReturnOK if the task state was successfully changed to
   *                  suspended, ReturnError if the operation failed (invalid
   *                  task handle or system error).
   *
   * @sa xTaskCreate() - Create a new task
   * @sa xTaskResume() - Reactivate a suspended task
   * @sa xTaskWait() - Place a task in event-waiting state
   * @sa xTaskDelete() - Permanently remove a task
   * @sa xTaskSuspendAll() - Suspend the entire scheduler
   * @sa xTaskGetTaskState() - Query current task state
   * @sa TaskState_t - Task state enumeration
   */
  Return_t xTaskSuspend(Task_t *task_);


  /**
   * @brief Place a task in event-driven waiting state
   *
   * Transitions a task to the TaskStateWaiting state, where it will not be
   * scheduled for execution until a specific event occurs. This enables
   * efficient event-driven multitasking where tasks sleep until notified,
   * reducing unnecessary CPU usage.
   *
   * Unlike TaskStateSuspended (which requires explicit xTaskResume() to
   * reactivate), tasks in TaskStateWaiting automatically become schedulable
   * when their awaited event occurs. Once the event is consumed (via
   * xTaskNotifyTake() or xTaskNotificationStateClear()), the task automatically
   * returns to waiting state.
   *
   * State Transition:
   * - TaskStateRunning → TaskStateWaiting: Task enters event-waiting mode
   * - TaskStateSuspended → TaskStateWaiting: Task enters event-waiting mode
   * - TaskStateWaiting → TaskStateWaiting: No effect (already waiting)
   *
   * Supported Event Types:
   * 1. Task Timers: Task wakes when its timer expires (via xTaskResetTimer())
   * 2. Direct-to-Task Notifications: Task wakes when another task sends a
   * notification (via xTaskNotifyGive())
   *
   * Event-Driven Workflow:
   * 1. Task calls xTaskWait() to enter waiting state 2. Task stops executing
   * and does not consume CPU 3. Another task or timer triggers an event 4.
   * Scheduler automatically schedules the waiting task 5. Task executes and
   * processes the event 6. Task consumes the event (xTaskNotifyTake() or
   * xTaskNotificationStateClear()) 7. Task automatically returns to waiting
   * state
   *
   * @note Event-driven tasks are more efficient than polling-based tasks as
   * they only execute when needed, reducing CPU usage and power consumption.
   *
   * @note A task in waiting state will remain inactive until its event occurs.
   * To unconditionally activate a waiting task, use xTaskResume().
   *
   * @note The event mechanism is edge-triggered. If an event occurs before
   * xTaskWait() is called, the task will execute once to process it.
   *
   * Example Usage:
   * @code Task_t *buttonTask;
   * Task_t *ledTask;
   *
   * // Button task waits for button press notifications void
   * buttonHandler(Task_t *task, TaskParm_t *parm) {
   *   TaskNotification_t *notification;
   *
   *   // Wait for button press event xTaskWait(task);
   *
   *   // Check if notification is waiting if
   * (OK(xTaskNotificationIsWaiting(task, &notification))) {
   *     // Process button press if (OK(xTaskNotifyTake(task, &notification))) {
   *       handleButtonPress();
   *       // Task automatically returns to waiting state
   *     }
   *   }
   * }
   *
   * // Interrupt handler or another task sends notification void
   * buttonISR(void) {
   *   // Wake up button task xTaskNotifyGive(buttonTask);
   * }
   *
   * int main(void) {
   *   xTaskCreate(&buttonTask, "Button  ", buttonHandler, NULL);
   *   xTaskResume(buttonTask);  // Initially resume the task
   * xTaskStartScheduler();
   * }
   * @endcode
   *
   * @param[in] task_ Handle of the task to place in waiting state. Must be a
   *                  valid task handle previously returned by xTaskCreate().
   *
   * @return          ReturnOK if the task state was successfully changed to
   *                  waiting, ReturnError if the operation failed (invalid task
   *                  handle or system error).
   *
   * @sa xTaskCreate() - Create a new task
   * @sa xTaskResume() - Activate a task (overrides waiting state)
   * @sa xTaskSuspend() - Deactivate a task
   * @sa xTaskNotifyGive() - Send a notification to wake a waiting task
   * @sa xTaskNotifyTake() - Consume a notification event
   * @sa xTaskNotificationIsWaiting() - Check for pending notifications
   * @sa xTaskNotificationStateClear() - Clear notification state
   * @sa xTaskResetTimer() - Reset task timer for timer-based events
   * @sa TaskState_t - Task state enumeration
   */
  Return_t xTaskWait(Task_t *task_);


  /**
   * @brief Set the execution period for periodic task scheduling
   *
   * Changes the interval period that controls how frequently a task executes
   * under the HeliOS scheduler. The period determines the minimum time (in
   * system ticks) between successive executions of the task's callback
   * function. This enables periodic task execution for time-driven operations
   * like sensor sampling, LED blinking, or periodic communication.
   *
   * **Period semantics:**
   * - **Period = 0**: Task runs every scheduler cycle (maximum frequency)
   * - **Period > 0**: Task runs when elapsed ticks >= period
   *
   * The period is measured in system ticks, which are platform-dependent but
   * typically represent 1 millisecond. A task with period 100 executes
   * approximately every 100ms (10 Hz), assuming the scheduler runs at least
   * that frequently.
   *
   * Tasks created with xTaskCreate() default to period 0, meaning they run on
   * every scheduler cycle. Use this function to set periodic behavior after
   * creation. The period can be changed at any time - even while the scheduler
   * is running -
   * allowing dynamic adjustment of task execution frequency.
   *
   * **Common use cases:**
   * - Periodic sampling: Set sensor tasks to sample at regular intervals
   * - LED patterns: Control blink rates by adjusting period
   * - Communication timing: Schedule periodic transmissions
   * - Adaptive frequency: Dynamically adjust task rate based on workload
   * - Power management: Reduce update frequency to save power
   *
   * @param[in] task_   Task handle. Must be valid from xTaskCreate() or
   *                    xTaskGetHandleByName().
   * @param[in] period_ Execution period in ticks. 0 = every cycle, >0 =
   *                    periodic.
   *
   * @return            ReturnOK if period was set. ReturnError if task_
   *                    invalid.
   *
   * @warning Period 0 causes task to run every scheduler cycle, consuming
   * maximum CPU time. Use sparingly to avoid starving other tasks.
   *
   * @sa xTaskGetPeriod() - Query current task period
   * @sa xTaskCreate() - Tasks default to period 0
   */
  Return_t xTaskChangePeriod(Task_t *task_, const Ticks_t period_);


  /**
   * @brief Set task watchdog timeout for detecting runaway tasks
   *
   * Configures the per-task watchdog timer period that automatically suspends
   * tasks exceeding their execution time budget. This safety feature prevents
   * runaway tasks from monopolizing CPU time and starving other tasks in the
   * system.
   *
   * When a task's single execution exceeds the watchdog period, HeliOS
   * automatically suspends it (transitions to TaskStateSuspended), preventing
   * further execution until explicitly resumed. This protects system
   * responsiveness and prevents infinite loops or excessive computation from
   * blocking other tasks.
   *
   * **Watchdog semantics:**
   * - Monitors single execution duration, not cumulative time
   * - Automatically suspends tasks that exceed period
   * - Requires CONFIG_TASK_WD_TIMER_ENABLE at compile time
   * - Per-task configuration (each task has independent watchdog)
   *
   * **Common use cases:**
   * - Infinite loop protection: Catch tasks stuck in loops
   * - Execution time enforcement: Ensure tasks complete within budgets
   * - System stability: Prevent CPU monopolization
   * - Debug assistance: Identify performance problems
   *
   * @param[in] task_   Task handle. Must be valid.
   * @param[in] period_ Watchdog timeout in ticks. Task suspended if single
   *                    execution exceeds this duration.
   *
   * @return            ReturnOK if watchdog period set. ReturnError if task_
   *                    invalid or CONFIG_TASK_WD_TIMER_ENABLE not defined.
   *
   * @warning Requires CONFIG_TASK_WD_TIMER_ENABLE defined at compile time.
   * Without this, function has no effect.
   *
   * @warning Suspended tasks must be manually resumed with xTaskResume().
   * Watchdog does not auto-recover.
   *
   * @sa xTaskGetWDPeriod() - Query watchdog period
   * @sa xTaskResume() - Resume suspended task
   * @sa CONFIG_TASK_WD_TIMER_ENABLE - Enables watchdog feature
   */
  Return_t xTaskChangeWDPeriod(Task_t *task_, const Ticks_t period_);


  /**
   * @brief Query the current execution period of a task
   *
   * Retrieves the interval period that controls how frequently the task
   * executes. Returns the value previously set with xTaskChangePeriod() or the
   * default value (0) if never explicitly set.
   *
   * **Period values:**
   * - **0**: Task runs every scheduler cycle (maximum frequency)
   * - **>0**: Task runs when elapsed time >= period (in ticks)
   *
   * @param[in]  task_   Task handle. Must be valid.
   * @param[out] period_ Receives current period in ticks.
   *
   * @return             ReturnOK if period retrieved. ReturnError if task_ or
   *                     period_ invalid.
   *
   * @sa xTaskChangePeriod() - Set task execution period
   */
  Return_t xTaskGetPeriod(const Task_t *task_, Ticks_t *period_);


  /**
   * @brief Reset a task's elapsed time counter to zero
   *
   * Resets the task's internal elapsed time counter back to zero, effectively
   * restarting the timing measurement for periodic task execution. This forces
   * the task to restart its period measurement from the current moment,
   * delaying the next scheduled execution by one full period.
   *
   * Each task maintains an elapsed time counter that tracks how many ticks have
   * passed since the task last executed. When this elapsed time meets or
   * exceeds the task's configured period (set via xTaskChangePeriod()), the
   * scheduler runs the task. Resetting the timer to zero extends the wait time
   * before the next execution.
   *
   * **Use cases:**
   * - Synchronize task execution: Reset timing after external events
   * - Delay execution: Push back next execution without changing period
   * - Phase alignment: Coordinate multiple tasks to execute in sequence
   * - Event response: Restart timing after processing an event
   *
   * **Example 1: Delay next execution**
   * @code void sensorTask(Task_t *task, TaskParm_t *parm) {
   *   if (errorDetected()) {
   *     // Reset timer to delay next sample xTaskResetTimer(task);
   *     return;
   *   }
   *   readSensor();
   * }
   * @endcode
   *
   * **Example 2: Synchronize on external event**
   * @code void displayTask(Task_t *task, TaskParm_t *parm) {
   *   if (dataAvailable()) {
   *     updateDisplay();
   *     // Restart timing from now xTaskResetTimer(task);
   *   }
   * }
   * @endcode
   *
   * @param[in] task_ Task handle. Must be valid.
   *
   * @return          ReturnOK if timer reset. ReturnError if task_ invalid.
   *
   * @note This does not change the task's period, only resets the elapsed time
   * counter to zero.
   *
   * @note For tasks with period 0 (run every cycle), this has no practical
   * effect since they execute every scheduler cycle regardless.
   *
   * @sa xTaskChangePeriod() - Set task execution period
   * @sa xTaskGetPeriod() - Query current period
   */
  Return_t xTaskResetTimer(Task_t *task_);


  /**
   * @brief Start the HeliOS cooperative task scheduler
   *
   * Transfers control from application code to the HeliOS scheduler, which
   * begins executing tasks in a cooperative multitasking fashion. This is
   * typically the last function called in main() after all tasks, timers, and
   * other objects have been created and configured.
   *
   * The scheduler operates in a continuous loop, executing tasks based on their
   * state and elapsed time since last execution. Tasks in TaskStateRunning or
   * TaskStateWaiting (with pending events) will be scheduled according to their
   * configured period.
   *
   * Control Flow:
   * - If the scheduler state is SchedulerStateRunning, execution enters the
   * scheduler loop and does not return unless xTaskSuspendAll() is called from
   * within a running task
   * - If the scheduler state is SchedulerStateSuspended, this function returns
   * immediately without scheduling any tasks
   * - To resume after suspension, call xTaskResumeAll() followed by
   * xTaskStartScheduler() again
   *
   * @note HeliOS uses cooperative multitasking. Tasks must explicitly yield
   * control by returning from their task function to allow other tasks to
   * execute.
   *
   * @warning Do not call this function multiple times without first suspending
   * the scheduler using xTaskSuspendAll() and resuming it with
   * xTaskResumeAll().
   *
   * Example Usage:
   * @code int main(void) {
   *   Task_t *myTask;
   *
   *   // Create tasks xTaskCreate(&myTask, "Task1", myTaskFunction, NULL);
   *   xTaskResume(myTask);
   *
   *   // Start scheduler (does not return) xTaskStartScheduler();
   *
   *   return 0;
   * }
   * @endcode
   *
   * @return ReturnOK if the scheduler successfully starts or is already
   *         suspended. ReturnError if a critical error occurs preventing
   *         scheduler operation. Note that under normal operation with tasks
   *         running, this function does not return until xTaskSuspendAll() is
   *         called.
   *
   * @sa xTaskResumeAll() - Resume scheduler after suspension
   * @sa xTaskSuspendAll() - Suspend scheduler and return control
   * @sa xTaskGetSchedulerState() - Query current scheduler state
   * @sa xTaskCreate() - Create a new task
   * @sa SchedulerState_t - Scheduler state enumeration
   */
  Return_t xTaskStartScheduler(void);


  /**
   * @brief Resume the scheduler to enable task execution
   *
   * Sets the scheduler state to SchedulerStateRunning, allowing the scheduler
   * to execute tasks when xTaskStartScheduler() is called. This function does
   * not start the scheduler itself; it only changes the scheduler's state flag.
   *
   * This function is used in two scenarios:
   * 1. After calling xTaskSuspendAll() to re-enable scheduling 2. During system
   * initialization to set the scheduler to running state before calling
   * xTaskStartScheduler()
   *
   * Typical Usage Pattern:
   * - Call xTaskSuspendAll() to stop scheduler and regain control
   * - Perform critical operations while scheduler is suspended
   * - Call xTaskResumeAll() to mark scheduler as ready
   * - Call xTaskStartScheduler() to resume task execution
   *
   * @note This function only changes the scheduler state flag. You must still
   * call xTaskStartScheduler() to actually begin executing tasks.
   *
   * @note If xTaskStartScheduler() is called while the scheduler state is
   * SchedulerStateSuspended, it will return immediately without executing any
   * tasks.
   *
   * Example Usage:
   * @code
   * // Suspend scheduler from within a task xTaskSuspendAll();
   *
   * // Perform time-sensitive operations performCriticalOperation();
   *
   * // Resume scheduler state xTaskResumeAll();
   *
   * // Restart scheduler xTaskStartScheduler();
   * @endcode
   *
   * @return ReturnOK on success, ReturnError on failure.
   *
   * @sa xTaskStartScheduler() - Start executing tasks
   * @sa xTaskSuspendAll() - Suspend scheduler and return control
   * @sa xTaskGetSchedulerState() - Query current scheduler state
   * @sa SchedulerState_t - Scheduler state enumeration
   */
  Return_t xTaskResumeAll(void);


  /**
   * @brief Suspend the scheduler and return control to caller
   *
   * Sets the scheduler state to SchedulerStateSuspended and causes
   * xTaskStartScheduler() to exit its scheduling loop, returning control back
   * to the point where xTaskStartScheduler() was called. This provides a
   * mechanism to temporarily halt cooperative multitasking and regain direct
   * control of program execution.
   *
   * When called from within a running task, this function causes the scheduler
   * loop to terminate after the current task completes its execution. Control
   * returns to the line immediately following the original
   * xTaskStartScheduler() call.
   *
   * Use Cases:
   * - Temporarily halt all task execution for critical operations
   * - Enter a low-power mode that requires stopping the scheduler
   * - Transition to a different operating mode
   * - Debug or diagnostic operations that require full control
   *
   * To resume task execution:
   * 1. Call xTaskResumeAll() to set scheduler state to running 2. Call
   * xTaskStartScheduler() to re-enter the scheduling loop
   *
   * @warning When the scheduler is suspended, no tasks will execute, including
   * timer tasks and periodic operations. Ensure critical system functions are
   * maintained during suspension.
   *
   * @note This function must be called from within a task context, not from
   * main() or initialization code.
   *
   * Example Usage:
   * @code void myTaskFunction(Task_t *task, TaskParm_t *parm) {
   *   // Normal task operations...
   *
   *   if (needToSuspendScheduler) {
   *     // Suspend scheduler and return control xTaskSuspendAll();
   *
   *     // Execution returns to line after xTaskStartScheduler() in main()
   *   }
   * }
   *
   * int main(void) {
   *   Task_t *task;
   *   xTaskCreate(&task, "MyTask", myTaskFunction, NULL);
   *   xTaskResume(task);
   *   xTaskStartScheduler(); // Will return here when xTaskSuspendAll() called
   *
   *   // Code here executes after scheduler suspension
   * performCriticalOperation();
   *
   *   // Resume if needed xTaskResumeAll();
   *   xTaskStartScheduler();
   * }
   * @endcode
   *
   * @return ReturnOK on success, ReturnError on failure.
   *
   * @sa xTaskStartScheduler() - Start the scheduler
   * @sa xTaskResumeAll() - Resume scheduler state
   * @sa xTaskGetSchedulerState() - Query current scheduler state
   * @sa SchedulerState_t - Scheduler state enumeration
   */
  Return_t xTaskSuspendAll(void);


  /**
   * @brief Query the current state of the task scheduler
   *
   * Retrieves the scheduler's current operational state, which indicates
   * whether the scheduler is actively running tasks or has been suspended. The
   * scheduler state determines whether xTaskStartScheduler() will enter the
   * scheduling loop or return immediately.
   *
   * **Scheduler states:**
   * - **SchedulerStateRunning**: Scheduler is active and executing tasks
   * - **SchedulerStateSuspended**: Scheduler suspended via xTaskSuspendAll()
   *
   * This function is useful for debugging, system monitoring, or conditional
   * logic that depends on scheduler status. Tasks can query scheduler state to
   * determine if they're running under active scheduling or during suspended
   * periods.
   *
   * **Common use cases:**
   * - Debug diagnostics: Log scheduler state during troubleshooting
   * - Conditional behavior: Execute different code paths based on scheduler
   * state
   * - System monitoring: Track scheduler transitions
   * - State verification: Confirm scheduler is in expected state
   *
   * @param[out] state_ Receives current scheduler state (SchedulerStateRunning
   *                    or SchedulerStateSuspended).
   *
   * @return            ReturnOK if state retrieved. ReturnError if state_ is
   *                    NULL.
   *
   * @sa xTaskStartScheduler() - Start scheduler (behavior depends on state)
   * @sa xTaskSuspendAll() - Suspend scheduler
   * @sa xTaskResumeAll() - Resume scheduler
   * @sa SchedulerState_t - Scheduler state enumeration
   */
  Return_t xTaskGetSchedulerState(SchedulerState_t *state_);


  /**
   * @brief Query the task watchdog timeout period
   *
   * Retrieves the watchdog timer period for a task. This is the maximum
   * execution time allowed for a single task invocation before automatic
   * suspension occurs.
   *
   * Returns the value previously set with xTaskChangeWDPeriod(). Requires
   * CONFIG_TASK_WD_TIMER_ENABLE defined at compile time.
   *
   * @param[in]  task_   Task handle. Must be valid.
   * @param[out] period_ Receives watchdog period in ticks.
   *
   * @return             ReturnOK if period retrieved. ReturnError if task_ or
   *                     period_ invalid, or CONFIG_TASK_WD_TIMER_ENABLE not
   *                     defined.
   *
   * @warning Requires CONFIG_TASK_WD_TIMER_ENABLE. Returns error if not
   * enabled.
   *
   * @sa xTaskChangeWDPeriod() - Set watchdog timeout
   * @sa CONFIG_TASK_WD_TIMER_ENABLE - Enables watchdog feature
   */
  Return_t xTaskGetWDPeriod(const Task_t *task_, Ticks_t *period_);


  /**
   * @brief Create a software timer for general-purpose timekeeping
   *
   * Creates a software timer that can be used for measuring time intervals,
   * implementing delays, or triggering periodic actions. Software timers are
   * independent of task timers and provide general-purpose timing facilities
   * for application use.
   *
   * Software timers in HeliOS are NOT the same as task timers used for
   * event-driven multitasking. Application timers are lightweight timing
   * objects that can be:
   * - Started and stopped on demand
   * - Reset to restart counting from zero
   * - Queried to check if the period has elapsed
   * - Modified to change their period dynamically
   *
   * Timer Operation:
   * Timers count system ticks from when they are started. When the elapsed
   * ticks exceed the configured period, the timer is considered "expired."
   * Applications check timer expiration using xTimerHasTimerExpired() and can
   * reset the timer to begin a new timing cycle.
   *
   * Common Use Cases:
   * - Implementing timeout detection
   * - Creating delays without blocking
   * - Measuring elapsed time between events
   * - Implementing periodic operations (e.g., blinking LED)
   * - Rate limiting operations
   * - Watchdog functionality
   *
   * @note Software timers are passive timing objects. They do not automatically
   * trigger callbacks or events when they expire. Applications must actively
   * check timer status using xTimerHasTimerExpired().
   *
   * @note Timer resolution is determined by the system tick rate, which depends
   * on how frequently xTaskStartScheduler() executes (typically tied to
   * hardware timer interrupts or main loop frequency).
   *
   * @note Multiple timers can be created for different timing needs. Each timer
   * maintains independent state and period configuration.
   *
   * Example Usage:
   * @code Timer_t *blinkTimer;
   * Timer_t *timeoutTimer;
   *
   * // Create a timer with 1000 tick period (e.g., 1 second if 1ms ticks) if
   * (OK(xTimerCreate(&blinkTimer, 1000))) {
   *   // Start the timer xTimerStart(blinkTimer);
   *
   *   // In your task or main loop:
   *   Base_t expired;
   *   if (OK(xTimerHasTimerExpired(blinkTimer, &expired)) && expired) {
   *     // Timer has expired - toggle LED toggleLED();
   *
   *     // Reset for next cycle xTimerReset(blinkTimer);
   *   }
   * }
   *
   * // Create a timeout timer if (OK(xTimerCreate(&timeoutTimer, 5000))) {  //
   * 5 second timeout xTimerStart(timeoutTimer);
   *
   *   // Wait for operation with timeout while (!operationComplete) {
   *     Base_t timedOut;
   *     if (OK(xTimerHasTimerExpired(timeoutTimer, &timedOut)) && timedOut) {
   *       // Timeout occurred handleTimeout();
   *       break;
   *     }
   *     // Continue waiting...
   *   }
   *
   *   // Clean up xTimerDelete(timeoutTimer);
   * }
   * @endcode
   *
   * @param[out] timer_  Pointer to Timer_t *variable that will receive the
   *                     timer handle. This handle is used in subsequent timer
   *                     operations.
   * @param[in]  period_ Timer period in system ticks. The timer expires when
   *                     elapsed ticks exceed this value. Must be greater than
   *                     zero.
   *
   * @return             ReturnOK if the timer was successfully created,
   *                     ReturnError if creation failed (out of memory, invalid
   *                     parameters, or system error).
   *
   * @sa xTimerDelete() - Delete a timer and free its resources
   * @sa xTimerStart() - Start a timer counting
   * @sa xTimerStop() - Stop a timer
   * @sa xTimerReset() - Reset a timer to zero
   * @sa xTimerHasTimerExpired() - Check if timer has expired
   * @sa xTimerChangePeriod() - Change timer period
   * @sa xTimerGetPeriod() - Get current timer period
   * @sa xTimerIsTimerActive() - Check if timer is running
   */
  Return_t xTimerCreate(Timer_t **timer_, const Ticks_t period_);


  /**
   * @brief Delete an application timer and free its resources
   *
   * Permanently removes an application timer created with xTimerCreate() and
   * releases all associated kernel resources. After deletion, the timer handle
   * becomes invalid and must not be used in any subsequent timer operations.
   * This operation is typically performed during cleanup, reconfiguration, or
   * when a timer is no longer needed by the application.
   *
   * xTimerDelete() immediately removes the timer from the kernel timer list
   * regardless of its current state (stopped, running, or expired). If the
   * timer was active, it is stopped automatically before deletion. No further
   * timer events will occur, and all internal timer state is freed.
   *
   * Key characteristics:
   * - **Immediate deletion**: Timer removed regardless of active/stopped state
   * - **Resource cleanup**: All kernel memory associated with timer is freed
   * - **Handle invalidation**: Timer handle cannot be reused after deletion
   * - **Idempotent operation**: Safe to call even if timer was never started
   * - **No blocking**: Returns immediately after cleanup completes
   *
   * Typical deletion scenarios:
   * - **Application cleanup**: Remove timers during task or module shutdown
   * - **Dynamic timer management**: Delete and recreate timers with different
   * periods
   * - **Resource reclamation**: Free timers no longer needed to reduce memory
   * usage
   * - **Error recovery**: Clean up timers after configuration or initialization
   * failures
   *
   * Example 1: Timer lifecycle management
   * @code Timer_t *watchdogTimer;
   *
   * // Create and use timer if (OK(xTimerCreate(&watchdogTimer, 1000))) {
   *   xTimerStart(watchdogTimer);
   *
   *   // ... use timer for some period ...
   *
   *   // Clean up when done xTimerStop(watchdogTimer);
   *   xTimerDelete(watchdogTimer);
   *   // watchdogTimer handle is now invalid
   * }
   * @endcode
   *
   * Example 2: Dynamic timer reconfiguration
   * @code Timer_t *intervalTimer;
   *
   * void setMonitoringInterval(Ticks_t newInterval) {
   *   // Delete existing timer if present if (intervalTimer != null) {
   *     xTimerDelete(intervalTimer);
   *   }
   *
   *   // Create new timer with updated period if
   * (OK(xTimerCreate(&intervalTimer, newInterval))) {
   *     xTimerStart(intervalTimer);
   *   }
   * }
   * @endcode
   *
   * Example 3: Cleanup multiple timers
   * @code Timer_t *timers[MAX_SENSORS];
   * Base_t timerCount = 0;
   *
   * void initSensorTimers(void) {
   *   for (Base_t i = 0; i < MAX_SENSORS; i++) {
   *     if (OK(xTimerCreate(&timers[i], 100 * (i + 1)))) {
   *       xTimerStart(timers[i]);
   *       timerCount++;
   *     }
   *   }
   * }
   *
   * void shutdownSensorTimers(void) {
   *   for (Base_t i = 0; i < timerCount; i++) {
   *     xTimerDelete(timers[i]);
   *   }
   *   timerCount = 0;
   * }
   * @endcode
   *
   * Example 4: Conditional timer cleanup
   * @code Timer_t *periodicTimer;
   * Base_t timerActive = 0;
   *
   * void disablePeriodicTask(void) {
   *   if (timerActive) {
   *     xTimerDelete(periodicTimer);
   *     timerActive = 0;
   *   }
   * }
   *
   * void enablePeriodicTask(Ticks_t period) {
   *   // Clean up old timer if exists disablePeriodicTask();
   *
   *   // Create new timer if (OK(xTimerCreate(&periodicTimer, period))) {
   *     xTimerStart(periodicTimer);
   *     timerActive = 1;
   *   }
   * }
   * @endcode
   *
   * @param[in] timer_ Handle to the timer to delete. Must be a valid timer
   *                   created with xTimerCreate(). After deletion, this handle
   *                   becomes invalid.
   *
   * @return           ReturnOK if timer deleted successfully, ReturnError if
   *                   deletion failed due to invalid timer handle or timer not
   *                   found.
   *
   * @warning After xTimerDelete() returns successfully, the timer handle is
   * invalid and must not be used in any subsequent operations. Using a deleted
   * timer handle will result in ReturnError.
   *
   * @warning Deleting a timer that is referenced by multiple tasks can lead to
   * errors if those tasks attempt to use the timer after deletion. Coordinate
   * timer deletion across all tasks using the timer.
   *
   * @note xTimerDelete() automatically stops the timer before deletion if it
   * was running. You do not need to explicitly call xTimerStop() first.
   *
   * @note Unlike task and queue deletion, timer deletion does not require
   * waiting for operations to complete. The timer is removed immediately.
   *
   * @note Deletion is the only way to reclaim kernel memory allocated for a
   * timer. Simply stopping a timer does not free its resources.
   *
   * @sa xTimerCreate() - Create an application timer
   * @sa xTimerStop() - Stop a running timer without deleting it
   * @sa xTimerStart() - Start or restart a timer
   * @sa xTimerReset() - Reset timer elapsed time to zero
   * @sa xTimerIsTimerActive() - Check if timer is currently running
   * @sa xTaskDelete() - Delete a task (similar resource cleanup pattern)
   */
  Return_t xTimerDelete(const Timer_t *timer_);


  /**
   * @brief Change the period of an existing application timer
   *
   * Modifies the expiration period of an application timer without requiring
   * timer deletion and recreation. The new period takes effect immediately and
   * will be used for all subsequent timer expirations. This operation allows
   * dynamic adjustment of timer intervals based on runtime conditions, enabling
   * adaptive timing behavior in response to system state or application needs.
   *
   * When xTimerChangePeriod() is called, the timer's period is updated to the
   * new value, but the timer's current state (active/stopped) and elapsed time
   * are preserved. If the timer is running, it continues running with the new
   * period. The timer will expire when the elapsed time reaches the new period
   * value, which may be sooner or later than the original period.
   *
   * Key characteristics:
   * - **Immediate effect**: New period applies to current and future timing
   * cycles
   * - **State preservation**: Timer remains active or stopped as before the
   * change
   * - **Elapsed time preserved**: Current elapsed time is not reset
   * automatically
   * - **No recreation needed**: Avoids overhead of deleting and recreating
   * timer
   * - **Non-blocking**: Returns immediately after period update
   *
   * Common period change scenarios:
   * - **Adaptive sampling**: Adjust sensor polling rate based on value changes
   * - **Power management**: Slow down non-critical timers to conserve energy
   * - **Load balancing**: Distribute timer expirations to avoid processing
   * spikes
   * - **Configuration updates**: Apply new timing settings from user
   * preferences
   * - **Error recovery**: Increase retry intervals during communication
   * failures
   *
   * Example 1: Adaptive sensor polling
   * @code Timer_t *sensorPollTimer;
   * Byte_t lastReading = 0;
   *
   * void sensorTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (OK(xTimerHasTimerExpired(sensorPollTimer, &expired)) && expired) {
   *     Byte_t reading = readSensor();
   *     Byte_t change = abs(reading - lastReading);
   *
   *     // Adapt polling rate based on change magnitude if (change > 20) {
   *       // Fast changes - poll rapidly xTimerChangePeriod(sensorPollTimer,
   * 10);
   *     } else if (change > 5) {
   *       // Moderate changes - normal rate xTimerChangePeriod(sensorPollTimer,
   * 50);
   *     } else {
   *       // Stable - slow down polling xTimerChangePeriod(sensorPollTimer,
   * 200);
   *     }
   *
   *     lastReading = reading;
   *     xTimerReset(sensorPollTimer);
   *   }
   * }
   * @endcode
   *
   * Example 2: Power-aware timing
   * @code Timer_t *backgroundTimer;
   *
   * typedef enum {
   *   POWER_MODE_ACTIVE, POWER_MODE_SLEEP, POWER_MODE_DEEP_SLEEP
   * } PowerMode_t;
   *
   * void setPowerMode(PowerMode_t mode) {
   *   switch (mode) {
   *     case POWER_MODE_ACTIVE:
   *       xTimerChangePeriod(backgroundTimer, 50);   // Fast updates break;
   *     case POWER_MODE_SLEEP:
   *       xTimerChangePeriod(backgroundTimer, 500);  // Slower updates break;
   *     case POWER_MODE_DEEP_SLEEP:
   *       xTimerChangePeriod(backgroundTimer, 5000); // Minimal updates break;
   *   }
   * }
   * @endcode
   *
   * Example 3: Communication retry with exponential backoff
   * @code Timer_t *retryTimer;
   * Ticks_t currentRetryInterval = 100;
   *
   * void attemptConnection(void) {
   *   if (connectionSuccessful()) {
   *     // Reset to initial interval on success currentRetryInterval = 100;
   *     xTimerChangePeriod(retryTimer, currentRetryInterval);
   *     xTimerStop(retryTimer);
   *   } else {
   *     // Exponential backoff on failure (max 10 seconds) if
   * (currentRetryInterval < 10000) {
   *       currentRetryInterval *= 2;
   *       xTimerChangePeriod(retryTimer, currentRetryInterval);
   *     }
   *     xTimerReset(retryTimer);
   *   }
   * }
   * @endcode
   *
   * Example 4: User-configurable timer
   * @code Timer_t *userTimer;
   *
   * void applyUserSettings(Ticks_t newInterval) {
   *   // Validate interval range if (newInterval < 10) {
   *     newInterval = 10;     // Minimum 10 ticks
   *   } else if (newInterval > 60000) {
   *     newInterval = 60000;  // Maximum 60 seconds
   *   }
   *
   *   // Apply new interval if (OK(xTimerChangePeriod(userTimer, newInterval)))
   * {
   *     // Reset to start timing with new period xTimerReset(userTimer);
   *     saveSettings(newInterval);
   *   }
   * }
   * @endcode
   *
   * @param[in] timer_  Handle to the timer whose period should be changed. Must
   *                    be a valid timer created with xTimerCreate().
   * @param[in] period_ The new timer period in system ticks. Must be greater
   *                    than zero. The timer will expire when elapsed time
   *                    equals this value.
   *
   * @return            ReturnOK if period changed successfully, ReturnError if
   *                    operation failed due to invalid timer handle, timer not
   *                    found, or invalid period value.
   *
   * @warning Changing the period does NOT automatically reset the elapsed time.
   * If a timer has already accumulated elapsed time, it may expire immediately
   * if the new period is less than the current elapsed time. Call xTimerReset()
   * after changing the period if you want to start timing from zero with the
   * new period.
   *
   * @warning Reducing the period on a running timer may cause immediate
   * expiration if the elapsed time already exceeds the new period. Always check
   * or reset the timer after reducing the period.
   *
   * @note xTimerChangePeriod() can be called whether the timer is running or
   * stopped. The new period takes effect in both cases.
   *
   * @note The timer's active state is preserved—if it was running, it continues
   * running; if stopped, it remains stopped.
   *
   * @note Changing to period value 0 is invalid and will return ReturnError. To
   * disable a timer, use xTimerStop() instead.
   *
   * @sa xTimerGetPeriod() - Query current timer period
   * @sa xTimerReset() - Reset elapsed time after changing period
   * @sa xTimerCreate() - Create timer with initial period
   * @sa xTimerStart() - Start timer with its current period
   * @sa xTimerStop() - Stop timer to prevent expiration
   * @sa xTimerHasTimerExpired() - Check if timer has expired
   */
  Return_t xTimerChangePeriod(Timer_t *timer_, const Ticks_t period_);


  /**
   * @brief Query the current period of an application timer
   *
   * Retrieves the configured expiration period for an application timer in
   * system ticks. This non-destructive query allows tasks to inspect timer
   * configuration without affecting timer state or operation. The returned
   * period value reflects the most recent setting from xTimerCreate() or
   * xTimerChangePeriod().
   *
   * xTimerGetPeriod() provides read-only access to the timer's period
   * configuration. This is useful for validation, diagnostics, dynamic
   * adjustments based on current settings, or coordination between multiple
   * tasks that share timer resources. The query does not modify the timer
   * state, elapsed time, or active/stopped status.
   *
   * Common use cases:
   * - **Configuration validation**: Verify timer period matches expected values
   * - **Dynamic adjustments**: Calculate new periods based on current setting
   * - **Diagnostics and logging**: Report timer configuration for debugging
   * - **Coordinated timing**: Synchronize related timers based on period ratios
   * - **Settings persistence**: Save current timer configuration to
   * non-volatile memory
   *
   * Example 1: Validate timer configuration
   * @code Timer_t *watchdogTimer;
   *
   * void initWatchdog(void) {
   *   Ticks_t expectedPeriod = 5000;  // 5 seconds
   *
   *   if (OK(xTimerCreate(&watchdogTimer, expectedPeriod))) {
   *     // Verify timer created with correct period Ticks_t actualPeriod;
   *     if (OK(xTimerGetPeriod(watchdogTimer, &actualPeriod))) {
   *       if (actualPeriod == expectedPeriod) {
   *         xTimerStart(watchdogTimer);
   *       } else {
   *         // Configuration mismatch - handle error logError("Watchdog period
   * mismatch");
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Adaptive period adjustment
   * @code Timer_t *pollTimer;
   *
   * void speedUpPolling(void) {
   *   Ticks_t currentPeriod;
   *
   *   if (OK(xTimerGetPeriod(pollTimer, &currentPeriod))) {
   *     // Halve the period (double the rate), but enforce minimum Ticks_t
   * newPeriod = currentPeriod / 2;
   *     if (newPeriod < 10) {
   *       newPeriod = 10;  // Minimum 10 ticks
   *     }
   *
   *     xTimerChangePeriod(pollTimer, newPeriod);
   *     xTimerReset(pollTimer);
   *   }
   * }
   *
   * void slowDownPolling(void) {
   *   Ticks_t currentPeriod;
   *
   *   if (OK(xTimerGetPeriod(pollTimer, &currentPeriod))) {
   *     // Double the period (halve the rate), but enforce maximum Ticks_t
   * newPeriod = currentPeriod * 2;
   *     if (newPeriod > 10000) {
   *       newPeriod = 10000;  // Maximum 10 seconds
   *     }
   *
   *     xTimerChangePeriod(pollTimer, newPeriod);
   *     xTimerReset(pollTimer);
   *   }
   * }
   * @endcode
   *
   * Example 3: Coordinated timer relationships
   * @code Timer_t *fastTimer;
   * Timer_t *slowTimer;
   *
   * void setupCoordinatedTimers(void) {
   *   // Fast timer runs at base rate xTimerCreate(&fastTimer, 100);
   *   xTimerStart(fastTimer);
   *
   *   // Slow timer should run at 10x the fast timer period Ticks_t fastPeriod;
   *   if (OK(xTimerGetPeriod(fastTimer, &fastPeriod))) {
   *     xTimerCreate(&slowTimer, fastPeriod * 10);
   *     xTimerStart(slowTimer);
   *   }
   * }
   * @endcode
   *
   * Example 4: System diagnostics and reporting
   * @code Timer_t *timers[MAX_TIMERS];
   * Base_t timerCount = 0;
   *
   * void reportTimerStatus(void) {
   *   for (Base_t i = 0; i < timerCount; i++) {
   *     Ticks_t period;
   *     Base_t isActive;
   *     Base_t hasExpired;
   *
   *     if (OK(xTimerGetPeriod(timers[i], &period))) {
   *       xTimerIsTimerActive(timers[i], &isActive);
   *       xTimerHasTimerExpired(timers[i], &hasExpired);
   *
   *       // Log timer status printf("Timer %d: period=%lu, active=%d,
   * expired=%d\n", i, period, isActive, hasExpired);
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  timer_  Handle to the timer to query. Must be a valid timer
   *                     created with xTimerCreate().
   * @param[out] period_ Pointer to variable receiving the timer period in
   *                     system ticks. On success, contains the current period
   *                     value.
   *
   * @return             ReturnOK if period retrieved successfully, ReturnError
   *                     if query failed due to invalid timer handle or timer
   *                     not found.
   *
   * @warning This function returns the configured period, not the elapsed time
   * or remaining time. To determine how close a timer is to expiration, you
   * need to track elapsed time or check expiration status with
   * xTimerHasTimerExpired().
   *
   * @note xTimerGetPeriod() is a non-destructive query that does not modify
   * timer state, elapsed time, or active/stopped status.
   *
   * @note The returned period reflects the most recent value set by either
   * xTimerCreate() (initial period) or xTimerChangePeriod() (updated period).
   *
   * @note HeliOS does not provide direct access to elapsed time. To track
   * progress toward expiration, implement your own elapsed time tracking using
   * the scheduler's tick count.
   *
   * @sa xTimerChangePeriod() - Modify timer period
   * @sa xTimerCreate() - Create timer with initial period
   * @sa xTimerIsTimerActive() - Check if timer is running
   * @sa xTimerHasTimerExpired() - Check if timer has expired
   * @sa xTimerReset() - Reset elapsed time to zero
   * @sa xTimerStart() - Start timer with its current period
   */
  Return_t xTimerGetPeriod(const Timer_t *timer_, Ticks_t *period_);


  /**
   * @brief Check if an application timer is currently running
   *
   * Queries whether an application timer is in the active (running) state. A
   * timer is considered active if it has been started with xTimerStart() and
   * has not been stopped with xTimerStop(). Active timers accumulate elapsed
   * time on each scheduler tick and will eventually expire when elapsed time
   * reaches the configured period.
   *
   * This non-destructive query allows tasks to check timer state before
   * performing operations, coordinate timing between tasks, or implement
   * conditional logic based on whether timing is currently in progress. The
   * query does not affect timer operation or elapsed time accumulation.
   *
   * Timer state transitions:
   * - **Inactive (stopped)**: Initial state after creation, or after
   * xTimerStop()
   * - **Active (running)**: After xTimerStart(), accumulating elapsed time
   * - **Active and expired**: Timer remains active until xTimerReset() or
   * xTimerStop()
   *
   * Key characteristics:
   * - **Non-destructive query**: Does not modify timer state or elapsed time
   * - **State only**: Returns active/stopped status, not expiration status
   * - **Instant snapshot**: Reflects current state at time of call
   *
   * Example 1: Conditional timer start
   * @code Timer_t *processingTimer;
   *
   * void startProcessing(void) {
   *   Base_t isActive;
   *
   *   // Only start if not already running if
   * (OK(xTimerIsTimerActive(processingTimer, &isActive))) {
   *     if (!isActive) {
   *       xTimerReset(processingTimer);
   *       xTimerStart(processingTimer);
   *       beginProcessing();
   *     } else {
   *       logWarning("Processing already in progress");
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Watchdog monitoring
   * @code Timer_t *watchdogTimer;
   *
   * void checkWatchdog(void) {
   *   Base_t isActive;
   *   Base_t hasExpired;
   *
   *   if (OK(xTimerIsTimerActive(watchdogTimer, &isActive))) {
   *     if (!isActive) {
   *       // Watchdog not running - critical error handleWatchdogFailure();
   *     } else if (OK(xTimerHasTimerExpired(watchdogTimer, &hasExpired)) &&
   * hasExpired) {
   *       // Watchdog expired - system hung handleWatchdogTimeout();
   *     } else {
   *       // Watchdog running normally
   *     }
   *   }
   * }
   *
   * void petWatchdog(void) {
   *   Base_t isActive;
   *
   *   if (OK(xTimerIsTimerActive(watchdogTimer, &isActive)) && isActive) {
   *     xTimerReset(watchdogTimer);  // Reset only if active
   *   }
   * }
   * @endcode
   *
   * Example 3: Performance measurement
   * @code Timer_t *perfTimer;
   *
   * void startMeasurement(void) {
   *   Base_t isActive;
   *
   *   if (OK(xTimerIsTimerActive(perfTimer, &isActive)) && isActive) {
   *     logWarning("Measurement already in progress");
   *     return;
   *   }
   *
   *   xTimerReset(perfTimer);
   *   xTimerStart(perfTimer);
   * }
   *
   * void endMeasurement(void) {
   *   Base_t isActive;
   *
   *   if (OK(xTimerIsTimerActive(perfTimer, &isActive)) && isActive) {
   *     xTimerStop(perfTimer);
   *     // Analyze performance metrics
   *   } else {
   *     logError("No measurement in progress");
   *   }
   * }
   * @endcode
   *
   * Example 4: System diagnostics
   * @code Timer_t *systemTimers[MAX_TIMERS];
   * Base_t timerCount = 0;
   *
   * void reportSystemStatus(void) {
   *   Base_t activeCount = 0;
   *   Base_t expiredCount = 0;
   *
   *   for (Base_t i = 0; i < timerCount; i++) {
   *     Base_t isActive, hasExpired;
   *
   *     if (OK(xTimerIsTimerActive(systemTimers[i], &isActive)) && isActive) {
   *       activeCount++;
   *
   *       if (OK(xTimerHasTimerExpired(systemTimers[i], &hasExpired)) &&
   * hasExpired) {
   *         expiredCount++;
   *       }
   *     }
   *   }
   *
   *   printf("Timers: %d total, %d active, %d expired\n", timerCount,
   * activeCount, expiredCount);
   * }
   * @endcode
   *
   * @param[in]  timer_ Handle to the timer to query. Must be a valid timer
   *                    created with xTimerCreate().
   * @param[out] res_   Pointer to variable receiving the active state. Set to
   *                    non-zero (true) if timer is active/running, zero (false)
   *                    if timer is stopped.
   *
   * @return            ReturnOK if query succeeded, ReturnError if query failed
   *                    due to invalid timer handle or timer not found.
   *
   * @warning Active state does not indicate whether the timer has expired. A
   * timer can be both active and expired simultaneously. Use
   * xTimerHasTimerExpired() to check expiration status.
   *
   * @note xTimerIsTimerActive() returns the state at the moment of the call. In
   * multitasking environments, the state may change immediately after if
   * another task starts or stops the timer.
   *
   * @note A timer that has expired remains active until explicitly stopped with
   * xTimerStop() or reset with xTimerReset(). Expiration does not automatically
   * stop the timer.
   *
   * @note Newly created timers are inactive by default. They must be explicitly
   * started with xTimerStart() to begin accumulating elapsed time.
   *
   * @sa xTimerStart() - Start timer (make it active)
   * @sa xTimerStop() - Stop timer (make it inactive)
   * @sa xTimerHasTimerExpired() - Check if active timer has expired
   * @sa xTimerReset() - Reset elapsed time while keeping timer active
   * @sa xTimerCreate() - Create timer in inactive state
   */
  Return_t xTimerIsTimerActive(const Timer_t *timer_, Base_t *res_);


  /**
   * @brief Check if an application timer has expired
   *
   * Queries whether an application timer's elapsed time has reached or exceeded
   * its configured period, indicating that the timer has expired. This query is
   * the primary mechanism for detecting timer events in polling-based timing
   * patterns. Once expired, the timer remains in the expired state until reset
   * with xTimerReset() or stopped with xTimerStop().
   *
   * A timer can only expire if it is active (started with xTimerStart()).
   * Inactive timers never expire, even if sufficient time has passed. The
   * expired state is "sticky"—once a timer expires, it remains expired until
   * explicitly cleared, allowing multiple tasks to observe the expiration event
   * without race conditions.
   *
   * Typical expiration handling workflow:
   * 1. Check if timer has expired with xTimerHasTimerExpired() 2. If expired,
   * perform the timed action 3. Reset the timer with xTimerReset() to clear
   * expiration and restart timing 4. Repeat the cycle
   *
   * Key characteristics:
   * - **Sticky expiration**: Remains expired until xTimerReset() or
   * xTimerStop()
   * - **Requires active timer**: Only active timers can expire
   * - **Non-destructive query**: Does not clear expiration or modify state
   * - **Event detection**: Primary method for implementing periodic actions
   *
   * Example 1: Periodic sensor polling
   * @code Timer_t *pollTimer;
   *
   * void initSensor(void) {
   *   xTimerCreate(&pollTimer, 100);  // Poll every 100 ticks
   * xTimerStart(pollTimer);
   * }
   *
   * void sensorTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (OK(xTimerHasTimerExpired(pollTimer, &expired)) && expired) {
   *     // Timer expired - read sensor Byte_t sensorValue = readSensor();
   *     processSensorData(sensorValue);
   *
   *     // Reset for next period xTimerReset(pollTimer);
   *   }
   * }
   * @endcode
   *
   * Example 2: Watchdog timeout detection
   * @code Timer_t *watchdogTimer;
   *
   * void initWatchdog(void) {
   *   xTimerCreate(&watchdogTimer, 5000);  // 5 second timeout
   * xTimerStart(watchdogTimer);
   * }
   *
   * void watchdogTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (OK(xTimerHasTimerExpired(watchdogTimer, &expired)) && expired) {
   *     // Watchdog expired - system not responding logError("Watchdog timeout
   * - system hung");
   *     performSystemReset();
   *   }
   * }
   *
   * void petWatchdog(void) {
   *   // Called periodically by application to prevent timeout
   * xTimerReset(watchdogTimer);
   * }
   * @endcode
   *
   * Example 3: Multi-rate execution with multiple timers
   * @code Timer_t *fastTimer, mediumTimer, slowTimer;
   *
   * void initTimers(void) {
   *   xTimerCreate(&fastTimer, 10);    // Every 10 ticks
   * xTimerCreate(&mediumTimer, 100); // Every 100 ticks
   * xTimerCreate(&slowTimer, 1000);  // Every 1000 ticks
   *
   *   xTimerStart(fastTimer);
   *   xTimerStart(mediumTimer);
   *   xTimerStart(slowTimer);
   * }
   *
   * void controlTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   // Fast update - 10 ticks if (OK(xTimerHasTimerExpired(fastTimer,
   * &expired)) && expired) {
   *     updateFastController();
   *     xTimerReset(fastTimer);
   *   }
   *
   *   // Medium update - 100 ticks if (OK(xTimerHasTimerExpired(mediumTimer,
   * &expired)) && expired) {
   *     updateMediumController();
   *     xTimerReset(mediumTimer);
   *   }
   *
   *   // Slow update - 1000 ticks if (OK(xTimerHasTimerExpired(slowTimer,
   * &expired)) && expired) {
   *     updateSlowController();
   *     xTimerReset(slowTimer);
   *   }
   * }
   * @endcode
   *
   * Example 4: Timeout for communication protocol
   * @code Timer_t *responseTimer;
   *
   * typedef enum {
   *   STATE_IDLE, STATE_WAITING_RESPONSE, STATE_COMPLETE
   * } CommState_t;
   *
   * CommState_t commState = STATE_IDLE;
   *
   * void sendRequest(void) {
   *   transmitData();
   *   commState = STATE_WAITING_RESPONSE;
   *
   *   // Start timeout timer xTimerReset(responseTimer);
   *   xTimerStart(responseTimer);
   * }
   *
   * void commTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (commState == STATE_WAITING_RESPONSE) {
   *     if (OK(xTimerHasTimerExpired(responseTimer, &expired)) && expired) {
   *       // Timeout - no response received logError("Communication timeout");
   *       xTimerStop(responseTimer);
   *       commState = STATE_IDLE;
   *       retryRequest();
   *     }
   *   }
   * }
   *
   * void onResponseReceived(void) {
   *   // Stop timer on successful response xTimerStop(responseTimer);
   *   commState = STATE_COMPLETE;
   *   processResponse();
   * }
   * @endcode
   *
   * @param[in]  timer_ Handle to the timer to query. Must be a valid timer
   *                    created with xTimerCreate().
   * @param[out] res_   Pointer to variable receiving the expiration status. Set
   *                    to non-zero (true) if timer has expired, zero (false) if
   *                    timer has not expired or is not active.
   *
   * @return            ReturnOK if query succeeded, ReturnError if query failed
   *                    due to invalid timer handle or timer not found.
   *
   * @warning xTimerHasTimerExpired() does NOT automatically reset the timer.
   * Once expired, the timer remains expired until you explicitly call
   * xTimerReset() or xTimerStop(). Failing to reset an expired timer will cause
   * it to appear expired on every subsequent check.
   *
   * @warning Inactive (stopped) timers never expire. Always verify the timer is
   * active with xTimerStart() before expecting expiration events.
   *
   * @note The expiration state is "sticky"—it persists until cleared. This
   * allows multiple tasks to safely check the same timer expiration without
   * missing the event.
   *
   * @note Expiration occurs when elapsed time >= period. At the moment the
   * elapsed time equals the period, the timer transitions to expired state.
   *
   * @note Unlike some RTOS timers that provide callback functions on
   * expiration, HeliOS timers use polling with xTimerHasTimerExpired(). Tasks
   * must explicitly check for expiration in their execution loops.
   *
   * @sa xTimerReset() - Clear expiration and restart timing
   * @sa xTimerStart() - Activate timer to enable expiration
   * @sa xTimerStop() - Deactivate timer and clear expiration
   * @sa xTimerIsTimerActive() - Check if timer is running
   * @sa xTimerCreate() - Create timer with period
   * @sa xTimerChangePeriod() - Modify timer period
   */
  Return_t xTimerHasTimerExpired(const Timer_t *timer_, Base_t *res_);


  /**
   * @brief Reset an application timer's elapsed time to zero
   *
   * Clears an application timer's accumulated elapsed time, returning it to the
   * state immediately after creation or start. This operation is the primary
   * method for acknowledging timer expiration and beginning a new timing cycle.
   * The timer's configured period and active/stopped state are preserved—only
   * the elapsed time is reset to zero.
   *
   * xTimerReset() is typically called after detecting timer expiration with
   * xTimerHasTimerExpired() to clear the expired state and restart the timing
   * cycle for periodic operations. The reset does not affect whether the timer
   * is active or stopped; an active timer continues running from zero elapsed
   * time, while a stopped timer remains stopped with zero elapsed time.
   *
   * Key characteristics:
   * - **Clears elapsed time**: Resets accumulated time to zero
   * - **Clears expiration**: Timer no longer reports as expired after reset
   * - **Preserves state**: Active/stopped state unchanged
   * - **Preserves period**: Configured timer period unchanged
   * - **Immediate effect**: Next expiration occurs one full period after reset
   *
   * Common reset scenarios:
   * - **Periodic operations**: Reset after each expiration to start next cycle
   * - **Watchdog petting**: Reset watchdog timer to prevent timeout
   * - **Event acknowledgment**: Clear timer after handling timed event
   * - **Synchronization**: Align timer phases with system events
   * - **Period changes**: Reset after calling xTimerChangePeriod() to start
   * fresh
   *
   * Example 1: Periodic sensor polling pattern
   * @code Timer_t *sensorTimer;
   *
   * void initSensor(void) {
   *   xTimerCreate(&sensorTimer, 100);  // 100 tick period
   * xTimerStart(sensorTimer);
   * }
   *
   * void sensorTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (OK(xTimerHasTimerExpired(sensorTimer, &expired)) && expired) {
   *     // Read and process sensor Byte_t value = readSensor();
   *     processSensorValue(value);
   *
   *     // Reset for next cycle xTimerReset(sensorTimer);
   *   }
   * }
   * @endcode
   *
   * Example 2: Watchdog timer implementation
   * @code Timer_t *watchdogTimer;
   *
   * void initWatchdog(void) {
   *   xTimerCreate(&watchdogTimer, 5000);  // 5 second timeout
   * xTimerStart(watchdogTimer);
   * }
   *
   * void petWatchdog(void) {
   *   // Called periodically by application to prevent timeout
   * xTimerReset(watchdogTimer);
   * }
   *
   * void watchdogTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (OK(xTimerHasTimerExpired(watchdogTimer, &expired)) && expired) {
   *     // System hung - watchdog not reset in time logCritical("Watchdog
   * timeout");
   *     performSystemReset();
   *   }
   * }
   * @endcode
   *
   * Example 3: Debouncing button input
   * @code Timer_t *debounceTimer;
   * Base_t buttonPressed = 0;
   *
   * void onButtonPress(void) {
   *   if (!buttonPressed) {
   *     // First press - start debounce timer buttonPressed = 1;
   *     xTimerReset(debounceTimer);
   *     xTimerStart(debounceTimer);
   *     handleButtonPress();
   *   }
   * }
   *
   * void buttonTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (buttonPressed) {
   *     if (OK(xTimerHasTimerExpired(debounceTimer, &expired)) && expired) {
   *       // Debounce period elapsed - ready for next press buttonPressed = 0;
   *       xTimerStop(debounceTimer);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 4: Synchronized multi-timer operations
   * @code Timer_t *timer1, timer2, timer3;
   *
   * void synchronizeTimers(void) {
   *   // Reset all timers simultaneously to align phases xTimerReset(timer1);
   *   xTimerReset(timer2);
   *   xTimerReset(timer3);
   *
   *   // All timers now start from zero elapsed time
   * }
   *
   * void onSystemEvent(void) {
   *   // Synchronize timer phases on important system event
   * synchronizeTimers();
   * }
   * @endcode
   *
   * @param[in] timer_ Handle to the timer to reset. Must be a valid timer
   *                   created with xTimerCreate().
   *
   * @return           ReturnOK if timer reset successfully, ReturnError if
   *                   operation failed due to invalid timer handle or timer not
   *                   found.
   *
   * @warning xTimerReset() only clears elapsed time—it does NOT start a stopped
   * timer. If the timer is not active, call xTimerStart() before or after
   * xTimerReset() to begin timing.
   *
   * @warning After reset, the timer will not expire until a full period has
   * elapsed. If you reset a timer that has partially elapsed, you lose that
   * partial progress and must wait the full period again.
   *
   * @note xTimerReset() is typically called after detecting expiration to
   * acknowledge the event and start a new timing cycle.
   *
   * @note For periodic operations, the pattern is: check expired → perform
   * action →
   *       reset timer. This ensures you don't miss expiration events.
   *
   * @note Resetting does not change the timer's period. To change the period,
   * use xTimerChangePeriod() and then optionally xTimerReset().
   *
   * @note Calling xTimerReset() on an already-reset or newly-created timer is
   * safe and has no adverse effects.
   *
   * @sa xTimerHasTimerExpired() - Check if timer has expired before resetting
   * @sa xTimerStart() - Activate timer after reset
   * @sa xTimerStop() - Stop timer (also clears expiration)
   * @sa xTimerChangePeriod() - Change period (usually followed by reset)
   * @sa xTimerCreate() - Create timer (starts with zero elapsed time)
   * @sa xTimerIsTimerActive() - Check if timer is running
   */
  Return_t xTimerReset(Timer_t *timer_);


  /**
   * @brief Start an application timer to begin timing
   *
   * Activates an application timer, transitioning it from the stopped
   * (inactive) state to the running (active) state. Once started, the timer
   * begins accumulating elapsed time on each scheduler tick and will eventually
   * expire when elapsed time reaches the configured period. This operation is
   * required to enable timer expiration—only active timers can expire.
   *
   * xTimerStart() preserves the timer's current elapsed time. If you start a
   * timer that was previously stopped with xTimerStop(), it resumes from the
   * elapsed time it had accumulated before stopping. To start timing from zero,
   * call xTimerReset() before or after xTimerStart(). For newly created timers,
   * elapsed time is already zero, so xTimerReset() is not necessary.
   *
   * Key characteristics:
   * - **Activates timer**: Enables elapsed time accumulation
   * - **Preserves elapsed time**: Continues from current elapsed time
   * - **Enables expiration**: Only active timers can expire
   * - **Idempotent**: Starting an already-running timer has no effect
   * - **Non-blocking**: Returns immediately after state change
   *
   * Typical startup sequences:
   * - **New timer, start from zero**: xTimerCreate() → xTimerStart()
   * - **New timer, explicit reset**: xTimerCreate() → xTimerReset() →
   * xTimerStart()
   * - **Resume stopped timer**: xTimerStart() (continues from stopped elapsed
   * time)
   * - **Restart from zero**: xTimerReset() → xTimerStart()
   *
   * Example 1: Basic timer startup
   * @code Timer_t *periodicTimer;
   *
   * void initTimer(void) {
   *   // Create timer with 100 tick period if (OK(xTimerCreate(&periodicTimer,
   * 100))) {
   *     // Start timer - begins timing from zero xTimerStart(periodicTimer);
   *   }
   * }
   *
   * void periodicTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (OK(xTimerHasTimerExpired(periodicTimer, &expired)) && expired) {
   *     performPeriodicAction();
   *     xTimerReset(periodicTimer);  // Reset for next cycle
   *   }
   * }
   * @endcode
   *
   * Example 2: Conditional start based on system state
   * @code Timer_t *monitorTimer;
   *
   * void enableMonitoring(void) {
   *   Base_t isActive;
   *
   *   if (OK(xTimerIsTimerActive(monitorTimer, &isActive)) && !isActive) {
   *     // Timer not running - start it xTimerReset(monitorTimer);  // Start
   * from zero xTimerStart(monitorTimer);
   *     logInfo("Monitoring enabled");
   *   }
   * }
   *
   * void disableMonitoring(void) {
   *   Base_t isActive;
   *
   *   if (OK(xTimerIsTimerActive(monitorTimer, &isActive)) && isActive) {
   *     xTimerStop(monitorTimer);
   *     logInfo("Monitoring disabled");
   *   }
   * }
   * @endcode
   *
   * Example 3: Pause and resume pattern
   * @code Timer_t *processTimer;
   *
   * void pauseProcessing(void) {
   *   // Stop timer - preserves elapsed time xTimerStop(processTimer);
   *   logInfo("Processing paused");
   * }
   *
   * void resumeProcessing(void) {
   *   // Start timer - continues from paused elapsed time
   * xTimerStart(processTimer);
   *   logInfo("Processing resumed");
   * }
   *
   * void resetAndRestart(void) {
   *   // Restart from zero elapsed time xTimerReset(processTimer);
   *   xTimerStart(processTimer);
   *   logInfo("Processing restarted from zero");
   * }
   * @endcode
   *
   * Example 4: Multiple timer coordination
   * @code Timer_t *timer1, timer2, timer3;
   *
   * void startAllTimers(void) {
   *   // Reset all to zero xTimerReset(timer1);
   *   xTimerReset(timer2);
   *   xTimerReset(timer3);
   *
   *   // Start all simultaneously xTimerStart(timer1);
   *   xTimerStart(timer2);
   *   xTimerStart(timer3);
   *
   *   logInfo("All timers started synchronously");
   * }
   *
   * void stopAllTimers(void) {
   *   xTimerStop(timer1);
   *   xTimerStop(timer2);
   *   xTimerStop(timer3);
   *
   *   logInfo("All timers stopped");
   * }
   * @endcode
   *
   * @param[in] timer_ Handle to the timer to start. Must be a valid timer
   *                   created with xTimerCreate().
   *
   * @return           ReturnOK if timer started successfully, ReturnError if
   *                   operation failed due to invalid timer handle or timer not
   *                   found.
   *
   * @warning xTimerStart() does NOT reset elapsed time. If you stopped a timer
   * partway through its period and then start it again, it will continue from
   * where it left off. Call xTimerReset() before xTimerStart() if you want to
   * begin timing from zero.
   *
   * @warning Only active (started) timers can expire. Calling
   * xTimerHasTimerExpired() on a stopped timer will always return false, even
   * if sufficient time has passed.
   *
   * @note xTimerStart() is idempotent—calling it multiple times on an
   * already-running timer has no effect and is not an error.
   *
   * @note For newly created timers, xTimerReset() is not necessary before
   * xTimerStart() because elapsed time is already zero.
   *
   * @note The timer begins accumulating elapsed time immediately on the next
   * scheduler tick after xTimerStart() is called.
   *
   * @sa xTimerStop() - Stop timer (opposite operation)
   * @sa xTimerReset() - Reset elapsed time to zero
   * @sa xTimerCreate() - Create timer (starts in stopped state)
   * @sa xTimerIsTimerActive() - Check if timer is running
   * @sa xTimerHasTimerExpired() - Check for expiration (requires active timer)
   * @sa xTimerChangePeriod() - Change timer period
   */
  Return_t xTimerStart(Timer_t *timer_);


  /**
   * @brief Stop an application timer and halt timing
   *
   * Deactivates an application timer, transitioning it from the running
   * (active) state to the stopped (inactive) state. Once stopped, the timer
   * ceases accumulating elapsed time and cannot expire, even if sufficient
   * scheduler ticks pass. The timer's current elapsed time is preserved,
   * allowing for pause-and-resume scenarios if needed.
   *
   * xTimerStop() is the primary method for temporarily disabling timing without
   * destroying the timer. Unlike xTimerDelete(), which permanently removes the
   * timer, xTimerStop() keeps the timer structure intact and allows restarting
   * with xTimerStart(). If the timer was expired when stopped, the expiration
   * state is cleared—xTimerHasTimerExpired() will return false after stopping.
   *
   * Key characteristics:
   * - **Deactivates timer**: Halts elapsed time accumulation
   * - **Preserves elapsed time**: Current elapsed time is maintained
   * - **Clears expiration**: Expired state is reset to non-expired
   * - **Prevents expiration**: Stopped timers cannot expire
   * - **Idempotent**: Stopping an already-stopped timer has no effect
   * - **Non-blocking**: Returns immediately after state change
   *
   * Common stop scenarios:
   * - **Conditional timing**: Stop timer when condition is met
   * - **Pause-resume patterns**: Stop to pause, xTimerStart() to resume
   * - **Event completion**: Stop timer after timed event finishes early
   * - **Power management**: Stop non-essential timers to reduce overhead
   * - **Timeout cancellation**: Stop timer when expected response arrives
   *
   * Example 1: Communication timeout with early completion
   * @code Timer_t *timeoutTimer;
   *
   * void sendRequest(void) {
   *   transmitData();
   *
   *   // Start timeout timer xTimerReset(timeoutTimer);
   *   xTimerStart(timeoutTimer);
   * }
   *
   * void onResponseReceived(void) {
   *   // Response arrived - cancel timeout xTimerStop(timeoutTimer);
   *   processResponse();
   * }
   *
   * void commTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (OK(xTimerHasTimerExpired(timeoutTimer, &expired)) && expired) {
   *     // Timeout - no response received xTimerStop(timeoutTimer);
   *     handleTimeout();
   *   }
   * }
   * @endcode
   *
   * Example 2: Conditional timer management
   * @code Timer_t *activityTimer;
   *
   * void onActivityDetected(void) {
   *   Base_t isActive;
   *
   *   // Start timer on activity if (OK(xTimerIsTimerActive(activityTimer,
   * &isActive)) && !isActive) {
   *     xTimerReset(activityTimer);
   *     xTimerStart(activityTimer);
   *   }
   * }
   *
   * void onIdleCondition(void) {
   *   Base_t isActive;
   *
   *   // Stop timer when system goes idle if
   * (OK(xTimerIsTimerActive(activityTimer, &isActive)) && isActive) {
   *     xTimerStop(activityTimer);
   *   }
   * }
   * @endcode
   *
   * Example 3: Pause and resume timing
   * @code Timer_t *processTimer;
   *
   * void pauseProcessing(void) {
   *   // Stop timer - preserves elapsed time for resume
   * xTimerStop(processTimer);
   *   suspendProcessing();
   * }
   *
   * void resumeProcessing(void) {
   *   // Start timer - continues from paused elapsed time
   * xTimerStart(processTimer);
   *   continueProcessing();
   * }
   *
   * void cancelProcessing(void) {
   *   // Stop and reset - clears elapsed time xTimerStop(processTimer);
   *   xTimerReset(processTimer);
   *   cleanupProcessing();
   * }
   * @endcode
   *
   * Example 4: Power-aware timer management
   * @code Timer_t *backgroundTimers[MAX_TIMERS];
   * Base_t timerCount = 0;
   *
   * void enterLowPowerMode(void) {
   *   // Stop all non-critical timers for (Base_t i = 0; i < timerCount; i++) {
   *     xTimerStop(backgroundTimers[i]);
   *   }
   *   enableSleepMode();
   * }
   *
   * void exitLowPowerMode(void) {
   *   // Resume all timers disableSleepMode();
   *   for (Base_t i = 0; i < timerCount; i++) {
   *     xTimerStart(backgroundTimers[i]);
   *   }
   * }
   * @endcode
   *
   * Example 5: One-shot timer pattern
   * @code Timer_t *oneShotTimer;
   *
   * void startOneShotTimer(Ticks_t delay) {
   *   // Configure and start one-shot timer xTimerChangePeriod(oneShotTimer,
   * delay);
   *   xTimerReset(oneShotTimer);
   *   xTimerStart(oneShotTimer);
   * }
   *
   * void timerTask(Task_t *task, TaskParm_t *parm) {
   *   Base_t expired;
   *
   *   if (OK(xTimerHasTimerExpired(oneShotTimer, &expired)) && expired) {
   *     // Timer expired - stop to prevent re-triggering
   * xTimerStop(oneShotTimer);
   *     performOneShotAction();
   *   }
   * }
   * @endcode
   *
   * @param[in] timer_ Handle to the timer to stop. Must be a valid timer
   *                   created with xTimerCreate().
   *
   * @return           ReturnOK if timer stopped successfully, ReturnError if
   *                   operation failed due to invalid timer handle or timer not
   *                   found.
   *
   * @warning xTimerStop() preserves the current elapsed time. If you later call
   * xTimerStart() without calling xTimerReset(), the timer will continue from
   * its stopped elapsed time, not from zero. Call xTimerReset() after
   * xTimerStop() if you want to clear elapsed time.
   *
   * @warning Stopped timers cannot expire. xTimerHasTimerExpired() will return
   * false for stopped timers regardless of elapsed time.
   *
   * @note xTimerStop() clears the expiration flag. Even if the timer was
   * expired when stopped, xTimerHasTimerExpired() will return false after the
   * stop.
   *
   * @note xTimerStop() is idempotent—calling it multiple times on an
   * already-stopped timer has no effect and is not an error.
   *
   * @note Stopping a timer does not free its resources. The timer remains
   * allocated and can be restarted with xTimerStart(). To free timer resources,
   * use xTimerDelete().
   *
   * @note Newly created timers are already in the stopped state. Calling
   * xTimerStop() on a newly created timer has no effect but is not an error.
   *
   * @sa xTimerStart() - Start timer (opposite operation)
   * @sa xTimerReset() - Reset elapsed time (often called after stop)
   * @sa xTimerDelete() - Permanently remove timer and free resources
   * @sa xTimerIsTimerActive() - Check if timer is running
   * @sa xTimerHasTimerExpired() - Check for expiration (always false for
   * stopped timers)
   * @sa xTimerCreate() - Create timer (starts in stopped state)
   */
  Return_t xTimerStop(Timer_t *timer_);


  /* Filesystem syscalls */


  /**
   * @brief Format a block device with a FAT32 filesystem
   *
   * Initializes a block device with a FAT32 filesystem structure, creating the
   * boot sector, file allocation tables, and root directory. This operation
   * prepares a storage device for use with HeliOS filesystem operations by
   * writing the complete FAT32 metadata structures.
   *
   * Formatting is destructive—all existing data on the block device is lost.
   * This operation should only be performed on new devices or when explicitly
   * reinitializing storage. After formatting, the device must be mounted with
   * xFSMount() before files can be created or accessed.
   *
   * The formatting process:
   * 1. Validates block device is registered and operational 2. Calculates
   * optimal FAT32 parameters based on device size 3. Writes boot sector with
   * filesystem parameters 4. Initializes two file allocation tables (FAT1 and
   * FAT2 for redundancy) 5. Creates empty root directory 6. Sets volume label
   * if provided
   *
   * Common scenarios:
   * - **Initial setup**: Format new RAM disk or flash storage
   * - **Factory reset**: Erase all data and reinitialize filesystem
   * - **Corruption recovery**: Reformat after filesystem corruption
   * - **Testing**: Create clean filesystem for unit tests
   *
   * Example 1: Format RAM disk with volume label
   * @code
   * #include "block_driver.h"
   * #include "ramdisk_driver.h"
   *
   * // Setup and format RAM disk HalfWord_t ramDiskUID = 0;
   * HalfWord_t blockDevUID = 0;
   *
   * // Initialize drivers if (OK(__RamDiskRegister__(&ramDiskUID))) {
   *   if (OK(__BlockDeviceRegister__(&blockDevUID))) {
   *     // Configure block device to use RAM disk BlockDeviceConfig_t config;
   *     config.protocol = BLOCK_PROTOCOL_RAW;
   *     config.backingDeviceUID = ramDiskUID;
   *
   *     if (OK(__DeviceConfigDevice__(blockDevUID, sizeof(config),
   * (Byte_t*)&config))) {
   *       // Format with volume label if (OK(xFSFormat(blockDevUID,
   * (Byte_t*)"HELIOS_VOL"))) {
   *         // Filesystem ready to mount
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Format and mount in one operation
   * @code Return_t initializeFilesystem(HalfWord_t blockDeviceUID, Volume_t *
   * volume) {
   *   // Format the device if (ERROR(xFSFormat(blockDeviceUID,
   * (Byte_t*)"STORAGE"))) {
   *     return ReturnError;
   *   }
   *
   *   // Mount the freshly formatted filesystem if (ERROR(xFSMount(volume,
   * blockDeviceUID))) {
   *     return ReturnError;
   *   }
   *
   *   // Filesystem formatted and ready for use return ReturnOK;
   * }
   * @endcode
   *
   * Example 3: Factory reset function
   * @code Volume_t *systemVolume;
   * HalfWord_t storageDeviceUID;
   *
   * void factoryReset(void) {
   *   // Unmount if currently mounted xFSUnmount(systemVolume);
   *
   *   // Reformat - destroys all data if (OK(xFSFormat(storageDeviceUID,
   * (Byte_t*)"FACTORY"))) {
   *     // Remount fresh filesystem if (OK(xFSMount(&systemVolume,
   * storageDeviceUID))) {
   *       // Create default configuration files
   * createDefaultConfig(systemVolume);
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in] blockDeviceUID_ UID of the block device to format. Must be a
   *                            registered block device obtained from block
   *                            device driver registration. The device must
   *                            support read and write operations.
   * @param[in] volumeLabel_    Pointer to null-terminated string containing
   *                            volume label (max 11 characters). Can be null
   *                            for no label. Label is stored in boot sector and
   *                            visible in volume information queries.
   *
   * @return                    ReturnOK if format succeeded, ReturnError if
   *                            format failed due to invalid block device UID,
   *                            device not available, insufficient device size,
   *                            or write errors during format operation.
   *
   * @warning This operation is DESTRUCTIVE. All existing data on the block
   * device will be permanently lost. Ensure the correct device UID is specified
   * and any important data is backed up before formatting.
   *
   * @warning The block device must be large enough to hold FAT32 structures.
   * Very small devices may fail to format. Minimum practical size depends on
   * sector size and cluster size calculations.
   *
   * @warning Do not format a currently mounted volume. Always call xFSUnmount()
   * before formatting to prevent filesystem corruption.
   *
   * @note After formatting, the volume must be mounted with xFSMount() before
   * any file operations can be performed.
   *
   * @note HeliOS uses FAT32 filesystem format for compatibility with standard
   * FAT32 implementations and tools.
   *
   * @note The volume label is optional but recommended for identifying storage
   * volumes in systems with multiple devices.
   *
   * @sa xFSMount() - Mount formatted volume for file operations
   * @sa xFSUnmount() - Unmount volume before reformatting
   * @sa xFSGetVolumeInfo() - Query volume information including label
   * @sa __BlockDeviceRegister__() - Register block device before formatting
   */
  Return_t xFSFormat(const HalfWord_t blockDeviceUID_, const Byte_t *volumeLabel_);


  /**
   * @brief Mount a FAT32 filesystem from a block device
   *
   * Mounts a FAT32 filesystem from a formatted block device, making it
   * available for file and directory operations. Mounting reads and validates
   * the filesystem metadata (boot sector, FAT tables) and creates a volume
   * handle used in all subsequent filesystem operations.
   *
   * A volume must be mounted before any file operations (open, read, write,
   * etc.) can be performed. The volume handle returned by this function is
   * required by all file and directory functions to identify which filesystem
   * to operate on.
   *
   * Mount process:
   * 1. Validates block device UID and availability 2. Reads boot sector from
   * block device 3. Validates FAT32 boot sector signature and parameters 4.
   * Allocates volume structure in kernel memory 5. Stores filesystem parameters
   * for subsequent operations 6. Returns volume handle for use in
   * file/directory operations
   *
   * Common usage patterns:
   * - **System initialization**: Mount filesystem during startup
   * - **Removable media**: Mount when device inserted, unmount when removed
   * - **Multi-volume systems**: Mount multiple block devices as separate
   * volumes
   * - **Filesystem switching**: Unmount and remount different filesystems
   *
   * Example 1: Basic mount operation
   * @code Volume_t *myVolume;
   * HalfWord_t blockDeviceUID = 1;  // From block device registration
   *
   * // Mount the filesystem if (OK(xFSMount(&myVolume, blockDeviceUID))) {
   *   // Volume mounted - can now open files File_t *file;
   *   if (OK(xFileOpen(&file, myVolume, (Byte_t*)"/data.txt", FS_MODE_READ))) {
   *     // File opened successfully xFileClose(file);
   *   }
   *
   *   // Always unmount when done xFSUnmount(myVolume);
   * }
   * @endcode
   *
   * Example 2: System initialization with mount
   * @code Volume_t *systemVolume;
   * HalfWord_t storageUID;
   *
   * void initStorage(void) {
   *   // Register and configure block device if
   * (OK(__BlockDeviceRegister__(&storageUID))) {
   *     BlockDeviceConfig_t config;
   *     config.protocol = BLOCK_PROTOCOL_RAW;
   *     config.backingDeviceUID = ramDiskUID;
   *
   *     if (OK(__DeviceConfigDevice__(storageUID, sizeof(config),
   * (Byte_t*)&config))) {
   *       // Try to mount existing filesystem if (ERROR(xFSMount(&systemVolume,
   * storageUID))) {
   *         // Mount failed - format and mount fresh filesystem if
   * (OK(xFSFormat(storageUID, (Byte_t*)"SYSTEM"))) {
   *           xFSMount(&systemVolume, storageUID);
   *         }
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Multi-volume system
   * @code Volume_t *dataVolume;
   * Volume_t *configVolume;
   * HalfWord_t dataDeviceUID = 1;
   * HalfWord_t configDeviceUID = 2;
   *
   * void mountAllVolumes(void) {
   *   // Mount data volume if (OK(xFSMount(&dataVolume, dataDeviceUID))) {
   *     // Mount config volume if (OK(xFSMount(&configVolume,
   * configDeviceUID))) {
   *       // Both volumes mounted - use different handles xFileOpen(&file1,
   * dataVolume, (Byte_t*)"/data.bin", FS_MODE_READ);
   *       xFileOpen(&file2, configVolume, (Byte_t*)"/config.txt",
   * FS_MODE_READ);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 4: Mount with volume information query
   * @code Volume_t *vol;
   * HalfWord_t devUID = 1;
   *
   * if (OK(xFSMount(&vol, devUID))) {
   *   // Query volume information VolumeInfo_t *info;
   *   if (OK(xFSGetVolumeInfo(vol, &info))) {
   *     // Check available space if (info.freeBytes > (100 * 1024)) {  // 100
   * KB free
   *       // Sufficient space for operations performFileOperations(vol);
   *     }
   *   }
   *
   *   xFSUnmount(vol);
   * }
   * @endcode
   *
   * @param[out] volume_         Pointer to Volume_t *handle to be initialized.
   *                             On success, this handle is used in all
   *                             subsequent file and directory operations. The
   *                             handle remains valid until xFSUnmount() is
   *                             called.
   * @param[in]  blockDeviceUID_ UID of the block device containing the FAT32
   *                             filesystem. Must be a registered block device
   *                             that has been formatted with xFSFormat().
   *
   * @return                     ReturnOK if mount succeeded, ReturnError if
   *                             mount failed due to invalid block device UID,
   *                             device not available, invalid FAT32 filesystem,
   *                             corrupted boot sector, or memory allocation
   *                             failure.
   *
   * @warning The block device must contain a valid FAT32 filesystem created by
   * xFSFormat() or another FAT32-compatible tool. Attempting to mount an
   * unformatted or corrupted device will fail.
   *
   * @warning Only one volume can be mounted per block device at a time.
   * Attempting to mount an already-mounted device will fail.
   *
   * @warning The volume handle must remain valid for the duration of all file
   * operations. Do not unmount a volume while files are still open.
   *
   * @note The volume handle is allocated from kernel memory and persists until
   * xFSUnmount() is called. Always unmount volumes when finished to free kernel
   * resources.
   *
   * @note Multiple volumes can be mounted simultaneously if you have multiple
   * block devices, each with its own volume handle.
   *
   * @note After mounting, use xFSGetVolumeInfo() to query volume capacity, free
   * space, and other filesystem parameters.
   *
   * @sa xFSUnmount() - Unmount volume and free resources
   * @sa xFSFormat() - Format block device with FAT32 filesystem
   * @sa xFSGetVolumeInfo() - Query volume information
   * @sa xFileOpen() - Open file on mounted volume
   * @sa xDirOpen() - Open directory on mounted volume
   */
  Return_t xFSMount(Volume_t **volume_, const HalfWord_t blockDeviceUID_);


  /**
   * @brief Unmount a FAT32 filesystem volume
   *
   * Unmounts a previously mounted FAT32 volume, flushing any pending writes and
   * freeing all associated kernel resources. After unmounting, the volume
   * handle becomes invalid and must not be used in any subsequent filesystem
   * operations.
   *
   * Unmounting ensures filesystem consistency by finalizing all pending
   * operations and is essential for proper resource management in embedded
   * systems. Always unmount volumes before system shutdown, device removal, or
   * when the filesystem is no longer needed.
   *
   * Unmount process:
   * 1. Validates volume handle 2. Flushes any cached filesystem data to block
   * device 3. Updates FAT tables if necessary 4. Frees volume structure from
   * kernel memory 5. Invalidates volume handle
   *
   * Common scenarios:
   * - **System shutdown**: Unmount all volumes before power-off
   * - **Resource cleanup**: Free kernel memory when filesystem not needed
   * - **Device removal**: Safely unmount before removing storage media
   * - **Filesystem switching**: Unmount before reformatting or mounting
   * different volume
   *
   * Example 1: Basic mount/unmount lifecycle
   * @code Volume_t *vol;
   * HalfWord_t deviceUID = 1;
   *
   * // Mount volume if (OK(xFSMount(&vol, deviceUID))) {
   *   // Perform file operations File_t *file;
   *   xFileOpen(&file, vol, (Byte_t*)"/data.txt", FS_MODE_WRITE |
   * FS_MODE_CREATE);
   *   xFileWrite(file, 5, (Byte_t*)"Hello");
   *   xFileClose(file);
   *
   *   // Always unmount when done xFSUnmount(vol);
   *   // vol is now invalid - do not use
   * }
   * @endcode
   *
   * Example 2: System shutdown cleanup
   * @code Volume_t *dataVolume;
   * Volume_t *configVolume;
   *
   * void shutdownFilesystems(void) {
   *   // Unmount all volumes before shutdown xFSUnmount(dataVolume);
   *   xFSUnmount(configVolume);
   *
   *   // Volumes now unmounted and resources freed
   * }
   * @endcode
   *
   * Example 3: Safe volume handle cleanup
   * @code Volume_t *vol = null;
   * HalfWord_t devUID = 1;
   *
   * void useVolume(void) {
   *   if (OK(xFSMount(&vol, devUID))) {
   *     performOperations(vol);
   *
   *     // Unmount and nullify handle xFSUnmount(vol);
   *     vol = null;  // Prevent use-after-unmount
   *   }
   * }
   * @endcode
   *
   * @param[in] volume_ Handle to the mounted volume to unmount. Must be a valid
   *                    volume obtained from a previous successful xFSMount()
   *                    call. After unmounting, this handle becomes invalid.
   *
   * @return            ReturnOK if unmount succeeded, ReturnError if unmount
   *                    failed due to invalid volume handle or volume not found.
   *
   * @warning All files and directories must be closed before unmounting.
   * Unmounting a volume with open file handles may cause resource leaks or
   * undefined behavior.
   *
   * @warning After calling xFSUnmount(), the volume handle becomes invalid and
   * must not be used in any filesystem operations. Attempting to use an
   * unmounted volume will result in ReturnError.
   *
   * @warning Always unmount volumes before reformatting the block device or
   * removing storage media to prevent filesystem corruption.
   *
   * @note It is good practice to set the volume handle to null after unmounting
   * to prevent accidental use of an invalid handle.
   *
   * @note Unmounting frees kernel memory allocated during xFSMount(). In
   * long-running systems, always unmount volumes when they are no longer
   * needed.
   *
   * @sa xFSMount() - Mount a FAT32 filesystem volume
   * @sa xFileClose() - Close files before unmounting
   * @sa xDirClose() - Close directories before unmounting
   * @sa xFileSync() - Explicitly flush file data before unmount
   */
  Return_t xFSUnmount(Volume_t *volume_);


  /**
   * @brief Query information about a mounted FAT32 volume
   *
   * Retrieves comprehensive information about a mounted volume including
   * capacity, free space, and filesystem parameters. This information is useful
   * for monitoring storage utilization, checking available space before write
   * operations, and reporting filesystem status.
   *
   * The volume information includes both logical (cluster-based) and physical
   * (byte-based) size metrics, allowing applications to make informed decisions
   * about storage management and space allocation.
   *
   * Information provided:
   * - Total and free space in clusters and bytes
   * - Bytes per sector (typically 512)
   * - Sectors per cluster
   * - Bytes per cluster (sector size × sectors per cluster)
   *
   * Common use cases:
   * - **Space checking**: Verify sufficient space before write operations
   * - **Usage monitoring**: Track filesystem utilization over time
   * - **Status reporting**: Display storage capacity to user/diagnostic
   * interface
   * - **Quota management**: Implement storage quotas based on available space
   *
   * Example 1: Check space before writing file
   * @code Volume_t *vol;
   * VolumeInfo_t *info;
   *
   * Return_t writeDataFile(Volume_t *vol, Byte_t *data, Size_t dataSize) {
   *   // Check available space if (OK(xFSGetVolumeInfo(vol, &info))) {
   *     if (info.freeBytes >= dataSize) {
   *       // Sufficient space - proceed with write File_t *file;
   *       if (OK(xFileOpen(&file, vol, (Byte_t*)"/data.bin", FS_MODE_WRITE |
   * FS_MODE_CREATE))) {
   *         xFileWrite(file, dataSize, data);
   *         xFileClose(file);
   *         return ReturnOK;
   *       }
   *     } else {
   *       // Insufficient space return ReturnError;
   *     }
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 2: Storage utilization monitoring
   * @code Volume_t *vol;
   *
   * void reportStorageStatus(void) {
   *   VolumeInfo_t *info;
   *
   *   if (OK(xFSGetVolumeInfo(vol, &info))) {
   *     Word_t usedBytes = info.totalBytes - info.freeBytes;
   *     Byte_t percentUsed = (Byte_t)((usedBytes * 100) / info.totalBytes);
   *
   *     // Report to diagnostic interface printf("Storage: %lu / %lu bytes
   * (%u%% used)\n", usedBytes, info.totalBytes, percentUsed);
   *
   *     // Warn if nearly full if (percentUsed > 90) {
   *       logWarning("Storage nearly full");
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Filesystem parameter reporting
   * @code void displayVolumeDetails(Volume_t *vol) {
   *   VolumeInfo_t *info;
   *
   *   if (OK(xFSGetVolumeInfo(vol, &info))) {
   *     printf("Volume Information:\n");
   *     printf("  Total: %lu bytes (%lu clusters)\n", info.totalBytes,
   * info.totalClusters);
   *     printf("  Free:  %lu bytes (%lu clusters)\n", info.freeBytes,
   * info.freeClusters);
   *     printf("  Cluster size: %lu bytes\n", info.bytesPerCluster);
   *     printf("  Sector size:  %u bytes\n", info.bytesPerSector);
   *     printf("  Sectors/cluster: %u\n", info.sectorsPerCluster);
   *   }
   * }
   * @endcode
   *
   * @param[in]  volume_ Handle to the mounted volume to query. Must be a valid
   *                     volume obtained from xFSMount().
   * @param[out] info_   Pointer to VolumeInfo_t *structure to receive volume
   *                     information. On success, this structure is populated
   *                     with current volume statistics and parameters.
   *
   * @return             ReturnOK if query succeeded, ReturnError if query
   *                     failed due to invalid volume handle or volume not
   *                     mounted.
   *
   * @note Free space calculation requires scanning the File Allocation Table
   * (FAT), which may take time on large volumes. Cache the result if you need
   * to check free space frequently.
   *
   * @note The free space reported is the total free space on the volume. Actual
   * usable space may be less due to filesystem overhead and cluster alignment.
   *
   * @note All byte values are in units of bytes. Cluster values represent the
   * number of allocation units (clusters) used by the filesystem.
   *
   * @sa xFSMount() - Mount volume before querying information
   * @sa xFSFormat() - Format operation sets volume parameters
   * @sa xFileGetSize() - Get size of individual files
   */
  Return_t xFSGetVolumeInfo(const Volume_t *volume_, VolumeInfo_t **info_);


  /**
   * @brief Open a file on a mounted FAT32 volume
   *
   * Opens a file for reading, writing, or both, creating a file handle used for
   * subsequent file operations. The file can be opened in various modes
   * controlling read/write access, creation behavior, and positioning. File
   * paths use forward slash notation (/path/to/file.txt) following standard
   * POSIX conventions.
   *
   * Opening a file allocates kernel resources and establishes the connection
   * between the application and the filesystem. The returned file handle must
   * be used in all subsequent operations on that file and must be closed with
   * xFileClose() when finished to prevent resource leaks.
   *
   * File open modes (can be combined with bitwise OR):
   * - **FS_MODE_READ (0x01)**: Open for reading. File must exist.
   * - **FS_MODE_WRITE (0x02)**: Open for writing. Truncates existing file to
   * zero length.
   * - **FS_MODE_APPEND (0x04)**: Open for appending. Positions at end of file.
   * - **FS_MODE_CREATE (0x08)**: Create file if it doesn't exist. Combine with
   * WRITE or APPEND.
   *
   * Common mode combinations:
   * - Read-only: FS_MODE_READ
   * - Write (truncate): FS_MODE_WRITE
   * - Write (create if needed): FS_MODE_WRITE | FS_MODE_CREATE
   * - Append: FS_MODE_APPEND | FS_MODE_CREATE
   * - Read/Write: FS_MODE_READ | FS_MODE_WRITE
   *
   * Path format:
   * - Use forward slashes: "/file.txt" or "/dir/subdir/file.dat"
   * - Root directory files: "/filename.ext"
   * - Subdirectory files: "/directory/filename.ext"
   * - Maximum path length: CONFIG_FS_MAX_PATH_LENGTH (default 256 characters)
   *
   * Example 1: Read existing file
   * @code Volume_t *vol;
   * File_t *file;
   *
   * if (OK(xFSMount(&vol, blockDeviceUID))) {
   *   // Open file for reading if (OK(xFileOpen(&file, vol,
   * (Byte_t*)"/config.txt", FS_MODE_READ))) {
   *     Byte_t *data;
   *     Word_t fileSize;
   *
   *     // Get file size and read all data if (OK(xFileGetSize(file,
   * &fileSize))) {
   *       if (OK(xFileRead(file, fileSize, &data))) {
   *         processData(data, fileSize);
   *         xMemFree((Addr_t *)data);
   *       }
   *     }
   *
   *     xFileClose(file);
   *   }
   *   xFSUnmount(vol);
   * }
   * @endcode
   *
   * Example 2: Create and write new file
   * @code Volume_t *vol;
   * File_t *file;
   *
   * if (OK(xFSMount(&vol, deviceUID))) {
   *   // Open file for writing, create if doesn't exist if (OK(xFileOpen(&file,
   * vol, (Byte_t*)"/data.bin", FS_MODE_WRITE | FS_MODE_CREATE))) {
   *     Byte_t buffer[128];
   *     generateData(buffer, sizeof(buffer));
   *
   *     // Write data if (OK(xFileWrite(file, sizeof(buffer), buffer))) {
   *       // Data written successfully
   *     }
   *
   *     xFileClose(file);
   *   }
   *   xFSUnmount(vol);
   * }
   * @endcode
   *
   * Example 3: Append to log file
   * @code Volume_t *vol;
   *
   * void logMessage(const char *message) {
   *   File_t *logFile;
   *
   *   // Open log file in append mode, create if doesn't exist if
   * (OK(xFileOpen(&logFile, vol, (Byte_t*)"/system.log", FS_MODE_APPEND |
   * FS_MODE_CREATE))) {
   *     Size_t msgLen = strlen(message);
   *     xFileWrite(logFile, msgLen, (Byte_t*)message);
   *     xFileWrite(logFile, 1, (Byte_t*)"\n");  // Add newline
   *
   *     xFileClose(logFile);
   *   }
   * }
   * @endcode
   *
   * Example 4: Update existing file (read/write)
   * @code File_t *file;
   *
   * // Open for both reading and writing if (OK(xFileOpen(&file, vol,
   * (Byte_t*)"/counter.dat", FS_MODE_READ | FS_MODE_WRITE))) {
   *   Byte_t *data;
   *   Word_t counter;
   *
   *   // Read current value if (OK(xFileRead(file, sizeof(Word_t), &data))) {
   *     counter = *(Word_t*)data;
   *     xMemFree((Addr_t *)data);
   *
   *     // Increment counter counter++;
   *
   *     // Seek back to start and write new value xFileSeek(file, 0,
   * FS_SEEK_SET);
   *     xFileWrite(file, sizeof(Word_t), (Byte_t*)&counter);
   *   }
   *
   *   xFileClose(file);
   * }
   * @endcode
   *
   * @param[out] file_   Pointer to File_t *handle to be initialized. On
   *                     success, this handle is used for all subsequent
   *                     operations on the opened file. The handle remains valid
   *                     until xFileClose() is called.
   * @param[in]  volume_ Handle to mounted volume containing the file. Must be a
   *                     valid volume from xFSMount().
   * @param[in]  path_   Pointer to null-terminated string containing file path.
   *                     Use forward slashes (/) for directory separators. Path
   *                     is relative to volume root.
   * @param[in]  mode_   File open mode flags. Combine flags with bitwise OR
   *                     (|). See FS_MODE_* constants for available modes.
   *
   * @return             ReturnOK if file opened successfully, ReturnError if
   *                     open failed due to invalid volume, file not found (read
   *                     mode without CREATE), insufficient space (create mode),
   *                     invalid path, or too many open files.
   *
   * @warning The file handle must be closed with xFileClose() when finished.
   * Failure to close files causes resource leaks and may prevent other files
   * from being opened.
   *
   * @warning Opening a file with FS_MODE_WRITE (without APPEND) truncates the
   * file to zero length immediately, destroying existing content. Use
   * FS_MODE_READ |
   *          FS_MODE_WRITE if you need to modify existing content without
   * truncation.
   *
   * @warning File paths are case-sensitive. "/File.txt" and "/file.txt" are
   * different files.
   *
   * @warning Do not attempt to open the same file multiple times
   * simultaneously. This may cause data corruption or filesystem
   * inconsistencies.
   *
   * @note File handles are allocated from kernel memory and limited by system
   * resources. Always close files promptly after use.
   *
   * @note When creating files with FS_MODE_CREATE, parent directories must
   * already exist. Use xDirMake() to create directories before creating files
   * within them.
   *
   * @note File position starts at 0 for READ and WRITE modes, and at
   * end-of-file for APPEND mode. Use xFileSeek() to change position after
   * opening.
   *
   * @sa xFileClose() - Close file and free resources
   * @sa xFileRead() - Read data from file
   * @sa xFileWrite() - Write data to file
   * @sa xFileSeek() - Change file position
   * @sa xFSMount() - Mount volume before opening files
   */
  Return_t xFileOpen(File_t **file_, Volume_t *volume_, const Byte_t *path_, const Byte_t mode_);


  /**
   * @brief Close an open file
   *
   * Closes a previously opened file, flushing any pending writes to storage and
   * freeing the file handle and associated kernel resources. After closing, the
   * file handle becomes invalid and must not be used in any subsequent file
   * operations.
   *
   * Closing a file ensures data integrity by committing all buffered writes to
   * the filesystem and updating file metadata (size, modification time, etc.).
   * Always close files when finished to prevent data loss and resource leaks.
   *
   * Close process:
   * 1. Flushes any buffered write data to block device 2. Updates file
   * directory entry (size, attributes) 3. Updates File Allocation Table if file
   * size changed 4. Frees file handle from kernel memory 5. Invalidates file
   * handle
   *
   * Common scenarios:
   * - **Normal completion**: Close file after successful read/write operations
   * - **Error handling**: Close file even if operations failed
   * - **Resource management**: Close files promptly to free kernel resources
   * - **Shutdown**: Close all open files before unmounting volume
   *
   * Example 1: Basic open/close pattern
   * @code File_t *file;
   *
   * if (OK(xFileOpen(&file, vol, (Byte_t*)"/data.txt", FS_MODE_READ))) {
   *   // Perform file operations Byte_t *data;
   *   if (OK(xFileRead(file, 100, &data))) {
   *     processData(data, 100);
   *     xMemFree((Addr_t *)data);
   *   }
   *
   *   // Always close when done xFileClose(file);
   *   // file is now invalid - do not use
   * }
   * @endcode
   *
   * Example 2: Error handling with guaranteed close
   * @code Return_t writeData(Volume_t *vol, const char *path, Byte_t *data,
   * Size_t size) {
   *   File_t *file;
   *   Return_t result = ReturnError;
   *
   *   if (OK(xFileOpen(&file, vol, (Byte_t*)path, FS_MODE_WRITE |
   * FS_MODE_CREATE))) {
   *     if (OK(xFileWrite(file, size, data))) {
   *       result = ReturnOK;
   *     }
   *
   *     // Close file regardless of write success/failure xFileClose(file);
   *   }
   *
   *   return result;
   * }
   * @endcode
   *
   * Example 3: Multiple file handling
   * @code void processFiles(Volume_t *vol) {
   *   File_t *input, output;
   *
   *   // Open input file if (OK(xFileOpen(&input, vol, (Byte_t*)"/input.dat",
   * FS_MODE_READ))) {
   *     // Open output file if (OK(xFileOpen(&output, vol,
   * (Byte_t*)"/output.dat", FS_MODE_WRITE | FS_MODE_CREATE))) {
   *       // Process data from input to output Byte_t *data;
   *       Word_t size;
   *       if (OK(xFileGetSize(input, &size)) && OK(xFileRead(input, size,
   * &data))) {
   *         xFileWrite(output, size, data);
   *         xMemFree((Addr_t *)data);
   *       }
   *
   *       xFileClose(output);  // Close output first
   *     }
   *
   *     xFileClose(input);  // Close input
   *   }
   * }
   * @endcode
   *
   * Example 4: Safe file handle cleanup
   * @code File_t *file = null;
   *
   * void useFile(Volume_t *vol) {
   *   if (OK(xFileOpen(&file, vol, (Byte_t*)"/temp.dat", FS_MODE_WRITE))) {
   *     performOperations(file);
   *
   *     // Close and nullify handle xFileClose(file);
   *     file = null;  // Prevent use-after-close
   *   }
   * }
   * @endcode
   *
   * @param[in] file_ Handle to the open file to close. Must be a valid file
   *                  handle obtained from a previous successful xFileOpen()
   *                  call. After closing, this handle becomes invalid.
   *
   * @return          ReturnOK if close succeeded, ReturnError if close failed
   *                  due to invalid file handle, file not open, or flush/write
   *                  errors during close.
   *
   * @warning After calling xFileClose(), the file handle becomes invalid and
   * must not be used in any file operations. Attempting to use a closed file
   * will result in ReturnError.
   *
   * @warning If xFileClose() fails (returns ReturnError), data may not have
   * been fully written to storage. Consider using xFileSync() before close for
   * critical data to detect write failures earlier.
   *
   * @warning All files must be closed before unmounting the volume. Unmounting
   * with open files may cause data loss or filesystem corruption.
   *
   * @note It is good practice to set the file handle to null after closing to
   * prevent accidental use of an invalid handle (use-after-close bugs).
   *
   * @note Closing a file that was opened for writing commits the final file
   * size to the directory entry. File size is updated only on close, not during
   * write.
   *
   * @note File handles are a limited resource. Always close files promptly
   * after use to make handles available for other operations.
   *
   * @sa xFileOpen() - Open a file
   * @sa xFileSync() - Flush writes before close
   * @sa xFSUnmount() - Unmount volume (close all files first)
   */
  Return_t xFileClose(File_t *file_);


  /**
   * @brief Read data from a file
   *
   * Reads a specified number of bytes from the current file position,
   * allocating memory to hold the retrieved data and advancing the file
   * position. The caller is responsible for freeing the allocated memory with
   * xMemFree() after processing the data.
   *
   * Reading retrieves data from the file starting at the current file position
   * (initially 0, or set by xFileSeek()). After a successful read, the file
   * position advances by the number of bytes read, positioning for the next
   * sequential read.
   *
   * Memory allocation pattern:
   * - xFileRead() allocates memory from user heap for the read data
   * - Caller receives pointer to allocated buffer via data_ parameter
   * - Caller must call xMemFree() after processing to prevent memory leaks
   * - Memory size equals the number of bytes successfully read
   *
   * Read behavior:
   * - Reads up to size_ bytes, may read fewer if EOF reached
   * - Returns actual bytes read (could be less than requested)
   * - File position advances by number of bytes read
   * - Reading past EOF returns 0 bytes
   * - Sequential reads retrieve consecutive file data
   *
   * Common scenarios:
   * - **Full file read**: Get file size, read entire file in one call
   * - **Chunked reading**: Read file in smaller blocks to conserve memory
   * - **Partial reads**: Read specific portions using xFileSeek()
   * - **Sequential processing**: Read file sequentially in fixed-size chunks
   *
   * Example 1: Read entire file
   * @code File_t *file;
   *
   * if (OK(xFileOpen(&file, vol, (Byte_t*)"/config.txt", FS_MODE_READ))) {
   *   Word_t fileSize;
   *   Byte_t *data;
   *
   *   // Get file size if (OK(xFileGetSize(file, &fileSize))) {
   *     // Read all data if (OK(xFileRead(file, fileSize, &data))) {
   *       // Process data parseConfig(data, fileSize);
   *
   *       // Always free allocated memory xMemFree((Addr_t *)data);
   *     }
   *   }
   *
   *   xFileClose(file);
   * }
   * @endcode
   *
   * Example 2: Read file in chunks
   * @code File_t *file;
   *
   * #define CHUNK_SIZE 512
   *
   * void processLargeFile(File_t *file) {
   *   Base_t eof = 0;
   *   Byte_t *chunk;
   *
   *   // Read until end of file while (!eof) {
   *     if (OK(xFileRead(file, CHUNK_SIZE, &chunk))) {
   *       // Process this chunk processChunk(chunk, CHUNK_SIZE);
   *       xMemFree((Addr_t *)chunk);
   *
   *       // Check if reached EOF xFileEOF(file, &eof);
   *     } else {
   *       break;
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Read specific portion of file
   * @code File_t *file;
   *
   * // Read 100 bytes starting at offset 500 Return_t readFileSection(File_t *
   * file, Word_t offset, Size_t count, Byte_t **data) {
   *   // Seek to desired position if (ERROR(xFileSeek(file, offset,
   * FS_SEEK_SET))) {
   *     return ReturnError;
   *   }
   *
   *   // Read data from that position if (ERROR(xFileRead(file, count, data)))
   * {
   *     return ReturnError;
   *   }
   *
   *   return ReturnOK;
   * }
   *
   * // Usage Byte_t *sectionData;
   * if (OK(readFileSection(file, 500, 100, &sectionData))) {
   *   processData(sectionData, 100);
   *   xMemFree((Addr_t *)sectionData);
   * }
   * @endcode
   *
   * Example 4: Read with EOF detection
   * @code File_t *file;
   *
   * void copyFileData(File_t *srcFile, File_t *dstFile) {
   *   Byte_t *buffer;
   *   Base_t eof;
   *
   *   do {
   *     // Read chunk from source if (OK(xFileRead(srcFile, 1024, &buffer))) {
   *       // Write to destination xFileWrite(dstFile, 1024, buffer);
   *       xMemFree((Addr_t *)buffer);
   *     }
   *
   *     // Check if source EOF reached xFileEOF(srcFile, &eof);
   *   } while (!eof);
   * }
   * @endcode
   *
   * @param[in]  file_ Handle to the open file to read from. Must be a valid
   *                   file opened with FS_MODE_READ permission.
   * @param[in]  size_ Number of bytes to read from file. Actual bytes read may
   *                   be less if end-of-file is reached before size_ bytes are
   *                   available.
   * @param[out] data_ Pointer to variable receiving address of newly allocated
   *                   buffer containing the read data. Memory is allocated from
   *                   user heap and must be freed with xMemFree() after use.
   *
   * @return           ReturnOK if read succeeded (even if fewer than size_
   *                   bytes read), ReturnError if read failed due to invalid
   *                   file handle, file not open for reading, memory allocation
   *                   failure, or device read error.
   *
   * @warning The memory allocated for data_ MUST be freed by the caller using
   * xMemFree() after processing. Failure to free this memory will cause a
   * memory leak. Always pair xFileRead() with xMemFree() in your code.
   *
   * @warning The file must have been opened with FS_MODE_READ permission.
   * Attempting to read from a write-only file will fail.
   *
   * @warning xFileRead() may return fewer bytes than requested if EOF is
   * reached. Check actual data size or use xFileEOF() to detect end-of-file
   * condition.
   *
   * @note After a successful read, the file position advances by the number of
   * bytes read. Subsequent reads continue from the new position.
   *
   * @note Reading 0 bytes or reading past EOF is not an error—it returns
   * ReturnOK with zero bytes allocated. Always check the actual bytes read.
   *
   * @note For large files, consider reading in chunks rather than loading the
   * entire file into memory to conserve heap space.
   *
   * @sa xFileWrite() - Write data to file
   * @sa xFileSeek() - Change file position before reading
   * @sa xFileGetSize() - Get total file size
   * @sa xFileEOF() - Check if end-of-file reached
   * @sa xMemFree() - Free memory allocated by xFileRead()
   * @sa xFileOpen() - Open file with read permission
   */
  Return_t xFileRead(File_t *file_, const Size_t size_, Byte_t **data_);


  /**
   * @brief Write data to a file
   *
   * Writes a specified number of bytes to the file at the current file
   * position, advancing the file position and potentially extending the file
   * size. Data is written from the provided buffer to the filesystem, and
   * changes are committed to storage when the file is closed or explicitly
   * synced.
   *
   * Writing appends or overwrites data at the current file position, which
   * depends on the mode the file was opened with (WRITE starts at 0, APPEND
   * starts at EOF). After a successful write, the file position advances by the
   * number of bytes written, and the file size is updated if the write extended
   * beyond the previous end-of-file.
   *
   * Write behavior:
   * - Writes size_ bytes from data_ buffer to file
   * - File position advances by number of bytes written
   * - File size updated if write extends file
   * - Data buffered until xFileClose() or xFileSync()
   * - Allocates clusters from filesystem as needed
   *
   * File modes and write behavior:
   * - **FS_MODE_WRITE**: Position starts at 0, overwrites from beginning
   * - **FS_MODE_APPEND**: Position starts at EOF, extends file
   * - **FS_MODE_READ | FS_MODE_WRITE**: Can write at any position via
   * xFileSeek()
   *
   * Common scenarios:
   * - **Create new file**: Open with WRITE | CREATE, write data
   * - **Append to log**: Open with APPEND | CREATE, write entries
   * - **Update existing**: Open with READ | WRITE, seek and write
   * - **Replace content**: Open with WRITE (truncates), write new data
   *
   * Example 1: Write new file
   * @code File_t *file;
   *
   * if (OK(xFileOpen(&file, vol, (Byte_t*)"/data.bin", FS_MODE_WRITE |
   * FS_MODE_CREATE))) {
   *   Byte_t buffer[256];
   *   generateData(buffer, sizeof(buffer));
   *
   *   // Write data if (OK(xFileWrite(file, sizeof(buffer), buffer))) {
   *     // Data written successfully
   *   }
   *
   *   xFileClose(file);  // Commits write to storage
   * }
   * @endcode
   *
   * Example 2: Append to log file
   * @code void logEvent(Volume_t *vol, const char *event) {
   *   File_t *logFile;
   *
   *   // Open in append mode if (OK(xFileOpen(&logFile, vol,
   * (Byte_t*)"/events.log", FS_MODE_APPEND | FS_MODE_CREATE))) {
   *     Size_t eventLen = strlen(event);
   *
   *     // Write event message xFileWrite(logFile, eventLen, (Byte_t*)event);
   *     xFileWrite(logFile, 1, (Byte_t*)"\n");
   *
   *     xFileClose(logFile);
   *   }
   * }
   * @endcode
   *
   * Example 3: Update specific file location
   * @code File_t *file;
   *
   * // Update bytes at specific offset Return_t updateFileBytes(File_t *file,
   * Word_t offset, Byte_t *newData, Size_t size) {
   *   // Seek to update position if (ERROR(xFileSeek(file, offset,
   * FS_SEEK_SET))) {
   *     return ReturnError;
   *   }
   *
   *   // Write new data at that position return xFileWrite(file, size,
   * newData);
   * }
   *
   * // Usage if (OK(xFileOpen(&file, vol, (Byte_t*)"/config.dat", FS_MODE_READ
   * |
   * FS_MODE_WRITE))) {
   *   Byte_t newValue = 0x42;
   *   updateFileBytes(file, 100, &newValue, 1);  // Update byte at offset 100
   * xFileClose(file);
   * }
   * @endcode
   *
   * Example 4: Write with explicit sync
   * @code File_t *file;
   *
   * if (OK(xFileOpen(&file, vol, (Byte_t*)"/critical.dat", FS_MODE_WRITE |
   * FS_MODE_CREATE))) {
   *   Byte_t importantData[512];
   *   prepareData(importantData, sizeof(importantData));
   *
   *   // Write data if (OK(xFileWrite(file, sizeof(importantData),
   * importantData))) {
   *     // Force write to storage immediately if (OK(xFileSync(file))) {
   *       // Data guaranteed on disk
   *     } else {
   *       // Sync failed - write may not be persistent handleSyncError();
   *     }
   *   }
   *
   *   xFileClose(file);
   * }
   * @endcode
   *
   * @param[in] file_ Handle to the open file to write to. Must be a valid file
   *                  opened with FS_MODE_WRITE or FS_MODE_APPEND permission.
   * @param[in] size_ Number of bytes to write to file. Data buffer must contain
   *                  at least this many bytes.
   * @param[in] data_ Pointer to buffer containing data to write. Buffer must be
   *                  at least size_ bytes in length.
   *
   * @return          ReturnOK if write succeeded, ReturnError if write failed
   *                  due to invalid file handle, file not open for writing,
   *                  insufficient disk space, device write error, or FAT
   *                  allocation failure.
   *
   * @warning The file must have been opened with FS_MODE_WRITE or
   * FS_MODE_APPEND permission. Attempting to write to a read-only file will
   * fail.
   *
   * @warning Writes are buffered and not guaranteed to be on storage until
   * xFileClose() or xFileSync() is called. For critical data, call xFileSync()
   * explicitly to ensure persistence before continuing.
   *
   * @warning If insufficient disk space is available, write will fail with
   * ReturnError. Use xFSGetVolumeInfo() to check free space before writing
   * large amounts of data.
   *
   * @warning Opening a file with FS_MODE_WRITE (without APPEND) truncates the
   * file immediately. The first write starts at position 0, replacing all
   * previous content.
   *
   * @note After a successful write, the file position advances by size_ bytes.
   * Use xFileTell() to query current position or xFileSeek() to change it.
   *
   * @note The data_ buffer is not modified by xFileWrite(). It can be a const
   * buffer or reused after the call returns.
   *
   * @note File size is updated as writes extend the file, but the final size is
   * only committed to the directory entry when xFileClose() is called.
   *
   * @sa xFileRead() - Read data from file
   * @sa xFileOpen() - Open file with write permission
   * @sa xFileClose() - Close file and commit writes
   * @sa xFileSync() - Flush writes to storage
   * @sa xFileSeek() - Change write position
   * @sa xFSGetVolumeInfo() - Check available disk space
   */
  Return_t xFileWrite(File_t *file_, const Size_t size_, const Byte_t *data_);


  /**
   * @brief Change the file position for reading or writing
   *
   * Moves the file position indicator to a new location within the file,
   * controlling where subsequent read or write operations will occur. The file
   * position can be set relative to the beginning of the file, the current
   * position, or the end of the file using the origin parameter.
   *
   * File positioning is essential for random access operations, allowing
   * applications to read or write specific portions of a file without
   * processing the entire file sequentially. The position is measured in bytes
   * from the specified origin point.
   *
   * Seek origins (defined in fs.h):
   * - **FS_SEEK_SET (0)**: Offset from beginning of file. offset_ is absolute
   * position.
   * - **FS_SEEK_CUR (1)**: Offset from current position. offset_ is relative
   * (can be negative).
   * - **FS_SEEK_END (2)**: Offset from end of file. offset_ is typically 0 or
   * negative.
   *
   * Common positioning operations:
   * - **Rewind to start**: xFileSeek(file, 0, FS_SEEK_SET)
   * - **Jump to end**: xFileSeek(file, 0, FS_SEEK_END)
   * - **Skip forward**: xFileSeek(file, 100, FS_SEEK_CUR)
   * - **Skip backward**: xFileSeek(file, -50, FS_SEEK_CUR)
   * - **Absolute position**: xFileSeek(file, 1000, FS_SEEK_SET)
   *
   * Common scenarios:
   * - **Read file header then skip to data**: Seek past header to data section
   * - **Update specific record**: Seek to record position, write new data
   * - **Append detection**: Seek to end to get file size
   * - **Reread data**: Seek back to previous position
   *
   * Example 1: Read file header and data separately
   * @code File_t *file;
   *
   * typedef struct {
   *   Word_t magic;
   *   Word_t version;
   *   Word_t dataOffset;
   * } FileHeader_t;
   *
   * if (OK(xFileOpen(&file, vol, (Byte_t*)"/data.bin", FS_MODE_READ))) {
   *   Byte_t *headerData;
   *   FileHeader_t *header;
   *
   *   // Read header at beginning if (OK(xFileRead(file, sizeof(FileHeader_t),
   * &headerData))) {
   *     header = (FileHeader_t*)headerData;
   *
   *     // Validate and seek to data section if (header->magic == 0xDEADBEEF) {
   *       xFileSeek(file, header->dataOffset, FS_SEEK_SET);
   *
   *       // Read data from new position Byte_t *data;
   *       xFileRead(file, 1024, &data);
   *       xMemFree((Addr_t *)data);
   *     }
   *     xMemFree((Addr_t *)headerData);
   *   }
   *   xFileClose(file);
   * }
   * @endcode
   *
   * Example 2: Update specific record in file
   * @code typedef struct {
   *   Word_t id;
   *   Byte_t status;
   *   Byte_t data[64];
   * } Record_t;
   *
   * Return_t updateRecord(File_t *file, Word_t recordIndex, Record_t newRecord)
   * {
   *   Word_t recordOffset = recordIndex * sizeof(Record_t);
   *
   *   // Seek to record position if (ERROR(xFileSeek(file, recordOffset,
   * FS_SEEK_SET))) {
   *     return ReturnError;
   *   }
   *
   *   // Write updated record return xFileWrite(file, sizeof(Record_t),
   * (Byte_t*)newRecord);
   * }
   * @endcode
   *
   * Example 3: Get file size using seek to end
   * @code Word_t getFileSize(File_t *file) {
   *   Word_t size;
   *   Word_t currentPos;
   *
   *   // Save current position xFileTell(file, &currentPos);
   *
   *   // Seek to end and get position (which equals file size) xFileSeek(file,
   * 0, FS_SEEK_END);
   *   xFileTell(file, &size);
   *
   *   // Restore original position xFileSeek(file, currentPos, FS_SEEK_SET);
   *
   *   return size;
   * }
   * @endcode
   *
   * Example 4: Skip chunks when processing
   * @code File_t *file;
   *
   * #define CHUNK_SIZE 512
   * #define CHUNK_GAP 128
   *
   * void processAlternateChunks(File_t *file) {
   *   Byte_t *chunk;
   *   Base_t eof = 0;
   *
   *   while (!eof) {
   *     // Read chunk if (OK(xFileRead(file, CHUNK_SIZE, &chunk))) {
   *       processChunk(chunk, CHUNK_SIZE);
   *       xMemFree((Addr_t *)chunk);
   *
   *       // Skip gap to next chunk xFileSeek(file, CHUNK_GAP, FS_SEEK_CUR);
   *
   *       xFileEOF(file, &eof);
   *     } else {
   *       break;
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in] file_   Handle to the open file. Must be a valid file from
   *                    xFileOpen().
   * @param[in] offset_ Number of bytes to offset from origin. Can be positive
   *                    or negative depending on origin. For FS_SEEK_SET, must
   *                    be >= 0.
   * @param[in] origin_ Reference point for offset. Use FS_SEEK_SET for absolute
   *                    position, FS_SEEK_CUR for relative to current,
   *                    FS_SEEK_END for relative to end of file.
   *
   * @return            ReturnOK if seek succeeded, ReturnError if seek failed
   *                    due to invalid file handle, invalid origin value, or
   *                    resulting position would be before start of file or
   *                    beyond reasonable file bounds.
   *
   * @warning Seeking beyond the end of the file and then writing creates a
   * sparse file with undefined data in the gap. The gap will contain whatever
   * data was previously in those disk sectors.
   *
   * @warning Some seek operations may fail if they would position before the
   * start of the file (negative absolute position).
   *
   * @note After a seek operation, the next read or write occurs at the new
   * position. The position indicator is updated immediately.
   *
   * @note Seeking does not change the file size. Only writing beyond the
   * current end-of-file extends the file.
   *
   * @note For FS_SEEK_CUR, offset_ can be negative to move backwards, or
   * positive to move forwards from the current position.
   *
   * @sa xFileTell() - Get current file position
   * @sa xFileGetSize() - Get file size without seeking
   * @sa xFileRead() - Read from current position
   * @sa xFileWrite() - Write at current position
   * @sa xFileEOF() - Check if at end of file
   */
  Return_t xFileSeek(File_t *file_, const Word_t offset_, const Byte_t origin_);


  /**
   * @brief Get the current file position
   *
   * Retrieves the current byte offset of the file position indicator, which
   * indicates where the next read or write operation will occur. The position
   * is measured in bytes from the beginning of the file, with 0 representing
   * the first byte.
   *
   * The file position changes automatically after read and write operations,
   * and can be explicitly set with xFileSeek(). Knowing the current position is
   * useful for saving and restoring read/write locations, calculating how much
   * data has been processed, or implementing custom file navigation.
   *
   * Position characteristics:
   * - Measured in bytes from beginning of file (0-based)
   * - Advances automatically after read/write operations
   * - Can be set explicitly with xFileSeek()
   * - Equals file size when at end-of-file
   * - Starts at 0 for READ/WRITE modes, at EOF for APPEND mode
   *
   * Common use cases:
   * - **Save/restore position**: Remember position to return later
   * - **Progress tracking**: Monitor how much of file has been processed
   * - **Size calculation**: Position after seek-to-end equals file size
   * - **Offset calculation**: Compute relative offsets for records
   *
   * Example 1: Save and restore file position
   * @code File_t *file;
   * Word_t savedPosition;
   *
   * // Save current position xFileTell(file, &savedPosition);
   *
   * // Perform some operation that changes position xFileSeek(file, 0,
   * FS_SEEK_SET);
   * Byte_t *header;
   * xFileRead(file, 64, &header);
   * processHeader(header);
   * xMemFree((Addr_t *)header);
   *
   * // Restore original position xFileSeek(file, savedPosition, FS_SEEK_SET);
   *
   * // Continue from where we left off Byte_t *data;
   * xFileRead(file, 512, &data);
   * xMemFree((Addr_t *)data);
   * @endcode
   *
   * Example 2: Track read progress
   * @code File_t *file;
   * Word_t fileSize;
   * Word_t currentPos;
   *
   * xFileGetSize(file, &fileSize);
   *
   * while (1) {
   *   Byte_t *chunk;
   *   if (OK(xFileRead(file, 1024, &chunk))) {
   *     processChunk(chunk, 1024);
   *     xMemFree((Addr_t *)chunk);
   *
   *     // Show progress xFileTell(file, &currentPos);
   *     Byte_t percent = (Byte_t)((currentPos * 100) / fileSize);
   *     updateProgressBar(percent);
   *
   *     if (currentPos >= fileSize) break;
   *   } else {
   *     break;
   *   }
   * }
   * @endcode
   *
   * Example 3: Record file offset of data sections
   * @code typedef struct {
   *   Word_t sectionOffset;
   *   Word_t sectionSize;
   * } SectionInfo_t;
   *
   * SectionInfo_t sections[10];
   * Byte_t sectionCount = 0;
   *
   * void indexFileSections(File_t *file) {
   *   Byte_t *sectionHeader;
   *
   *   while (sectionCount < 10) {
   *     // Record where this section starts xFileTell(file,
   * &sections[sectionCount].sectionOffset);
   *
   *     // Read section header to get size if (OK(xFileRead(file, 4,
   * &sectionHeader))) {
   *       sections[sectionCount].sectionSize = *(Word_t*)sectionHeader;
   *       xMemFree((Addr_t *)sectionHeader);
   *
   *       // Skip to next section xFileSeek(file,
   * sections[sectionCount].sectionSize, FS_SEEK_CUR);
   *       sectionCount++;
   *     } else {
   *       break;
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  file_     Handle to the open file. Must be a valid file from
   *                       xFileOpen().
   * @param[out] position_ Pointer to variable receiving the current file
   *                       position in bytes. Position 0 is the first byte of
   *                       the file.
   *
   * @return               ReturnOK if query succeeded, ReturnError if query
   *                       failed due to invalid file handle or file not open.
   *
   * @note The position returned is always measured from the beginning of the
   * file, regardless of how the file was opened or what seek operations were
   * used.
   *
   * @note When a file is first opened with FS_MODE_APPEND, xFileTell() will
   * return the file size (position is at end of file).
   *
   * @note The position advances after each read/write by the number of bytes
   * transferred.
   *
   * @sa xFileSeek() - Change file position
   * @sa xFileGetSize() - Get total file size
   * @sa xFileEOF() - Check if at end of file
   * @sa xFileRead() - Read from current position
   * @sa xFileWrite() - Write at current position
   */
  Return_t xFileTell(const File_t *file_, Word_t *position_);


  /**
   * @brief Get the size of a file in bytes
   *
   * Retrieves the total size of an open file in bytes. This is the number of
   * bytes of actual data content in the file, not including any filesystem
   * metadata or directory entries. File size is useful for validating file
   * integrity, allocating read buffers, calculating progress, or checking
   * available data.
   *
   * The file size represents the current data content and may change as write
   * operations extend the file. Size is updated dynamically during write
   * operations but is only committed to the directory entry when the file is
   * closed.
   *
   * Size characteristics:
   * - Measured in bytes of actual file content
   * - Does not include filesystem metadata or overhead
   * - Updates as file is written and extended
   * - Committed to directory entry on xFileClose()
   * - Zero for newly created empty files
   *
   * Common use cases:
   * - **Buffer allocation**: Allocate exactly enough memory for file contents
   * - **Validation**: Verify file has expected size
   * - **Progress tracking**: Calculate percentage read/written
   * - **Bounds checking**: Ensure reads don't exceed file bounds
   * - **Empty file detection**: Check if size is zero
   *
   * Example 1: Read entire file into memory
   * @code File_t *file;
   *
   * Return_t readEntireFile(Volume_t *vol, const char *path, Byte_t **fileData,
   * Word_t *size) {
   *   File_t *file;
   *
   *   if (OK(xFileOpen(&file, vol, (Byte_t*)path, FS_MODE_READ))) {
   *     // Get file size if (OK(xFileGetSize(file, size))) {
   *       // Read entire file if (OK(xFileRead(file, *size, fileData))) {
   *         xFileClose(file);
   *         return ReturnOK;
   *       }
   *     }
   *     xFileClose(file);
   *   }
   *   return ReturnError;
   * }
   *
   * // Usage Byte_t *data;
   * Word_t dataSize;
   * if (OK(readEntireFile(vol, "/config.txt", &data, &dataSize))) {
   *   processData(data, dataSize);
   *   xMemFree((Addr_t *)data);
   * }
   * @endcode
   *
   * Example 2: Validate file size before processing
   * @code
   * #define MIN_VALID_SIZE 100
   * #define MAX_VALID_SIZE 10000
   *
   * Return_t processValidatedFile(File_t *file) {
   *   Word_t size;
   *
   *   // Check file size is in valid range if (OK(xFileGetSize(file, &size))) {
   *     if (size < MIN_VALID_SIZE) {
   *       logError("File too small");
   *       return ReturnError;
   *     }
   *     if (size > MAX_VALID_SIZE) {
   *       logError("File too large");
   *       return ReturnError;
   *     }
   *
   *     // Size valid - proceed with processing return processFile(file, size);
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 3: Copy file with progress reporting
   * @code void copyFileWithProgress(File_t *src, File_t *dst) {
   *   Word_t totalSize;
   *   Word_t bytesProcessed = 0;
   *   Byte_t *buffer;
   *
   *   xFileGetSize(src, &totalSize);
   *
   *   while (bytesProcessed < totalSize) {
   *     Size_t chunkSize = (totalSize - bytesProcessed > 512) ? 512 :
   * (totalSize
   * - bytesProcessed);
   *
   *     if (OK(xFileRead(src, chunkSize, &buffer))) {
   *       xFileWrite(dst, chunkSize, buffer);
   *       xMemFree((Addr_t *)buffer);
   *
   *       bytesProcessed += chunkSize;
   *
   *       // Report progress Byte_t percent = (Byte_t)((bytesProcessed * 100) /
   * totalSize);
   *       updateProgress(percent);
   *     } else {
   *       break;
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 4: Check for empty file
   * @code Base_t isFileEmpty(File_t *file) {
   *   Word_t size;
   *
   *   if (OK(xFileGetSize(file, &size))) {
   *     return (size == 0) ? 1 : 0;
   *   }
   *   return 0;
   * }
   *
   * // Usage if (isFileEmpty(logFile)) {
   *   // File is empty - write header xFileWrite(logFile, strlen(HEADER),
   * (Byte_t*)HEADER);
   * }
   * @endcode
   *
   * @param[in]  file_ Handle to the open file. Must be a valid file from
   *                   xFileOpen().
   * @param[out] size_ Pointer to variable receiving the file size in bytes.
   *
   * @return           ReturnOK if query succeeded, ReturnError if query failed
   *                   due to invalid file handle or file not open.
   *
   * @note The size returned is the current file content size. For files open
   * for writing, the size may increase as write operations extend the file.
   *
   * @note File size is independent of file position. You can get the size at
   * any time without affecting the current read/write position.
   *
   * @note The size reported is the logical file size (bytes of content), not
   * the physical storage size (which may be larger due to cluster allocation).
   *
   * @sa xFileSeek() - Position within file based on size
   * @sa xFileTell() - Get current position (may equal size at EOF)
   * @sa xFileEOF() - Check if at end of file
   * @sa xFileRead() - Read file data
   * @sa xFSGetVolumeInfo() - Get total volume space
   */
  Return_t xFileGetSize(const File_t *file_, Word_t *size_);


  /**
   * @brief Flush file writes to storage
   *
   * Forces any buffered write data for the file to be written to the underlying
   * storage device immediately, ensuring data persistence. This operation
   * updates the file's directory entry and File Allocation Table (FAT) to
   * reflect the current file size and cluster allocation.
   *
   * Normally, write operations are buffered and only committed to storage when
   * xFileClose() is called. For critical data that must survive system failures
   * or power loss, call xFileSync() explicitly to guarantee the data is
   * physically written to the storage device.
   *
   * Sync ensures:
   * - All buffered write data is written to storage
   * - Directory entry is updated with current file size
   * - FAT is updated with current cluster allocation
   * - Data is persistent even if system crashes after sync
   * - File remains open for continued operations
   *
   * When to use sync:
   * - **Critical data**: Financial transactions, configuration changes
   * - **Checkpointing**: Periodic saves during long operations
   * - **Error detection**: Verify writes succeeded before continuing
   * - **Power-loss protection**: Ensure data written before risky operations
   * - **Real-time logging**: Guarantee log entries are stored immediately
   *
   * Example 1: Write critical configuration with sync
   * @code Return_t saveConfig(Volume_t *vol, Byte_t *configData, Size_t
   * dataSize) {
   *   File_t *file;
   *
   *   if (OK(xFileOpen(&file, vol, (Byte_t*)"/config.dat", FS_MODE_WRITE |
   * FS_MODE_CREATE))) {
   *     // Write configuration data if (OK(xFileWrite(file, dataSize,
   * configData))) {
   *       // Force write to storage before continuing if (OK(xFileSync(file)))
   * {
   *         // Data guaranteed on disk xFileClose(file);
   *         return ReturnOK;
   *       } else {
   *         // Sync failed - data may be lost xFileClose(file);
   *         return ReturnError;
   *       }
   *     }
   *     xFileClose(file);
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 2: Periodic checkpointing during long write
   * @code File_t *dataFile;
   *
   * #define CHECKPOINT_INTERVAL 10
   *
   * void processLargeDataset(Byte_t *dataset, Word_t recordCount) {
   *   Word_t i;
   *
   *   for (i = 0; i < recordCount; i++) {
   *     // Write record xFileWrite(dataFile, RECORD_SIZE, &dataset[i *
   * RECORD_SIZE]);
   *
   *     // Checkpoint every N records if ((i % CHECKPOINT_INTERVAL) == 0) {
   *       if (OK(xFileSync(dataFile))) {
   *         logInfo("Checkpoint at record %lu", i);
   *       } else {
   *         logError("Checkpoint failed - aborting");
   *         break;
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 3: Real-time event logging
   * @code File_t *eventLog;
   *
   * void logCriticalEvent(const char *eventMsg) {
   *   Size_t msgLen = strlen(eventMsg);
   *
   *   // Write event xFileWrite(eventLog, msgLen, (Byte_t*)eventMsg);
   *   xFileWrite(eventLog, 1, (Byte_t*)"\n");
   *
   *   // Force immediate write to storage if (ERROR(xFileSync(eventLog))) {
   *     // Could not guarantee persistence handleLogFailure();
   *   }
   *   // Event now safely on disk
   * }
   * @endcode
   *
   * Example 4: Transaction-style write with rollback
   * @code Return_t atomicUpdate(File_t *file, Byte_t *newData, Size_t dataSize)
   * {
   *   Word_t originalPos;
   *   Word_t originalSize;
   *
   *   // Save state for rollback xFileTell(file, &originalPos);
   *   xFileGetSize(file, &originalSize);
   *
   *   // Attempt write if (OK(xFileWrite(file, dataSize, newData))) {
   *     // Try to commit if (OK(xFileSync(file))) {
   *       return ReturnOK;  // Success - data committed
   *     } else {
   *       // Sync failed - truncate back to original size xFileTruncate(file,
   * originalSize);
   *       xFileSeek(file, originalPos, FS_SEEK_SET);
   *       return ReturnError;
   *     }
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[in] file_ Handle to the open file to sync. Must be a valid file
   *                  opened with write permission (FS_MODE_WRITE or
   *                  FS_MODE_APPEND).
   *
   * @return          ReturnOK if sync succeeded, ReturnError if sync failed due
   *                  to invalid file handle, file not open for writing, or
   *                  device write errors during flush operation.
   *
   * @warning Sync operations involve multiple disk writes (data, FAT, directory
   * entry) and may take significant time, especially on slow storage devices.
   * Use sync judiciously to balance data safety with performance.
   *
   * @warning If xFileSync() returns ReturnError, some data may have been
   * written but the filesystem may be in an inconsistent state. Close and
   * reopen the file or check filesystem integrity.
   *
   * @note xFileSync() does NOT close the file. After sync completes, the file
   * remains open and can be used for additional read or write operations.
   *
   * @note Calling xFileClose() automatically performs a sync before closing, so
   * explicit xFileSync() is not required at the end of normal file operations.
   *
   * @note For read-only files, xFileSync() has no effect and returns ReturnOK
   * immediately.
   *
   * @sa xFileWrite() - Write data that may be buffered until sync
   * @sa xFileClose() - Close file (automatically syncs)
   * @sa xFileTruncate() - Change file size
   */
  Return_t xFileSync(File_t *file_);


  /**
   * @brief Resize a file to a specified size
   *
   * Changes the size of an open file to the specified number of bytes, either
   * extending the file with undefined data or truncating it by discarding data
   * beyond the new size. This operation allows precise control over file size
   * independent of write operations.
   *
   * Truncation behavior:
   * - If new size < current size: File is shortened, data beyond new size is
   * lost
   * - If new size > current size: File is extended, gap filled with undefined
   * data
   * - If new size == current size: No change, operation succeeds
   * - File position is preserved if still valid after truncation
   * - Freed clusters are returned to filesystem free space
   *
   * Common use cases:
   * - **Preallocate space**: Extend file to reserve disk space before writing
   * - **Remove trailing data**: Truncate to exact data size
   * - **Rollback writes**: Shorten file to previous checkpoint
   * - **Clear file**: Truncate to zero to erase all content
   * - **Fixed-size files**: Ensure file is exact required size
   *
   * Example 1: Truncate file to zero (clear contents)
   * @code File_t *file;
   *
   * Return_t clearFile(Volume_t *vol, const char *path) {
   *   File_t *file;
   *
   *   // Open for writing if (OK(xFileOpen(&file, vol, (Byte_t*)path,
   * FS_MODE_WRITE))) {
   *     // Truncate to zero bytes if (OK(xFileTruncate(file, 0))) {
   *       xFileClose(file);
   *       return ReturnOK;
   *     }
   *     xFileClose(file);
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 2: Preallocate file space
   * @code
   * #define PREALLOCATE_SIZE (100 * 1024)  // 100 KB
   *
   * Return_t createPreallocatedFile(Volume_t *vol, const char *path) {
   *   File_t *file;
   *
   *   // Create new file if (OK(xFileOpen(&file, vol, (Byte_t*)path,
   * FS_MODE_WRITE | FS_MODE_CREATE))) {
   *     // Preallocate space if (OK(xFileTruncate(file, PREALLOCATE_SIZE))) {
   *       // Space reserved - reset position to start xFileSeek(file, 0,
   * FS_SEEK_SET);
   *       xFileClose(file);
   *       return ReturnOK;
   *     }
   *     xFileClose(file);
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 3: Trim file to actual data size
   * @code File_t *file;
   *
   * void trimLogFile(File_t *file, Word_t actualDataSize) {
   *   Word_t currentSize;
   *
   *   // Get current file size if (OK(xFileGetSize(file, &currentSize))) {
   *     if (currentSize > actualDataSize) {
   *       // File is larger than needed - truncate excess xFileTruncate(file,
   * actualDataSize);
   *       logInfo("Trimmed %lu bytes from log file", currentSize -
   * actualDataSize);
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 4: Rollback to checkpoint
   * @code File_t *dataFile;
   * Word_t checkpointSize;
   *
   * void saveCheckpoint(File_t *file) {
   *   // Save current size as checkpoint xFileGetSize(file, &checkpointSize);
   * }
   *
   * void rollbackToCheckpoint(File_t *file) {
   *   Word_t currentSize;
   *
   *   xFileGetSize(file, &currentSize);
   *
   *   if (currentSize > checkpointSize) {
   *     // Discard data written after checkpoint if (OK(xFileTruncate(file,
   * checkpointSize))) {
   *       xFileSeek(file, checkpointSize, FS_SEEK_SET);
   *       logInfo("Rolled back to checkpoint");
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in] file_ Handle to the open file to resize. Must be a valid file
   *                  opened with write permission (FS_MODE_WRITE or
   *                  FS_MODE_APPEND).
   * @param[in] size_ New file size in bytes. Can be larger (extend) or smaller
   *                  (truncate) than current size.
   *
   * @return          ReturnOK if truncate succeeded, ReturnError if operation
   *                  failed due to invalid file handle, file not open for
   *                  writing, insufficient disk space (for extension), or FAT
   *                  update errors.
   *
   * @warning Truncating a file to a smaller size permanently deletes data
   * beyond the new size. This data cannot be recovered. Ensure the new size is
   * correct before calling this function.
   *
   * @warning If extending a file, the gap between the old size and new size
   * contains undefined data (whatever was previously in those disk sectors). Do
   * not rely on gap data being zero or any specific value.
   *
   * @warning The file position is not changed by truncation unless it exceeds
   * the new file size, in which case it is clamped to the new size.
   *
   * @note Truncating to zero is an efficient way to clear a file while keeping
   * it open.
   *
   * @note Extending a file with xFileTruncate() allocates disk space but does
   * not initialize the new space with any specific values.
   *
   * @note The new file size is committed when xFileClose() or xFileSync() is
   * called.
   *
   * @sa xFileGetSize() - Query current file size
   * @sa xFileWrite() - Write data (changes size automatically)
   * @sa xFileSeek() - Change file position
   * @sa xFileSync() - Commit size change to storage
   * @sa xFileClose() - Close file and commit size
   */
  Return_t xFileTruncate(File_t *file_, const Word_t size_);


  /**
   * @brief Check if file position is at end-of-file
   *
   * Determines whether the current file position is at or beyond the end of the
   * file, indicating that all data has been read and no more data is available.
   * This is essential for implementing read loops that process entire files
   * without knowing the file size in advance.
   *
   * End-of-file (EOF) occurs when the file position equals or exceeds the file
   * size. After reading the last byte of a file, subsequent reads will not
   * advance the position further, and xFileEOF() will return true. EOF is a
   * normal condition, not an error.
   *
   * EOF behavior:
   * - True when file position >= file size
   * - False when more data available to read
   * - Not an error condition - indicates normal end of data
   * - Can occur mid-read if reading past end
   * - Unaffected by write operations that extend file
   *
   * Common use cases:
   * - **Read loops**: Continue reading until EOF reached
   * - **Data validation**: Verify all expected data was read
   * - **Sequential processing**: Process file completely
   * - **Stream detection**: Know when to request more data
   *
   * Example 1: Read file in chunks until EOF
   * @code File_t *file;
   *
   * #define CHUNK_SIZE 512
   *
   * void processEntireFile(File_t *file) {
   *   Base_t eof = 0;
   *   Byte_t *chunk;
   *   Word_t totalBytesRead = 0;
   *
   *   // Read until EOF while (!eof) {
   *     if (OK(xFileRead(file, CHUNK_SIZE, &chunk))) {
   *       processChunk(chunk, CHUNK_SIZE);
   *       totalBytesRead += CHUNK_SIZE;
   *       xMemFree((Addr_t *)chunk);
   *
   *       // Check for EOF xFileEOF(file, &eof);
   *     } else {
   *       break;  // Read error
   *     }
   *   }
   *
   *   logInfo("Processed %lu bytes", totalBytesRead);
   * }
   * @endcode
   *
   * Example 2: Validate file completely read
   * @code Return_t readAndValidate(File_t *file, Word_t expectedSize) {
   *   Byte_t *data;
   *   Base_t eof;
   *
   *   // Read expected amount if (OK(xFileRead(file, expectedSize, &data))) {
   *     processData(data, expectedSize);
   *     xMemFree((Addr_t *)data);
   *
   *     // Verify we're at EOF (no extra data) if (OK(xFileEOF(file, &eof))) {
   *       if (eof) {
   *         return ReturnOK;  // File size matches expected
   *       } else {
   *         logWarning("File has unexpected extra data");
   *         return ReturnError;
   *       }
   *     }
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 3: Copy file with EOF detection
   * @code Return_t copyFile(File_t *source, File_t *dest) {
   *   Base_t eof = 0;
   *   Byte_t *buffer;
   *
   *   while (!eof) {
   *     // Read chunk if (OK(xFileRead(source, 1024, &buffer))) {
   *       // Write to destination if (ERROR(xFileWrite(dest, 1024, buffer))) {
   *         xMemFree((Addr_t *)buffer);
   *         return ReturnError;
   *       }
   *       xMemFree((Addr_t *)buffer);
   *
   *       // Check if source exhausted xFileEOF(source, &eof);
   *     } else {
   *       return ReturnError;
   *     }
   *   }
   *
   *   return ReturnOK;  // Complete file copied
   * }
   * @endcode
   *
   * Example 4: Skip to end check
   * @code Base_t atEndOfFile(File_t *file) {
   *   Base_t eof;
   *
   *   if (OK(xFileEOF(file, &eof))) {
   *     return eof;
   *   }
   *   return 0;
   * }
   *
   * // Usage in parsing void parseRecords(File_t *file) {
   *   while (!atEndOfFile(file)) {
   *     Record_t record;
   *     if (readRecord(file, &record)) {
   *       processRecord(&record);
   *     }
   *   }
   * }
   * @endcode
   *
   * @param[in]  file_ Handle to the open file. Must be a valid file from
   *                   xFileOpen().
   * @param[out] eof_  Pointer to variable receiving EOF status. Set to non-zero
   *                   (true) if at end-of-file, zero (false) if more data
   *                   available.
   *
   * @return           ReturnOK if query succeeded, ReturnError if query failed
   *                   due to invalid file handle or file not open.
   *
   * @note EOF is determined by comparing file position to file size. Position
   * equals size when all data has been read.
   *
   * @note For files opened with FS_MODE_APPEND, EOF is initially true since
   * position starts at end of file.
   *
   * @note After reaching EOF, you can seek back to earlier positions with
   * xFileSeek() to reread data, which will clear the EOF condition.
   *
   * @note Write operations that extend the file beyond the current position
   * will clear the EOF condition.
   *
   * @sa xFileRead() - Read data (may reach EOF)
   * @sa xFileGetSize() - Get file size to calculate EOF
   * @sa xFileTell() - Get current position
   * @sa xFileSeek() - Move position (may clear EOF)
   */
  Return_t xFileEOF(const File_t *file_, Base_t *eof_);


  /**
   * @brief Check if a file exists on the volume
   *
   * Determines whether a file exists at the specified path on the mounted
   * volume. This is useful for conditional file operations, avoiding errors
   * when opening files, or checking for the presence of configuration or data
   * files before attempting to access them.
   *
   * The function searches the filesystem directory structure for an entry
   * matching the specified path. It returns true if a file (not a directory)
   * exists at that path, or false if the path does not exist or refers to a
   * directory rather than a file.
   *
   * Common use cases:
   * - **Conditional file access**: Open file only if it exists
   * - **Configuration detection**: Check for config files before loading
   * - **Backup verification**: Verify backup files exist before restore
   * - **File creation logic**: Create new file only if it doesn't exist
   * - **Validation**: Ensure required files are present
   *
   * Example 1: Open file only if it exists
   * @code Volume_t *vol;
   * Base_t exists;
   *
   * if (OK(xFileExists(vol, (Byte_t*)"/config.txt", &exists)) && exists) {
   *   // File exists - safe to open File_t *file;
   *   if (OK(xFileOpen(&file, vol, (Byte_t*)"/config.txt", FS_MODE_READ))) {
   *     // Process file xFileClose(file);
   *   }
   * } else {
   *   // File doesn't exist - create default createDefaultConfig(vol);
   * }
   * @endcode
   *
   * Example 2: Check multiple configuration sources
   * @code Return_t loadConfig(Volume_t *vol, Config_t *config) {
   *   Base_t exists;
   *   const char *configPaths[] = {
   *     "/config.user.txt",
   *     "/config.default.txt",
   *     "/config.txt"
   *   };
   *
   *   // Try each config file in priority order for (int i = 0; i < 3; i++) {
   *     if (OK(xFileExists(vol, (Byte_t*)configPaths[i], &exists)) && exists) {
   *       return loadConfigFromFile(vol, configPaths[i], config);
   *     }
   *   }
   *
   *   // No config file found - use defaults return loadDefaultConfig(config);
   * }
   * @endcode
   *
   * Example 3: Conditional file creation
   * @code Return_t ensureLogFile(Volume_t *vol) {
   *   Base_t exists;
   *
   *   // Check if log file exists if (OK(xFileExists(vol,
   * (Byte_t*)"/system.log", &exists))) {
   *     if (!exists) {
   *       // Create new log file with header File_t *logFile;
   *       if (OK(xFileOpen(&logFile, vol, (Byte_t*)"/system.log", FS_MODE_WRITE
   * | FS_MODE_CREATE))) {
   *         const char *header = "=== System Log ===\n";
   *         xFileWrite(logFile, strlen(header), (Byte_t*)header);
   *         xFileClose(logFile);
   *         return ReturnOK;
   *       }
   *     } else {
   *       // Log file already exists return ReturnOK;
   *     }
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 4: Validate required files
   * @code Return_t validateRequiredFiles(Volume_t *vol) {
   *   const char *requiredFiles[] = {
   *     "/firmware.bin",
   *     "/config.dat",
   *     "/calibration.dat"
   *   };
   *   Base_t exists;
   *
   *   for (int i = 0; i < 3; i++) {
   *     if (ERROR(xFileExists(vol, (Byte_t*)requiredFiles[i], &exists)) ||
   * !exists) {
   *       logError("Required file missing: %s", requiredFiles[i]);
   *       return ReturnError;
   *     }
   *   }
   *
   *   // All required files present return ReturnOK;
   * }
   * @endcode
   *
   * @param[in]  volume_ Handle to the mounted volume to search. Must be a valid
   *                     volume from xFSMount().
   * @param[in]  path_   Pointer to null-terminated string containing file path.
   *                     Use forward slashes (/) for directory separators.
   * @param[out] exists_ Pointer to variable receiving existence status. Set to
   *                     non-zero (true) if file exists, zero (false) if not
   *                     found or path refers to a directory.
   *
   * @return             ReturnOK if query succeeded (regardless of whether file
   *                     exists), ReturnError if query failed due to invalid
   *                     volume handle or path.
   *
   * @note This function returns true only for files, not directories. Use
   * directory operations or xFileGetInfo() to check for directory existence.
   *
   * @note File paths are case-sensitive. "/File.txt" and "/file.txt" are
   * different.
   *
   * @note Checking for existence and then opening the file is not atomic. In
   * multitasking systems, the file could be deleted between the check and open
   * operations.
   *
   * @sa xFileOpen() - Open file (fails if doesn't exist without CREATE flag)
   * @sa xFileGetInfo() - Get detailed file information including type
   * @sa xDirOpen() - Open directory
   */
  Return_t xFileExists(Volume_t *volume_, const Byte_t *path_, Base_t *exists_);


  /**
   * @brief Delete a file from the filesystem
   *
   * Removes a file from the filesystem, freeing its disk space and removing its
   * directory entry. This operation is permanent—deleted files cannot be
   * recovered. The file must not be open when deleted; close all file handles
   * before calling xFileUnlink().
   *
   * Deletion process:
   * - Removes file's directory entry
   * - Marks file's clusters as free in FAT
   * - Returns disk space to available pool
   * - Operation is permanent and irreversible
   *
   * Common scenarios:
   * - **Temporary file cleanup**: Remove temporary or cache files
   * - **Old log removal**: Delete old log files to free space
   * - **File replacement**: Delete old file before writing new version
   * - **Error recovery**: Remove corrupted or incomplete files
   * - **Space management**: Delete unnecessary files to reclaim storage
   *
   * Example 1: Delete temporary file
   * @code Volume_t *vol;
   *
   * void cleanupTempFile(void) {
   *   Base_t exists;
   *
   *   // Check if temp file exists if (OK(xFileExists(vol,
   * (Byte_t*)"/temp.dat",
   * &exists)) && exists) {
   *     // Delete temp file if (OK(xFileUnlink(vol, (Byte_t*)"/temp.dat"))) {
   *       logInfo("Temp file deleted");
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Replace file with new version
   * @code Return_t updateConfigFile(Volume_t *vol, Byte_t *newConfig, Size_t
   * size) {
   *   Base_t exists;
   *
   *   // Check if old config exists if (OK(xFileExists(vol,
   * (Byte_t*)"/config.dat", &exists)) && exists) {
   *     // Delete old version if (ERROR(xFileUnlink(vol,
   * (Byte_t*)"/config.dat"))) {
   *       return ReturnError;
   *     }
   *   }
   *
   *   // Write new config File_t *file;
   *   if (OK(xFileOpen(&file, vol, (Byte_t*)"/config.dat", FS_MODE_WRITE |
   * FS_MODE_CREATE))) {
   *     xFileWrite(file, size, newConfig);
   *     xFileClose(file);
   *     return ReturnOK;
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 3: Delete old log files to free space
   * @code void cleanOldLogs(Volume_t *vol) {
   *   const char *oldLogs[] = {
   *     "/log.old.3",
   *     "/log.old.2",
   *     "/log.old.1"
   *   };
   *   Base_t exists;
   *   VolumeInfo_t *volInfo;
   *
   *   // Check if we need space if (OK(xFSGetVolumeInfo(vol, &volInfo))) {
   *     if (volInfo.freeBytes < (50 * 1024)) {  // Less than 50KB free
   *       // Delete old logs for (int i = 0; i < 3; i++) {
   *         if (OK(xFileExists(vol, (Byte_t*)oldLogs[i], &exists)) && exists) {
   *           xFileUnlink(vol, (Byte_t*)oldLogs[i]);
   *         }
   *       }
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 4: Remove corrupted file
   * @code Return_t validateAndClean(Volume_t *vol, const char *path) {
   *   File_t *file;
   *   Base_t isValid = 0;
   *
   *   // Try to validate file if (OK(xFileOpen(&file, vol, (Byte_t*)path,
   * FS_MODE_READ))) {
   *     Byte_t *data;
   *     Word_t size;
   *     if (OK(xFileGetSize(file, &size)) && OK(xFileRead(file, size, &data)))
   * {
   *       isValid = validateFileData(data, size);
   *       xMemFree((Addr_t *)data);
   *     }
   *     xFileClose(file);
   *   }
   *
   *   // Delete if corrupted if (!isValid) {
   *     logWarning("Deleting corrupted file: %s", path);
   *     return xFileUnlink(vol, (Byte_t*)path);
   *   }
   *
   *   return ReturnOK;
   * }
   * @endcode
   *
   * @param[in] volume_ Handle to the mounted volume containing the file. Must
   *                    be a valid volume from xFSMount().
   * @param[in] path_   Pointer to null-terminated string containing path to
   *                    file to delete. Use forward slashes (/) for directory
   *                    separators.
   *
   * @return            ReturnOK if file deleted successfully, ReturnError if
   *                    deletion failed due to invalid volume, file not found,
   *                    file is open, or filesystem error.
   *
   * @warning File deletion is permanent and irreversible. Deleted files cannot
   * be recovered. Ensure you have the correct path before calling this
   * function.
   *
   * @warning The file must not be open. Close all file handles to the file
   * before attempting to delete it. Deleting an open file will fail.
   *
   * @warning Do not delete files while other tasks may be accessing them. This
   * may cause those tasks' operations to fail unexpectedly.
   *
   * @note Deleting a file frees its disk space immediately, making it available
   * for new files.
   *
   * @note You cannot delete directories with xFileUnlink(). Use xDirRemove() to
   * delete empty directories.
   *
   * @sa xFileExists() - Check if file exists before deleting
   * @sa xFileRename() - Rename file instead of deleting
   * @sa xFileClose() - Close file before deleting
   * @sa xDirRemove() - Delete empty directory
   */
  Return_t xFileUnlink(Volume_t *volume_, const Byte_t *path_);


  /**
   * @brief Rename or move a file
   *
   * Changes the name and/or location of a file within the filesystem. This
   * operation can rename a file in the same directory or move it to a different
   * directory with an optional new name. The file content and attributes are
   * preserved; only the directory entry is updated.
   *
   * Rename is an atomic operation that updates the directory structure without
   * copying file data. This makes it efficient even for large files. The file
   * must not be open during the rename operation.
   *
   * Rename capabilities:
   * - Rename file in same directory: "/file.txt" → "/newname.txt"
   * - Move file to different directory: "/file.txt" → "/backup/file.txt"
   * - Move and rename: "/old.txt" → "/archive/new.txt"
   * - Preserves file content and attributes
   * - Efficient (no data copying required)
   *
   * Common use cases:
   * - **Versioning**: Rename old file before creating new version
   * - **Organization**: Move files to appropriate directories
   * - **Backup**: Rename file to indicate backup or archive status
   * - **Atomic updates**: Write new file, rename old, rename new to final name
   * - **Temporary files**: Rename temp file to final name after completion
   *
   * Example 1: Simple rename in same directory
   * @code Volume_t *vol;
   *
   * Return_t renameLogFile(void) {
   *   Base_t exists;
   *
   *   // Check if current log exists if (OK(xFileExists(vol,
   * (Byte_t*)"/system.log", &exists)) && exists) {
   *     // Rename to backup if (OK(xFileRename(vol, (Byte_t*)"/system.log",
   * (Byte_t*)"/system.log.old"))) {
   *       logInfo("Log file renamed for archival");
   *       return ReturnOK;
   *     }
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 2: Log rotation with multiple versions
   * @code void rotateLogs(Volume_t *vol) {
   *   Base_t exists;
   *
   *   // Rotate log.2 → log.3 (delete log.3 if exists) if (OK(xFileExists(vol,
   * (Byte_t*)"/log.3", &exists)) && exists) {
   *     xFileUnlink(vol, (Byte_t*)"/log.3");
   *   }
   *   if (OK(xFileExists(vol, (Byte_t*)"/log.2", &exists)) && exists) {
   *     xFileRename(vol, (Byte_t*)"/log.2", (Byte_t*)"/log.3");
   *   }
   *
   *   // Rotate log.1 → log.2 if (OK(xFileExists(vol, (Byte_t*)"/log.1",
   * &exists)) && exists) {
   *     xFileRename(vol, (Byte_t*)"/log.1", (Byte_t*)"/log.2");
   *   }
   *
   *   // Rotate current → log.1 if (OK(xFileExists(vol, (Byte_t*)"/system.log",
   * &exists)) && exists) {
   *     xFileRename(vol, (Byte_t*)"/system.log", (Byte_t*)"/log.1");
   *   }
   * }
   * @endcode
   *
   * Example 3: Atomic file replacement
   * @code Return_t atomicConfigUpdate(Volume_t *vol, Byte_t *newConfig, Size_t
   * size)
   * {
   *   File_t *tmpFile;
   *
   *   // Write to temporary file first if (OK(xFileOpen(&tmpFile, vol,
   * (Byte_t*)"/config.tmp", FS_MODE_WRITE | FS_MODE_CREATE))) {
   *     if (OK(xFileWrite(tmpFile, size, newConfig))) {
   *       xFileSync(tmpFile);  // Ensure written to disk xFileClose(tmpFile);
   *
   *       // Rename old config to backup Base_t exists;
   *       if (OK(xFileExists(vol, (Byte_t*)"/config.dat", &exists)) && exists)
   * {
   *         xFileRename(vol, (Byte_t*)"/config.dat", (Byte_t*)"/config.bak");
   *       }
   *
   *       // Rename temp to final name if (OK(xFileRename(vol,
   * (Byte_t*)"/config.tmp", (Byte_t*)"/config.dat"))) {
   *         return ReturnOK;  // Atomic update successful
   *       }
   *     } else {
   *       xFileClose(tmpFile);
   *     }
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 4: Move file to archive directory
   * @code Return_t archiveOldData(Volume_t *vol, const char *filename) {
   *   Byte_t oldPath[256];
   *   Byte_t newPath[256];
   *   Base_t exists;
   *
   *   // Build paths snprintf((char*)oldPath, sizeof(oldPath), "/%s",
   * filename);
   *   snprintf((char*)newPath, sizeof(newPath), "/archive/%s", filename);
   *
   *   // Check if file exists if (OK(xFileExists(vol, oldPath, &exists)) &&
   * exists) {
   *     // Move to archive directory if (OK(xFileRename(vol, oldPath,
   * newPath))) {
   *       logInfo("Archived: %s", filename);
   *       return ReturnOK;
   *     }
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[in] volume_  Handle to the mounted volume containing the file. Must
   *                     be a valid volume from xFSMount().
   * @param[in] oldPath_ Pointer to null-terminated string containing current
   *                     file path. The file must exist at this location.
   * @param[in] newPath_ Pointer to null-terminated string containing new file
   *                     path. If a file already exists at this path, the
   *                     operation fails.
   *
   * @return             ReturnOK if file renamed successfully, ReturnError if
   *                     rename failed due to invalid volume, source file not
   *                     found, destination file already exists, file is open,
   *                     or filesystem error.
   *
   * @warning The file must not be open during rename. Close all file handles
   * before calling xFileRename(). Renaming an open file will fail.
   *
   * @warning If a file already exists at newPath_, the rename operation will
   * fail. Delete the existing destination file first if you want to replace it.
   *
   * @warning Both paths must be on the same volume. You cannot use rename to
   * move files between different volumes or storage devices.
   *
   * @warning The destination directory must exist. Create directories with
   * xDirMake() before moving files into them.
   *
   * @note Rename is very efficient since it only updates directory entries
   * without copying file data, regardless of file size.
   *
   * @note File attributes and content are preserved during rename.
   *
   * @sa xFileExists() - Check if source/destination exists
   * @sa xFileUnlink() - Delete file instead of renaming
   * @sa xFileClose() - Close file before renaming
   * @sa xDirMake() - Create destination directory
   */
  Return_t xFileRename(Volume_t *volume_, const Byte_t *oldPath_, const Byte_t *newPath_);


  /**
   * @brief Get detailed information about a file
   *
   * Retrieves comprehensive information about a file including its name, size,
   * attributes (read-only, hidden, system), and whether it's a file or
   * directory. This information is useful for file browsing, validation, or
   * determining how to process a filesystem entry.
   *
   * The returned DirEntry structure contains all metadata about the file as
   * stored in the directory entry, without needing to open the file. This is
   * more efficient than opening the file when you only need to inspect its
   * properties.
   *
   * Information provided in DirEntry_t *:
   * - **name**: File or directory name (up to 256 characters)
   * - **size**: File size in bytes (0 for directories)
   * - **firstCluster**: Starting cluster number on disk
   * - **isDirectory**: True if entry is a directory, false if file
   * - **isReadOnly**: True if file is marked read-only
   * - **isHidden**: True if file is marked hidden
   * - **isSystem**: True if file is marked as system file
   *
   * Common use cases:
   * - **File browser**: Display file details in directory listings
   * - **Validation**: Check file attributes before processing
   * - **Filtering**: Select files based on attributes or size
   * - **Metadata inspection**: Examine file properties without opening
   *
   * Example 1: Get file size without opening
   * @code Volume_t *vol;
   * DirEntry_t *entry;
   *
   * if (OK(xFileGetInfo(vol, (Byte_t*)"/data.bin", &entry))) {
   *   if (!entry.isDirectory) {
   *     logInfo("File size: %lu bytes", entry.size);
   *
   *     // Check if file is too large for processing if (entry.size >
   * MAX_FILE_SIZE) {
   *       logError("File too large to process");
   *     }
   *   }
   * }
   * @endcode
   *
   * Example 2: Check file type before opening
   * @code Return_t processPath(Volume_t *vol, const char *path) {
   *   DirEntry_t *entry;
   *
   *   if (OK(xFileGetInfo(vol, (Byte_t*)path, &entry))) {
   *     if (entry.isDirectory) {
   *       // It's a directory - process directory return processDirectory(vol,
   * path);
   *     } else {
   *       // It's a file - process file return processFile(vol, path);
   *     }
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * Example 3: Display file attributes
   * @code void displayFileInfo(Volume_t *vol, const char *path) {
   *   DirEntry_t *entry;
   *
   *   if (OK(xFileGetInfo(vol, (Byte_t*)path, &entry))) {
   *     printf("Name: %s\n", entry.name);
   *     printf("Size: %lu bytes\n", entry.size);
   *     printf("Type: %s\n", entry.isDirectory ? "Directory" : "File");
   *
   *     // Display attributes printf("Attributes:");
   *     if (entry.isReadOnly) printf(" [READ-ONLY]");
   *     if (entry.isHidden) printf(" [HIDDEN]");
   *     if (entry.isSystem) printf(" [SYSTEM]");
   *     printf("\n");
   *   }
   * }
   * @endcode
   *
   * Example 4: Filter files by size and attributes
   * @code Return_t findLargeFiles(Volume_t *vol, const char *dirPath, Word_t
   * minSize) {
   *   Dir_t *dir;
   *   DirEntry_t *entry;
   *
   *   if (OK(xDirOpen(&dir, vol, (Byte_t*)dirPath))) {
   *     while (OK(xDirRead(dir, &entry))) {
   *       // Skip directories and hidden files if (!entry.isDirectory &&
   * !entry.isHidden) {
   *         if (entry.size >= minSize) {
   *           printf("Large file: %s (%lu bytes)\n", entry.name, entry.size);
   *         }
   *       }
   *     }
   *     xDirClose(dir);
   *     return ReturnOK;
   *   }
   *   return ReturnError;
   * }
   * @endcode
   *
   * @param[in]  volume_ Handle to the mounted volume containing the file. Must
   *                     be a valid volume from xFSMount().
   * @param[in]  path_   Pointer to null-terminated string containing path to
   *                     file or directory. Use forward slashes (/) for
   *                     separators.
   * @param[out] entry_  Pointer to DirEntry_t *structure to receive file
   *                     information. Structure is populated with all available
   *                     metadata.
   *
   * @return             ReturnOK if information retrieved successfully,
   *                     ReturnError if operation failed due to invalid volume,
   *                     file/directory not found, or path error.
   *
   * @note This function works for both files and directories. Check the
   * isDirectory field in the returned entry to determine which type it is.
   *
   * @note The entry_ structure is populated with a copy of the directory entry
   * data. Changes to this structure do not affect the file on disk.
   *
   * @note The size field is 0 for directories, as directories don't have data
   * content in the same way files do.
   *
   * @sa xFileExists() - Check if file exists (simpler boolean check)
   * @sa xDirRead() - Read directory entries sequentially
   * @sa xFileOpen() - Open file to access content
   * @sa xFileGetSize() - Get size of open file
   */
  Return_t xFileGetInfo(Volume_t *volume_, const Byte_t *path_, DirEntry_t **entry_);


  /**
   * @brief Open a directory for reading
   *
   * Opens a directory for reading its contents, creating a directory handle
   * used to iterate through directory entries with xDirRead(). This is the
   * first step in browsing directory contents, listing files, or searching for
   * specific entries.
   *
   * Opening a directory allocates kernel resources and positions the read
   * pointer at the first entry. Use xDirRead() to retrieve entries
   * sequentially, xDirRewind() to return to the beginning, and xDirClose() to
   * free resources when finished.
   *
   * Directory reading workflow:
   * 1. Open directory with xDirOpen() 2. Read entries with xDirRead() in a loop
   * 3. Optionally rewind with xDirRewind() to re-read 4. Close with xDirClose()
   * when finished
   *
   * Common use cases:
   * - **File listing**: Display all files in a directory
   * - **File searching**: Find specific files by name or attributes
   * - **Directory scanning**: Process all files in directory
   * - **File counting**: Count files or calculate total size
   * - **Filtering**: Select files matching criteria
   *
   * Example 1: List all files in directory
   * @code Volume_t *vol;
   * Dir_t *dir;
   * DirEntry_t *entry;
   *
   * if (OK(xDirOpen(&dir, vol, (Byte_t*)"/"))) {
   *   printf("Files in root directory:\n");
   *
   *   while (OK(xDirRead(dir, &entry))) {
   *     if (entry.isDirectory) {
   *       printf("  [DIR]  %s\n", entry.name);
   *     } else {
   *       printf("  [FILE] %s (%lu bytes)\n", entry.name, entry.size);
   *     }
   *   }
   *
   *   xDirClose(dir);
   * }
   * @endcode
   *
   * Example 2: Find specific file in directory
   * @code Base_t findFileInDirectory(Volume_t *vol, const char *dirPath, const
   * char *filename) {
   *   Dir_t *dir;
   *   DirEntry_t *entry;
   *   Base_t found = 0;
   *
   *   if (OK(xDirOpen(&dir, vol, (Byte_t*)dirPath))) {
   *     while (OK(xDirRead(dir, &entry))) {
   *       if (strcmp((char*)entry.name, filename) == 0) {
   *         found = 1;
   *         break;
   *       }
   *     }
   *     xDirClose(dir);
   *   }
   *
   *   return found;
   * }
   * @endcode
   *
   * Example 3: Calculate directory size
   * @code Word_t calculateDirectorySize(Volume_t *vol, const char *path) {
   *   Dir_t *dir;
   *   DirEntry_t *entry;
   *   Word_t totalSize = 0;
   *
   *   if (OK(xDirOpen(&dir, vol, (Byte_t*)path))) {
   *     while (OK(xDirRead(dir, &entry))) {
   *       if (!entry.isDirectory) {
   *         totalSize += entry.size;
   *       }
   *     }
   *     xDirClose(dir);
   *   }
   *
   *   return totalSize;
   * }
   * @endcode
   *
   * Example 4: Count files by type
   * @code void countFileTypes(Volume_t *vol) {
   *   Dir_t *dir;
   *   DirEntry_t *entry;
   *   Word_t fileCount = 0, dirCount = 0;
   *
   *   if (OK(xDirOpen(&dir, vol, (Byte_t*)"/"))) {
   *     while (OK(xDirRead(dir, &entry))) {
   *       if (entry.isDirectory) {
   *         dirCount++;
   *       } else {
   *         fileCount++;
   *       }
   *     }
   *     xDirClose(dir);
   *
   *     printf("Files: %lu, Directories: %lu\n", fileCount, dirCount);
   *   }
   * }
   * @endcode
   *
   * @param[out] dir_    Pointer to Dir_t *handle to be initialized. On success,
   *                     this handle is used for reading directory entries and
   *                     must be closed with xDirClose().
   * @param[in]  volume_ Handle to mounted volume containing the directory. Must
   *                     be a valid volume from xFSMount().
   * @param[in]  path_   Pointer to null-terminated string containing directory
   *                     path. Use "/" for root directory, or "/dirname" for
   *                     subdirectories.
   *
   * @return             ReturnOK if directory opened successfully, ReturnError
   *                     if open failed due to invalid volume, directory not
   *                     found, path refers to a file, or insufficient
   *                     resources.
   *
   * @warning The directory handle must be closed with xDirClose() when
   * finished. Failure to close directories causes resource leaks.
   *
   * @warning The path must refer to a directory, not a file. Opening a file
   * path as a directory will fail.
   *
   * @note After opening, the read position is at the first entry. Use
   * xDirRead() to retrieve entries sequentially.
   *
   * @note Directory handles are allocated from kernel memory and are a limited
   * resource. Always close directories promptly after use.
   *
   * @sa xDirClose() - Close directory and free resources
   * @sa xDirRead() - Read next directory entry
   * @sa xDirRewind() - Reset to beginning of directory
   * @sa xFileGetInfo() - Get info about specific path
   */
  Return_t xDirOpen(Dir_t **dir_, Volume_t *volume_, const Byte_t *path_);


  /**
   * @brief Close an open directory
   *
   * Closes a previously opened directory, freeing the directory handle and
   * associated kernel resources. After closing, the directory handle becomes
   * invalid and must not be used in any subsequent directory operations.
   *
   * Always close directories when finished reading to free kernel resources and
   * prevent resource leaks. Directory handles are limited, and failing to close
   * them may prevent other directories from being opened.
   *
   * Example 1: Basic directory listing with close
   * @code Dir_t *dir;
   * DirEntry_t *entry;
   *
   * if (OK(xDirOpen(&dir, vol, (Byte_t*)"/"))) {
   *   while (OK(xDirRead(dir, &entry))) {
   *     printf("%s\n", entry.name);
   *   }
   *
   *   // Always close when done xDirClose(dir);
   * }
   * @endcode
   *
   * Example 2: Error handling with guaranteed close
   * @code Return_t processDirectory(Volume_t *vol, const char *path) {
   *   Dir_t *dir;
   *   DirEntry_t *entry;
   *   Return_t result = ReturnError;
   *
   *   if (OK(xDirOpen(&dir, vol, (Byte_t*)path))) {
   *     while (OK(xDirRead(dir, &entry))) {
   *       if (processEntry(&entry)) {
   *         result = ReturnOK;
   *       }
   *     }
   *
   *     // Close regardless of processing success/failure xDirClose(dir);
   *   }
   *
   *   return result;
   * }
   * @endcode
   *
   * @param[in] dir_ Handle to the open directory to close. Must be a valid
   *                 directory handle from xDirOpen(). After closing, becomes
   *                 invalid.
   *
   * @return         ReturnOK if close succeeded, ReturnError if close failed
   *                 due to invalid directory handle or directory not open.
   *
   * @warning After calling xDirClose(), the directory handle becomes invalid
   * and must not be used in any directory operations.
   *
   * @note It is good practice to set the directory handle to null after closing
   * to prevent accidental use of an invalid handle.
   *
   * @sa xDirOpen() - Open directory for reading
   * @sa xDirRead() - Read directory entries
   */
  Return_t xDirClose(Dir_t *dir_);


  /**
   * @brief Read the next entry from a directory
   *
   * Retrieves the next entry from an open directory, advancing the read
   * position. Entries are returned sequentially including both files and
   * subdirectories. Use the isDirectory field in the returned entry to
   * distinguish between them.
   *
   * Reading continues until all entries have been retrieved. When no more
   * entries are available, xDirRead() returns ReturnError, indicating the end
   * of the directory. Use xDirRewind() to return to the beginning and re-read
   * entries if needed.
   *
   * Example 1: Process all files (skip directories)
   * @code Dir_t *dir;
   * DirEntry_t *entry;
   *
   * if (OK(xDirOpen(&dir, vol, (Byte_t*)"/"))) {
   *   while (OK(xDirRead(dir, &entry))) {
   *     // Process only files, not directories if (!entry.isDirectory) {
   *       processFile(vol, (char*)entry.name);
   *     }
   *   }
   *   xDirClose(dir);
   * }
   * @endcode
   *
   * Example 2: Filter files by extension
   * @code Base_t hasExtension(const char *filename, const char *ext) {
   *   const char *dot = strrchr(filename, '.');
   *   return (dot && strcmp(dot, ext) == 0);
   * }
   *
   * void processLogFiles(Volume_t *vol) {
   *   Dir_t *dir;
   *   DirEntry_t *entry;
   *
   *   if (OK(xDirOpen(&dir, vol, (Byte_t*)"/"))) {
   *     while (OK(xDirRead(dir, &entry))) {
   *       if (!entry.isDirectory && hasExtension((char*)entry.name, ".log")) {
   *         printf("Log file: %s (%lu bytes)\n", entry.name, entry.size);
   *       }
   *     }
   *     xDirClose(dir);
   *   }
   * }
   * @endcode
   *
   * @param[in]  dir_   Handle to open directory from xDirOpen().
   * @param[out] entry_ Pointer to DirEntry_t *structure to receive entry
   *                    information.
   *
   * @return            ReturnOK if entry read successfully, ReturnError if no
   *                    more entries (end of directory) or invalid directory
   *                    handle.
   *
   * @note When xDirRead() returns ReturnError, it indicates the end of the
   * directory, not necessarily a read error.
   *
   * @note Use xDirRewind() to return to the beginning and re-read entries.
   *
   * @sa xDirOpen() - Open directory for reading
   * @sa xDirClose() - Close directory
   * @sa xDirRewind() - Reset to beginning
   */
  Return_t xDirRead(Dir_t *dir_, DirEntry_t **entry_);


  /**
   * @brief Reset directory read position to beginning
   *
   * Resets the read position of an open directory back to the first entry,
   * allowing the directory to be re-read from the start. This is useful when
   * you need to make multiple passes through a directory without closing and
   * reopening it.
   *
   * Example: Count and then process files
   * @code Dir_t *dir;
   * DirEntry_t *entry;
   * Word_t fileCount = 0;
   *
   * if (OK(xDirOpen(&dir, vol, (Byte_t*)"/"))) {
   *   // First pass - count files while (OK(xDirRead(dir, &entry))) {
   *     if (!entry.isDirectory) {
   *       fileCount++;
   *     }
   *   }
   *
   *   printf("Processing %lu files...\n", fileCount);
   *
   *   // Rewind to beginning xDirRewind(dir);
   *
   *   // Second pass - process files while (OK(xDirRead(dir, &entry))) {
   *     if (!entry.isDirectory) {
   *       processFile((char*)entry.name);
   *     }
   *   }
   *
   *   xDirClose(dir);
   * }
   * @endcode
   *
   * @param[in] dir_ Handle to open directory from xDirOpen().
   *
   * @return         ReturnOK if rewind succeeded, ReturnError if invalid
   *                 directory handle.
   *
   * @sa xDirOpen() - Open directory
   * @sa xDirRead() - Read entries
   * @sa xDirClose() - Close directory
   */
  Return_t xDirRewind(Dir_t *dir_);


  /**
   * @brief Create a new directory
   *
   * Creates a new directory at the specified path. The parent directory must
   * already exist. Use this function to organize files into a directory
   * structure.
   *
   * Example: Create directory and add file
   * @code if (OK(xDirMake(vol, (Byte_t*)"/logs"))) {
   *   File_t *file;
   *   if (OK(xFileOpen(&file, vol, (Byte_t*)"/logs/system.log", FS_MODE_WRITE |
   * FS_MODE_CREATE))) {
   *     xFileWrite(file, 10, (Byte_t*)"Log start\n");
   *     xFileClose(file);
   *   }
   * }
   * @endcode
   *
   * @param[in] volume_ Handle to mounted volume.
   * @param[in] path_   Path to new directory.
   *
   * @return            ReturnOK if directory created, ReturnError if failed
   *                    (already exists, parent doesn't exist, or insufficient
   *                    space).
   *
   * @warning Parent directory must exist. Create parent directories first if
   * needed.
   *
   * @sa xDirRemove() - Delete empty directory
   * @sa xDirOpen() - Open directory
   */
  Return_t xDirMake(Volume_t *volume_, const Byte_t *path_);


  /**
   * @brief Delete an empty directory
   *
   * Removes an empty directory from the filesystem. The directory must be empty
   * (contain no files or subdirectories) for the operation to succeed.
   *
   * Example: Remove temporary directory
   * @code
   * // Remove all files first Dir_t *dir;
   * DirEntry_t *entry;
   *
   * if (OK(xDirOpen(&dir, vol, (Byte_t*)"/temp"))) {
   *   while (OK(xDirRead(dir, &entry))) {
   *     if (!entry.isDirectory) {
   *       Byte_t path[256];
   *       snprintf((char*)path, sizeof(path), "/temp/%s", entry.name);
   *       xFileUnlink(vol, path);
   *     }
   *   }
   *   xDirClose(dir);
   * }
   *
   * // Now remove empty directory xDirRemove(vol, (Byte_t*)"/temp");
   * @endcode
   *
   * @param[in] volume_ Handle to mounted volume.
   * @param[in] path_   Path to directory to remove.
   *
   * @return            ReturnOK if directory removed, ReturnError if failed
   *                    (not empty, doesn't exist, or is root directory).
   *
   * @warning Directory must be empty. Remove all files and subdirectories
   * first.
   *
   * @warning Cannot remove the root directory.
   *
   * @sa xDirMake() - Create directory
   * @sa xFileUnlink() - Delete file
   */
  Return_t xDirRemove(Volume_t *volume_, const Byte_t *path_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */

#endif /* ifndef HELIOS_H_ */