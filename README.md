<picture>
  <source media="(prefers-color-scheme: dark)" srcset="/extras/HeliOS_OG_Logo_Dark.png">
  <source media="(prefers-color-scheme: light)" srcset="/extras/HeliOS_OG_Logo_Light.png">
  <img alt="HeliOS Logo" src="/extras/HeliOS_OG_Logo_Light.png">
</picture>

[![License: GPL Version 2](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://github.com/heliosproj/HeliOS/blob/master/LICENSE.md) ![GitHub last commit](https://img.shields.io/github/last-commit/heliosproj/HeliOS) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/heliosproj/HeliOS) [![PlatformIO Registry](https://badges.registry.platformio.org/packages/heliosproj/library/HeliOS.svg)](https://registry.platformio.org/libraries/heliosproj/HeliOS) [![arduino-library-badge](https://www.ardu-badge.com/badge/HeliOS.svg?)](https://www.ardu-badge.com/HeliOS) ![GitHub stars](https://img.shields.io/github/stars/heliosproj/HeliOS?style=social) ![GitHub watchers](https://img.shields.io/github/watchers/heliosproj/HeliOS?style=social)
***
# :rocket: Overview
HeliOS is an open source embedded operating system that is free for everyone to use. While called an operating system, HeliOS is a multitasking kernel for use in embedded applications. Its rich, fully documented, API allows the user to control every aspect of the system and access kernel services (syscalls) for task (process) management, scheduler management, inter-process communication, memory management, device management (i.e., device drivers) and more while maintaining a tiny footprint for a broad range of low-power embedded devices. HeliOS is also easily customized to fit the user’s specific needs through a single header file ``/src/config.h``.

HeliOS supports two multitasking models that can be leveraged concurrently within the same application. The first multitasking model is event-driven. When a task is placed in the "waiting" state, the task will only respond to task events. HeliOS supports two types of task events. The first is direct-to-task notifications, which allow one task to send a notification to another task. In this scenario, the HeliOS scheduler will wake the recipient task and schedule it for execution. After the recipient task clears the direct-to-task notification, the recipient task will returning to "waiting" until another notification is received. The second type of task event is timer based. Task timers can be configured to tell HeliOS to schedule the task to run every so many ticks (typically milliseconds), though task timers should not be confused with application timers (or simply timers) as HeliOS supports both.

The second model for multitasking is a conventional cooperative model. In this model, cooperative tasks are always scheduled to run, unless suspended. Additionally, the cooperative model in HeliOS contains a unique scheduler feature that builds on the traditional cooperative model. In most cooperatively scheduled multitasking models, a simple round-robin approach is used (i.e., each task is executed consecutively). However, the HeliOS scheduler uses a “runtime balanced” algorithm for scheduling cooperative tasks. In other words, tasks that consume more runtime are deprioritized (i.e., executed less frequently) in favor of tasks that consume less runtime. This design prevents long running tasks from monopolizing the system’s execution time. Event-driven and cooperatively scheduled tasks run together seamlessly, although event-driven tasks always receive execution priority over cooperatively scheduled tasks.

One important aspect of multitasking in HeliOS is it does not rely on context switching. This reduces the need for the user to manage access to shared resources in a “thread safe” way using mutexes and semaphores. This also eliminates the need for the “port” or portability code required to save the context during a context switch. As a result, the user can focus his or her development effort on their specific application without having to contend with concurrent access to shared resources. Like everything in life, there are drawbacks. While a conventional cooperative model spares the user from contending with concurrent access to shared resources, if a task does not relinquish control to the HeliOS scheduler, it will monopolize all available runtime. This also means that the HeliOS scheduler does not enforce hard-timing (i.e., real-time). The HeliOS scheduler enforces soft-timing so if a waiting task timer has elapsed, the scheduler will prioritize the task but may miss the "deadline".

HeliOS also provides services for three inter-process communication models. The first, as discussed previously, is direct-to-task notifications. Direct-to-task notifications are an efficient communication channel between tasks that prevent a task from consuming runtime when there is nothing for the task to process. The second model is message queues. Message queues can be created at runtime and can be shared among any number of tasks. Queues are highly flexible FIFO communication channels that require very little code to implement. The third model is stream buffers. Stream buffers are very much like message queues with one important difference. While queues operate on multi-byte messages, stream buffers operate similarly on single-byte streams. Finally, while technically not one of HeliOS’s models for inter-process communication, HeliOS supports task parameters that can be leveraged for rudimentary inter-process communication if so desired.

The HeliOS kernel includes built-in memory management that improves the safety margin of dynamically allocated memory. While HeliOS’s dynamic memory allocation allocates “heap” memory, the heap in HeliOS is not a true heap. HeliOS uses a private heap that is implemented as static memory allocated at compile time. HeliOS does not use the standard library ``malloc()`` and ``free()`` functions and it is recommended that the user also avoid those functions in favor of HeliOS’s memory management syscalls. HeliOS also maintains a separate memory region for kernel objects which reduces the risk that memory access, in the user's application, would corrupt critical kernel objects. As of kernel 0.4.0, HeliOS also supports sophisticated memory defragmentation and consistency checking to ensure memory is utilized efficiently and with a high degree of integrity. 

HeliOS also supports a kernel mode device driver model. Device drivers for virtually any feature or peripheral can be easily developed using the provided device driver template. While device drivers are not needed in most applications, when the microcontroller's MMU or MPU is enabled it may not be possible to access memory mapped registers and I/O from the user's code. While implementation of the ARM MMU and MPU in HeliOS is forthcoming, device driver support had to be added to HeliOS first. Information about the device driver system calls can be found in the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf). A device driver template can be found here ``/drivers/template/driver.c`` and here ``/drivers/template/driver.h``.

HeliOS is built to be robust. HeliOS (0.3.0 and later) has undergone static analysis testing using a commercially licensed static analysis tool as well as MISRA C:2012 checks. While HeliOS is NOT certified for nor should be used (in full or in part) in any safety-critical application where a risk to life exists, user’s can be confident they are building their embedded application on a robust embedded operating system.

Lastly, for PlatformIO and Arduino users, HeliOS is easily added to their embedded application. The latest release of HeliOS is available directly through the [PlatformIO Registry](https://registry.platformio.org/libraries/heliosproj/HeliOS) and the [Arduino Library Manager](https://www.arduino.cc/reference/en/libraries/helios/). For users of other embedded platforms and/or tool-chains, simply download the [latest release](https://github.com/heliosproj/HeliOS/releases) of HeliOS from GitHub and add the sources to your project.
***
# :loudspeaker: What's New
The HeliOS 0.4.x series kernel was recently released which supersedes all prior kernel versions. The syscall API and internals have undergone significant development rendering applications built on earlier kernels incompatible with 0.4.x. The key change that will impact compatibility is the introduction of a consistent return type for all syscalls. This provides a better mechanism for error propagation and a consistent interface handling errors.

For example, prior to kernel 0.4.0, a task would be created as follows.

```C
xTask task = xTaskCreate("TASKMAIN", task_main, NULL);

if(task) {
  /* Use the task here. */
}
```
In this example, the user application would only know if an error or exception occurred by checking if "task" was null. In kernel 0.4.0 all syscalls have a standard return type ``xReturn`` that can either be ``ReturnOK`` or ``ReturnError``. See the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf) for more information about ``xReturn``. Thus, in kernel 0.4.0 the same process of creating a task is done as follows.

```C
xTask task;

if(ERROR(xTaskCreate(&task, (const xByte *) "TASKMAIN", task_main, null))) {
  xSystemHalt();
}

/* Use the task here. */
```
In this manner, the application can check all syscalls for success or failure even when a syscall does not modify or set arguments it is passed. For the very latest on what development is occurring, please check out the [HeliOS Trello board](https://trello.com/b/XNKDpuGR/helios). Anyone wanting to contribute to HeliOS should refer to the “Contributing” section.
***
# :computer_mouse: HeliOS Around The Web

* **[HeliOS is a Tiny Embedded OS Designed for Arduino Boards](https://www.cnx-software.com/2020/08/14/helios-is-a-tiny-embedded-os-designed-for-arduino-boards/)**

* **[HeliOS for Arduino](https://linuxhint.com/linux_on_arduino/)**

* **[Newly-Launched "Embedded OS" HeliOS Brings Simple Multitasking to Arduino Microcontrollers](https://www.hackster.io/news/newly-launched-embedded-os-helios-brings-simple-multitasking-to-arduino-microcontrollers-11f6b137b75c)**

* **[New HeliOS, an embedded OS for Arduino Boards](https://iot-industrial-devices.com/new-helios-an-embedded-os-for-arduino-boards/)**

* **[HeliOS is a small and simple embedded operating system for Arduino](https://twitter.com/arduino/status/1293910675312357376)**

* **[Arduino Operating System: Best Options of 2021](https://all3dp.com/2/best-arduino-operating-system/)**

* **[HeliOS is a Tiny Embedded OS Designed for Arduino Boards](https://news.knowledia.com/US/en/articles/helios-is-a-tiny-embedded-os-designed-for-arduino-boards-f35f44fe6c88759fa13d8781ce09ac985b2fdd3a)**
***
# :dart: Getting Started
## Documentation
The HeliOS syscall API is documented in the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf). If you are in need of support, please refer to the "Contributing" section on how to submit an issue.

## Arduino IDE
Using the HeliOS embedded operating system in your Arduino sketch could not be easier. Open the Arduino IDE and use the Library Manager to search for and install HeliOS. The folks at Arduino have documented the steps to install a library [here](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries). Once installed, you can experiment with the example sketches that are included with HeliOS and can be found under File -> Examples -> HeliOS in the Arduino IDE.

## PlatformIO IDE
HeliOS is also available directly through the PlatformIO registry and can be added to your project either from the PlatformIO GUI or CLI. The steps for which are described in the PlatformIO documentation [here](https://docs.platformio.org/en/latest/librarymanager/index.html). Like the Arduino IDE, several examples are included with HeliOS for you to experiment with.

## ARM Cortex-M
If more advanced features are desired, HeliOS also has built-in support for CMSIS on ARM Cortex-M microcontrollers and can be easily integrated into your Keil uVision or vendor IDE project by:

1. Downloading the current release [here](https://github.com/heliosproj/HeliOS/releases) and unpacking the ZIP file into your project’s source directory
2. Downloading the CMSIS headers and vendor’s HAL/BSP headers and placing them into your project’s include directory
3. Adding the vendor’s HAL/BSP header to the HeliOS ``/src/port.h`` header directly below the ``#elif defined(CMSIS_ARCH_CORTEXM)`` statement (i.e., line 52)
4. Setting ``SYSTEM_CORE_CLOCK_FREQUENCY`` and ``SYSTEM_CORE_CLOCK_PRESCALER`` in HeliOS’s ``/src/config.h`` header to match the Cortex-M’s core clock frequency and your desired prescaler
5. Add the ``-DCMSIS_ARCH_CORTEXM`` compiler directive to your project’s build configuration

## Espressif ESP32
Please note that HeliOS is not supported on the Espressif ESP32 microcontroller when using the ESP32 Arduino core. This is because the ESP32 Arduino core is built on FreeRTOS and HeliOS and FreeRTOS cannot coexist in the same application. To target ESP32, HeliOS must be built using Espressif's SDK without the ESP32 Arduino core. The files ``/src/port.h`` and ``/src/port.c`` will also need to be updated with the necessary code to control interrupts and access the microcontroller's tick timer. Espressif's SDK can be found [here](https://idf.espressif.com/).
***
# :man_teacher: Example
Many embedded applications implement what is called a "super loop". A super loop is a loop that never exits (i.e., ``while(1) {}``) and contains most of the code executed by the microcontroller. The problem with super loops is they can grow out of control and become difficult to manage. This becomes especially challenging given the relatively few options for controlling timing (e.g., ``delay()``). Unfortunately the use of ``delay()`` to control timing also means the microcontroller is unable to perform other operations (at least without the help of an ISR) until ``delay()`` returns. Below is an example of how easy it is to leverage the event-driven multitasking capabilities within HeliOS to implement the Arduino "Blink" example.
## Arduino "Blink" Example
Below is the "Blink" example sketch included with the Arduino platform.
```C
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
}
```
## HeliOS "Blink" Example
Below is the Arduino "Blink" example sketch implemented using HeliOS. In this example, a HeliOS task, which alternates the microcontroller's GPIO pin state between high and low, is added in a "wait" state and a task timer is set instructing HeliOS's scheduler to execute the task every 1,000 ticks (milliseconds on many microcontrollers).
```C
#include <HeliOS.h>


/* Define the task's main function. This function is the entry point for the
 * task when executed by the scheduler. The "task_" parameter contains the task
 * itself and may be used to perform operations against the task such as
 * suspending it with xTaskSuspend(task_). The "parm_" parameter points to
 * memory containing the task parameter(s). This memory can be allocated by
 * xMemAlloc() if needed. The task parameter must be dereferenced inside the
 * task's main function. A convenient C macro, DEREF_TASKPARM(), is available to
 * simplify the task of dereferencing the task parameter. */
void blinkTask_main(xTask task_, xTaskParm parm_) {
  /* Dereference the task parameter and store its value in the local integer
   * "ledState". This integer contains the state of the LED (i.e., 1 (on) or 0
   * (off)). Global variables are discouraged in favor of task parameters when
   * sharing or persisting a value is required*/
  int ledState = DEREF_TASKPARM(int, parm_);


  /* Once inside the task's main function, do not call functions like Arduino's
   * delay(). HeliOS tasks should implement a state machine model like the one
   * used here to ensure control is returned to the scheduler as quickly as
   * possible so other tasks may run. */
  if(ledState) {
    digitalWrite(LED_BUILTIN, HIGH);
    ledState = 0;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    ledState = 1;
  }


  /* Because the value of "ledState" has changed, the task parameter must be
   * dereferenced again so that it may be updated. The task's main function will
   * receive the same value the next time the task is executed by the scheduler.
   * Task parameters are also the preferred method for sharing message queues,
   * stream buffers, etc. between tasks. */
  DEREF_TASKPARM(int, parm_) = ledState;

  return;
}


void setup() {
  int ledState = 0;


  pinMode(LED_BUILTIN, OUTPUT);


  /* Call xSystemInit() to initialize memory and call initialization functions
   * in the port layer. The xSystemInit() syscall must be made prior to making
   * any other syscall. The ERROR() and OK() C macros are a concise method for
   * checking the return value of the xSystemInit() syscall. A consistent return
   * type (xReturn) was introduced in kernel 0.4.0.  If the syscall fails,
   * xSystemHalt() will be called to halt the system.*/
  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }


  /* Declare the task which will be used inside of the Arduino setup() function
   * to configure the task prior to handing over control to the HeliOS
   * scheduler. */
  xTask blink;


  /* Call the xTaskCreate() syscall to create the task. The xTaskCreate()
   * syscall prototype and parameters are as follows.
   *
   * PROTOTYPE
   * ~~~~~~~~~
   *   xReturn xTaskCreate(xTask *task_,
   *                       const xByte *name_,
   *                       void (*callback_)(xTask task_, xTaskParm parm_),
   *                       xTaskParm taskParameter_)
   *
   * PARAMETERS
   * ~~~~~~~~~~
   *   task_     A reference to the task. To pass the task by reference, the
   *             address-of ("&") operator must be used (e.g., &blink).
   *
   *   name_     A reference to the first byte of a byte array containing the
   *             ASCII name of the task. The task name is not a null terminated C char array
   *             (sometimes called a "string"). The length of the byte array must be
   *             precisely CONFIG_TASK_NAME_BYTES (default is 8) bytes. If the task name is
   *             shorter, then it must be padded to meet the precise length requirement. To
   *             avoid compiler warnings when using a literal (e.g., "BLINKTSK"), the
   *             argument must be cast as "const xByte *".
   *
   *   callBack_ A reference to the task's main function. The task's main
   *             function's prototype must be as follows. The name of the task's main
   *             function does *NOT* need to match the name given to the task through the
   *             "name_" parameter.
   *
   *               void <taskname>(xTask task_, xTaskParm parm_)
   *
   * If the syscall fails, xSystemHalt() will be called to halt the system.*/
  if(ERROR(xTaskCreate(&blink, (const xByte *) "BLINKTSK", blinkTask_main, &ledState))) {
    xSystemHalt();
  }


  /* Because the blink task will be an event-driven task (i.e., scheduled for
   * execution only when a task event occurs), the task must be placed in the
   * "waiting" state by xTaskWait(). There are two types of task events,
   * direct-to-task notifications and task timers. In this example we will be
   * using a task timer. If the syscall fails, xSystemHalt() will be called to
   * halt the system. */
  if(ERROR(xTaskWait(blink))) {
    xSystemHalt();
  }


  /* In order to use the task timer, the task timer period must be set to a
   * positive non-zero value. In this example we are setting the task timer to
   * 1,000 ticks. This way the HeliOS scheduler will schedule the blink task for
   * execution every 1,000 ticks. The length of a tick is platform and/or
   * architecture dependent though on most platforms a tick will occur every one
   * millisecond. If the syscall fails, xSystemHalt() will be called to halt the
   * system. */
  if(ERROR(xTaskChangePeriod(blink, 1000))) {
    xSystemHalt();
  }


  /* Now that the task(s) are created and configured they way we want, control
   * must be passed to the HeliOS scheduler. Once this is done, the only way to
   * return control back to the Arduino setup() function is by calling
   * xTaskSuspendAll() which will cause the scheduler to quit. If the syscall
   * fails, xSystemHalt() will be called to halt the system. */
  if(ERROR(xTaskStartScheduler())) {
    xSystemHalt();
  }


  /* While not required, it is advised to call xSystemHalt() at the end of the
   * Arduino setup() function. In this way, if the scheduler is forced to quit,
   * the application will halt and no further code will be executed. */
  xSystemHalt();
}


void loop() {
  /* The Arduino loop() function is not used in a HeliOS application and must
   * remain empty. */
}
```
***
# :package: Releases
All releases, including the latest release, can be found [here](https://github.com/heliosproj/HeliOS/releases).
* 0.4.1 - Fixed PlatformIO library.json file and updated readme
* 0.4.0 - Consistent return type for all syscalls, additional memory consistency checking, new HeliOS Developer's Guide, new code documentation and many more changes and improvements.
* 0.3.5 - Several new features including device drivers, stream buffers, task watchdog timer, improved memory defragmentation and many more including improvements to source code and documentation.
* 0.3.4 - Corrected "blink" example in readme and in examples, fixed ESP8266 support, added queue locking and other improvements
* 0.3.3 - Multi-region memory support, memory defragmentation, CMSIS support, new portability layer and other code improvements
* 0.3.2 - Some fixes to the memory management system calls and related functions
* 0.3.1 - A lot of refactoring, code clean-up from the 0.3.0 release and code documentation/readability improvements
* 0.3.0 - First release of the new 0.3.x series kernel (many new features, most of the kernel rewritten, new example code and new documentation)
* 0.2.7 - Added a contributed example, privatized the list pointers for scheduler and added support for Teensy 3/4
* 0.2.6 - Added built-in support for ESP8266 and minor internal updates
* 0.2.5 - Numerous internal enhancements including improved time precision and scheduler now gracefully handles overflow of run-time timer
* 0.2.4 - Additional example Arduino sketches and other code improvements
* 0.2.3 - Improved protection of system state, new examples, improved code documentation and some maintainability enhancements
* 0.2.2 - Additional function calls, minor fixes and documentation enhancements
* 0.2.1 - The first official release
***
# :construction: Contributing
See the [contributing](/CONTRIBUTING.md) guidelines on how to contribute to HeliOS. **If you are going to make a source code or documentation contribution; please do not fork the ``master`` branch. Only pull requests forked from the ``develop`` branch will be accepted.**
***
# :scroll: Copyright & License
HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>

HeliOS is copyrighted open source software licensed under the Free Software Foundation's GNU General Public License (GPL) Version 2. The full text of the license can be found [here](/LICENSE.md).
***
# :skull_and_crossbones: Important Notice
HeliOS is **not** certified for use in safety-critical applications. The HeliOS source code, whether in full or in part, must **never** be used in applications where a risk to life exists. In other words, do not use HeliOS in your project if there is even a remote chance someone might get hurt.
***
# :speech_balloon: Other Notice
This project is not affiliated in any way, past or present, with the discontinued Unix-like operating system Helios developed by Dr. Tim King of Perihelion Software Ltd. or Axel Muhr's work on [Helios-NG](https://github.com/axelmuhr/Helios-NG). Any resemblance is purely coincidental.
***