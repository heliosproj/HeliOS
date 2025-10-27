# HeliOS Structure Validation Check Audit

This document lists all functions that take any of the 21 structure types as parameters.
These functions need to have validation checks updated from `__PointerIsNotNull__()` to `__ObjectIsValid__()`.

**Exception:** Functions marked as "Create/Allocate" do NOT need validation checks, as they initialize/create the objects.

**Date Generated:** 2025-10-26

---

## Structure Types Audited (21 total)

1. Device_t
2. TaskNotification_t
3. TaskRunTimeStats_t
4. MemoryRegionStats_t
5. TaskInfo_t
6. QueueMessage_t
7. SystemInfo_t
8. Task_t
9. TaskList_t
10. DeviceList_t
11. Timer_t
12. TimerList_t
13. Flags_t
14. Message_t
15. Queue_t
16. StreamBuffer_t
17. Volume_t
18. File_t
19. DirEntry_t
20. Dir_t
21. VolumeInfo_t

---

## Functions Requiring Validation Check Updates

| Function Name | File | Line | Structure Type(s) | Needs Validation | API Type | Notes |
|--------------|------|------|-------------------|------------------|----------|-------|
| **TASK FUNCTIONS** |
| xTaskCreate | src/task.c | 65 | Task_t** | NO | Public | Creates new Task_t |
| xTaskDelete | src/task.c | 123 | Task_t* | YES | Public | Deletes existing task |
| xTaskGetHandleByName | src/task.c | 176 | Task_t** | NO | Public | Returns handle (output param) |
| xTaskGetHandleById | src/task.c | 209 | Task_t** | NO | Public | Returns handle (output param) |
| xTaskGetAllRunTimeStats | src/task.c | 236 | TaskRunTimeStats_t** | NO | Public | Allocates stats array |
| xTaskGetTaskRunTimeStats | src/task.c | 285 | Task_t*, TaskRunTimeStats_t** | YES (Task_t*) | Public | Task_t needs validation |
| xTaskGetTaskInfo | src/task.c | 349 | Task_t*, TaskInfo_t** | YES (Task_t*) | Public | Task_t needs validation |
| xTaskGetAllTaskInfo | src/task.c | 386 | TaskInfo_t** | NO | Public | Allocates array |
| xTaskGetTaskState | src/task.c | 445 | Task_t* | YES | Public | Reads task state |
| xTaskGetName | src/task.c | 463 | Task_t* | YES | Public | Reads task name |
| xTaskGetId | src/task.c | 496 | Task_t* | YES | Public | Reads task ID |
| xTaskNotifyStateClear | src/task.c | 514 | Task_t* | YES | Public | Modifies task |
| xTaskNotificationIsWaiting | src/task.c | 540 | Task_t* | YES | Public | Checks task state |
| xTaskNotifyGive | src/task.c | 563 | Task_t* | YES | Public | Sends notification |
| xTaskNotifyTake | src/task.c | 590 | Task_t*, TaskNotification_t** | YES (Task_t*) | Public | Task_t needs validation |
| xTaskResume | src/task.c | 637 | Task_t* | YES | Public | Modifies task state |
| xTaskSuspend | src/task.c | 655 | Task_t* | YES | Public | Modifies task state |
| xTaskWait | src/task.c | 673 | Task_t* | YES | Public | Modifies task state |
| xTaskChangePeriod | src/task.c | 691 | Task_t* | YES | Public | Modifies task timer |
| xTaskChangeWDPeriod | src/task.c | 709 | Task_t* | YES | Public | Modifies watchdog |
| xTaskGetPeriod | src/task.c | 729 | Task_t* | YES | Public | Reads task period |
| xTaskResetTimer | src/task.c | 774 | Task_t* | YES | Public | Resets task timer |
| xTaskGetWDPeriod | src/task.c | 979 | Task_t* | YES | Public | Reads watchdog period |
| __TaskListFindTask__ | src/task.c | 747 | Task_t* | YES | Internal | Searches for task |
| vConsoleTask | src/console.c | - | Task_t* | YES | Public | Task callback |
| **QUEUE FUNCTIONS** |
| xQueueCreate | src/queue.c | 45 | Queue_t** | NO | Public | Creates new Queue_t |
| xQueueDelete | src/queue.c | 71 | Queue_t* | YES | Public | Deletes existing queue |
| xQueueGetLength | src/queue.c | 96 | Queue_t* | YES | Public | Reads queue length |
| xQueueIsQueueEmpty | src/queue.c | 125 | Queue_t* | YES | Public | Checks if empty |
| xQueueIsQueueFull | src/queue.c | 160 | Queue_t* | YES | Public | Checks if full |
| xQueueMessagesWaiting | src/queue.c | 196 | Queue_t* | YES | Public | Counts messages |
| xQueueSend | src/queue.c | 231 | Queue_t* | YES | Public | Sends message |
| xQueuePeek | src/queue.c | 292 | Queue_t*, QueueMessage_t** | YES (Queue_t*) | Public | Queue_t needs validation |
| xQueueDropMessage | src/queue.c | 344 | Queue_t* | YES | Public | Removes message |
| xQueueReceive | src/queue.c | 394 | Queue_t*, QueueMessage_t** | YES (Queue_t*) | Public | Queue_t needs validation |
| xQueueLockQueue | src/queue.c | 419 | Queue_t* | YES | Public | Locks queue |
| xQueueUnLockQueue | src/queue.c | 437 | Queue_t* | YES | Public | Unlocks queue |
| __QueuePeek__ | src/queue.c | 309 | Queue_t*, QueueMessage_t** | YES (Queue_t*) | Internal | Queue_t needs validation |
| __QueueDropmessage__ | src/queue.c | 361 | Queue_t* | YES | Internal | Removes message |
| **TIMER FUNCTIONS** |
| xTimerCreate | src/timer.c | 20 | Timer_t** | NO | Public | Creates new Timer_t |
| xTimerDelete | src/timer.c | 44 | Timer_t* | YES | Public | Deletes existing timer |
| xTimerChangePeriod | src/timer.c | 61 | Timer_t* | YES | Public | Modifies timer period |
| xTimerGetPeriod | src/timer.c | 75 | Timer_t* | YES | Public | Reads timer period |
| xTimerIsTimerActive | src/timer.c | 89 | Timer_t* | YES | Public | Checks timer state |
| xTimerHasTimerExpired | src/timer.c | 108 | Timer_t* | YES | Public | Checks if expired |
| xTimerReset | src/timer.c | 131 | Timer_t* | YES | Public | Resets timer |
| xTimerStart | src/timer.c | 145 | Timer_t* | YES | Public | Starts timer |
| xTimerStop | src/timer.c | 164 | Timer_t* | YES | Public | Stops timer |
| **STREAM FUNCTIONS** |
| xStreamCreate | src/streams.c | 25 | StreamBuffer_t** | NO | Public | Creates new StreamBuffer_t |
| xStreamDelete | src/streams.c | 47 | StreamBuffer_t* | YES | Public | Deletes existing stream |
| xStreamSend | src/streams.c | 64 | StreamBuffer_t* | YES | Public | Sends data |
| xStreamReceive | src/streams.c | 83 | StreamBuffer_t* | YES | Public | Receives data |
| xStreamBytesAvailable | src/streams.c | 129 | StreamBuffer_t* | YES | Public | Checks available bytes |
| xStreamReset | src/streams.c | 147 | StreamBuffer_t* | YES | Public | Resets stream |
| xStreamIsEmpty | src/streams.c | 170 | StreamBuffer_t* | YES | Public | Checks if empty |
| xStreamIsFull | src/streams.c | 189 | StreamBuffer_t* | YES | Public | Checks if full |
| **FILESYSTEM FUNCTIONS** |
| xFSMount | src/fs.c | 152 | Volume_t** | NO | Public | Creates new Volume_t |
| xFSUnmount | src/fs.c | 233 | Volume_t* | YES | Public | Unmounts volume |
| xFSGetVolumeInfo | src/fs.c | 255 | Volume_t*, VolumeInfo_t** | YES (Volume_t*) | Public | Volume_t needs validation |
| xFileOpen | src/fs.c | 468 | File_t**, Volume_t* | YES (Volume_t*) | Public | Volume_t needs validation |
| xFileClose | src/fs.c | 608 | File_t* | YES | Public | Closes file |
| xFileRead | src/fs.c | 687 | File_t* | YES | Public | Reads from file |
| xFileWrite | src/fs.c | 780 | File_t* | YES | Public | Writes to file |
| xFileSeek | src/fs.c | 933 | File_t* | YES | Public | Seeks in file |
| xFileTell | src/fs.c | 993 | File_t* | YES | Public | Gets file position |
| xFileGetSize | src/fs.c | 1007 | File_t* | YES | Public | Gets file size |
| xFileSync | src/fs.c | 1021 | File_t* | YES | Public | Syncs file to disk |
| xFileTruncate | src/fs.c | 1105 | File_t* | YES | Public | Truncates file |
| xFileEOF | src/fs.c | 1170 | File_t* | YES | Public | Checks end of file |
| xDirOpen | src/fs.c | 1184 | Dir_t**, Volume_t* | YES (Volume_t*) | Public | Volume_t needs validation |
| xDirClose | src/fs.c | 1242 | Dir_t* | YES | Public | Closes directory |
| xDirRead | src/fs.c | 1263 | Dir_t*, DirEntry_t** | YES (Dir_t*) | Public | Dir_t needs validation |
| xDirRewind | src/fs.c | 1384 | Dir_t* | YES | Public | Rewinds directory |
| xDirMake | src/fs.c | 1398 | Volume_t* | YES | Public | Creates directory |
| xDirRemove | src/fs.c | 1538 | Volume_t* | YES | Public | Removes directory |
| xFileExists | src/fs.c | 1675 | Volume_t* | YES | Public | Checks file existence |
| xFileUnlink | src/fs.c | 1709 | Volume_t* | YES | Public | Deletes file |
| xFileRename | src/fs.c | 1786 | Volume_t* | YES | Public | Renames file |
| xFileGetInfo | src/fs.c | 1885 | Volume_t*, DirEntry_t** | YES (Volume_t*) | Public | Volume_t needs validation |
| __ReadSector__ | src/fs.c | 2413 | Volume_t* | YES | Internal | Reads sector from volume |
| __WriteSector__ | src/fs.c | 2466 | Volume_t* | YES | Internal | Writes sector to volume |
| __ReadCluster__ | src/fs.c | - | Volume_t* | YES | Internal | Reads cluster from volume |
| __GetFATEntry__ | src/fs.c | - | Volume_t* | YES | Internal | Reads FAT entry |
| __SetFATEntry__ | src/fs.c | - | Volume_t* | YES | Internal | Writes FAT entry |
| __FindFreeCluster__ | src/fs.c | - | Volume_t* | YES | Internal | Finds free cluster |
| __FindDirEntry__ | src/fs.c | 2059 | Volume_t* | YES | Internal | Searches directory |
| __FindFileByPath__ | src/fs.c | 2150 | Volume_t* | YES | Internal | Searches file by path |
| __FreeClusters__ | src/fs.c | 2259 | Volume_t* | YES | Internal | Frees cluster chain |
| __CreateDirEntry__ | src/fs.c | 2297 | Volume_t* | YES | Internal | Creates directory entry |
| **DEVICE FUNCTIONS** |
| __RegisterDevice__ | src/device.c | 44 | Device_t* | NO | Internal | Registers new device |
| __DeviceListFind__ | src/device.c | 509 | Device_t** | NO | Internal | Returns handle (output) |
| __DeviceWrite__ | src/device.c | 287 | Device_t* | YES | Internal | Writes to device |
| __DeviceRead__ | src/device.c | 460 | Device_t* | YES | Internal | Reads from device |
| __DeviceConfigDevice__ | src/device.c | 664 | Device_t* | YES | Internal | Configures device |
| **MEMORY FUNCTIONS** |
| xMemGetHeapStats | src/mem.c | 674 | MemoryRegionStats_t** | NO | Public | Allocates stats structure |
| xMemGetKernelStats | src/mem.c | 688 | MemoryRegionStats_t** | NO | Public | Allocates stats structure |
| __MemGetRegionStats__ | src/mem.c | 607 | MemoryRegionStats_t** | NO | Internal | Allocates stats structure |
| **SYSTEM FUNCTIONS** |
| xSystemGetSystemInfo | src/sys.c | 86 | SystemInfo_t** | NO | Public | Allocates info structure |

