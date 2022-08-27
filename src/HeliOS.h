/**
 * @file HeliOS.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Header file for end-user application code
 * @version 0.3.5
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

#include "posix.h"

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
  TaskStateError,     /**< Returned by xTaskGetTaskState() when task cannot be found. */
  TaskStateSuspended, /**< State a task is in when it is first created by xTaskCreate() or suspended by xTaskSuspend(). */
  TaskStateRunning,   /**< State a task is in after xTaskResume() is called. */
  TaskStateWaiting    /**< State a task is in after xTaskWait() is called. */
} TaskState_t;

/**
 * @brief Enumerated type for scheduler states.
 *
 * The scheduler can be in one of four possible states defined in the SchedulerState_t
 * enumerated type. The state of the scheduler is changed by calling xTaskSuspendAll()
 * and xTaskResumeAll(). The state can be obtained by calling xTaskGetSchedulerState().
 *
 * @sa xSchedulerState
 * @sa xTaskSuspendAll()
 * @sa xTaskResumeAll()
 *
 */
typedef enum {
  SchedulerStateError,     /**< Not used. */
  SchedulerStateSuspended, /**< State the scheduler is in after xTaskSuspendAll() is called. */
  SchedulerStateRunning    /**< State the scheduler is in after xTaskResumeAll() is called. */
} SchedulerState_t;

/**
 * @brief Type definition for the base data type.
 *
 * A simple data type is often needed as an argument for a system call or a return type.
 * The Base_t type is used in such a case where there are no other structural data
 * requirements and is typically an unsigned 8-bit integer. The Base_t type should
 * be declared as xBase.
 *
 * @sa xBase
 *
 */
typedef uint8_t Base_t;

/**
 * @brief Type defintion for the word data type
 *
 * A word is a 32-bit data type in HeliOS.
 *
 * @sa xWord
 */
typedef uint32_t Word_t;

/**
 * @brief The type definition for time expressed in ticks.
 *
 * The xTicks type is used by several of the task and timer related system calls to express time.
 * The unit of measure for time is always ticks.
 *
 * @sa xTicks
 *
 */
typedef uint32_t Ticks_t;

/**
 * @brief The type defintion for storing the size of some object in memory.
 *
 * The Size_t type is used to store the size of an object in memory and is
 * always represented in bytes. Size_t should always be declared as xSize.
 *
 * @sa xSize
 *
 */
typedef size_t Size_t;

/**
 * @brief The type defintion for storing the size of some object in memory.
 *
 * The xSize type is used to store the size of an object in memory and is
 * always represented in bytes.
 *
 */
typedef Size_t xSize;

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
 * @sa xMemFree()
 *
 * @warning The memory allocated for an instance of xTaskRunTimeStats must be freed
 * using xMemFree().
 *
 */
typedef struct TaskRunTimeStats_s {
  Base_t id;            /**< The task identifier which is used by xTaskGetHandleById() to return the task handle. */
  Ticks_t lastRunTime;  /**< The runtime duration in ticks the last time the task was executed by the scheduler. */
  Ticks_t totalRunTime; /**< The total runtime duration in ticks the task has been executed by the scheduler. */
} TaskRunTimeStats_t;

/**
 * @brief Data structure for information about a task.
 *
 * The TaskInfo_t structure is similar to xTaskRuntimeStats_t in that it contains runtime statistics for
 * a task. However, TaskInfo_t also contains additional details about a task such as its identifier, ASCII name
 * and state. The TaskInfo_t structure is returned by xTaskGetTaskInfo(). If only runtime statistics are needed,
 * TaskRunTimeStats_t should be used because of its lower memory footprint. The TaskInfo_t type should be
 * declared as xTaskInfo.
 *
 * @sa xTaskInfo
 * @sa xTaskGetTaskInfo()
 * @sa xMemFree()
 * @sa CONFIG_TASK_NAME_BYTES
 *
 * @warning The memory allocated for an instance of xTaskInfo must be freed using
 * xMemFree().
 *
 */
typedef struct TaskInfo_s {
  Base_t id;                         /**< The task identifier which is used by xTaskGetHandleById() to return the task handle. */
  char name[CONFIG_TASK_NAME_BYTES]; /**< The name of the task which is used by xTaskGetHandleByName() to return the task handle. This is NOT a null terminated string. */
  TaskState_t state;                 /**< The state the task is in which is one of four states specified in the TaskState_t enumerated data type. */
  Ticks_t lastRunTime;               /**< The runtime duration in ticks the last time the task was executed by the scheduler. */
  Ticks_t totalRunTime;              /**< The total runtime duration in ticks the task has been executed by the scheduler. */
} TaskInfo_t;

