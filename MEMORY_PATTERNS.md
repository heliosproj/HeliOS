# HeliOS Memory Usage Analysis

## Memory Architecture Overview

HeliOS implements **two separate memory regions**:
- **Kernel Heap**: Used for internal OS data structures (tasks, queues, streams, devices, filesystem)
- **User Heap**: Used for data returned to applications

---

## Memory Allocation APIs

### Public Memory APIs

| API | Memory Type | Purpose | File Location |
|-----|-------------|---------|---------------|
| `xMemAlloc()` | **User Heap** | Allocate memory for user applications | src/mem.c:223 |
| `xMemFree()` | **User Heap** | Free user-allocated memory | src/mem.c:243 |
| `xMemFreeAll()` | **User Heap** | Reset entire user heap | src/mem.c:263 |
| `xMemGetUsed()` | **User Heap** | Get bytes used in user heap | src/mem.c:276 |
| `xMemGetSize()` | **User Heap** | Get size of user allocation | src/mem.c:316 |
| `xMemGetHeapStats()` | **User Heap** | Returns stats structure in user heap | src/mem.c:993 |
| `xMemGetKernelStats()` | **User Heap** | Returns stats structure in user heap | src/mem.c:1012 |

### Private/Internal Memory APIs

| API | Memory Type | Purpose | File Location |
|-----|-------------|---------|---------------|
| `__KernelAllocateMemory__()` | **Kernel Heap** | Internal kernel allocations | src/mem.c:782 |
| `__KernelFreeMemory__()` | **Kernel Heap** | Free kernel memory | src/mem.c:805 |
| `__HeapAllocateMemory__()` | **User Heap** | Internal wrapper for user heap | src/mem.c:844 |
| `__HeapFreeMemory__()` | **User Heap** | Internal wrapper for user heap | src/mem.c:867 |
| `__MemoryRegionCheckKernel__()` | N/A | Validate kernel memory pointer | src/mem.c:824 |
| `__MemoryRegionCheckHeap__()` | N/A | Validate user heap pointer | src/mem.c:886 |

---

## System Control APIs

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xSystemAssert()` | No | N/A | Assertion handling | src/sys.c |
| `xSystemInit()` | No | N/A | Initialize OS (initializes both memory regions) | src/sys.c |
| `xSystemHalt()` | No | N/A | Halt system | src/sys.c |
| `xSystemGetSystemInfo()` | **Yes** | **User Heap** | Returns SystemInfo_t structure | src/sys.c:87 |

---

## Task Management APIs

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xTaskCreate()` | **Yes** | **Kernel Heap** | Allocates Task_t and TaskList_t structures | src/task.c:63-66 |
| `xTaskDelete()` | **Yes** | **Kernel Heap** | Frees Task_t structure | src/task.c:113 |
| `xTaskGetHandleByName()` | No | N/A | Lookup task by name | src/task.c:166 |
| `xTaskGetHandleById()` | No | N/A | Lookup task by ID | src/task.c:199 |
| `xTaskGetAllRunTimeStats()` | **Yes** | **User Heap** | Returns TaskRunTimeStats_t array | src/task.c:244 |
| `xTaskGetTaskRunTimeStats()` | **Yes** | **User Heap** | Returns single TaskRunTimeStats_t | src/task.c:280 |
| `xTaskGetNumberOfTasks()` | No | N/A | Return count | src/task.c:303 |
| `xTaskGetTaskInfo()` | **Yes** | **User Heap** | Returns TaskInfo_t structure | src/task.c:345 |
| `xTaskGetAllTaskInfo()` | **Yes** | **User Heap** | Returns TaskInfo_t array | src/task.c:395 |
| `xTaskGetTaskState()` | No | N/A | Return state enum | src/task.c:429 |
| `xTaskGetName()` | **Yes** | **User Heap** | Returns task name copy | src/task.c:452 |
| `xTaskGetId()` | No | N/A | Return task ID | src/task.c:480 |
| `xTaskNotifyStateClear()` | No | N/A | Clear notification | src/task.c:498 |
| `xTaskNotificationIsWaiting()` | No | N/A | Check notification status | src/task.c:524 |
| `xTaskNotifyGive()` | No | N/A | Send notification (stored in Task_t) | src/task.c:547 |
| `xTaskNotifyTake()` | **Yes** | **User Heap** | Returns TaskNotification_t structure | src/task.c:580 |
| `xTaskResume()` | No | N/A | Resume task | src/task.c:621 |
| `xTaskSuspend()` | No | N/A | Suspend task | src/task.c:639 |
| `xTaskWait()` | No | N/A | Set task to waiting | src/task.c:657 |
| `xTaskChangePeriod()` | No | N/A | Change timer period | src/task.c:675 |
| `xTaskChangeWDPeriod()` | No | N/A | Change watchdog period | src/task.c:693 |
| `xTaskGetPeriod()` | No | N/A | Get timer period | src/task.c:713 |
| `xTaskResetTimer()` | No | N/A | Reset task timer | src/task.c:762 |
| `xTaskStartScheduler()` | No | N/A | Start scheduler loop | src/task.c:780 |
| `xTaskResumeAll()` | No | N/A | Resume scheduler | src/task.c:871 |
| `xTaskSuspendAll()` | No | N/A | Suspend scheduler | src/task.c:885 |
| `xTaskGetSchedulerState()` | No | N/A | Get scheduler state | src/task.c:899 |
| `xTaskGetWDPeriod()` | No | N/A | Get watchdog period | src/task.c:913 |