---

## Summary Statistics

- **Total Functions Found:** 95
- **Functions Requiring Validation Updates:** 74
- **Functions NOT Requiring Validation (Create/Allocate):** 21

### Breakdown by File:

| File | Functions Needing Validation | Create/Allocate Functions |
|------|------------------------------|---------------------------|
| src/task.c | 19 | 5 |
| src/queue.c | 11 | 3 |
| src/timer.c | 8 | 1 |
| src/streams.c | 7 | 1 |
| src/fs.c | 25 | 5 |
| src/device.c | 3 | 2 |
| src/mem.c | 0 | 3 |
| src/sys.c | 0 | 1 |
| src/console.c | 1 | 0 |

### Breakdown by Structure Type:

| Structure Type | Functions Using It |
|----------------|-------------------|
| Task_t | 23 |
| Queue_t | 12 |
| Timer_t | 9 |
| StreamBuffer_t | 8 |
| Volume_t | 17 |
| File_t | 11 |
| Dir_t | 4 |
| Device_t | 6 |
| TaskRunTimeStats_t | 3 |
| TaskInfo_t | 2 |
| TaskNotification_t | 1 |
| QueueMessage_t | 3 |
| DirEntry_t | 3 |
| VolumeInfo_t | 1 |
| MemoryRegionStats_t | 3 |
| SystemInfo_t | 1 |

