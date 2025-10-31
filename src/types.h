#ifndef TYPES_H_

#define TYPES_H_

#include "posix.h"

#include <stdint.h>

#ifndef TASKSTATE_T_

  #define TASKSTATE_T_

  typedef enum TaskState_e {

    TaskStateSuspended,

    TaskStateRunning,

    TaskStateWaiting

  } TaskState_t;

#endif /* ifndef TASKSTATE_T_ */

#ifndef SCHEDULERSTATE_T_

  #define SCHEDULERSTATE_T_

  typedef enum SchedulerState_e {

    SchedulerStateSuspended,

    SchedulerStateRunning

  } SchedulerState_t;

#endif /* ifndef SCHEDULERSTATE_T_ */

#ifndef RETURN_T_

  #define RETURN_T_

  typedef enum Return_e {

    ReturnOK,

    ReturnError

  } Return_t;

#endif /* ifndef RETURN_T_ */

#ifndef TIMERSTATE_T_

  #define TIMERSTATE_T_

  typedef enum TimerState_e {

    TimerStateSuspended,

    TimerStateRunning

  } TimerState_t;

#endif /* ifndef TIMERSTATE_T_ */

#ifndef DEVICESTATE_T_

  #define DEVICESTATE_T_

  typedef enum DeviceState_e {

    DeviceStateSuspended,

    DeviceStateRunning

  } DeviceState_t;

#endif /* ifndef DEVICESTATE_T_ */

#ifndef DEVICEMODE_T_

  #define DEVICEMODE_T_

  typedef enum DeviceMode_e {

    DeviceModeReadOnly,

    DeviceModeWriteOnly,

    DeviceModeReadWrite

  } DeviceMode_t;

#endif /* ifndef DEVICEMODE_T_ */

#ifndef BYTEORDER_T_

  #define BYTEORDER_T_

  typedef enum ByteOrder_e {

    ByteOrderLittleEndian,

    ByteOrderBigEndian

  } ByteOrder_t;

#endif /* ifndef BYTEORDER_T_ */

#ifndef TASKPARM_T_

  #define TASKPARM_T_

  typedef VOID_TYPE TaskParm_t;

#endif /* ifndef TASKPARM_T_ */

#ifndef BASE_T_

  #define BASE_T_

  typedef UINT8_TYPE Base_t;

#endif /* ifndef BASE_T_ */

#ifndef BYTE_T_

  #define BYTE_T_

  typedef UINT8_TYPE Byte_t;

#endif /* ifndef BYTE_T_ */

#ifndef ADDR_T_

  #define ADDR_T_

  typedef VOID_TYPE Addr_t;

#endif /* ifndef ADDR_T_ */

#ifndef SIZE_T_

  #define SIZE_T_

  typedef SIZE_TYPE Size_t;

#endif /* ifndef SIZE_T_ */

#ifndef HALFWORD_T_

  #define HALFWORD_T_

  typedef UINT16_TYPE HalfWord_t;

#endif /* ifndef HALFWORD_T_ */

#ifndef WORD_T_

  #define WORD_T_

  typedef UINT32_TYPE Word_t;

#endif /* ifndef WORD_T_ */

#ifndef TICKS_T_

  #define TICKS_T_

  typedef UINT32_TYPE Ticks_t;

#endif /* ifndef TICKS_T_ */

#ifndef DEVICE_T_

  #define DEVICE_T_

  typedef struct Device_s {

    Base_t valid;

    HalfWord_t uid;

    Byte_t name[CONFIG_DEVICE_NAME_BYTES];

    DeviceState_t state;

    DeviceMode_t mode;

    Word_t bytesWritten;

    Word_t bytesRead;

    Base_t available;

    Return_t (*init)(struct Device_s *device_);

    Return_t (*config)(struct Device_s *device_, Size_t *size_, Addr_t *config_);

    Return_t (*read)(struct Device_s *device_, Size_t *size_, Addr_t **data_);

    Return_t (*write)(struct Device_s *device_, Size_t *size_, Addr_t *data_);

    Return_t (*simple_read)(struct Device_s *device_, Byte_t *data_);

    Return_t (*simple_write)(struct Device_s *device_, Byte_t data_);

    struct Device_s *next;

  } Device_t;