/**
 * @brief Data structure for direct to task notifications.
 *
 * The TaskNotification_t data structure contains the direct to task notification returned by xTaskNotifyTake().
 * The TaskNotification_t type should be declared as xTaskNotification.
 *
 * @sa xTaskNotification
 * @sa xTaskNotifyTake()
 * @sa xMemFree()
 * @sa CONFIG_NOTIFICATION_VALUE_BYTES
 *
 * @warning The memory allocated for an instance of xTaskNotification must be freed using
 * xMemFree().
 *
 */
typedef struct TaskNotification_s {
  Base_t notificationBytes;                                /**< The number of bytes in the notificationValue member that makes up the notification value. This cannot exceed CONFIG_NOTIFICATION_VALUE_BYTES. */
  char notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES]; /**< The char array that contains the actual notification value. This is NOT a null terminated string. */
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
 * @sa xMemFree()
 * @sa CONFIG_MESSAGE_VALUE_BYTES
 *
 * @warning The memory allocated for an instance of xQueueMessage must be freed using xMemFree().
 *
 */
typedef struct QueueMessage_s {
  Base_t messageBytes;                           /**< The number of bytes in the messageValue member that makes up the message value. This cannot exceed CONFIG_MESSAGE_VALUE_BYTES. */
  char messageValue[CONFIG_MESSAGE_VALUE_BYTES]; /**< the char array that contains the actual message value. This is NOT a null terminated string. */
} QueueMessage_t;

/**
 * @brief Data structure for system informaiton.
 *
 * The SystemInfo_t data structure contains information about the HeliOS system and is returned
 * by xSystemGetSystemInfo(). The SystemInfo_t type should be declared as xSystemInfo.
 *
 * @sa xSystemInfo
 * @sa xSystemGetSystemInfo()
 * @sa xMemFree()
 *
 * @warning The memory allocated for an instance of xSystemInfo must be freed using xMemFree().
 *
 */
typedef struct SystemInfo_s {
  char productName[OS_PRODUCT_NAME_SIZE]; /**< The name of the operating system or product. Its length is defined by OS_PRODUCT_NAME_SIZE. This is NOT a null terminated string. */
  Base_t majorVersion;                    /**< The major version number of HeliOS and is Symantec Versioning Specification (SemVer) compliant. */
  Base_t minorVersion;                    /**< The minor version number of HeliOS and is Symantec Versioning Specification (SemVer) compliant. */
  Base_t patchVersion;                    /**< The patch version number of HeliOS and is Symantec Versioning Specification (SemVer) compliant. */
  Base_t numberOfTasks;                   /**< The number of tasks presently in a suspended, running or waiting state. */
} SystemInfo_t;

/**
 * @brief Data structure for statistics on a memory region.
 *
 * The MemoryRegionStats_t data structure is used to store statistics about a HeliOS
 * memory region. Statistics can be obtained for the heap and kernel memory regions.
 *
 * @sa xMemGetHeapStats()
 * @sa xMemGetKernelStats()
 * @sa xMemoryRegionStats
 *
 */
typedef struct MemoryRegionStats_s {
  Word_t largestFreeEntryInBytes;
  Word_t smallestFreeEntryInBytes;
  Word_t numberOfFreeBlocks;
  Word_t availableSpaceInBytes;
  Word_t successfulAllocations;
  Word_t successfulFrees;
  Word_t minimumEverFreeBytesRemaining;
} MemoryRegionStats_t;

/**
 * @brief Stub type definition for the task type.
 *
 * The Task_t type is a stub type definition for the internal task data structure and is treated
 * as a task handle by most of the task related system calls. The members of the data structure
 * are not accessible. The Task_t type should be declared as xTask.
 *
 * @sa xTask
 * @sa xTaskDelete()
 *
 * @warning The memory allocated for an instance of xTask must be freed by xTaskDelete()
 *
 */
typedef void Task_t;

