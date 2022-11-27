![HeliOS](/extras/HeliOS_OG_Logo.png)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/MannyPeterson/HeliOS/blob/master/LICENSE.md) ![GitHub last commit](https://img.shields.io/github/last-commit/MannyPeterson/HeliOS) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/MannyPeterson/HeliOS) [![PlatformIO Registry](https://badges.registry.platformio.org/packages/mannypeterson/library/HeliOS.svg)](https://registry.platformio.org/libraries/mannypeterson/HeliOS) [![arduino-library-badge](https://www.ardu-badge.com/badge/HeliOS.svg?)](https://www.ardu-badge.com/HeliOS) ![GitHub stars](https://img.shields.io/github/stars/MannyPeterson/HeliOS?style=social) ![GitHub watchers](https://img.shields.io/github/watchers/MannyPeterson/HeliOS?style=social)
***
# Overview
HeliOS is an embedded operating system that is free for everyone to use. While called an operating system, HeliOS is a multitasking kernel for use in embedded applications. Its rich, fully documented, API allows the user to control every aspect of the system and access kernel services for task (process) management, scheduler management, inter-process communication, memory management, device management (i.e., device drivers) and more while maintaining a tiny footprint for a broad range of low-power embedded devices. HeliOS is also easily customized to fit the user’s specific needs through a single header file ([config.h](/src/config.h)).

HeliOS supports two multitasking models that can be leveraged concurrently within the same application. The first multitasking model is event-driven. When a task is placed in the "waiting" state, the task will only respond to task events. HeliOS supports two types of task events. The first is direct-to-task notifications, which allow one task to send a notification to another task. In this scenario, the HeliOS scheduler will wake the recipient task and schedule it for execution. After the recipient task clears the direct-to-task notification, the recipient task will return to sleep until another notification is received. The second type of task event is timer based. Task timers can be configured to tell HeliOS to schedule the task to run every so many ticks (typically milliseconds), though task timers should not be confused with application timers (or simply timers) as HeliOS supports both.

The second model for multitasking is a conventional cooperative model. In this model, cooperative tasks are always scheduled to run, unless suspended. Additionally, the cooperative model in HeliOS contains a unique scheduler feature that builds on the traditional cooperative model. In most cooperatively scheduled multitasking models, a simple round-robin approach is used (i.e., each task is executed consecutively). However, the HeliOS scheduler uses a “runtime balanced” algorithm for scheduling cooperative tasks. In other words, tasks that consume more runtime are deprioritized (i.e., executed less frequently) in favor of tasks that consume less runtime. This design prevents long running tasks from monopolizing the system’s execution time. Event-driven and cooperatively scheduled tasks run together seamlessly, although event-driven tasks always receive execution priority over cooperatively scheduled tasks.

One important aspect of multitasking in HeliOS is it does not rely on context switching. This reduces the need for the user to manage access to shared resources in a “thread safe” way using mutexes and semaphores. This also eliminates the need for the “port” or portability code required to save the context during a context switch. As a result, the user can focus his or her development effort on their specific application without having to contend with concurrent access to shared resources.

HeliOS also provides services for three inter-process communication models. The first, as discussed previously, is direct-to-task notifications. Direct-to-task notifications are an efficient communication channel between tasks that prevent a task from consuming runtime when there is nothing for the task to process. The second model is message queues. Message queues can be created at runtime and can be shared among any number of tasks. Queues are highly flexible FIFO communication channels that require very little code to implement. The third model is stream buffers. Stream buffers are very much like message queues with one important difference. While queues operate on multi-byte messages, stream buffers operate similarly on single-byte streams. Finally, while technically not one of HeliOS’s models for inter-process communication, HeliOS supports task parameters that can be leveraged for rudimentary inter-process communication if so desired.

The HeliOS kernel includes built-in memory management that improves the safety margin of dynamically allocated memory. While HeliOS’s dynamic memory allocation allocates “heap” memory, the heap in HeliOS is not a true heap. HeliOS uses a private heap that is implemented as static memory allocated at compile time. HeliOS does not use the standard library malloc() and free() functions and it is recommended that the user also avoid those functions in favor of HeliOS’s memory management system calls. HeliOS also maintains a separate memory region for kernel objects which reduces the risk that memory access, in the user's application, would corrupt critical kernel objects.

HeliOS also supports a kernel mode device driver model. Device drivers for virtually any feature or peripheral can be easily developed using the provided device driver template. While device drivers are not needed in most applications, when the MCU's MMU or MPU is enabled it may not be possible to access memory mapped registers and I/O from the user's code. While implementation of the ARM MMU and MPU in HeliOS is forthcoming, device driver support had to be added to HeliOS first. Information about the device driver system calls can be found in the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf). A device driver template can be found here: [driver.c](/extras/drivers/template/driver.c) and [driver.h](/extras/drivers/template/driver.h).

