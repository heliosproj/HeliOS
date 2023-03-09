![HeliOS](/extras/HeliOS_OG_Logo.png)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/MannyPeterson/HeliOS/blob/master/LICENSE.md) ![GitHub last commit](https://img.shields.io/github/last-commit/MannyPeterson/HeliOS) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/MannyPeterson/HeliOS) [![PlatformIO Registry](https://badges.registry.platformio.org/packages/mannypeterson/library/HeliOS.svg)](https://registry.platformio.org/libraries/mannypeterson/HeliOS) [![arduino-library-badge](https://www.ardu-badge.com/badge/HeliOS.svg?)](https://www.ardu-badge.com/HeliOS) ![GitHub stars](https://img.shields.io/github/stars/MannyPeterson/HeliOS?style=social) ![GitHub watchers](https://img.shields.io/github/watchers/MannyPeterson/HeliOS?style=social)
***
# Overview
HeliOS is an open source embedded operating system that is free for everyone to use. While called an operating system, HeliOS is a multitasking kernel for use in embedded applications. Its rich, fully documented, API allows the user to control every aspect of the system and access kernel services (syscalls) for task (process) management, scheduler management, inter-process communication, memory management, device management (i.e., device drivers) and more while maintaining a tiny footprint for a broad range of low-power embedded devices. HeliOS is also easily customized to fit the user’s specific needs through a single header file ([config.h](/src/config.h)).

HeliOS supports two multitasking models that can be leveraged concurrently within the same application. The first multitasking model is event-driven. When a task is placed in the "waiting" state, the task will only respond to task events. HeliOS supports two types of task events. The first is direct-to-task notifications, which allow one task to send a notification to another task. In this scenario, the HeliOS scheduler will wake the recipient task and schedule it for execution. After the recipient task clears the direct-to-task notification, the recipient task will returning to "waiting" until another notification is received. The second type of task event is timer based. Task timers can be configured to tell HeliOS to schedule the task to run every so many ticks (typically milliseconds), though task timers should not be confused with application timers (or simply timers) as HeliOS supports both.

The second model for multitasking is a conventional cooperative model. In this model, cooperative tasks are always scheduled to run, unless suspended. Additionally, the cooperative model in HeliOS contains a unique scheduler feature that builds on the traditional cooperative model. In most cooperatively scheduled multitasking models, a simple round-robin approach is used (i.e., each task is executed consecutively). However, the HeliOS scheduler uses a “runtime balanced” algorithm for scheduling cooperative tasks. In other words, tasks that consume more runtime are deprioritized (i.e., executed less frequently) in favor of tasks that consume less runtime. This design prevents long running tasks from monopolizing the system’s execution time. Event-driven and cooperatively scheduled tasks run together seamlessly, although event-driven tasks always receive execution priority over cooperatively scheduled tasks.

One important aspect of multitasking in HeliOS is it does not rely on context switching. This reduces the need for the user to manage access to shared resources in a “thread safe” way using mutexes and semaphores. This also eliminates the need for the “port” or portability code required to save the context during a context switch. As a result, the user can focus his or her development effort on their specific application without having to contend with concurrent access to shared resources.

HeliOS also provides services for three inter-process communication models. The first, as discussed previously, is direct-to-task notifications. Direct-to-task notifications are an efficient communication channel between tasks that prevent a task from consuming runtime when there is nothing for the task to process. The second model is message queues. Message queues can be created at runtime and can be shared among any number of tasks. Queues are highly flexible FIFO communication channels that require very little code to implement. The third model is stream buffers. Stream buffers are very much like message queues with one important difference. While queues operate on multi-byte messages, stream buffers operate similarly on single-byte streams. Finally, while technically not one of HeliOS’s models for inter-process communication, HeliOS supports task parameters that can be leveraged for rudimentary inter-process communication if so desired.

The HeliOS kernel includes built-in memory management that improves the safety margin of dynamically allocated memory. While HeliOS’s dynamic memory allocation allocates “heap” memory, the heap in HeliOS is not a true heap. HeliOS uses a private heap that is implemented as static memory allocated at compile time. HeliOS does not use the standard library malloc() and free() functions and it is recommended that the user also avoid those functions in favor of HeliOS’s memory management syscalls. HeliOS also maintains a separate memory region for kernel objects which reduces the risk that memory access, in the user's application, would corrupt critical kernel objects. As of kernel 0.4.0, HeliOS also supports sophisticated memory defragmentation and consistency checking to ensure memory is utilized efficiently and with a high degree of integrity. 

HeliOS also supports a kernel mode device driver model. Device drivers for virtually any feature or peripheral can be easily developed using the provided device driver template. While device drivers are not needed in most applications, when the MCU's MMU or MPU is enabled it may not be possible to access memory mapped registers and I/O from the user's code. While implementation of the ARM MMU and MPU in HeliOS is forthcoming, device driver support had to be added to HeliOS first. Information about the device driver system calls can be found in the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf). A device driver template can be found here: [driver.c](/extras/drivers/template/driver.c) and [driver.h](/extras/drivers/template/driver.h).

