<picture>
  <source media="(prefers-color-scheme: dark)" srcset="/extras/HeliOS_OG_Logo_Dark.png">
  <source media="(prefers-color-scheme: light)" srcset="/extras/HeliOS_OG_Logo_Light.png">
  <img alt="HeliOS Logo" src="/extras/HeliOS_OG_Logo_Light.png">
</picture>

[![License: GPL Version 2](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://github.com/heliosproj/HeliOS/blob/master/LICENSE.md) ![GitHub last commit](https://img.shields.io/github/last-commit/heliosproj/HeliOS) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/heliosproj/HeliOS) [![PlatformIO Registry](https://badges.registry.platformio.org/packages/heliosproj/library/HeliOS.svg)](https://registry.platformio.org/libraries/heliosproj/HeliOS) [![arduino-library-badge](https://www.ardu-badge.com/badge/HeliOS.svg?)](https://www.ardu-badge.com/HeliOS) ![GitHub stars](https://img.shields.io/github/stars/heliosproj/HeliOS?style=social) ![GitHub watchers](https://img.shields.io/github/watchers/heliosproj/HeliOS?style=social)

***

# :rocket: Overview

HeliOS is a portable embedded operating system for resource‑constrained microcontrollers. It combines a deterministic, zero‑context‑switch scheduler with event‑driven and cooperative tasks, robust IPC, a private heap with defragmentation, a unified driver architecture, and a full FAT32 filesystem — all in clean and portable, C90‑compliant code.

## Why HeliOS

- Simple mental model: cooperative + event‑driven tasking without preemption
- Predictable and efficient: no context switch overhead, minimal footprint
- Portable by design: Arduino, CMSIS/ARM Cortex‑M, Teensy, ESP8266
- Batteries included: FAT32 filesystem, console/shell, drivers, utilities
- Quality first: strict coding standard, extensive tests, CI, and docs

## Core Features

### Scheduling and Tasks
- Event‑driven tasks wake on notifications or timers and run first
- Cooperative tasks share CPU via a runtime‑balanced algorithm
- No context switching; responsive behavior with explicit yields

### Inter‑Process Communication
- Direct‑to‑task notifications with optional payloads
- Message queues for FIFO delivery between tasks
- Stream buffers for byte‑oriented data and protocol parsing

### Memory Management
- Private, statically‑backed heap — no `malloc()`/`free()` dependency
- Variable‑sized allocations with automatic defragmentation
- Separate kernel region for critical objects and metadata
- Integrity checks and detailed heap/kernel statistics

### Filesystem (FAT32)
- Full FAT32 implementation: files, directories, timestamps, attributes
- Block device abstraction for RAM disk, SD cards, and flash
- Mount/unmount/format, seek/tell/truncate/sync, directory enumeration

### Drivers and I/O
- Unified Block I/O interface and Character I/O interface
- RAM Disk driver for fast testing and simulations
- STM32 USART driver with HAL integration (blocking/IRQ/DMA modes)

### Console and Shell
- Interactive shell with command buffer, echo control, and device readiness
- Current working directory support and path utilities
- Configurable command length and I/O subsystem enablement

## Design Principles

- No preemption or context switching; cooperative by default
- Event‑driven first: timers and notifications drive responsiveness
- Separation of concerns: kernel, drivers, filesystem, and I/O layers
- Configurable via `src/config.h` with documented trade‑offs

***

# :loudspeaker: What’s New in 0.5.0

HeliOS 0.5.0 focuses on capability, portability, and developer experience.

- FAT32 filesystem: full stack with block device abstraction and RAM disk
- Console subsystem: interactive shell with command buffer and utilities
- Unified driver architecture: standard Block/Char I/O interfaces
- STM32 USART driver: HAL‑based with blocking/IRQ/DMA support
- Memory revamp: variable‑sized allocator, better alignment, richer stats
- Object validity flags across kernel types for runtime safety
- New CI workflows and expanded harness‑based testing
- Documentation and templates refreshed across the project

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

The complete HeliOS syscall API is documented in the [HeliOS Developer's Guide](/doc/HeliOS_Developers_Guide.pdf). All API functions are also documented with detailed Doxygen comments in **`/src/HeliOS.h`**.

For support and help, see [SUPPORT.md](/SUPPORT.md).

## Quick Start: Arduino IDE

1. Open the Arduino IDE
2. Navigate to **Tools → Manage Libraries...**
3. Search for "HeliOS"
4. Click **Install**
5. Explore examples under **File → Examples → HeliOS**

Complete installation instructions are available in the [Arduino Library documentation](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries).

## Quick Start: PlatformIO

Add HeliOS to your **`platformio.ini`**:
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

For advanced use cases where you want to build HeliOS on top of CMSIS **without the Arduino Core**, see the [ST Build Guide](/BUILD_GUIDE_ST.md) for a complete example.

The build guide demonstrates building and deploying HeliOS 0.5.0 on an **ST NUCLEO-H723ZG** development board using STM32CubeIDE. While this example is specific to ST hardware, the same process applies to other ST microcontrollers and development boards. For microcontrollers and development boards from other vendors (e.g., NXP, Microchip, Nordic, etc.), the steps will differ, but the build guide still provides valuable background on the general process of integrating HeliOS with CMSIS-based toolchains.

**Quick reference for other ARM Cortex-M platforms:**

1. Download the [latest release](https://github.com/heliosproj/HeliOS/releases)
2. Extract into your project's source directory
3. Download CMSIS headers and your vendor's HAL/BSP headers
4. Edit **`/src/port.h`** line 52 to include your vendor's HAL header:
   ```c
   #elif defined(CMSIS_ARCH_CORTEXM)
   #include "stm32f4xx_hal.h"  // Example for STM32F4
   ```
5. Configure **`/src/config.h`**:
   - Set **`SYSTEM_CORE_CLOCK_FREQUENCY`** to your core clock frequency
   - Set **`SYSTEM_CORE_CLOCK_PRESCALER`** to your desired prescaler value
6. Add **`-DCMSIS_ARCH_CORTEXM`** to your compiler flags

## ESP32
**Important:** HeliOS is **not compatible** with the ESP32 Arduino core because it is built on FreeRTOS. HeliOS and FreeRTOS cannot coexist in the same application.

To use HeliOS on ESP32, you must:
- Use Espressif's ESP-IDF directly (without Arduino core)
- Update **`/src/port.h`** and **`/src/port.c`** with ESP32-specific interrupt and timer code
- Use the [ESP-IDF toolchain](https://idf.espressif.com/)

***

# :man_teacher: Examples

Explore ready‑to‑run sketches in `examples/` covering scheduling, IPC, memory, filesystem, console, and drivers. The classic “Blink” example is included alongside multi‑task patterns and I/O demos.

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

**Built with** :heart: **for the embedded systems community**