/**
 * @brief Type definition for the task parameter.
 *
 * The TaskParm_t type is used to pass a parameter to a task at the time of creation using
 * xTaskCreate(). A task parameter is a pointer of type void and can point to any number
 * of intrinsic types, arrays and/or user defined structures which can be passed to a
 * task. It is up the the end-user to manage, allocate and free the memory related to
 * these objects using xMemAlloc() and xMemFree(). The TaskParm_t should be declared
 * as xTaskParm.
 *
 * @sa xTaskParm
 * @sa xMemAlloc()
 * @sa xMemFree()
 *
 * @warning The memory allocated for an instance of xTaskParm must be freed using xMemFree().
 *
 */
typedef void TaskParm_t;

/**
 * @brief Stub type definition for the message queue type.
 *
 * The Queue_t type is a stub type definition for the internal message queue structure and is treated
 * as a message queue handle by most of the message queue related system calls. The members of the data structure
 * are not accessible. The Queue_t type should be declared as xQueue.
 *
 * @sa xQueue
 * @sa xQueueDelete()
 *
 * @warning The memory allocated for an instance of xQueue must be freed using xQueueDelete().
 *
 */
typedef void Queue_t;

/**
 * @brief Stub type definition for the timer type.
 *
 * The Timer_t type is a stub type definition for the internal timer data structure and is treated
 * as a timer handle by most of the timer related system calls. The members of the data structure
 * are not accessible. The Timer_t type should be declared as xTimer.
 *
 * @sa xTimer
 * @sa xTimerDelete()
 *
 * @warning The memory allocated for an instance of xTimer must be freed using xTimerDelete().
 *
 */
typedef void Timer_t;

/**
 * @brief Type defintion for the memory address data type.
 *
 * The xAddr type is used to store a memory address and is used to pass memory
 * addresses back and forth between system calls and the end-user application. It
 * is not necessary to use the xAddr type within the end-user application as long
 * as the type is not used to interact with the HeliOS kernel through system calls.
 *
 *
 */
typedef void Addr_t;

/**
 * @brief Type defintion for the memory address data type.
 *
 * The xAddr type is used to store a memory address and is used to pass memory
 * addresses back and forth between system calls and the end-user application. It
 * is not necessary to use the xAddr type within the end-user application as long
 * as the type is not used to interact with the HeliOS kernel through system calls.
 *
 *
 */
typedef Addr_t *xAddr;

/**
 * @brief Type definition for the base data type.
 *
 * A simple data type is often needed as an argument for a system call or a return type.
 * The xBase type is used in such a case where there are no other structural data
 * requirements is typically an unsigned 8-bit integer.
 *
 * @sa Base_t
 *
 */
typedef Base_t xBase;

/**
 * @brief Type defintion for the word data type
 *
 * A word is a 32-bit data type in HeliOS.
 *
 * @sa Word_t
 */
typedef Word_t xWord;

/**
 * @brief Stub type definition for the timer type.
 *
 * The xTimer type is a stub type definition for the internal timer data structure and is treated
 * as a timer handle by most of the timer related system calls. The members of the data structure
 * are not accessible.
 *
 * @sa Timer_t
 * @sa xTimerDelete()
 *
 * @warning The memory allocated for an instance of xTimer must be freed using xTimerDelete().
 *
 */
typedef Timer_t *xTimer;

/**
 * @brief Stub type definition for the message queue type.
 *
 * The xQueue type is a stub type definition for the internal message queue structure and is treated
 * as a message queue handle by most of the message queue related system calls. The members of the data structure
 * are not accessible.
 *
 * @sa Queue_t
 * @sa xQueueDelete()
 *
 * @warning The memory allocated for an instance of xQueue must be freed using xQueueDelete().
 *
 */
typedef Queue_t *xQueue;

/**
 * @brief Data structure for a message queue message.
 *
 * The xQueueMessage data structure contains the message queue message returned by xQueuePeek() and
 * xQueueReceive(). See QueueMessage_t for information about the data structure's members.
 *
 * @sa QueueMessage_t
 * @sa xQueuePeek()
 * @sa xQueueReceive()
 * @sa xMemFree()
 * @sa CONFIG_MESSAGE_VALUE_BYTES
 *
 * @warning The memory allocated for an instance of xQueueMessage must be freed using xMemFree().
 *
 */
typedef QueueMessage_t *xQueueMessage;