HeliOS is built to be robust. HeliOS (0.3.0 and later) has undergone static analysis testing using a commercially licensed static analysis tool as well as MISRA C:2012 checks. While HeliOS is NOT certified for nor should be used (in full or in part) in any safety-critical application where a risk to life exists, user’s can be confident they are building their embedded application on a robust embedded operating system.

Lastly, for PlatformIO and Arduino users, HeliOS is easily added to their embedded application. The current release of HeliOS is available directly through the [PlatformIO Registry](https://registry.platformio.org/libraries/mannypeterson/HeliOS) and the [Arduino Library Manager](https://www.arduino.cc/reference/en/libraries/helios/). For users of other embedded platforms and/or tool-chains, simply download the current [release](https://github.com/MannyPeterson/HeliOS/releases) of HeliOS from GitHub and add the sources to your project.
***
# What's New
The HeliOS 0.4.x series kernel was recently released which supersedes all prior kernel versions. The syscall API and internals have undergone significant development rendering applications built on earlier kernels incompatible with 0.4.x. The key change that will impact compatibility is the introduction of a consistent return type for all syscalls. This provides a better mechanism for error propagation and a consistent interface handling errors.

This provides the user a consistent interface for error errors or exceptions that occur deep within the kernel to propagate upwards through the kernel to the user's application.

For example, prior to kernel 0.4.0, a task would be created as follows.

```C
xTask task = xTaskCreate("TASK", task_main, NULL);

if(task) {
  /* Use the task object here. */
}
```
In this example, the user application would only know if an error or exception occurred by checking if "task" was null. In kernel 0.4.0 all syscalls have a standard return type (xReturn) that can either be ReturnOK or ReturnError. See the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf) for more information about xReturn. Thus, in kernel 0.4.0 the same process of creating a task is done as follows.