HeliOS is built to be robust. HeliOS (0.3.0 and later) has undergone static analysis testing using a commercially licensed static analysis tool as well as MISRA C:2012 checks. While HeliOS is NOT certified for nor should be used (in full or in part) in any safety-critical application where a risk to life exists, user’s can be confident they are building their embedded application on a robust embedded operating system.

Lastly, for PlatformIO and Arduino users, HeliOS is easily added to their embedded application. The current release of HeliOS is available directly through the [PlatformIO Registry](https://registry.platformio.org/libraries/mannypeterson/HeliOS) and the [Arduino Library Manager](https://www.arduino.cc/reference/en/libraries/helios/). For users of other embedded platforms and/or tool-chains, simply download the current [release](https://github.com/MannyPeterson/HeliOS/releases) of HeliOS from GitHub and add the sources to your project.
***
# What's Happening
The HeliOS 0.3.x series kernel was recently released and replaces the 0.2.x series kernel. With the 0.3.x series kernel, there have been changes to both the kernel internals and the API rendering it incompatible with applications built on 0.2.x. Despite the changes to the API, updating an application built with 0.2.x requires a minimal amount of time. A complete [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf) is available to assist the user in building applications on 0.3.x.

For the very latest on what development is occurring, please check out the [HeliOS Trello board](https://trello.com/b/XNKDpuGR/helios). **As always, contributions are welcome and anyone wishing to contribute to HeliOS should refer to the “Contributing” section.**
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
The HeliOS API is documented in the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf), which is available in PDF format in the HeliOS source tree under “doc”. If you are in need of support, please refer to the "Contributing" section.
## Microcontroller Support
If using the Arduino platform/tool-chain, HeliOS should work right out of the box for AVR, SAM, SAMD, ESP8266, and Teensy 3.x/4.x/MicroMod microcontrollers (though the latter is an ARM Cortex-M based development board) by adding HeliOS to the project from the [PlatformIO Registry](https://registry.platformio.org/libraries/mannypeterson/HeliOS) or [Arduino Library Manager](https://www.arduino.cc/reference/en/libraries/helios/).

If more advanced features are desired, HeliOS also has built-in support for CMSIS on ARM Cortex-M microcontrollers. To build HeliOS using CMSIS, the user must have the CMSIS core headers as well as the vendor headers for the specific microcontroller. The CMSIS core headers, including the vendor headers, are typically available for download from the vendor's website. Once the correct headers have been obtained, the headers must be placed in the project's include directory, the vendor's header file must be added to the file ([port.h](/src/port.h)) at the marked location and the project must be built with the "-DCMSIS_ARCH_CORTEXM" gcc flag. Depending on the microcontroller, the SYSTEM_CORE_CLOCK_FREQUENCY and SYSTEM_CORE_CLOCK_PRESCALER definitions may need to be changed if millisecond time resolution is desired. The default system core clock frequency is 16 Mhz with a prescaler of 1,000.

Please note, HeliOS does not have built-in support for ESP32. This is because the ESP32 Arduino core is dependent on FreeRTOS. HeliOS and FreeRTOS cannot coexist in the same application. To target ESP32, HeliOS must be built using Espressif's SDK without the ESP32 Arduino core. The files [port.h](/src/port.h) and [port.c](/src/port.c) will also need to be updated with the necessary code to control interrupts and access the MCU's tick timer. Espressif's SDK can be found [here](https://idf.espressif.com/).
***
# Example
Many embedded applications implement what is called a "super loop". A super loop is a loop that never exits (i.e., while(1){}) and contains most of the code executed by the microcontroller. The problem with super loops is they can grow out of control and become difficult to manage. This becomes especially challenging given the relatively few options for controlling timing (e.g., delay()). Unfortunately the use of delay() to control timing also means the microcontroller is unable to perform other operations (at least without the help of an ISR) until delay() returns. Below is an example of how easy it is to leverage the event-driven multitasking capabilities within HeliOS to implement the Arduino "Blink" example.
## Arduino "Blink" Example
Below is the "Blink" example code included with the Arduino platform.
```C
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
```
## HeliOS "Blink" Example
Below is the Arduino "Blink" example code implemented using HeliOS. In this example, a HeliOS task, which alternates the microcontroller's GPIO pin state between high and low, is added in a "wait" state and a timer is set instructing HeliOS's scheduler to execute the task every 1,000 ticks (milliseconds on many MCUs).
```C

/* Include the HeliOS header, do not include
any other HeliOS header. */
#include <HeliOS.h>

/* Define the task's main function. The definition must
include the xTask and xTaskParm parameters. */
void blinkTask_main(xTask task_, xTaskParm parm_) {

  /* Dereference the task parameter and store its value
  in a local integer. This integer contains the state
  of the LED (i.e., on or off). */
  int ledState = DEREF_TASKPARM(int, parm_);

  if (ledState) {
    digitalWrite(LED_BUILTIN, HIGH);

    ledState = 0;
  } else {
    digitalWrite(LED_BUILTIN, LOW);

    ledState = 1;
  }

  /* Dereference the task parameter to update its
  value. The task's main function will receive this
  value next time the task's main function is called
  by the scheduler. */
  DEREF_TASKPARM(int, parm_) = ledState;

  return;
}

void setup() {

  int ledState = 0;

  pinMode(LED_BUILTIN, OUTPUT);

  /* Call xSystemInit() to initialize any interrupt handlers and/or
  memory required by HeliOS to execute on the target platform/architecture. */
  xSystemInit();

  /* Create a new HeliOS task, give it an ASCII name, a reference to
  the task's main function and a reference to the task's parameter - in
  this case the state of the LED. */
  xTask blink = xTaskCreate("BLINK", blinkTask_main, &ledState);

  /* Check to make sure the task was created by xTaskCreate() before
  attempting to use the task. */
  if (blink) {

    /* Place the task in the "waiting" state so it will respond to task
    events. */
    xTaskWait(blink);

    /* Set the task timer period to 1,000 ticks. The HeliOS scheduler
    will execute the task every 1,000 ticks until the task is either suspended,
    its task timer period is changed or the task is deleted. */
    xTaskChangePeriod(blink, 1000);

    /* Pass control to the HeliOS scheduler. The HeliOS scheduler will
    not relinquish control unless xTaskSuspendAll() is called. */
    xTaskStartScheduler();


    /* If the scheduler relinquishes control, do some clean-up by
    deleting the task. */
    xTaskDelete(blink);
  }

  /* Halt the system. Once called, the system must be reset to
  recover. */
  xSystemHalt();
}

void loop() {
  /* The loop function is not used and should remain empty. */
}
```
***
# Releases
All releases, including the current release, can be found [here](https://github.com/MannyPeterson/HeliOS/releases).
* 0.3.6 - TBD
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