/**
 * @brief Data structure for direct to task notifications.
 *
 * The xTaskNotification data structure contains the direct to task notification returned by xTaskNotifyTake().
 * See TaskNotification_t for information about the data structure's members.
 *
 * @sa TaskNotification_t
 * @sa xTaskNotifyTake()
 * @sa xMemFree()
 * @sa CONFIG_NOTIFICATION_VALUE_BYTES
 *
 * @warning The memory allocated for an instance of xTaskNotification must be freed using
 * xMemFree().
 *
 */
typedef TaskNotification_t *xTaskNotification;

/**
 * @brief Data structure for information about a task.
 *
 * The xTaskInfo structure is similar to xTaskRunTimeStats in that it contains runtime statistics for
 * a task. However, xTaskInfo also contains additional details about a task such as its identifier, ASCII name
 * and state. The xTaskInfo structure is returned by xTaskGetTaskInfo(). If only runtime statistics are needed,
 * xTaskRunTimeStats should be used because of its lower memory footprint. See TaskInfo_t for information about
 * the data structure's members.
 *
 * @sa TaskInfo_t
 * @sa xTaskGetTaskInfo()
 * @sa xMemFree()
 * @sa CONFIG_TASK_NAME_BYTES
 *
 * @warning The memory allocated for an instance of xTaskInfo must be freed using
 * xMemFree().
 *
 */
typedef TaskInfo_t *xTaskInfo;

/**
 * @brief Data structure for task runtime statistics.
 *
 * The xTaskRunTimeStats structure contains task runtime statistics and is returned by
 * xTaskGetAllRunTimeStats() and xTaskGetTaskRunTimeStats(). See TaskRunTimeStats_t for information
 * about the data structure's members.
 *
 * @sa TaskRunTimeStats_t
 * @sa xTaskGetTaskRunTimeStats()
 * @sa xTaskGetAllRunTimeStats()
 * @sa xMemFree()
 *
 * @warning The memory allocated for an instance of xTaskRunTimeStats must be freed
 * using xMemFree().
 *
 */
typedef TaskRunTimeStats_t *xTaskRunTimeStats;

/**
 * @brief Data structure for statistics on a memory region.
 *
 * The xMemoryRegionStats data structure is used to store statistics about a HeliOS
 * memory region. Statistics can be obtained for the heap and kernel memory regions.
 *
 * @sa xMemGetHeapStats()
 * @sa xMemGetKernelStats()
 * @sa MemoryRegionStats_t
 *
 */
typedef MemoryRegionStats_t *xMemoryRegionStats;

/**
 * @brief Stub type definition for the task type.
 *
 * The xTask type is a stub type definition for the internal task data structure and is treated
 * as a task handle by most of the task related system calls. The members of the data structure
 * are not accessible.
 *
 * @sa Task_t
 * @sa xTaskCreate()
 * @sa xTaskDelete()
 *
 * @warning The memory allocated for an instance of xTask must be freed by xTaskDelete()
 *
 */
typedef Task_t *xTask;

/**
 * @brief Type definition for the task parameter.
 *
 * The xTaskParm type is used to pass a parameter to a task at the time of creation using
 * xTaskCreate(). A task parameter is a pointer of type void and can point to any number
 * of intrinsic types, arrays and/or user defined structures which can be passed to a
 * task. It is up the the end-user to manage allocate and free the memory related to
 * these objects using xMemAlloc() and xMemFree().
 *
 * @sa TaskParm_t
 * @sa xMemAlloc()
 * @sa xMemFree()
 *
 * @warning The memory allocated for an instance of xTaskParm must be freed using xMemFree().
 *
 */
typedef TaskParm_t *xTaskParm;

/**
 * @brief The type definition for time expressed in ticks.
 *
 * The xTicks type is used by several of the task and timer related system calls to express time.
 * The unit of measure for time is always ticks.
 *
 * @sa Ticks_t
 *
 */
typedef Ticks_t xTicks;

/**
 * @brief Enumerated type for task states.
 *
 * A task can be in one of the four possible states defined in the TaskState_t
 * enumerated type. The state of a task is changed by calling xTaskResume(),
 * xTaskSuspend() or xTaskWait().
 *
 * @sa TaskState_t
 * @sa xTaskResume()
 * @sa xTaskSuspend()
 * @sa xTaskWait()
 *
 */
typedef TaskState_t xTaskState;