---

## Implementation Notes

### Current Pattern (to be replaced):
```c
if(__PointerIsNotNull__(object_)) {
    // function body
}
```

### New Pattern:
```c
if(__ObjectIsValid__(object_)) {
    // function body
}
```

### Special Cases:

1. **Functions with multiple structure parameters:** Validate each input parameter separately
   - Example: `xTaskGetTaskRunTimeStats(Task_t *task, TaskRunTimeStats_t **stats)`
   - Validate: `task` (input) - YES
   - Validate: `stats` (output) - NO

2. **Output parameters (double pointers):** Do NOT validate - these are for returning newly created objects
   - Example: `xTaskCreate(Task_t **task, ...)`
   - Validate: `task` - NO (output parameter)

3. **Internal functions:** Also require validation checks for robustness

4. **Delete functions:** Must validate BEFORE freeing memory, then set `valid = INVALID` before calling `xMemFree()`

### Recommended Implementation Order:

1. Start with delete functions (xTaskDelete, xQueueDelete, xTimerDelete, xStreamDelete, etc.)
2. Then update simple accessor functions (xTaskGetId, xQueueGetLength, etc.)
3. Then update modifier functions (xTaskResume, xQueueSend, etc.)
4. Finally update complex multi-parameter functions

---

## Validation Macro Definitions

Located in `src/defines.h`:

```c
#define __ObjectIsValid__(obj_) (__PointerIsNotNull__(obj_) && (VALID == (obj_)->valid))
#define __ObjectIsNotValid__(obj_) (!__ObjectIsValid__(obj_))
```

Where:
- `VALID = 0xAAu` (170 decimal)
- `INVALID = 0x55u` (85 decimal)

---

**End of Audit Report**