```C
xTask task;

if(ERROR(xTaskCreate(&task, (const xByte *) "TASK", task_main, null))) {
  xSystemHalt();
}

/* Use the task object here. */
```
In this manner, the application can check all syscalls for success or failure even when a syscall does not modify or set arguments it is passed. For the very latest on what development is occurring, please check out the [HeliOS Trello board](https://trello.com/b/XNKDpuGR/helios). Anyone wishing to contribute to HeliOS should refer to the “Contributing” section.
***
# HeliOS Around The Web

* **[HeliOS is a Tiny Embedded OS Designed for Arduino Boards](https://www.cnx-software.com/2020/08/14/helios-is-a-tiny-embedded-os-designed-for-arduino-boards/)**

* **[HeliOS for Arduino](https://linuxhint.com/linux_on_arduino/)**

* **[Newly-Launched "Embedded OS" HeliOS Brings Simple Multitasking to Arduino Microcontrollers](https://www.hackster.io/news/newly-launched-embedded-os-helios-brings-simple-multitasking-to-arduino-microcontrollers-11f6b137b75c)**

* **[New HeliOS, an embedded OS for Arduino Boards](https://iot-industrial-devices.com/new-helios-an-embedded-os-for-arduino-boards/)**

* **[HeliOS is a small and simple embedded operating system for Arduino](https://twitter.com/arduino/status/1293910675312357376)**

* **[Arduino Operating System: Best Options of 2021](https://all3dp.com/2/best-arduino-operating-system/)**

* **[HeliOS is a Tiny Embedded OS Designed for Arduino Boards](https://news.knowledia.com/US/en/articles/helios-is-a-tiny-embedded-os-designed-for-arduino-boards-f35f44fe6c88759fa13d8781ce09ac985b2fdd3a)**
***
# Getting Started
## Documentation
The HeliOS syscall API is documented in the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf), which is available in PDF format in the HeliOS source tree under “doc”. If you are in need of support, please refer to the "Contributing" section.
## Microcontroller Support
If using the Arduino platform/tool-chain, HeliOS should work right out of the box for AVR, SAM, SAMD, ESP8266, and Teensy 3.x/4.x/MicroMod microcontrollers (though the latter is an ARM Cortex-M based development board) by adding HeliOS to the project from the [PlatformIO Registry](https://registry.platformio.org/libraries/mannypeterson/HeliOS) or [Arduino Library Manager](https://www.arduino.cc/reference/en/libraries/helios/).

If more advanced features are desired, HeliOS also has built-in support for CMSIS on ARM Cortex-M microcontrollers. To build HeliOS using CMSIS, the user must have the CMSIS core headers as well as the vendor headers for the specific microcontroller. The CMSIS core headers, including the vendor headers, are typically available for download from the vendor's website. Once the correct headers have been obtained, the headers must be placed in the project's include directory, the vendor's header file must be added to the file ([port.h](/src/port.h)) at the marked location and the project must be built with the "-DCMSIS_ARCH_CORTEXM" gcc flag. Depending on the microcontroller, the SYSTEM_CORE_CLOCK_FREQUENCY and SYSTEM_CORE_CLOCK_PRESCALER definitions may need to be changed if millisecond time resolution is desired. The default system core clock frequency is 16 Mhz with a prescaler of 1,000.

Please note, HeliOS does not have built-in support for ESP32. This is because the ESP32 Arduino core is dependent on FreeRTOS. HeliOS and FreeRTOS cannot coexist in the same application. To target ESP32, HeliOS must be built using Espressif's SDK without the ESP32 Arduino core. The files [port.h](/src/port.h) and [port.c](/src/port.c) will also need to be updated with the necessary code to control interrupts and access the MCU's tick timer. Espressif's SDK can be found [here](https://idf.espressif.com/).
***
# Example
Many embedded applications implement what is called a "super loop". A super loop is a loop that never exits (i.e., while(1) {}) and contains most of the code executed by the microcontroller. The problem with super loops is they can grow out of control and become difficult to manage. This becomes especially challenging given the relatively few options for controlling timing (e.g., delay()). Unfortunately the use of delay() to control timing also means the microcontroller is unable to perform other operations (at least without the help of an ISR) until delay() returns. Below is an example of how easy it is to leverage the event-driven multitasking capabilities within HeliOS to implement the Arduino "Blink" example.
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
Below is the Arduino "Blink" example sketch implemented using HeliOS. In this example, a HeliOS task, which alternates the microcontroller's GPIO pin state between high and low, is added in a "wait" state and a timer is set instructing HeliOS's scheduler to execute the task every 1,000 ticks (milliseconds on many MCUs).
```C
/* Include the HeliOS header. Do not include any other HeliOS headers. */
#include <HeliOS.h>


/* Define the task's main function. This is the function that will be called by
 * the scheduler to run the task. The "task_" parameter contains the task object
 * for the task itself. The "parm_" parameter points to the memory containing
 * the task parameter(s). The task parameter must be dereferenced inside the
 * task's main function by using the DEREF_TASKPARM() C macro. */
void blinkTask_main(xTask task_, xTaskParm parm_) {
  /* Dereference the task parameter and store its value in the local integer
   * "ledState". This integer contains the state of the LED (i.e., 1 (on) or 0
   * (off)). This method is recommended over using variables with a global
   * scope.*/
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
   * receive the same value the next time the task's main function is called by
   * the scheduler. Task parameters are also the preffered method for sharing
   * messages queues, stream buffers, etc. between tasks. */
  DEREF_TASKPARM(int, parm_) = ledState;

  return;
}


void setup() {
  int ledState = 0;


  pinMode(LED_BUILTIN, OUTPUT);


  /* Call xSystemInit() to initialize any interrupt handlers and/or memory
   * required by HeliOS to execute on the target platform or architecture. The
   * xSystemInit() syscall must be called prior to calling any other syscall.
   * The ERROR() and OK() C macros are a concise method for checking the return
   * value of the xTaskCreate() syscall. A consistent return type (xReturn) was
   * introduced in kernel 0.4.0.  If the syscall fails, call xSystemHalt().*/
  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }


  /* Declare the task object (a.k.a., task handle) which will be used inside of
   * the Arduino setup() function to configure the task prior to handing over
   * control to the HeliOS scheduler. */
  xTask blink;


  /* Call the xTaskCreate() syscall to create the task and pass back the task
   * object. The xTaskCreate() syscall prototype and parameters are as follows.
   *
   * xReturn xTaskCreate(xTask *task_, const xByte *name_, void
   * (*callback_)(xTask task_, xTaskParm parm_), xTaskParm taskParameter_)
   *
   * task_ A pointer to the task object (a.k.a., task handle). To pass a pointer
   * to the task object, the address-of ("&") operator must be used (e.g.,
   * &blink).
   *
   * name_ A pointer to a byte array containing the ASCII name of the task which
   * can be used later to obtain the task handle by using the xTaskGetName()
   * syscall. The length of the name cannot exceed CONFIG_TASK_NAME_BYTES
   * (default is 8 bytes). While not required (compiler with just throw a
   * warning), when using a string literal (e.g., "BLINK"), the argument must be
   * cast to (const xByte *) to avoid a warning from the compiler.
   *
   * callBack_ A pointer to the task's main function. The task's main function's
   * prototype must be as follows.
   *
   *    void <taskname>(xTask task_, xTaskParm parm_)
   *
   * If the syscall fails, call xSystemHalt(). */
  if(ERROR(xTaskCreate(&blink, (const xByte *) "BLINK", blinkTask_main, &ledState))) {
    xSystemHalt();
  }


  /* Because the blink task will be an event-driven task (i.e., scheduled for
   * execution only when a task event occurs), the task must be placed in the
   * "waiting" state by xTaskWait(). There are two types of task events,
   * direct-to-task notifications and task timers. In this example we will be
   * using a task timer. If the syscall fails, call xSystemHalt(). */
  if(ERROR(xTaskWait(blink))) {
    xSystemHalt();
  }


  /* In order to use the task timer, the task time period must be set to a
   * non-zero positive value. In this example we are setting the task timer to
   * 1,000 ticks. This way the HeliOS scheduler will schedule the blink task for
   * execution every 1,000 ticks. The length of a tick is platform and/or
   * architecture dependent though on most a tick will occur every one
   * millisecond. If the syscall fails, call xSystemHalt().*/
  if(ERROR(xTaskChangePeriod(blink, 1000))) {
    xSystemHalt();
  }


  /* Now that our task(s) are created and configured they way we want, control
   * must be passed to the HeliOS scheduler. Once this is done, the only way to
   * return control back to the Arduino setup() function is by calling
   * xTaskSuspendAll() which will cause the scheduler to quit. If the syscall
   * fails, call xSystemHalt(). */
  if(ERROR(xTaskStartScheduler())) {
    xSystemHalt();
  }


  /* While not required, it is advised to call xSystemHalt() at the end of the
   * Arduino setup() function. In this way,  if the scheduler is forced to quit,
   * the application will halt the execution of further application code. */
  xSystemHalt();
}


void loop() {
  /* The Arduino loop() function is not used in a HeliOS application and must remain
   * empty. */
}
```
***
# Releases
All releases, including the current release, can be found [here](https://github.com/MannyPeterson/HeliOS/releases).
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
# Contributing
To contribute, create a pull request with your changes. Please fork from the **develop** branch only, as **master** is kept even with the current release. If you would like to make a recommendation or are in need of support, please open an issue. If you are seeking support, please include your source code, details about your particular platform and/or tool-chain and a detailed description of the issue you are facing. All pull requests and issues are responded to as quickly as possible.
***
# Copyright & License
HeliOS Embedded Operating System Copyright (C) 2020-2023 Manny Peterson

HeliOS is copyrighted open source software licensed under the Free Software Foundation's GNU General Public License (GPL) Version 3. The full license text can be found [here](/LICENSE.md).
***
# Important Notice
HeliOS is **not** certified for use in safety-critical applications. The HeliOS source code, whether in full or in part, must **never** be used in applications where a risk to life exists. In other words, do not use HeliOS in your project if there is even a remote chance someone might get hurt.
***
# Other Notice
This project is not affiliated in any way, past or present, with the discontinued Unix-like operating system Helios developed by Dr. Tim King of Perihelion Software Ltd. or Axel Muhr's work on [Helios-NG](https://github.com/axelmuhr/Helios-NG). Any resemblance is purely coincidental.
***