/**
 * @brief Enumerated type for scheduler states.
 *
 * The scheduler can be in one of four possible states defined in the SchedulerState_t
 * enumerated type. The state of the scheduler is changed by calling xTaskSuspendAll()
 * and xTaskResumeAll(). The state can be obtained by calling xTaskGetSchedulerState().
 *
 * @sa xSchedulerState
 * @sa xTaskSuspendAll()
 * @sa xTaskResumeAll()
 * @sa xTaskGetSchedulerState()
 *
 */
typedef SchedulerState_t xSchedulerState;

/**
 * @brief Data structure for system informaiton.
 *
 * The xSystemInfo data structure contains information about the HeliOS system and is returned
 * by xSystemGetSystemInfo(). See xSystemInfo_t for information about the data structure's members.
 *
 * @sa SystemInfo_t
 * @sa xSystemGetSystemInfo()
 * @sa xMemFree()
 *
 * @warning The memory allocated for an instance of xSystemInfo must be freed using xMemFree().
 *
 */
typedef SystemInfo_t *xSystemInfo;

/**
 * @brief A C macro to simplify casting and dereferencing a task paramater.
 *
 * When a task paramater is passed to a task, it is passed as a pointer of
 * type void. To use the paramater, it must first be cast to the correct type
 * and dereferenced. The following is an example of how the DEREF_TASKPARM() C
 * macro simplifies that process.
 *
 * @code {.c}
 * void myTask_main(xTask task_, xTaskParm parm_) {
 *  int i;
 *
 *  i = DEREF_TASKPARM(int, parm_);
 *
 *  i++;
 *
 *  DEREF_TASKPARM(int, parm_) = i;
 *
 *  return;
 * }
 * @endcode
 *
 * @param t The data type to cast the task paramater to (e.g., int).
 * @param p The task pointer, typically named parm_.
 */
#if !defined(DEREF_TASKPARM)
#define DEREF_TASKPARM(t, p) *(( t *) p )
#endif

/* In the event HeliOS is compiled with a C++ compiler, make the system calls (written in C)
visible to C++. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System call to initialize the system.
 *
 * The xSystemInit() system call initializes the required interrupt handlers and
 * memory and must be called prior to calling any other system call.
 *
 *
 */
void xSystemInit(void);

/**
 * @brief System call to handle assertions.
 *
 * The _SystemAssert_() system call handles assertions. The _SystemAssert_() system
 * call should not be called directly. Instead, the SYSASSERT() macro should be used.
 * The system assertion functionality will only work when the CONFIG_ENABLE_SYSTEM_ASSERT
 * and CONFIG_SYSTEM_ASSERT_BEHAVIOR settings are defined.
 *
 * @sa SYSASSERT
 * @sa CONFIG_ENABLE_SYSTEM_ASSERT
 * @sa CONFIG_SYSTEM_ASSERT_BEHAVIOR
 *
 * @param file_ This is automatically defined by the compiler's definition of _FILE_
 * @param line_  This is automatically defined by the compiler's definition of _LINE_
 */
void _SystemAssert_(const char *file_, int line_);

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
 * be freed by their respective delete system calls (i.e., xTaskDelete()).
 */
void xMemFree(const xAddr addr_);

/**
 * @brief System call to return the amount of allocated heap memory.
 *
 * The xMemGetUsed() system call returns the amount of heap memory, in bytes,
 * that is currently allocated. Calls to xMemAlloc() increases and xMemFree()
 * decreases the amount of memory in use.
 *
 * @return Size_t The amount of memory currently allocated in bytes. If no heap
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
 * @return Size_t The amount of memory currently allocated to the specific address in bytes. If
 * the address is invalid or null, xMemGetSize() will return zero.
 *
 * @note If the address addr_ points to a structure that, for example, is 48 bytes in size
 * base on sizeof(), xMemGetSize() will return the number of bytes allocated by the block(s)
 * that contain the structure. Assuming the default block size of 32, a 48 byte structure would require
 * TWO blocks so xMemGetSize() would return 64 - not 48. xMemGetSize() also checks the health of the
 * heap and will return zero if it detects a consistency issue with the heap. Thus, xMemGetSize()
 * can be used to validate addresses before the objects they reference are accessed.
 */
xSize xMemGetSize(const xAddr addr_);