#endif /* ifndef DEVICE_T_ */

#ifndef TASKNOTIFICATION_T_

  #define TASKNOTIFICATION_T_

  typedef struct TaskNotification_s {

    Base_t valid;

    Base_t notificationBytes;

    Byte_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES];

  } TaskNotification_t;

#endif /* ifndef TASKNOTIFICATION_T_ */

#ifndef TASKRUNTIMESTATS_T_

  #define TASKRUNTIMESTATS_T_

  typedef struct TaskRunTimeStats_s {

    Base_t valid;

    Base_t id;

    Ticks_t lastRunTime;

    Ticks_t totalRunTime;

  } TaskRunTimeStats_t;

#endif /* ifndef TASKRUNTIMESTATS_T_ */

#ifndef MEMORYREGIONSTATS_T_

  #define MEMORYREGIONSTATS_T_

  typedef struct MemoryRegionStats_s {

    Base_t valid;

    Word_t largestFreeEntryInBytes;

    Word_t smallestFreeEntryInBytes;

    Word_t numberOfFreeBlocks;

    Word_t availableSpaceInBytes;

    Word_t successfulAllocations;

    Word_t successfulFrees;

    Word_t minimumEverFreeBytesRemaining;

  } MemoryRegionStats_t;

#endif /* ifndef MEMORYREGIONSTATS_T_ */

#ifndef TASKINFO_T_

  #define TASKINFO_T_

  typedef struct TaskInfo_s {

    Base_t valid;

    Base_t id;

    Byte_t name[CONFIG_TASK_NAME_BYTES];

    TaskState_t state;

    Ticks_t lastRunTime;

    Ticks_t totalRunTime;

  } TaskInfo_t;

#endif /* ifndef TASKINFO_T_ */

#ifndef QUEUEMESSAGE_T_

  #define QUEUEMESSAGE_T_

  typedef struct QueueMessage_s {

    Base_t valid;

    Base_t messageBytes;

    Byte_t messageValue[CONFIG_MESSAGE_VALUE_BYTES];

  } QueueMessage_t;

#endif /* ifndef QUEUEMESSAGE_T_ */

#ifndef SYSTEMINFO_T_

  #define SYSTEMINFO_T_

  typedef struct SystemInfo_s {

    Base_t valid;

    Byte_t productName[OS_PRODUCT_NAME_SIZE];

    Base_t majorVersion;

    Base_t minorVersion;

    Base_t patchVersion;

    Base_t numberOfTasks;

    Base_t littleEndian;

  } SystemInfo_t;

#endif /* ifndef SYSTEMINFO_T_ */

#ifndef TASK_T_

  #define TASK_T_

  typedef struct Task_s {

    Base_t valid;

    Base_t id;

    Byte_t name[CONFIG_TASK_NAME_BYTES];

    TaskState_t state;

    TaskParm_t *taskParameter;

    void (*callback)(struct Task_s *task_, TaskParm_t *parm_);

    Base_t notificationBytes;

    Byte_t notificationValue[CONFIG_NOTIFICATION_VALUE_BYTES];

    Ticks_t lastRunTime;

    Ticks_t totalRunTime;

    Ticks_t timerPeriod;

    Ticks_t timerStartTime;

  #if defined(CONFIG_TASK_WD_TIMER_ENABLE)

      Ticks_t wdTimerPeriod;

  #endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */

    struct Task_s *next;

  } Task_t;

#endif /* ifndef TASK_T_ */

#ifndef TASKLIST_T_

  #define TASKLIST_T_

  typedef struct TaskList_s {

    Base_t valid;

    Base_t nextId;

    Base_t length;

    Task_t *head;

  } TaskList_t;

#endif /* ifndef TASKLIST_T_ */

#ifndef DEVICELIST_T_

  #define DEVICELIST_T_

  typedef struct DeviceList_s {

    Base_t valid;

    Base_t length;

    Device_t *head;

  } DeviceList_t;

#endif /* ifndef DEVICELIST_T_ */

