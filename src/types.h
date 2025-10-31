/*UNCRUSTIFY-OFF*/


/**
 * @file types.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Type definitions and aliases
 * @details
 * Defines portable type aliases for integers, addresses, and sizes used throughout the kernel to ensure cross-platform compatibility.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */



/*UNCRUSTIFY-ON*/


#ifndef TYPES_H_


  #define TYPES_H_


  #include "posix.h"


  #include <stdint.h>


  #ifndef TASKSTATE_T_


    #define TASKSTATE_T_


    /**
     * @brief Task execution state enumeration
     * @details Defines the possible states a task can be in during its
     * lifecycle in the scheduler.
     */
    typedef enum TaskState_e {


      TaskStateSuspended, /**< Task is suspended and will not be scheduled for
                           * execution */
      TaskStateRunning, /**< Task is active and will be scheduled for execution
                         */
      TaskStateWaiting /**< Task is waiting for a condition or event before
                        * execution */
    } TaskState_t;


  #endif /* ifndef TASKSTATE_T_ */


  #ifndef SCHEDULERSTATE_T_


    #define SCHEDULERSTATE_T_


    /**
     * @brief Scheduler state enumeration
     * @details Defines the operational state of the task scheduler.
     */
    typedef enum SchedulerState_e {


      SchedulerStateSuspended, /**< Scheduler is suspended and not processing
                                * tasks */
      SchedulerStateRunning /**< Scheduler is active and processing tasks */
    } SchedulerState_t;


  #endif /* ifndef SCHEDULERSTATE_T_ */


  #ifndef RETURN_T_


    #define RETURN_T_


    /**
     * @brief Function return status enumeration
     * @details Standard return codes used throughout HeliOS to indicate success
     * or failure of operations.
     */
    typedef enum Return_e {


      ReturnOK, /**< Operation completed successfully */
      ReturnError /**< Operation failed due to an error */
    } Return_t;


  #endif /* ifndef RETURN_T_ */


  #ifndef TIMERSTATE_T_


    #define TIMERSTATE_T_


    /**
     * @brief Timer state enumeration
     * @details Defines the operational state of a software timer.
     */
    typedef enum TimerState_e {


      TimerStateSuspended, /**< Timer is stopped and not counting */
      TimerStateRunning /**< Timer is active and counting system ticks */
    } TimerState_t;


  #endif /* ifndef TIMERSTATE_T_ */


  #ifndef DEVICESTATE_T_


    #define DEVICESTATE_T_


    /**
     * @brief Device state enumeration
     * @details Defines the operational state of a device driver.
     */
    typedef enum DeviceState_e {


      DeviceStateSuspended, /**< Device is suspended and not available for I/O
                             * operations */
      DeviceStateRunning /**< Device is active and available for I/O operations
                          */
    } DeviceState_t;


  #endif /* ifndef DEVICESTATE_T_ */


  #ifndef DEVICEMODE_T_


    #define DEVICEMODE_T_


    /**
     * @brief Device access mode enumeration
     * @details Defines the permitted I/O operations for a device.
     */
    typedef enum DeviceMode_e {


      DeviceModeReadOnly, /**< Device supports read operations only */
      DeviceModeWriteOnly, /**< Device supports write operations only */
      DeviceModeReadWrite /**< Device supports both read and write operations */
    } DeviceMode_t;


  #endif /* ifndef DEVICEMODE_T_ */


  #ifndef BYTEORDER_T_


    #define BYTEORDER_T_


    /**
     * @brief Byte order enumeration
     * @details Defines the endianness of multi-byte data storage and
     * transmission.
     */
    typedef enum ByteOrder_e {


      ByteOrderLittleEndian, /**< Least significant byte stored at lowest memory
                              * address */
      ByteOrderBigEndian /**< Most significant byte stored at lowest memory
                          * address */
    } ByteOrder_t;


  #endif /* ifndef BYTEORDER_T_ */


  #ifndef TASKPARM_T_


    #define TASKPARM_T_


    /**
     * @brief Task parameter type
     * @details Type alias for task callback function parameters, allowing any
     * data type to be passed.
     */
    typedef VOID_TYPE TaskParm_t;


  #endif /* ifndef TASKPARM_T_ */


  #ifndef BASE_T_


    #define BASE_T_


    /**
     * @brief Base integer type
     * @details 8-bit unsigned integer type used for flags, booleans, and small
     * counters.
     */
    typedef UINT8_TYPE Base_t;


  #endif /* ifndef BASE_T_ */


  #ifndef BYTE_T_


    #define BYTE_T_


    /**
     * @brief Byte type
     * @details 8-bit unsigned integer type representing a single byte of data.
     */
    typedef UINT8_TYPE Byte_t;


  #endif /* ifndef BYTE_T_ */


  #ifndef ADDR_T_


    #define ADDR_T_


    /**
     * @brief Address type
     * @details Generic pointer type for memory addresses and data buffers.
     */
    typedef VOID_TYPE Addr_t;


  #endif /* ifndef ADDR_T_ */


  #ifndef SIZE_T_


    #define SIZE_T_


    /**
     * @brief Size type
     * @details Platform-specific unsigned integer type for representing sizes
     * and lengths.
     */
    typedef SIZE_TYPE Size_t;


  #endif /* ifndef SIZE_T_ */


  #ifndef HALFWORD_T_


    #define HALFWORD_T_


    /**
     * @brief Half-word type
     * @details 16-bit unsigned integer type for medium-range values and
     * hardware registers.
     */
    typedef UINT16_TYPE HalfWord_t;


  #endif /* ifndef HALFWORD_T_ */


  #ifndef WORD_T_


    #define WORD_T_


    /**
     * @brief Word type
     * @details 32-bit unsigned integer type for large values, addresses, and
     * counters.
     */
    typedef UINT32_TYPE Word_t;


  #endif /* ifndef WORD_T_ */


  #ifndef TICKS_T_


    #define TICKS_T_


    /**
     * @brief System ticks type
     * @details 32-bit unsigned integer type for system tick counts and timing
     * values.
     */
    typedef UINT32_TYPE Ticks_t;


  #endif /* ifndef TICKS_T_ */


  #ifndef DEVICE_T_


    #define DEVICE_T_


    /**
     * @brief Device driver structure
     * @details Represents a device driver with function pointers for device
     * operations and metadata. Devices are managed in a linked list by the
     * device manager.
     */
    typedef struct Device_s {


      Base_t valid; /**< Validity flag indicating if structure is initialized */
      HalfWord_t uid; /**< Unique identifier for the device */
      Byte_t name[CONFIG_DEVICE_NAME_BYTES]; /**< Human-readable device name */
      DeviceState_t state; /**< Current operational state of the device */
      DeviceMode_t mode; /**< Access mode (read-only, write-only, read-write) */
      Word_t bytesWritten; /**< Total bytes written to the device */
      Word_t bytesRead; /**< Total bytes read from the device */
      Base_t available; /**< Flag indicating if device is ready for I/O */

      Return_t (*init)(struct Device_s *device_); /**< Initialization function
                                                   * pointer */
      Return_t (*config)(struct Device_s *device_, Size_t *size_, Addr_t *config_); /**<
                                                                                     *
                                                                                     * Configuration
                                                                                     * function
                                                                                     * pointer
                                                                                     */
      Return_t (*read)(struct Device_s *device_, Size_t *size_, Addr_t **data_); /**<
                                                                                  *
                                                                                  * Read
                                                                                  * function
                                                                                  * pointer
                                                                                  */
      Return_t (*write)(struct Device_s *device_, Size_t *size_, Addr_t *data_); /**<
                                                                                  *
                                                                                  * Write
                                                                                  * function
                                                                                  * pointer
                                                                                  */
      Return_t (*simple_read)(struct Device_s *device_, Byte_t *data_); /**<
                                                                         * Simple
                                                                         * single-byte
                                                                         * read
                                                                         * function
                                                                         * pointer
                                                                         */
      Return_t (*simple_write)(struct Device_s *device_, Byte_t data_); /**<
                                                                         * Simple
                                                                         * single-byte
                                                                         * write
                                                                         * function
                                                                         * pointer
                                                                         */

      struct Device_s *next; /**< Pointer to next device in linked list */
    } Device_t;


  #endif /* ifndef DEVICE_T_ */


  #ifndef TASKNOTIFICATION_T_


    #define TASKNOTIFICATION_T_


    /**
     * @brief Task notification structure
     * @details Contains notification data passed between tasks for inter-task
     * communication.
     */
    typedef struct TaskNotification_s {


      Base_t valid; /**< Validity flag indicating if notification is valid */
      Base_t notificationBytes; /**< Number of bytes in the notification value
                                 */
      Byte_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES]; /**<
                                                                  * Notification
                                                                  * data buffer
                                                                  */
    } TaskNotification_t;


  #endif /* ifndef TASKNOTIFICATION_T_ */


  #ifndef TASKRUNTIMESTATS_T_


    #define TASKRUNTIMESTATS_T_


    /**
     * @brief Task runtime statistics structure
     * @details Contains timing information for task execution profiling and
     * performance analysis.
     */
    typedef struct TaskRunTimeStats_s {


      Base_t valid; /**< Validity flag indicating if statistics are valid */
      Base_t id; /**< Task identifier */
      Ticks_t lastRunTime; /**< System ticks at last execution */
      Ticks_t totalRunTime; /**< Total accumulated execution time in ticks */
    } TaskRunTimeStats_t;


  #endif /* ifndef TASKRUNTIMESTATS_T_ */


  #ifndef MEMORYREGIONSTATS_T_


    #define MEMORYREGIONSTATS_T_


    /**
     * @brief Memory region statistics structure
     * @details Contains detailed statistics about heap memory usage for
     * debugging and monitoring.
     */
    typedef struct MemoryRegionStats_s {


      Base_t valid; /**< Validity flag indicating if statistics are valid */
      Word_t largestFreeEntryInBytes; /**< Size of largest contiguous free block
                                       * in bytes */
      Word_t smallestFreeEntryInBytes; /**< Size of smallest free block in bytes
                                        */
      Word_t numberOfFreeBlocks; /**< Total number of free memory blocks */
      Word_t availableSpaceInBytes; /**< Total free memory available in bytes */
      Word_t successfulAllocations; /**< Count of successful memory allocations
                                     */
      Word_t successfulFrees; /**< Count of successful memory deallocations */
      Word_t minimumEverFreeBytesRemaining; /**< Low water mark of free memory
                                             */
    } MemoryRegionStats_t;


  #endif /* ifndef MEMORYREGIONSTATS_T_ */


  #ifndef TASKINFO_T_


    #define TASKINFO_T_


    /**
     * @brief Task information structure
     * @details Contains metadata and runtime statistics for a task, used for
     * task querying and monitoring.
     */
    typedef struct TaskInfo_s {


      Base_t valid; /**< Validity flag indicating if information is valid */
      Base_t id; /**< Task identifier */
      Byte_t name[CONFIG_TASK_NAME_BYTES]; /**< Human-readable task name */
      TaskState_t state; /**< Current execution state of the task */
      Ticks_t lastRunTime; /**< System ticks at last execution */
      Ticks_t totalRunTime; /**< Total accumulated execution time in ticks */
    } TaskInfo_t;


  #endif /* ifndef TASKINFO_T_ */


  #ifndef QUEUEMESSAGE_T_


    #define QUEUEMESSAGE_T_


    /**
     * @brief Queue message structure
     * @details Contains message data for inter-task communication through
     * queues.
     */
    typedef struct QueueMessage_s {


      Base_t valid; /**< Validity flag indicating if message is valid */
      Base_t messageBytes; /**< Number of bytes in the message value */
      Byte_t messageValue[CONFIG_MESSAGE_VALUE_BYTES]; /**< Message data buffer
                                                        */
    } QueueMessage_t;


  #endif /* ifndef QUEUEMESSAGE_T_ */


  #ifndef SYSTEMINFO_T_


    #define SYSTEMINFO_T_


    /**
     * @brief System information structure
     * @details Contains operating system metadata including version and
     * configuration details.
     */
    typedef struct SystemInfo_s {


      Base_t valid; /**< Validity flag indicating if information is valid */
      Byte_t productName[OS_PRODUCT_NAME_SIZE]; /**< Operating system product
                                                 * name string */
      Base_t majorVersion; /**< Major version number */
      Base_t minorVersion; /**< Minor version number */
      Base_t patchVersion; /**< Patch version number */
      Base_t numberOfTasks; /**< Current number of registered tasks */
      Base_t littleEndian; /**< Endianness flag (true if little-endian) */
    } SystemInfo_t;


  #endif /* ifndef SYSTEMINFO_T_ */


  #ifndef TASK_T_


    #define TASK_T_


    /**
     * @brief Task control block structure
     * @details Main task structure containing all metadata, state, and timing
     * information for a scheduled task. Tasks are managed in a linked list by
     * the scheduler.
     */
    typedef struct Task_s {


      Base_t valid; /**< Validity flag indicating if task structure is
                     * initialized */
      Base_t id; /**< Unique task identifier */
      Byte_t name[CONFIG_TASK_NAME_BYTES]; /**< Human-readable task name */
      TaskState_t state; /**< Current execution state */
      TaskParm_t *taskParameter; /**< Pointer to task-specific parameters */

      void (*callback)(struct Task_s *task_, TaskParm_t *parm_); /**< Task
                                                                  * callback
                                                                  * function
                                                                  * pointer */

      Base_t notificationBytes; /**< Number of bytes in pending notification */
      Byte_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES]; /**< Pending
                                                                  * notification
                                                                  * data */
      Ticks_t lastRunTime; /**< System ticks at last execution */
      Ticks_t totalRunTime; /**< Total accumulated execution time in ticks */
      Ticks_t timerPeriod; /**< Task timer period for periodic execution */
      Ticks_t timerStartTime; /**< System ticks when timer was started */

    #if defined(CONFIG_TASK_WD_TIMER_ENABLE)


        Ticks_t wdTimerPeriod; /**< Watchdog timer period for task monitoring */

    #endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */


      struct Task_s *next; /**< Pointer to next task in linked list */
    } Task_t;


  #endif /* ifndef TASK_T_ */


  #ifndef TASKLIST_T_


    #define TASKLIST_T_


    /**
     * @brief Task list structure
     * @details Manages the linked list of all tasks in the system with metadata
     * for task ID allocation.
     */
    typedef struct TaskList_s {


      Base_t valid; /**< Validity flag indicating if list is initialized */
      Base_t nextId; /**< Next available task identifier */
      Base_t length; /**< Number of tasks in the list */
      Task_t *head; /**< Pointer to first task in linked list */
    } TaskList_t;


  #endif /* ifndef TASKLIST_T_ */


  #ifndef DEVICELIST_T_


    #define DEVICELIST_T_


    /**
     * @brief Device list structure
     * @details Manages the linked list of all registered device drivers in the
     * system.
     */
    typedef struct DeviceList_s {


      Base_t valid; /**< Validity flag indicating if list is initialized */
      Base_t length; /**< Number of devices in the list */
      Device_t *head; /**< Pointer to first device in linked list */
    } DeviceList_t;


  #endif /* ifndef DEVICELIST_T_ */


  #ifndef TIMER_T_


    #define TIMER_T_


    /**
     * @brief Software timer structure
     * @details Contains state and timing information for a software timer used
     * for periodic events.
     */
    typedef struct Timer_s {


      Base_t valid; /**< Validity flag indicating if timer is initialized */
      TimerState_t state; /**< Current operational state */
      Ticks_t timerPeriod; /**< Timer period in system ticks */
      Ticks_t timerStartTime; /**< System ticks when timer was started */
    } Timer_t;


  #endif /* ifndef TIMER_T_ */


  #ifndef TIMERLIST_T_


    #define TIMERLIST_T_


    /**
     * @brief Timer list structure
     * @details Manages the linked list of all software timers in the system.
     */
    typedef struct TimerList_s {


      Base_t valid; /**< Validity flag indicating if list is initialized */
      Base_t length; /**< Number of timers in the list */
      Timer_t *head; /**< Pointer to first timer in linked list */
    } TimerList_t;


  #endif /* ifndef TIMERLIST_T_ */


  #ifndef FLAGS_T_


    #define FLAGS_T_


    /**
     * @brief System flags structure
     * @details Contains global system state flags for the kernel runtime.
     */
    typedef struct Flags_s {


      Base_t valid; /**< Validity flag indicating if structure is initialized */
      Base_t running; /**< Flag indicating if scheduler is running */
      Base_t overflow; /**< Flag indicating if system tick overflow occurred */
      Base_t memfault; /**< Flag indicating if memory fault was detected */
      Base_t littleend; /**< Flag indicating if system uses little-endian byte
                         * order */
    } Flags_t;


  #endif /* ifndef FLAGS_T_ */


  #ifndef MESSAGE_T_


    #define MESSAGE_T_


    /**
     * @brief Message structure
     * @details Internal message node used in queue implementation for
     * inter-task communication.
     */
    typedef struct Message_s {


      Base_t valid; /**< Validity flag indicating if message is valid */
      Base_t messageBytes; /**< Number of bytes in the message value */
      Byte_t messageValue[CONFIG_MESSAGE_VALUE_BYTES]; /**< Message data buffer
                                                        */
      struct Message_s *next; /**< Pointer to next message in queue */
    } Message_t;


  #endif /* ifndef MESSAGE_T_ */


  #ifndef QUEUE_T_


    #define QUEUE_T_


    /**
     * @brief Queue structure
     * @details Manages a FIFO message queue for inter-task communication with
     * capacity limiting and locking.
     */
    typedef struct Queue_s {


      Base_t valid; /**< Validity flag indicating if queue is initialized */
      Base_t length; /**< Current number of messages in queue */
      Base_t limit; /**< Maximum allowed messages in queue */
      Base_t locked; /**< Lock flag to prevent concurrent access */
      Message_t *head; /**< Pointer to first message in queue */
      Message_t *tail; /**< Pointer to last message in queue */
    } Queue_t;


  #endif /* ifndef QUEUE_T_ */


  #ifndef STREAMBUFFER_T_


    #define STREAMBUFFER_T_


    /**
     * @brief Stream buffer structure
     * @details Fixed-size buffer for streaming data operations with length
     * tracking.
     */
    typedef struct StreamBuffer_s {


      Base_t valid; /**< Validity flag indicating if buffer is initialized */
      Byte_t buffer[CONFIG_STREAM_BUFFER_BYTES]; /**< Data buffer array */
      HalfWord_t length; /**< Current number of bytes in buffer */
    } StreamBuffer_t;


  #endif /* ifndef STREAMBUFFER_T_ */


  #ifndef VOLUME_T_


    #define VOLUME_T_


    /**
     * @brief FAT filesystem volume structure
     * @details Contains FAT32 filesystem metadata for a mounted volume
     * including layout and configuration.
     */
    typedef struct Volume_s {


      Base_t valid; /**< Validity flag indicating if volume structure is
                     * initialized */
      HalfWord_t blockDeviceUID; /**< Unique identifier of underlying block
                                  * device */
      Word_t fatStartSector; /**< Sector number where FAT begins */
      Word_t dataStartSector; /**< Sector number where data area begins */
      Word_t rootDirCluster; /**< Cluster number of root directory */
      Byte_t sectorsPerCluster; /**< Number of sectors in each cluster */
      HalfWord_t bytesPerSector; /**< Size of each sector in bytes */
      HalfWord_t reservedSectors; /**< Number of reserved sectors before FAT */
      Byte_t numFATs; /**< Number of FAT copies (typically 2) */
      Word_t sectorsPerFAT; /**< Number of sectors per FAT table */
      Base_t mounted; /**< Flag indicating if volume is currently mounted */
    } Volume_t;


  #endif /* ifndef VOLUME_T_ */


  #ifndef FILE_T_


    #define FILE_T_


    /**
     * @brief File handle structure
     * @details Represents an open file with position tracking and metadata for
     * FAT filesystem operations.
     */
    typedef struct File_s {


      Base_t valid; /**< Validity flag indicating if file handle is initialized
                     */
      struct Volume_s *volume; /**< Pointer to the volume containing this file
                                */
      Word_t firstCluster; /**< Starting cluster number of file data */
      Word_t currentCluster; /**< Current cluster being accessed */
      Word_t fileSize; /**< Total size of file in bytes */
      Word_t position; /**< Current read/write position in file */
      Byte_t mode; /**< File access mode (read, write, append, etc.) */
      Base_t isOpen; /**< Flag indicating if file is currently open */
      Base_t isDirty; /**< Flag indicating if file has been modified */
      Byte_t path[0x100]; /**< Full path of the file */
      Word_t parentDirCluster; /**< Cluster number of parent directory */
    } File_t;


  #endif /* ifndef FILE_T_ */


  #ifndef DIRENTRY_T_


    #define DIRENTRY_T_


    /**
     * @brief Directory entry structure
     * @details Contains metadata for a single directory entry including
     * attributes and location.
     */
    typedef struct DirEntry_s {


      Base_t valid; /**< Validity flag indicating if entry is valid */
      Byte_t name[0x100]; /**< Entry name (file or directory) */
      Word_t size; /**< Size in bytes (0 for directories) */
      Word_t firstCluster; /**< Starting cluster number */
      Base_t isDirectory; /**< Flag indicating if entry is a directory */
      Base_t isReadOnly; /**< Read-only attribute flag */
      Base_t isHidden; /**< Hidden attribute flag */
      Base_t isSystem; /**< System attribute flag */
    } DirEntry_t;


  #endif /* ifndef DIRENTRY_T_ */


  #ifndef DIR_T_


    #define DIR_T_


    /**
     * @brief Directory handle structure
     * @details Represents an open directory for iteration over directory
     * entries in the FAT filesystem.
     */
    typedef struct Dir_s {


      Base_t valid; /**< Validity flag indicating if directory handle is
                     * initialized */
      struct Volume_s *volume; /**< Pointer to the volume containing this
                                * directory */
      Word_t currentCluster; /**< Current cluster being read */
      HalfWord_t entryIndex; /**< Current entry index within cluster */
      Base_t isOpen; /**< Flag indicating if directory is currently open */
    } Dir_t;


  #endif /* ifndef DIR_T_ */


  #ifndef VOLUMEINFO_T_


    #define VOLUMEINFO_T_


    /**
     * @brief Volume information structure
     * @details Contains statistics and configuration information about a FAT
     * filesystem volume.
     */
    typedef struct VolumeInfo_s {


      Base_t valid; /**< Validity flag indicating if information is valid */
      Word_t totalClusters; /**< Total number of clusters on volume */
      Word_t freeClusters; /**< Number of free clusters available */
      Word_t totalBytes; /**< Total storage capacity in bytes */
      Word_t freeBytes; /**< Free storage space in bytes */
      HalfWord_t bytesPerSector; /**< Size of each sector in bytes */
      Byte_t sectorsPerCluster; /**< Number of sectors per cluster */
      Word_t bytesPerCluster; /**< Calculated size of each cluster in bytes */
    } VolumeInfo_t;


  #endif /* ifndef VOLUMEINFO_T_ */


  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)


    #ifndef BLOCKDEVICECOMMAND_T_


      #define BLOCKDEVICECOMMAND_T_


      /**
       * @brief Block device command structure
       * @details Command structure for block device I/O operations specifying
       * block range and transfer mode.
       */
      typedef struct BlockDeviceCommand_s {


        Byte_t command; /**< Command type identifier */
        Word_t blockNumber; /**< Starting block number for the operation */
        HalfWord_t blockCount; /**< Number of blocks to transfer */
        Byte_t transferMode; /**< Transfer mode (blocking, DMA, interrupt, etc.)
                              */
      } BlockDeviceCommand_t;


    #endif /* ifndef BLOCKDEVICECOMMAND_T_ */


  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */


#endif /* ifndef TYPES_H_ */