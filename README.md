![HeliOS](/extras/HeliOS_OG_Logo.png)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/MannyPeterson/HeliOS/blob/master/LICENSE.md) ![GitHub last commit](https://img.shields.io/github/last-commit/MannyPeterson/HeliOS) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/MannyPeterson/HeliOS) [![arduino-library-badge](https://www.ardu-badge.com/badge/HeliOS.svg?)](https://www.ardu-badge.com/HeliOS) ![GitHub stars](https://img.shields.io/github/stars/MannyPeterson/HeliOS?style=social) ![GitHub watchers](https://img.shields.io/github/watchers/MannyPeterson/HeliOS?style=social)
# Overview
HeliOS is an embedded operating system that is free for anyone to use. While called an operating system for simplicity, HeliOS is better described as a multitasking kernel for embedded systems. HeliOS is very small. In fact, it is small enough to run on most 8-bit microcontrollers including the popular AVR based Arduino Uno. Written entirely in C, HeliOS runs on a variety of microcontrollers and integrates easily into any project. HeliOS is also easy to learn with an Application Programming Interface (API) consisting of only 21 function calls. HeliOS contains the following key features:
* Cooperative Multitasking (Run-time Balanced)
* Event Driven Multitasking
* Task Notification/Messaging
* Timers
* Managed Memory
* Non-Preemptive (no mutexs! no deadlocks! no race conditions! no headaches!)

As noted in the key features, HeliOS supports two types of multitasking: cooperative and event driven. Cooperative multitasking embedded operating systems and task schedulers often use a round-robin scheduling strategy. However, HeliOS uses a run-time balanced strategy which ensures tasks with shorter run-times are prioritized over tasks with longer run-times. This ensures all running tasks receive approximately equal total run-time without using context switching. The other multitasking option available in HeliOS is event driven multitasking, which uses the wait/notify and timer interfaces. Mixing cooperative and event driven tasks in HeliOS is not a problem.
# Why HeliOS
There are already so many great and proven RTOS and scheduler options available, then why HeliOS? That is certainly a valid question. HeliOS was never intended to replace or compete with the other options already available today (if you have not checked out FreeRTOS - you should, it is nothing short of an amazing RTOS for embedded and is more accessible today than ever before through the Arduino Library Manager). HeliOS, however, due to its size and simplicity, is intended to play in the space between RTOS's and task schedulers. HeliOS is tiny (smaller than FreeRTOS), easy to use and a great place to start for enthusiasts, hobbyists and researchers.
# What's Happening
HeliOS is continuously being improved. Development is currently occurring in weekly sprints resulting in a new release at the end of each sprint. This schedule will continue until a substantial portion of the development backlog, which includes improvements to the HeliOS Programmer's Guide, code documentation, additional example sketches and testing, is complete. If you are looking to check-out the latest developments in HeliOS, clone the **develop** branch. But remember that the source code and documentation in the **develop** branch is under active development and may contain errors. In the meantime, don't forget to **star** or **watch** the HeliOS repository and check back frequently.
# Getting Started
## Arduino
Because HeliOS is compliant with the Arduino 1.5 (rev. 2.2) Library Specification, getting up and running is quick and easy. HeliOS can be installed directly from the Arduino Library Manager or downloaded and installed manually. Both options are described [here](https://www.arduino.cc/en/Guide/Libraries#toc3). You can also refer to the auto-generated instructions from ArduBadge [here](https://www.ardu-badge.com/HeliOS). Once up and running, check out one of the example sketches or refer to the HeliOS Programmer's Guide in the Documentation section.
## Other Microcontrollers
While built-in support currently only exists for the Arduino AVR, SAM and SAMD architectures, HeliOS is easily ported to a variety of other microcontrollers. Because the project is currently focused on supporting the Arduino community, no plans currently exist for adding built-in support for additional microcontrollers and tool-chains.
## Linux & Microsoft Windows
Built-in support exists for building and running HeliOS in user-land on Linux and Microsoft Windows. When running in user-land, HeliOS acts like a threading library for applications. To target Linux or Microsoft Windows, (as shown below) simply un-comment the appropriate C preprocessor directive in the header file HeliOS.h and build using GCC or Microsoft Visual C++. The files needed to build HeliOS in user-land on Linux and Microsoft Windows can be found in extras/linux and extras/windows directories respectively.
```C
/*
 * Un-comment to compile on Linux or Microsoft
 * Windows.
 * #define OTHER_ARCH_LINUX
 * #define OTHER_ARCH_WINDOWS
 */
```
# Documentation
The HeliOS Programmer's Guide is a work in progress. What is available today can be found [here](/extras/HeliOS_Programmers_Guide.md). If you are interested in contributing to the HeliOS Programmer's Guide, please see the Contributing section for details.
# Example
Many embedded projects on microcontrollers implement what is called a "super loop". A super loop is a loop that never exits (i.e while(1){}) and contains most of the code executed by the microcontroller. The problem with super loops is they can grow out of control and become difficult to manage. This becomes especially challenging given the relatively few options for controling timing (e.g. delay()). Unfortunately the use of delay() to control timing also means the microcontroller is unable to perform other operations (at least without the help of an ISR) until delay() returns. Below is an example of how easy it is to leverage the event driven multitasking capabilities within HeliOS.
## Arduino "Blink" Example
Below is a copy of the traditional Arduino "Blink" example code.
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
Below is the Arduino "Blink" example code implemented using HeliOS. In this example, a HeliOS task, which alternates the microcontroller's GPIO pin state between high and low, is added in a "wait" state and a timer is set instructing HeliOS's scheduler to execute the task every 1,000,000 microseconds. That's it!
```C
/*
 * Include the standard HeliOS header for Arduino sketches. This header
 * includes the required HeliOS header files automatically.
 */
#include <HeliOS_Arduino.h>

/*
 * Declare and initialize an int to maintain the state of
 * the built-in LED.
 */
volatile int ledState = 0;

/*
 * The task definition for taskBlink() which will
 * be executed by HeliOS every 1,000,000 microseconds
 * (1 second).
 */
void taskBlink(xTaskId id_) {
  /*
   * If the state is 0 or LOW then set the state to
   * 1 or HIGH. Likewise, if the state is 1 or HIGH
   * then set the state to LOW.
   */
  if (ledState) {
    /*
     * Set the state of the digital GPIO pin associated
     * with the built-in LED to LOW.
     */
    digitalWrite(LED_BUILTIN, LOW);

    /*
     * Update the int containing the state of the build-in
     * LED accordingly.
     */
    ledState = 0;
  } else {
    /*
     * Set the state of the digital GPIO pin associated
     * with the built-in LED to HIGH.
     */
    digitalWrite(LED_BUILTIN, HIGH);

    /*
     * Update the int containing the state of the built-in
     * LED accordingly.
     */
    ledState = 1;
  }
}

void setup() {
  /*
   * Declare an xTaskId to hold the the task id
   * and initialize.
   */
  xTaskId id = 0;

  /*
   * Call xHeliOSSetup() to initialize HeliOS and
   * its data structures. xHeliOSSetup() must be
   * called before any other HeliOS function call.
   */
  xHeliOSSetup();

  /*
   * Set the mode of the digital GPIO pin associated
   * with the built-in LED to OUTPUT only.
   */
  pinMode(LED_BUILTIN, OUTPUT);

  /*
   * Add the task taskBlink() to HeliOS by passing
   * xTaskAdd() the friendly name of the task as well
   * as a callback pointer to the task function.
   */
  id = xTaskAdd("TASKBLINK", &taskBlink);

  /*
   * Call xTaskWait() to place taskBlink() into a wait
   * state by passing xTaskWait() the task id. A task
   * must be in a wait state to respond to timer events.
   */
  xTaskWait(id);

  /*
   * Set the timer interval for taskBlink() to 1,000,000 microseconds
   * (1 second). HeliOS automatically begins incrementing
   * the timer for the task once the timer interval is set.
   */
  xTaskSetTimer(id, 1000000);
}

void loop() {
  /*
   * Momentarily pass control to HeliOS by calling the
   * xHeliOSLoop() function call. xHeliOSLoop() should be
   * the only code inside of the sketch's loop() function.
   */
  xHeliOSLoop();
}
```
# Releases
All releases, including the current release, can be found [here](https://github.com/MannyPeterson/HeliOS/releases).
* 0.2.6 - added built-in support for ESP8266 and minor internal updates
* 0.2.5 - numerous internal enhancements including improved time precision and scheduler now gracefully handles overflow of run-time timer
* 0.2.4 - additional example Arduino sketches and other code improvements
* 0.2.3 - Improved protection of system state, new examples, improved code documentation and some maintainability enhancements
* 0.2.2 - Additional function calls, minor fixes and documentation enhancements
* 0.2.1 - The first official release
# Contributing
While all contributions are welcome, contributions are needed most in the following areas:
* HeliOS Programmer's Guide
* Code Documentation, Readability and Maintainability
* Example Sketches
* Testing

To contribute, simply create a pull request with your changes. Please fork from the **develop** branch as **master** is kept even with the current release. Pull requests are typically responded to within 48 hours.
# Thank you
No free and open source software project has been successful without the contributions of many. This space is reserved for recognizing people who have made meaningful contributions to HeliOS. All contributors are listed in alphabetical order.
* [Jakub Rakus](https://github.com/JakubRakus)
* [Julien Peyregne](https://github.com/JuPrgn)
* [Kai Wurtz](https://github.com/kwrtz)
* [Konidem](https://github.com/Konidem)
* [Stig Bjorlykke](https://github.com/stigbjorlykke)
* [Thomas Hornschuh](https://github.com/ThomasHornschuh)

Also, special thanks to [Gil Maimon](https://github.com/gilmaimon) for the great website [ArduBadge](https://www.ardu-badge.com/).
# License & Trademarks
HeliOS is copyrighted open source software licensed under the Free Software Foundation's GNU General Public License Version 3. The license can be found [here](/LICENSE.md).

Microsoft Windows and Microsoft Visual C++ are registered trademarks of Microsoft Corporation in the United States and/or other countries.
# Important
HeliOS is **not** certified for use in safety-critical applications. The HeliOS source code, whether in full or in part, must **never** be used in applications where a risk to life exists.
