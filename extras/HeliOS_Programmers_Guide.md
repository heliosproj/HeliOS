# HeliOS Programmer's Guide
This guide contains documentation of HeliOS and its Application Programming Interface (API). This is a work in progress.
# Multitasking
## Cooperative Multitasking
## Event Driven Multitasking
# Notification & Timers
## Notification
## Timers
# Critical Blocking
HeliOS implements cricitcal blocking which prevents some functions from being called that may alter the state of HeliOS during critical operations. Critical blocking is in effect while the scheduler is running and prevents functions like xTaskAdd() and xTaskRemove() from altering the state of the HeliOS. Thus, you cannot, for example, remove a task using xTaskRemove() while inside a running task. Functions that do not alter the state of HeliOS may be called during critical blocking. For example, function calls like xTaskGetInfo() may be called during critical blocking as they do not update the state of HeliOS.
# Data Types
## Enumerations
### xTaskState
| Member | Type | Description |
| --- | --- | --- |
| TaskStateErrored | N/A | The task is in an error state and cannot be run - tasks in this state cannot be started or placed in a wait state |
| TaskStateStopped | N/A | The task is stopped and will not be executed until started or placed in a wait state |
| TaskStateRunning | N/A | The task is running and will be executed |
| TaskStateWaiting | N/A | The task is waiting either for a notification or, if set, a timer to elapse |
## Structures
### xTaskGetInfoResult
| Member | Type | Description |
| --- | --- | --- |
| id | int | Task identification number |
| name | char[TASKNAMESIZE] | Friendly name of the task |
| state | enum TaskState | State of the task - see TaskState |
| notifyBytes| int | Number of notification bytes in notification value |
| notifyValue| char[NOTIFYVALUESIZE] | Notification value |
| lastRuntime| unsigned long | Last runtime duration in microseconds |
| totalRuntime | unsigned long | Total runtime duraiton in microseconds |
| timerInterval | unsigned long | The timer interval in microseconds |
| timerStartTime | unsigned long | The time in microseconds the timer was last reset |
#### xHeliOSGetInfoResult
| Member | Type | Description |
| --- | --- | --- |
| tasks | int | The current number of tasks regardless of task state |
| productName | char[PRODUCTNAMESIZE] | The name of the operating system |
| majorVersion | int | The major version number of the product |
| minorVersion | int | The minor version number of the product |
#### xTaskGetListResult
| Member | Type | Description |
| --- | --- | --- |
| id | int | Task identification number |
| name | char[TASKNAMESIZE] | Friendly name of the task |
| state | enum TaskState | State of the task - see TaskState |
| lastRuntime| unsigned long | Last runtime duration in microseconds |
| totalRuntime | unsigned long | Total runtime duraiton in microseconds |
## Function Calls
### About HeliOS API Functions
The HeliOS API is exposed through the 'x' functions - xTaskAdd() for example. These functions are intended to be used by the programmer and are the only functions that are documented in the HeliOS API Documentation. If a function or data structure is not prefixed by an 'x' it should not be accessed by the programmer as these functions are internal to HeliOS and accessing them may cause unpredicable behavior.
### void xHeliOSSetup()
#### Description
This function initializes HeliOS, must be called before calling any HeliOS function and must only be called once. Generally this would be the first line in the setup() function in an Arduino sketch.
#### Parameters
None
#### Return Value
None
### void xHeliOSLoop()
#### Description
This function transfers control to HeliOS and must be called inside a loop. Generally this would be the only line of code in the loop() function in an Arduino sketch.
#### Parameters
None
#### Return Value
None
### struct xHeliOSGetInfoResult* xHeliOSGetInfo()
#### Description
This function returns information about the name of the product, its version and the number of tasks (regardless of state).
#### Parameters
None
#### Return Value
If successful, the function returns a pointer to the structure xHeliOSGetInfoResult. See the details of the structure xHeliOSGetInfoResult for information about its members. If unsuccessful, the function returns a null pointer.
**The memory must be freed by calling xMemFree().**
### void* xMemAlloc(size_t)
#### Description
This function allocates HeliOS managed memory and zeros the memory.
#### Parameters
The size of the managed memory block to be allocated.
#### Return Value
If successful, the function returns a pointer to the block of memory. If unsuccessful, the function returns a null pointer.
**The memory must be freed by calling xMemFree().**
### void xMemFree(void*)
#### Description
This function frees HeliOS managed memory allocated by xMemAlloc().
#### Parameters
The pointer of the managed memory to be freed.
#### Return Value
None
### int xMemGetUsed()
#### Description
This function returns the amount of managed memory (in bytes) currently allocated. This function is helpful in identifying potential memory leaks.
#### Parameters
None
#### Return Value
The number of bytes of managed memory currently allocated. If no managed memory is currently allocated, the function returns zero.
### int xMemGetSize(void*)
#### Description
This function returns the amount of managed memory (in bytes) currently allocated for the given pointer.
#### Parameters
The pointer of the managed memory.
#### Return Value
The number of bytes of managed memory currently allocated for the given pointer. If no managed memory is currently allocated for the given pointer, the function returns zero.
### int xTaskAdd(const char*, void (*)(int))
#### Description
This function adds a task to be scheduled and executed by HeliOS. All tasks are added in a stopped state. Calling this function while HeliOS is critical blocking will have no affect.
#### Parameters
A pointer to a char array containing the friendly name of the task. A pointer to the callback function that will be executed by HeliOS.
#### Return Value
If successful, the id of the newly added task is returned. The first task added always has an id of one (1). If either of the parameters are null or the function is called during critical blocking, the function returns an id of zero (0).
### void xTaskRemove(int)
#### Description
This function removes a task from HeliOS and it will no longer be scheduled and executed. This is different than stopping a task. When a task is stopped, the task still exists within HeliOS - it is just not scheduled for execution. Tasks that are removed cannot be started or stopped once removed. Calling this function while HeliOS is critical blocking will have no affect.
#### Parameters
The id of the task to be removed.
#### Return Value
None
### void xTaskClear()
#### Description
This function removes all tasks from HeliOS. Calling this function while HeliOS is critical blocking will have no affect.
#### Parameters
None
#### Return Value
None
### void xTaskStart(int)
#### Description
This function sets the state of a task to running. Any task in a stopped or waiting state can be set to running.
#### Parameters
The id of the task to be started.
#### Return Value
None
### void xTaskStop(int)
#### Description
This function sets the state of a task to stopped. Any task in a running or waiting state can be set to stopped.
#### Parameters
The id of the task to be stopped.
#### Return Value
None
### void xTaskWait(int)
#### Description
This function sets the state of a task to waiting. A waiting task will not execute without the help of a notification or timer event. Any task in a stopped or running state can be set to waiting.
#### Parameters
The id of the task to wait.
#### Return Value
None
### int xTaskGetId(const char*)
### void xTaskNotify(int, int, char*)
### void xTaskNotifyClear(int)
### struct xTaskGetInfoResult* xTaskGetInfo(int)
### int TaskListSeek(int)
### struct xTaskGetListResult* xTaskGetList(int*)
### void xTaskSetTimer(int, int)
### void xTaskResetTimer(int)