---

## Queue APIs

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xQueueCreate()` | **Yes** | **Kernel Heap** | Allocates Queue_t structure | src/queue.c:49 |
| `xQueueDelete()` | **Yes** | **Kernel Heap** | Frees Queue_t and all messages | src/queue.c:71 |
| `xQueueGetLength()` | No | N/A | Return queue length | src/queue.c:99 |
| `xQueueIsQueueEmpty()` | No | N/A | Check if empty | src/queue.c:132 |
| `xQueueIsQueueFull()` | No | N/A | Check if full | src/queue.c:171 |
| `xQueueMessagesWaiting()` | No | N/A | Check for messages | src/queue.c:211 |
| `xQueueSend()` | **Yes** | **Kernel Heap** | Allocates Message_t structure | src/queue.c:266 |
| `xQueuePeek()` | **Yes** | **User Heap** | Returns QueueMessage_t copy | src/queue.c:339 |
| `xQueueDropMessage()` | **Yes** | **Kernel Heap** | Frees Message_t structure | src/queue.c:372 |
| `xQueueReceive()` | **Yes** | **Both** | Returns QueueMessage_t (user heap), frees Message_t (kernel) | src/queue.c:426 |
| `xQueueLockQueue()` | No | N/A | Lock queue | src/queue.c:455 |
| `xQueueUnLockQueue()` | No | N/A | Unlock queue | src/queue.c:477 |

---

## Stream Buffer APIs

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xStreamCreate()` | **Yes** | **Kernel Heap** | Allocates StreamBuffer_t structure | src/streams.c:29 |
| `xStreamDelete()` | **Yes** | **Kernel Heap** | Frees StreamBuffer_t structure | src/streams.c:47 |
| `xStreamSend()` | No | N/A | Store byte in stream buffer | src/streams.c:68 |
| `xStreamReceive()` | **Yes** | **User Heap** | Returns copy of stream data | src/streams.c:97 |
| `xStreamBytesAvailable()` | No | N/A | Return bytes available | src/streams.c:138 |
| `xStreamReset()` | No | N/A | Clear stream | src/streams.c:160 |
| `xStreamIsEmpty()` | No | N/A | Check if empty | src/streams.c:185 |
| `xStreamIsFull()` | No | N/A | Check if full | src/streams.c:208 |

---

## Device I/O APIs

