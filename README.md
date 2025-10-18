<picture>
  <source media="(prefers-color-scheme: dark)" srcset="/extras/HeliOS_OG_Logo_Dark.png">
  <source media="(prefers-color-scheme: light)" srcset="/extras/HeliOS_OG_Logo_Light.png">
  <img alt="HeliOS Logo" src="/extras/HeliOS_OG_Logo_Light.png">
</picture>

[![License: GPL Version 2](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://github.com/heliosproj/HeliOS/blob/master/LICENSE.md) ![GitHub last commit](https://img.shields.io/github/last-commit/heliosproj/HeliOS) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/heliosproj/HeliOS) [![PlatformIO Registry](https://badges.registry.platformio.org/packages/heliosproj/library/HeliOS.svg)](https://registry.platformio.org/libraries/heliosproj/HeliOS) [![arduino-library-badge](https://www.ardu-badge.com/badge/HeliOS.svg?)](https://www.ardu-badge.com/HeliOS) ![GitHub stars](https://img.shields.io/github/stars/heliosproj/HeliOS?style=social) ![GitHub watchers](https://img.shields.io/github/watchers/heliosproj/HeliOS?style=social)

***

# :rocket: Overview

HeliOS is a lightweight, open-source embedded operating system designed for resource-constrained microcontrollers. Despite its small footprint, HeliOS provides a rich feature set including cooperative multitasking, event-driven task scheduling, inter-process communication, memory management with defragmentation, device driver support, and a complete FAT32 filesystem implementation.

## Key Features

### Multitasking Models
HeliOS supports two complementary multitasking approaches that work seamlessly together:

**Event-Driven Tasks**
- Tasks respond only to specific events (direct-to-task notifications or timer expirations)
- Remain dormant when no events are pending, conserving CPU cycles
- Automatically scheduled by the kernel when events occur
- Ideal for periodic operations, interrupt handling, and reactive behaviors

**Cooperative Tasks**
- Always eligible for execution unless explicitly suspended
- Scheduled using HeliOS's unique **runtime-balanced algorithm**
- Tasks consuming more CPU time are automatically deprioritized
- Prevents any single task from monopolizing system resources
- No context switching required—simplifies development and reduces overhead

Event-driven tasks always receive execution priority over cooperative tasks, ensuring responsive behavior when consistent execution is important.

### Inter-Process Communication
HeliOS provides three powerful IPC mechanisms:

1. **Direct-to-Task Notifications** - Lightweight signaling between tasks with optional data payloads
2. **Message Queues** - FIFO communication channels for variable-length messages between multiple tasks
3. **Stream Buffers** - Byte-oriented circular buffers optimized for serial data and protocol parsing

### Memory Management
- **Private heap implementation** using statically allocated memory—no reliance on `malloc()/free()`
- **Separate kernel memory region** protects critical kernel objects from user code corruption
- **Automatic defragmentation** maintains memory efficiency over long run times
- **Consistency checking** detects memory corruption early
- **Detailed statistics** for monitoring heap usage and fragmentation

### Device Driver Framework
- Kernel-mode device driver support with self-registration pattern
- Abstract device layer enables portable driver development
- Essential for systems with MMU/MPU memory protection
- Template drivers provided in `/drivers/template/`

### FAT32 Filesystem (New in 0.5.0)
- **Full FAT32 implementation** with support for files and directories
- **Block device abstraction** works with RAM disks, SD cards, flash memory
- **Standard file operations**: open, close, read, write, seek, truncate
- **Directory management**: create, remove, enumerate directory entries
- **Volume operations**: format, mount, unmount, query volume information
- **File metadata**: timestamps, attributes, file size tracking
- **Internal kernel API** for filesystem drivers and device access

### Safety and Robustness
- Static analysis tested with commercial tools
- MISRA C:2012 compliance checking
- Comprehensive error handling with consistent return types
- Extensive documentation and code examples
- **Not certified for safety-critical applications**

### Broad Platform Support
- **Arduino** - Available via Arduino Library Manager
- **PlatformIO** - Available via PlatformIO Registry
- **ARM Cortex-M** - CMSIS support for Keil µVision and vendor IDEs
- **ESP8266** - Native support (note: ESP32 requires ESP-IDF, not Arduino core)
- **Teensy 3/4** - Full support
- Easily portable to other microcontroller platforms

## Design Philosophy

### No Context Switching
HeliOS uses cooperative multitasking without context switching. This design:
- **Eliminates race conditions** on shared resources—no mutexes or semaphores needed
- **Simplifies portability**—no architecture-specific context save/restore code
- **Reduces overhead**—lower memory footprint and faster task switching
- **Eases debugging**—predictable execution flow

**Trade-offs:**
- Tasks must voluntarily yield control to the scheduler
- Soft real-time guarantees only (no hard deadlines)
- Long-running tasks can impact system responsiveness if not properly designed

### Configurable and Customizable
Many kernel parameters are easily configured through `/src/config.h`:

***

# :loudspeaker: What's New

## Version 0.5.0 - FAT32 Filesystem & Enhanced Testing

The latest HeliOS 0.5.0 release brings major improvements to both functionality and quality assurance:

### FAT32 Filesystem Support

HeliOS now includes a complete FAT32 filesystem implementation:

- **Complete filesystem stack**: boot sector, FAT tables, directory entries, cluster allocation
- **Block device layer**: abstract interface for storage devices (RAM disk, SD card, flash)
- **Comprehensive file API**: Full POSIX-style file operations
- **Directory support**: Create, remove, and traverse directory hierarchies
- **RAM disk driver**: Built-in RAM-based storage for testing and embedded applications
- **Internal kernel APIs**: Pattern `__Function__()` for kernel-to-kernel calls

Example filesystem usage:
```c
xVolume vol;
xFile file;

// Format and mount a block device
xFSFormat(blockDeviceUID, (xByte*)"HELIOS_VOL");
xFSMount(&vol, blockDeviceUID);

// Create and write to a file
xFileOpen(&file, vol, (xByte*)"/config.txt", FS_MODE_WRITE | FS_MODE_CREATE);
xFileWrite(file, sizeof(data), data);
xFileClose(file);

// Read from a file
xFileOpen(&file, vol, (xByte*)"/config.txt", FS_MODE_READ);
xByte *data;
xFileRead(file, 100, &data);
// ... process data ...
xMemFree((xAddr)data);
xFileClose(file);
```

### Enhanced Unit Testing Framework

The testing framework has been significantly enhanced with modern development practices:

- **Enhanced assertion macros** with detailed failure diagnostics (file, line, expected vs actual values)
- **Descriptive test case names** that clearly explain what is being tested
- **Named constants** replacing magic numbers throughout test code for better readability
- **Color-coded output** for easy visual scanning of test results
- **147 comprehensive test cases** covering all kernel subsystems including edge cases
- **C90 compliance** with strict `-ansi -pedantic` compilation

***

# :computer_mouse: HeliOS Around The Web

* **[HeliOS is a Tiny Embedded OS Designed for Arduino Boards](https://www.cnx-software.com/2020/08/14/helios-is-a-tiny-embedded-os-designed-for-arduino-boards/)**
* **[HeliOS for Arduino](https://linuxhint.com/linux_on_arduino/)**
* **[Newly-Launched "Embedded OS" HeliOS Brings Simple Multitasking to Arduino Microcontrollers](https://www.hackster.io/news/newly-launched-embedded-os-helios-brings-simple-multitasking-to-arduino-microcontrollers-11f6b137b75c)**
* **[New HeliOS, an embedded OS for Arduino Boards](https://iot-industrial-devices.com/new-helios-an-embedded-os-for-arduino-boards/)**
* **[HeliOS is a small and simple embedded operating system for Arduino](https://twitter.com/arduino/status/1293910675312357376)**
* **[Arduino Operating System: Best Options of 2021](https://all3dp.com/2/best-arduino-operating-system/)**

***

# :dart: Getting Started

## Documentation

The complete HeliOS syscall API is documented in the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf). All API functions are also documented with detailed Doxygen comments in `/src/HeliOS.h`.

For support and help, see [SUPPORT.md](/SUPPORT.md).

## Quick Start: Arduino IDE

1. Open the Arduino IDE
2. Navigate to **Tools → Manage Libraries...**
3. Search for "HeliOS"
4. Click **Install**
5. Explore examples under **File → Examples → HeliOS**

Complete installation instructions are available in the [Arduino Library documentation](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries).

## Quick Start: PlatformIO

Add HeliOS to your `platformio.ini`:
```ini
[env:myboard]
platform = ...
board = ...
framework = arduino
lib_deps = heliosproj/HeliOS
```

Or install via PlatformIO CLI:
```bash
pio pkg install --library "heliosproj/HeliOS"
```

See the [PlatformIO installation guide](https://registry.platformio.org/libraries/heliosproj/HeliOS/installation) for more details.

## Advanced: ARM Cortex-M with CMSIS

For ARM Cortex-M microcontrollers using CMSIS:

1. Download the [latest release](https://github.com/heliosproj/HeliOS/releases)
2. Extract into your project's source directory
3. Download CMSIS headers and your vendor's HAL/BSP headers
4. Edit `/src/port.h` line 52 to include your vendor's HAL header:
   ```c
   #elif defined(CMSIS_ARCH_CORTEXM)
   #include "stm32f4xx_hal.h"  // Example for STM32F4
   ```
5. Configure `/src/config.h`:
   - Set `SYSTEM_CORE_CLOCK_FREQUENCY` to your core clock frequency
   - Set `SYSTEM_CORE_CLOCK_PRESCALER` to your desired prescaler value
6. Add `-DCMSIS_ARCH_CORTEXM` to your compiler flags

## ESP32
**Important:** HeliOS is **not compatible** with the ESP32 Arduino core because it is built on FreeRTOS. HeliOS and FreeRTOS cannot coexist in the same application.

To use HeliOS on ESP32, you must:
- Use Espressif's ESP-IDF directly (without Arduino core)
- Update `/src/port.h` and `/src/port.c` with ESP32-specific interrupt and timer code
- Use the [ESP-IDF toolchain](https://idf.espressif.com/)

***

# :man_teacher: Example: Arduino "Blink" with HeliOS

This example demonstrates HeliOS's event-driven multitasking by reimplementing the classic Arduino "Blink" sketch.

## Traditional Arduino "Blink"

```c
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

**Problems with this approach:**
- `delay()` blocks all other operations
- Difficult to add additional tasks
- Hard to implement different timing for multiple tasks
- No way to respond to events while waiting

## HeliOS Event-Driven "Blink"

```c
#include <HeliOS.h>

// Task function: toggles LED state
void blinkTask_main(xTask task_, xTaskParm parm_) {
  // Retrieve LED state from task parameter
  int ledState = DEREF_TASKPARM(int, parm_);

  // Toggle LED
  if(ledState) {
    digitalWrite(LED_BUILTIN, HIGH);
    ledState = 0;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    ledState = 1;
  }

  // Save new state back to task parameter
  DEREF_TASKPARM(int, parm_) = ledState;
}

void setup() {
  int ledState = 0;
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize HeliOS
  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }

  // Create blink task
  xTask blink;
  if(ERROR(xTaskCreate(&blink, (const xByte *) "BLINKTSK", blinkTask_main, &ledState))) {
    xSystemHalt();
  }

  // Place task in waiting state (event-driven)
  if(ERROR(xTaskWait(blink))) {
    xSystemHalt();
  }

  // Set task timer to 1000 ticks (1 second on most platforms)
  if(ERROR(xTaskChangePeriod(blink, 1000))) {
    xSystemHalt();
  }

  // Start the scheduler (never returns)
  if(ERROR(xTaskStartScheduler())) {
    xSystemHalt();
  }

  xSystemHalt();
}

void loop() {
  // Not used - HeliOS scheduler handles execution
}
```

**Benefits of the HeliOS approach:**
- Non-blocking—other tasks can run concurrently
- Easy to add more tasks with different timings
- Event-driven tasks only run when needed
- Can respond to notifications and other events
- Clean separation of task logic

## Adding More Tasks

With HeliOS, adding additional tasks is straightforward:

```c
void sensorTask_main(xTask task_, xTaskParm parm_) {
  int sensorValue = analogRead(A0);
  // Process sensor data...
}

void setup() {
  // ... initialize HeliOS ...

  // Create multiple tasks
  xTask blink, sensor;
  xTaskCreate(&blink, (const xByte *) "BLINKTSK", blinkTask_main, &ledState);
  xTaskCreate(&sensor, (const xByte *) "SENSRTSK", sensorTask_main, null);

  // Configure timers
  xTaskWait(blink);
  xTaskChangePeriod(blink, 1000);  // Run every 1 second

  xTaskWait(sensor);
  xTaskChangePeriod(sensor, 100);   // Run every 100ms

  xTaskStartScheduler();
}
```

Each task runs independently at its own rate without blocking others.

***

# :package: Releases

Download the latest release from the [GitHub releases page](https://github.com/heliosproj/HeliOS/releases).

## Version History

* **0.5.0** - Complete FAT32 filesystem implementation with block device layer, file/directory operations, RAM disk driver, and internal kernel APIs
* **0.4.1** - Fixed PlatformIO library.json and updated documentation
* **0.4.0** - Consistent return types for all syscalls, enhanced memory consistency checking, comprehensive developer's guide, improved documentation
* **0.3.5** - Device drivers, stream buffers, task watchdog timer, improved memory defragmentation, enhanced documentation
* **0.3.4** - Fixed "blink" example, ESP8266 support, queue locking, various improvements
* **0.3.3** - Multi-region memory support, memory defragmentation, CMSIS support, new portability layer
* **0.3.2** - Memory management syscall fixes
* **0.3.1** - Extensive refactoring and code cleanup from 0.3.0
* **0.3.0** - Major rewrite with many new features and updated documentation
* **0.2.7** - Community examples, scheduler improvements, Teensy 3/4 support
* **0.2.6** - Built-in ESP8266 support
* **0.2.5** - Improved time precision, scheduler overflow handling
* **0.2.4** - Additional examples and code improvements
* **0.2.3** - Enhanced system state protection, new examples, better documentation
* **0.2.2** - Additional API functions and documentation
* **0.2.1** - First official release

***

# :raising_hand: Getting Support

Need help using HeliOS? Start with the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf), then see [SUPPORT.md](/SUPPORT.md) for details on requesting support.

***

# :construction: Contributing

Ready to contribute code? See [CONTRIBUTING.md](/CONTRIBUTING.md) for complete details on the pull request process, code quality standards, and contribution workflow.

***

# :scroll: Copyright & License

**HeliOS Embedded Operating System**
Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>

HeliOS is licensed under the **GNU General Public License (GPL) Version 2** as published by the Free Software Foundation.

This program is free software; you can redistribute it and/or modify it under the terms of the GPL v2. This program is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY**; without even the implied warranty of **MERCHANTABILITY** or **FITNESS FOR A PARTICULAR PURPOSE**.

See the [full license text](/LICENSE.md) for complete details.

***

# :skull_and_crossbones: Safety-Critical Notice

**HeliOS is NOT certified for safety-critical applications.**

Do **NOT** use HeliOS (in whole or in part) in any application where:
- Human life or safety could be at risk
- Failure could result in injury, death, or property damage
- Regulatory certification is required (medical, automotive, aerospace, etc.)

HeliOS provides **soft real-time** scheduling only. While robust and well-tested, it has not undergone the rigorous certification processes required for safety-critical systems.

**When in doubt, don't use HeliOS in safety-critical applications.**

***

# :speech_balloon: Disclaimer

This project is not affiliated with:
- The discontinued Unix-like operating system "Helios" developed by Dr. Tim King of Perihelion Software Ltd.
- Axel Muhr's [Helios-NG](https://github.com/axelmuhr/Helios-NG) project

Any similarities are purely coincidental.

***

# :question: FAQ

**Q: Can I use HeliOS on ESP32?**
A: Not with the Arduino core. ESP32 Arduino uses FreeRTOS, which conflicts with HeliOS. Use ESP-IDF directly instead.

**Q: Does HeliOS support preemptive multitasking?**
A: No, HeliOS uses cooperative multitasking. Tasks must voluntarily yield control to the scheduler.

**Q: Is HeliOS real-time?**
A: HeliOS provides soft real-time guarantees but is not a hard real-time operating system.

**Q: Can I use Arduino libraries with HeliOS?**
A: Yes, most Arduino libraries work fine with HeliOS. Avoid using `delay()` inside tasks.

**Q: How much memory does HeliOS require?**
A: Depends on configuration, but typical footprint is 2-8 KB RAM and 8-16 KB flash. Configure in `/src/config.h`.

**Q: Can I use global variables?**
A: Yes, but task parameters are preferred for sharing data between tasks to maintain clean architecture.

***

**Built with** :heart: **for the embedded systems community**
