# HeliOS Programmer's Guide
This guide contains documentation of HeliOS and its Application Programming Interface (API). This guide a work in progress.
# Multitasking
HeliOS provides two types of multitasking: cooperative and event driven multitasking. Both types can be used together. This section provides an explanation of and how to use each.
## Cooperative Multitasking
The most straightforward type of multitasking in HeliOS is cooperative multitasking. Cooperative multitasking is best used in cases where continuous polling of a sensor or other device is required.

Any task with a state of **running** is scheduled cooperatively by the HeliOS scheduler. The **xTaskStart()** function call sets the state of any task, provided it is not in an **errored** state, to **running**. However, just because a task is in a **running** state, does not guarantee the scheduler will schedule the task for execution. A task in a **running** state will only be scheduled for execution if (1) it is the only task in a **running** state, or (2) it is the task with the least total run-time.

This cooperative multitasking strategy is called **balanced run-time**. This strategy allows the HeliOS scheduler to ensure that all tasks in a **running** state will receive approximately the same amount of run-time within any given period. The total run-time of a task can be obtained using the **xTaskGetInfo()** function call and inspecting the **totalRuntime** member of the **xTaskGetInfoResult** structure.

As a practical example of how the balanced run-time strategy works, imagine a microcontroller with two tasks. Both tasks are in a **running** state and each task's run-time differs from the other. Task "A" has a 5 microsecond run-time and task "B" has a 50 microsecond run-time. Assuming each task's run-time remains constant, Task "A" will be executed 10 times for every one time task "B" executed. This ensures that task "A" receives approximately the same total run-time as task "B" without using priorities or context switching.

If the run-time of either task "A" or "B" were to change during run-time, the HeliOS scheduler would dynamically adjust the schedule to ensure the total run-times of both tasks would remain approximately equal.

It is important to note that the use of function calls like **delay()** must be avoided. If specific timing requirements exists, then event driven multitasking must be used. See the section on **Event Driven Multitasking** for further details.

Below is an Arduino example of how two tasks are added to HeliOS and scheduled cooperatively.
```C
#include <HeliOS_Arduino.h>

/*
 * Create the first task "A"
 */
void taskA(int id_) {
  // DO SOMETHING
}

/*
 * Create the second task "B"
 */
void taskB(int id_) {
  // DO SOMETHING
}

void setup() {
    /*
     * xHeliOSSetup() must be the first function call
     * made to initialize HeliOS and its data structures
     */
    xHeliOSSetup();

    /*
     * Declare and initialize an int to temporarily hold the
     * task id.
     */
    int id = 0;

    /*
     * Pass the task friendly name and function to xTaskAdd()
     * to add the task to HeliOS. xTaskAdd() will return a
     * task id greater than zero if the task is added unsuccessfully.
     */
    id = xTaskAdd("TASKA", &taskA);

    /*
     * Pass the task id of the task to set its state from stopped
     * to running.
     */
    xTaskStart(id);

    /*
     * Pass the task friendly name and function to xTaskAdd()
     * to add the task to HeliOS. xTaskAdd() will return a
     * task id greater than zero if the task is added unsuccessfully.
     */
    id = xTaskAdd("TASKB", &taskB);

    /*
     * Pass the task id of the task to set its state from stopped
     * to running.
     */
    xTaskStart(id);
}

void loop() {
  /*
   * Pass control to the the HeliOS scheduler. xHeliOSLoop() should
   * be the only code inside the microcontroller project's
   * main loop.
   */
  xHeliOSLoop();
}
```

## Event Driven Multitasking
The other type of multitasking supported by HeliOS is event drive multitasking. Event driven works as its name suggests. The HeliOS schedule will only schedule an even driven task for execution when an event is raised. Any tasking in a **waiting** state is considered to be using event driven multitasking. To place a task in the **waiting** state, the **xTaskWait()** function call must be called. The HeliOS scheduler supports two types of events. The first is notification and the second is timer. Both types are covered in more detail below.
*** Notification
A notification occurs when the **xTaskNotify()** function call occurs. All tasks in the **waiting** state will respond to notification events, even tasks configured with a timer. Calling the **xTaskNotify()** function call on tasks in a **stopped** or **running** state has no affect. When calling **xTaskNotify()** a notification value and notification bytes must be specified along with the id of the receiving task. A notification value is a small character buffer (default size is 16 bytes). The notification bytes is the size of the notification value. For example, **xTaskNotify(3, 5, "ABCDE")** would send task 3 a 5 byte notification value of "ABCDE". When task 3 receives the notification, it will use the notification bytes to determine the number of bytes to read from the notification value. It is the task notification bytes and value feature of HeliOS that allows inter-task messaging as part of the wait/notify functionality.

Below is an Arduino example of two tasks. The first task continuously polls a GPIO pin and the second task waits for a notification that the pin his high.

```C
#include <HeliOS_Arduino.h>

/*
 * Create the first task "A"
 */
void taskA(int id_) {
  /*
   * Obtain the task id of task "B" using its friendly name
   * since it should not be assumed that task "B" will always
   * have a task id of 2. Then pass then send a 5 byte notification
   * value with "HELLO" in the character buffer.
   */
  xTaskNotify(xTaskGetId("TASKB"), 5, "HELLO");
}

/*
 * Create the second task "B"
 */
void taskB(int id_) {
  /*
   * Obtain the notification bytes and value from the xTaskGetInfoResult
   * structure by first making a call to the xTaskGetInfo() function call
   * using the task's id stored in id_.
   */
  struct xTaskGetInfoResult* res = xTaskGetInfo(id_);
  /* Because xTaskInfo() can return a null pointer, always check
   * that the structure's pointer is not null before accessing
   * its members.
   */
  if(res) {
    /*
     * res->notificationValue contains the notification value
     * res->notificationBytes contains the notification bytes
     */
  }
  /* Always call xMemFree() to free the managed memory allocated by
   * xTaskGetInfo();
   */
  xMemFree(res);
}

void setup() {
    /*
     * xHeliOSSetup() must be the first function call
     * made to initialize HeliOS and its data structures
     */
    xHeliOSSetup();

    /*
     * Declare and initialize an int to temporarily hold the
     * task id.
     */
    int id = 0;

    /*
     * Pass the task friendly name and function to xTaskAdd()
     * to add the task to HeliOS. xTaskAdd() will return a
     * task id greater than zero if the task is added unsuccessfully.
     */
    id = xTaskAdd("TASKA", &taskA);

    /*
     * Pass the task id of the task to set its state from stopped
     * to running.
     */
    xTaskSart(id);

    /*
     * Pass the task friendly name and function to xTaskAdd()
     * to add the task to HeliOS. xTaskAdd() will return a
     * task id greater than zero if the task is added unsuccessfully.
     */
    id = xTaskAdd("TASKB", &taskB);

    /*
     * Pass the task id of the task to set its state from stopped
     * to waiting.
     */
    xTaskWait(id);
}

void loop() {
  /*
   * Pass control to the the HeliOS scheduler. xHeliOSLoop() should
   * be the only code inside the microcontroller project's
   * main loop.
   */
  xHeliOSLoop();
}
```



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