### Public Device APIs

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xDeviceRegisterDevice()` | No | N/A | Register device (calls `__RegisterDevice__`) | src/device.c:26 |
| `xDeviceIsAvailable()` | No | N/A | Check device availability | src/device.c:137 |
| `xDeviceSimpleWrite()` | No | N/A | Write single byte | src/device.c:170 |
| `xDeviceWrite()` | **Yes** | **Kernel Heap (temp)** | Copies user heap data to kernel heap for driver | src/device.c:229 |
| `xDeviceSimpleRead()` | No | N/A | Read single byte | src/device.c:282 |
| `xDeviceRead()` | **Yes** | **Both** | Driver returns kernel heap, copies to user heap | src/device.c:398 |
| `xDeviceInitDevice()` | No | N/A | Initialize device | src/device.c:504 |
| `xDeviceConfigDevice()` | **Yes** | **Kernel Heap (temp)** | Copies config to kernel heap for driver | src/device.c:607 |

### Private Device APIs

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `__RegisterDevice__()` | **Yes** | **Kernel Heap** | Allocates Device_t and DeviceList_t | src/device.c:62, 74 |
| `__DeviceWrite__()` | No | N/A | Kernel-level write (no copy) | src/device.c:245 |
| `__DeviceRead__()` | **Yes** | **Kernel Heap** | Returns kernel memory (caller must free) | src/device.c:423 |
| `__DeviceConfigDevice__()` | No | N/A | Kernel-level config (no copy) | src/device.c:634 |

---

## Filesystem APIs

### Volume Management

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xFSMount()` | **Yes** | **Kernel Heap** | Allocates Volume_t and boot sector buffer | src/fs.c:158 |
| `xFSUnmount()` | **Yes** | **Kernel Heap** | Frees Volume_t structure | src/fs.c:225 |
| `xFSGetVolumeInfo()` | **Yes** | **User Heap** | Returns VolumeInfo_t structure | src/fs.c:248 |
| `xFSFormat()` | **Yes** | **User Heap (temp)** | Uses user heap for boot/FAT sector buffers | src/fs.c:135 |

### File Operations

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xFileOpen()` | **Yes** | **Kernel Heap** | Allocates File_t structure | src/fs.c:440 |
| `xFileClose()` | **Yes** | **Kernel Heap** | Frees File_t structure | src/fs.c:315 |
| `xFileRead()` | **Yes** | **User Heap + Kernel (temp)** | Returns data in user heap, uses kernel for clusters | src/fs.c:337 |
| `xFileWrite()` | **Yes** | **Kernel Heap (temp)** | Uses kernel heap for cluster read-modify-write | src/fs.c:435 |
| `xFileSeek()` | No | N/A | Update file position | src/fs.c:570 |
| `xFileTell()` | No | N/A | Get file position | src/fs.c:632 |
| `xFileGetSize()` | No | N/A | Get file size | src/fs.c:646 |
| `xFileSync()` | No | N/A | Flush (stub) | src/fs.c:660 |
| `xFileTruncate()` | No | N/A | Truncate file | src/fs.c:670 |
| `xFileEOF()` | No | N/A | Check EOF | src/fs.c:740 |

### Directory Operations

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xDirOpen()` | **Yes** | **Kernel Heap** | Allocates Dir_t structure | src/fs.c:918 |
| `xDirClose()` | **Yes** | **Kernel Heap** | Frees Dir_t structure | src/fs.c:788 |
| `xDirRead()` | **Yes** | **User Heap + Kernel (temp)** | Returns DirEntry_t, uses kernel for cluster | src/fs.c:886 |
| `xDirRewind()` | No | N/A | Reset directory position | src/fs.c:936 |
| `xDirMake()` | No | N/A | Stub (not implemented) | src/fs.c:950 |
| `xDirRemove()` | No | N/A | Stub (not implemented) | src/fs.c:970 |