/**
 * @brief System call to obtain statistics on the heap.
 *
 * The xMemGetHeapStats() system call will return statistics about the heap
 * so the end-user can better understand the state of the heap.
 *
 * @sa xMemoryRegionStats
 *
 * @return xMemoryRegionStats Returns the xMemoryRegionStats structure or null
 * if unsuccesful.
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
 * if unsuccesful.
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
xQueue xQueueCreate(xBase limit_);

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
xBase xQueueGetLength(xQueue queue_);

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
xBase xQueueIsQueueEmpty(xQueue queue_);

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
xBase xQueueIsQueueFull(xQueue queue_);

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
xBase xQueueMessagesWaiting(xQueue queue_);

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
xBase xQueueSend(xQueue queue_, xBase messageBytes_, const char *messageValue_);

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
xQueueMessage xQueuePeek(xQueue queue_);

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
 */
void xTaskSuspendAll(void);

/**
 * @brief The xSystemGetSystemInfo() system call will return the type xSystemInfo containing
 * information about the system including the OS (product) name, its version and how many tasks
 * are currently in the running, suspended or waiting states.
 *
 * @return xSystemInfo The system info is returned if successful, otherwise null is
 * returned if unsuccessful.
 *
 * @sa xSystemInfo
 * @sa xMemFree()
 *
 * @warning The memory allocated by the xSystemGetSystemInfo() must be freed with xMemFree()
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
xTask xTaskCreate(const char *name_, void (*callback_)(xTask, xTaskParm), xTaskParm taskParameter_);

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
void xTaskDelete(xTask task_);

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
xTask xTaskGetHandleByName(const char *name_);

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
xTask xTaskGetHandleById(xBase id_);

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
xTaskRunTimeStats xTaskGetTaskRunTimeStats(xTask task_);

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
xTaskInfo xTaskGetTaskInfo(xTask task_);

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
xTaskState xTaskGetTaskState(xTask task_);

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
 * @return char* A pointer to the char array containing the ASCII name of the task. The task name
 * is NOT a null terminated char string. xTaskGetName() will return null if the task cannot be found.
 *
 * @warning The memory allocated by xTaskGetName() must be free by xMemFree().
 */
char *xTaskGetName(xTask task_);

/**
 * @brief System call to return the task identifier for a task.
 *
 * The xTaskGetId() system call returns the task identifier for the task.
 *
 * @param task_ The task to return the identifier of.
 * @return xBase The identifier of the task. If the task cannot be found, xTaskGetId()
 * returns zero (all tasks identifiers are 1 or greater).
 */
xBase xTaskGetId(xTask task_);

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
xBase xTaskNotificationIsWaiting(xTask task_);

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
Base_t xTaskNotifyGive(xTask task_, xBase notificationBytes_, const char *notificationValue_);

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
void xTaskChangePeriod(xTask task_, xTicks timerPeriod_);

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
xTicks xTaskGetPeriod(xTask task_);

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
xTimer xTimerCreate(xTicks timerPeriod_);

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
void xTimerDelete(xTimer timer_);

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
void xTimerChangePeriod(xTimer timer_, xTicks timerPeriod_);

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
xTicks xTimerGetPeriod(xTimer timer_);

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
xBase xTimerIsTimerActive(xTimer timer_);

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
xBase xTimerHasTimerExpired(xTimer timer_);

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
 * @brief The xTimerStop() system call will place the timer in the stopped state. Neither xTimerStart() nor
 * xTimerStop() will reset the timer. Timers can only be reset with xTimerReset().
 *
 * @sa xTimerStart()
 * @sa xTimerReset()
 *
 * @param timer_ The timer to be stopped.
 */
void xTimerStop(xTimer timer_);

/**
 * @brief The xSystemHalt() system call will halt HeliOS.
 *
 * The xSystemHalt() system call will halt HeliOS. Once xSystemHalt() is called,
 * the system must be reset.
 *
 */
void xSystemHalt(void);


#if defined(POSIX_ARCH_OTHER)
void __MemoryClear__(void);
void __SysStateClear__(void);
void __TimerStateClear__(void);
void __TaskStateClear__(void);
void __MemoryRegionDumpKernel__(void);
void __MemoryRegionDumpHeap__(void);
#endif

/* In the event HeliOS is compiled with a C++ compiler, make the system calls (written in C)
visible to C++. */
#ifdef __cplusplus
}  // extern "C" {
#endif
#endif