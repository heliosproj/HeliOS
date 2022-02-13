![HeliOS](/extras/HeliOS_OG_Logo.png)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/MannyPeterson/HeliOS/blob/master/LICENSE.md) ![GitHub last commit](https://img.shields.io/github/last-commit/MannyPeterson/HeliOS) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/MannyPeterson/HeliOS) [![arduino-library-badge](https://www.ardu-badge.com/badge/HeliOS.svg?)](https://www.ardu-badge.com/HeliOS) ![GitHub stars](https://img.shields.io/github/stars/MannyPeterson/HeliOS?style=social) ![GitHub watchers](https://img.shields.io/github/watchers/MannyPeterson/HeliOS?style=social)
# Overview
HeliOS is an embedded operating system that is free for everyone to use. While called an operating system, HeliOS is a multitasking kernel for use in embedded applications. The feature-rich, thoroughly documented, API allows the user to control every aspect of the system and access kernel services for task (process) management, scheduler management, inter-process communication, memory management and more while maintaining a tiny footprint for a broad range of low-power embedded devices. The HeliOS sources are well documented and HeliOS is easily customized to fit the user’s specific needs through a single header file (config.h).

The multitasking features of HeliOS are the most sought after feature of HeliOS. For this reason there are two coexisting multitasking models available in HeliOS that can be leveraged concurrently within the same application.
The first multitasking model is event-driven. When a task’s state is set to “waiting”, the task will only respond to task events. HeliOS currently supports two types of task events. The first is direct to task notifications. Direct to task notifications allow one task to send a notification to another task. In this scenario, the HeliOS scheduler will wake the recipient task and schedule it for execution. After the recipient task clears the direct to task notification, it will continue to sleep until another direct to task notification is received. The second type of task event is timer based. Task timers can be configured to tell HeliOS to schedule the task to run every so many microseconds, though task timers should not be confused with application timers (or simply timers) as HeliOS supports both.

The second model for multitasking is a more conventional cooperative model. In this model cooperative tasks are always scheduled to run, unless they are suspended. Additionally, the cooperative model in HeliOS contains a unique scheduler feature that builds on the traditional cooperative model. In most cooperatively scheduled multitasking models, a simple round-robin approach is used (i.e., each task is executed consecutively). However, the HeliOS scheduler uses a “runtime balanced” algorithm for scheduling cooperative tasks. In other words, tasks that consume more runtime are scheduled less frequently than tasks that consume less runtime. This design prevents long running tasks from monopolizing the system’s execution time. Event-driven and cooperatively scheduled tasks run together seamlessly though event-driven tasks always receive execution priority over cooperatively scheduled tasks.

One important aspect of multitasking is HeliOS is it does not use context switching. This reduces the need for the user to manage access to shared resources in a “thread safe” way using mutexes and semaphores. This also eliminates the need for the “port” or portability code required to save the context upon switch as well as accessing ISR’s for “tick” timers. As a result, the user can focus his or her efforts on their specific application without having too, at the same time, contend with concurrent access to shared resources.

HeliOS also provides services for inter-process communication and presently supports two models for inter-process communication. The first, as discussed previously, is direct to task notifications. Direct to task notifications are an efficient communication channel between tasks that prevent a task from consuming runtime when there is no need to do so. The second is message queues. Message queues can be created at runtime and can be shared among any number of tasks. Queues are highly flexible FIFO communication channels that require very little code to implement. Finally, while technically not one of HeliOS’s models for inter-process communication, HeliOS supports task parameters that can be leveraged for rudimentary inter-process communication if so desired.

HeliOS also has built in memory management that greatly increases the safety of dynamically allocated memory. While HeliOS’s dynamic memory allocation allocates “heap” memory, the heap in HeliOS is not a traditional heap. HeliOS uses a private heap that is technically static memory allocated at compile time. HeliOS does not use the standard library malloc() and free() functions and it is recommended that the user also avoid those functions in favor of HeliOS’s memory management services. HeliOS does implement a “protected” memory feature that pseudo-protects the memory it allocates but not to the extent of an MPU or MMU. The implementation of MPU or MMU features in an embedded operating system is complex, costly in terms of performance and requires extensive portability code.

HeliOS is built to be robust. Each HeliOS release (0.3.0 and later) undergoes static analysis testing using a commercially licensed static analysis tool as well as MISRA C:2012 checks. While HeliOS is NOT certified for nor should be used (in full or in part) in any safety-critical application where a risk to life exists, user’s can be confident they are building their embedded application on a robust embedded operating system.

Lastly, for PlatformIO and Arduino users, HeliOS is easily added to their embedded application. The current release of HeliOS is available directly through the PlatformIO Registry and the Arduino Library Manager. For users of other embedded platforms and/or tool-chains, simply download HeliOS from GitHub and add the sources to your project.
# What's Happening
The HeliOS 0.3.x series kernel was recently released and replaces the 0.2.x series kernel. With the 0.3.x series kernel, there have been significant changes to both the kernel internals and the API rendering it incompatible with applications built on 0.2.x. While the changes are significant, updating an application built with 0.2.x requires only a small amount of time as all of the features of HeliOS 0.2.x have been retained in 0.3.x. The key difference is the breadth of features offered by the 0.3.x series kernel has been expanded and existing features rewritten. Along with 0.3.x is a complete developer’s guide to assist the user in building applications on 0.3.x. The focus for HeliOS development going forward will be expanding available example code, further enhancing documentation and addressing any quality issues. As always, contributions are welcome and anyone wishing to contribute to HeliOS should refer to the “Contributing” section.
# Getting Started
## Documentation
The entire HeliOS API is documented in the HeliOS Developer’s Guide is available in PDF format in the HeliOS sources tree under “doc”.
## Microcontroller Support
Other than four define statements, HeliOS requires zero additional portability code. Currently HeliOS has built-in support for AVR, SAMD, SAM, ESP8266, ESP32 and Teensy 3/4/MM microcontrollers (though the latter is an ARM development board). If using the Arduino platform/tool-chain, HeliOS should work right out of the box by adding HeliOS to the project from the PlatformIO Registry or Arduino Library Manager.
## Adding Support
If the user’s desired microcontroller is not supported out of the box, support for other platforms and/or tool-chains only require the user to define the following four defines (and any required headers for the defines) in defines.h.
```C
/* Example defines for a Microchip SAM D MCU based
development board using the Arduino platform. */
#elif defined(ARDUINO_ARCH_SAMD)
#include <Arduino.h>
#define CURRENTTIME() micros()
#define DISABLE_INTERRUPTS() noInterrupts()
#define ENABLE_INTERRUPTS() interrupts()
#define TIME_T_TYPE uint32_t
```
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
Below is the Arduino "Blink" example code implemented using HeliOS. In this example, a HeliOS task, which alternates the microcontroller's GPIO pin state between high and low, is added in a "wait" state and a timer is set instructing HeliOS's scheduler to execute the task every 1,000,000 microseconds.
```C
#include "HeliOS.h"

void blinkTask_main(xTask task_, xTaskParm parm_) {

  int ledState = DEREF_TASKPARM(int, parm_);

  if (ledState) {
    printf("ON!!!\n");

    ledState = 0;
  } else {
    printf("OFF!!!\n");

    ledState = 1;
  }

  DEREF_TASKPARM(int, parm_) = ledState;

  return;
}

void setup() {

  int ledState = 0;

  xTask blink = xTaskCreate("BLINK", blinkTask_main, &ledState);

  if (blink) {
    xTaskWait(blink);

    xTaskChangePeriod(blink, 1000000);

    xTaskStartScheduler();
  }

  xSystemHalt();
}

void loop() {

}
```
# Releases
All releases, including the current release, can be found [here](https://github.com/MannyPeterson/HeliOS/releases).
* 0.2.8 - added xTaskSetTimerWOReset(), added task a task parameter, improved headers and added support for queues
* 0.2.7 - added a contributed example, privatized the list pointers for scheduler and added support for Teensy 3/4
* 0.2.6 - added built-in support for ESP8266 and minor internal updates
* 0.2.5 - numerous internal enhancements including improved time precision and scheduler now gracefully handles overflow of run-time timer
* 0.2.4 - additional example Arduino sketches and other code improvements
* 0.2.3 - Improved protection of system state, new examples, improved code documentation and some maintainability enhancements
* 0.2.2 - Additional function calls, minor fixes and documentation enhancements
* 0.2.1 - The first official release
# Contributing
While all contributions are welcome, contributions are needed most in the following areas:
* Testing, Testing and More Testing
* Code Documentation, Readability and Maintainability
* New Features
* Additional Microcontroller Support
* Example Sketches
* HeliOS Programmer's Guide

To contribute, simply create a pull request with your changes. Please fork from the **develop** branch as **master** is kept even with the current release. Pull requests are typically responded to as quickly as possible.
# Thank you
No free and open source software project has been successful without the contributions of many. This space is reserved for recognizing people who have made meaningful contributions to HeliOS. All contributors are listed in alphabetical order.
* [Jakub Rakus](https://github.com/JakubRakus)
* [Julien Peyregne](https://github.com/JuPrgn)
* [Kai Wurtz](https://github.com/kwrtz)
* [Konidem](https://github.com/Konidem)
* [Stig Bjorlykke](https://github.com/stigbjorlykke)
* [Thomas Hornschuh](https://github.com/ThomasHornschuh)
* [XXIITEAM](https://github.com/IIXXTEAM)
* [Mats Tage Axelsson](https://github.com/matstage)

Also, special thanks to [Gil Maimon](https://github.com/gilmaimon) for the great website [ArduBadge](https://www.ardu-badge.com/).
# License & Trademarks
HeliOS is copyrighted open source software licensed under the Free Software Foundation's GNU General Public License Version 3. The license can be found [here](/LICENSE.md).

Microsoft Windows and Microsoft Visual C++ are registered trademarks of Microsoft Corporation in the United States and/or other countries.
# Important
HeliOS is **not** certified for use in safety-critical applications. The HeliOS source code, whether in full or in part, must **never** be used in applications where a risk to life exists.