### File/Directory Management

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xFileExists()` | No | N/A | Stub (not implemented) | src/fs.c:990 |
| `xFileUnlink()` | No | N/A | Stub (not implemented) | src/fs.c:1017 |
| `xFileRename()` | No | N/A | Stub (not implemented) | src/fs.c:1037 |
| `xFileGetInfo()` | No | N/A | Stub (not implemented) | src/fs.c:1057 |

### Internal FS Helper Functions

| Function | Uses Memory? | Memory Type | Purpose | File Location |
|----------|--------------|-------------|---------|---------------|
| `__ReadSector__()` | **Yes** | **Kernel Heap** | Returns sector data buffer (caller must free) | src/fs.c:1248 |
| `__WriteSector__()` | **Yes** | **Kernel Heap (temp)** | Uses kernel heap for command structure | src/fs.c:1301 |
| `__ReadCluster__()` | **Yes** | **Kernel Heap** | Returns cluster data buffer (caller must free) | src/fs.c:1373 |
| `__GetFATEntry__()` | **Yes** | **Kernel Heap (temp)** | Uses kernel heap for sector buffer | src/fs.c:1262 |
| `__SetFATEntry__()` | **Yes** | **Kernel Heap (temp)** | Uses kernel heap for sector buffer | src/fs.c:1308 |
| `__FindFreeCluster__()` | **Yes** | **Kernel Heap (temp)** | Uses `__GetFATEntry__()` internally | src/fs.c:1363 |

---

## Timer APIs

| API | Uses Memory? | Memory Type | Purpose | File Location |
|-----|--------------|-------------|---------|---------------|
| `xTimerCreate()` | **Yes** | **Kernel Heap** | Allocates Timer_t structure | src/timer.c:24 |
| `xTimerDelete()` | **Yes** | **Kernel Heap** | Frees Timer_t structure | src/timer.c |
| `xTimerChangePeriod()` | No | N/A | Change timer period | src/timer.c |
| `xTimerGetPeriod()` | No | N/A | Get timer period | src/timer.c |
| `xTimerIsTimerActive()` | No | N/A | Check if active | src/timer.c |
| `xTimerHasTimerExpired()` | No | N/A | Check if expired | src/timer.c |
| `xTimerReset()` | No | N/A | Reset timer | src/timer.c |
| `xTimerStart()` | No | N/A | Start timer | src/timer.c |
| `xTimerStop()` | No | N/A | Stop timer | src/timer.c |

---

## Summary Statistics

### Memory Usage by API Category

| Category | Kernel Heap APIs | User Heap APIs | No Memory APIs | Total |
|----------|------------------|----------------|----------------|-------|
| Memory Management | 4 | 7 | 0 | 11 |
| System Control | 0 | 1 | 3 | 4 |
| Task Management | 2 | 7 | 19 | 28 |
| Queue Management | 4 | 2 | 6 | 12 |
| Stream Buffers | 2 | 1 | 5 | 8 |
| Device I/O | 5 | 2 | 5 | 12 |
| Filesystem | 15 | 4 | 14 | 33 |
| Timers | 2 | 0 | 7 | 9 |
| **Total** | **34** | **24** | **59** | **117** |

### Design Principles

HeliOS uses a **memory protection model** with clear separation between kernel and user memory:

1. **Kernel Heap Usage**
   - OS-owned structures: Task_t, Queue_t, StreamBuffer_t, Timer_t, Device_t, File_t, Dir_t, Volume_t
   - Internal message structures: Message_t (queue messages)
   - Temporary I/O buffers for filesystem operations
   - Never exposed to user applications

2. **User Heap Usage**
   - Application-owned data
   - Return values from info/stats APIs
   - Data read from devices and files
   - Queue and stream data received by applications
   - Must be freed by the application using `xMemFree()`

3. **Copy-on-Boundary Pattern**
   - Data crossing kernel/user boundary is copied
   - Prevents pointer leaks between heaps
   - Ensures proper memory ownership
   - Examples:
     - `xDeviceWrite()`: User data → copied to kernel → passed to driver
     - `xDeviceRead()`: Driver returns kernel buffer → copied to user heap
     - `xQueueReceive()`: Kernel message → copied to user heap

4. **Internal Kernel-to-Kernel APIs**
   - `__DeviceRead__()`, `__DeviceWrite__()`, `__DeviceConfigDevice__()`
   - Used by filesystem for block device access
   - No heap boundary crossing (kernel-only)
   - More efficient than public APIs (no copying)

### Memory Ownership Rules

| Heap Type | Allocated By | Freed By | User Access |
|-----------|--------------|----------|-------------|
| **Kernel Heap** | Internal APIs | OS automatically | No - opaque pointers only |
| **User Heap** | `xMemAlloc()` or info APIs | User calls `xMemFree()` | Yes - full access |

This architecture ensures memory safety and prevents applications from corrupting kernel data structures.