#ifndef TIMER_T_

  #define TIMER_T_

  typedef struct Timer_s {

    Base_t valid;

    TimerState_t state;

    Ticks_t timerPeriod;

    Ticks_t timerStartTime;

  } Timer_t;

#endif /* ifndef TIMER_T_ */

#ifndef TIMERLIST_T_

  #define TIMERLIST_T_

  typedef struct TimerList_s {

    Base_t valid;

    Base_t length;

    Timer_t *head;

  } TimerList_t;

#endif /* ifndef TIMERLIST_T_ */

#ifndef FLAGS_T_

  #define FLAGS_T_

  typedef struct Flags_s {

    Base_t valid;

    Base_t running;

    Base_t overflow;

    Base_t memfault;

    Base_t littleend;

  } Flags_t;

#endif /* ifndef FLAGS_T_ */

#ifndef MESSAGE_T_

  #define MESSAGE_T_

  typedef struct Message_s {

    Base_t valid;

    Base_t messageBytes;

    Byte_t messageValue[CONFIG_MESSAGE_VALUE_BYTES];

    struct Message_s *next;

  } Message_t;

#endif /* ifndef MESSAGE_T_ */

#ifndef QUEUE_T_

  #define QUEUE_T_

  typedef struct Queue_s {

    Base_t valid;

    Base_t length;

    Base_t limit;

    Base_t locked;

    Message_t *head;

    Message_t *tail;

  } Queue_t;

#endif /* ifndef QUEUE_T_ */

#ifndef STREAMBUFFER_T_

  #define STREAMBUFFER_T_

  typedef struct StreamBuffer_s {

    Base_t valid;

    Byte_t buffer[CONFIG_STREAM_BUFFER_BYTES];

    HalfWord_t length;

  } StreamBuffer_t;

#endif /* ifndef STREAMBUFFER_T_ */

#ifndef VOLUME_T_

  #define VOLUME_T_

  typedef struct Volume_s {

    Base_t valid;

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

#endif /* ifndef VOLUME_T_ */

#ifndef FILE_T_

  #define FILE_T_

  typedef struct File_s {

    Base_t valid;

    struct Volume_s *volume;

    Word_t firstCluster;

    Word_t currentCluster;

    Word_t fileSize;

    Word_t position;

    Byte_t mode;

    Base_t isOpen;

    Base_t isDirty;

    Byte_t path[0x100];

    Word_t parentDirCluster;

  } File_t;

#endif /* ifndef FILE_T_ */

#ifndef DIRENTRY_T_

  #define DIRENTRY_T_

  typedef struct DirEntry_s {

    Base_t valid;

    Byte_t name[0x100];

    Word_t size;

    Word_t firstCluster;

    Base_t isDirectory;

    Base_t isReadOnly;

    Base_t isHidden;

    Base_t isSystem;

  } DirEntry_t;

#endif /* ifndef DIRENTRY_T_ */

#ifndef DIR_T_

  #define DIR_T_

  typedef struct Dir_s {

    Base_t valid;

    struct Volume_s *volume;

    Word_t currentCluster;

    HalfWord_t entryIndex;

    Base_t isOpen;

  } Dir_t;

#endif /* ifndef DIR_T_ */

#ifndef VOLUMEINFO_T_

  #define VOLUMEINFO_T_

  typedef struct VolumeInfo_s {

    Base_t valid;

    Word_t totalClusters;

    Word_t freeClusters;

    Word_t totalBytes;

    Word_t freeBytes;

    HalfWord_t bytesPerSector;

    Byte_t sectorsPerCluster;

    Word_t bytesPerCluster;

  } VolumeInfo_t;

#endif /* ifndef VOLUMEINFO_T_ */

#if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

  #ifndef BLOCKDEVICECOMMAND_T_

    #define BLOCKDEVICECOMMAND_T_

    typedef struct BlockDeviceCommand_s {

      Byte_t command;

      Word_t blockNumber;

      HalfWord_t blockCount;

      Byte_t transferMode;

    } BlockDeviceCommand_t;

  #endif /* ifndef BLOCKDEVICECOMMAND_T_ */

#endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

#endif /* ifndef TYPES_H_ */