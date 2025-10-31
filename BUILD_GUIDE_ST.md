# HeliOS Build Guide for ST NUCLEO-H723ZG

## Overview

The purpose of this guide is to demonstrate how **HeliOS** can be built and deployed to other MCUs without the Arduino Core. This guide is meant for more advanced use cases and provides step-by-step instructions for building and deploying **HeliOS** on the **ST NUCLEO-H723ZG** development board. While this guide is specific to the **ST NUCLEO-H723ZG**, these same steps may be taken to build and deploy **HeliOS** on other ST MCUs and development boards.

### Target Hardware

- **Board**: NUCLEO-H723ZG (Nucleo-144 form factor)
- **MCU**: STM32H723ZGT6
  - Arm Cortex-M7 core
  - 550 MHz maximum frequency
  - 1 MB Flash memory
  - 564 KB RAM
- **Clock**: HSI at 64 MHz (default at reset)
- **LED**: LD1 (Green) on PB0

---

## Prerequisites

Before starting, ensure you have:

1. A Windows, macOS, or Linux computer
2. ST NUCLEO-H723ZG development board
3. USB cable (USB A/C to Micro USB)
4. ST account (free registration required)

---

## Step 1: Download STM32CubeIDE

1. Navigate to the STM32CubeIDE download page:
   ```
   https://www.st.com/en/development-tools/stm32cubeide.html
   ```

2. Create an ST account if you don't have one, then log in

3. Download the appropriate version for your operating system

4. Note the download location for installation

---

## Step 2: Install STM32CubeIDE

### Windows
1. Run the downloaded installer (.exe file)
2. Follow the installation wizard
3. Accept the license agreement
4. Choose installation directory
5. Complete the installation

### macOS
1. Open the downloaded .dmg file
2. Drag STM32CubeIDE to Applications folder
3. Launch from Applications

### Linux
1. Extract the downloaded archive
2. Run the installer script as the root user (sudo)
3. Follow the on-screen instructions

---

## Step 3: Download Required CMSIS and HAL Components

### Download CMSIS Device MCU Component

1. Navigate to:
   ```
   https://github.com/STMicroelectronics/cmsis-device-h7
   ```

2. Download the latest release (e.g., **`cmsis-device-h7-1.10.6.zip`**)
   - Click "Releases" on the right side
   - Download the source code archive

### Download STM32CubeH7 MCU Firmware Package

1. Navigate to:
   ```
   https://github.com/STMicroelectronics/STM32CubeH7
   ```

2. Download the latest release (e.g., **`STM32CubeH7-1.12.1.zip`**)
   - Click "Releases" on the right side
   - Download the source code archive

3. Keep both archives for extraction later

---

## Step 4: Create New STM32 Project

1. Launch **STM32CubeIDE**

2. Create a new project:
   - **File** → **New** → **STM32 Project**

3. In the Target Selection window:
   - In the **"Commercial Part Number"** textbox, enter:
     ```
     STM32H723ZG
     ```
   - From the **"MCUs/MPUs"** list, select:
     ```
     NUCLEO-H723ZG
     ```
   - Click **Next**

4. Configure project settings:
   - **Project Name**: Enter your desired project name (e.g., **`helios_h723`**)
   - **Targeted Project Type**: Select **Empty**
   - **Targeted Language**: Ensure **C** is selected
   - Click **Finish**

5. If prompted, accept to open the C/C++ perspective

---

## Step 5: Extract and Copy CMSIS Files

### Extract Archives

1. Extract **`cmsis-device-h7-1.10.6.zip`** to a temporary location
2. Extract **`STM32CubeH7-1.12.1.zip`** to a temporary location

### Copy CMSIS Device Files

1. Navigate to the extracted folder:
   ```
   cmsis-device-h7-1.10.6/Include
   ```

2. Copy **all files** from this directory to your project's **`Inc`** directory:
   ```
   <workspace>/your_project_name/Inc/
   ```

   Files include:
   - **`stm32h723xx.h`**
   - **`stm32h7xx.h`**
   - **`system_stm32h7xx.h`**
   - And other device-specific headers

### Copy CMSIS Core Files

1. Navigate to the extracted folder:
   ```
   STM32CubeH7-1.12.1/Drivers/CMSIS/Core/Include
   ```

2. Copy **all files** from this directory to your project's **`Inc`** directory:
   ```
   <workspace>/your_project_name/Inc/
   ```

   Files include:
   - **`core_cm7.h`**
   - **`cmsis_version.h`**
   - **`cmsis_compiler.h`**
   - **`cmsis_gcc.h`**
   - And other CMSIS core headers

---

## Step 6: Copy HeliOS Source Files

1. Locate the HeliOS source files directory (where you have the HeliOS distribution)

2. Copy **all files** from the HeliOS **`src`** directory to your project's **`Src`** directory:
   ```
   <workspace>/your_project_name/Src/
   ```

   Files include:
   - **`HeliOS.h`**
   - **`config.h`**
   - **`defines.h`**
   - **`device.c`** & **`device.h`**
   - **`fs.c`** & **`fs.h`**
   - **`mem.c`** & **`mem.h`**
   - **`port.c`** & **`port.h`**
   - **`posix.h`**
   - **`queue.c`** & **`queue.h`**
   - **`streams.c`** & **`streams.h`**
   - **`sys.c`** & **`sys.h`**
   - **`task.c`** & **`task.h`**
   - **`timer.c`** & **`timer.h`**
   - **`types.h`**
   - **`syscalls.c`**
   - **`sysmem.c`**

---

## Step 7: Configure Preprocessor Symbols

1. Right-click on your project in **Project Explorer**

2. Select **Properties**

3. Navigate to:
   - **C/C++ Build** → **Settings** → **MCU/MPU GCC Compiler** → **Preprocessor**

4. In the **"Define symbols (-D)"** section:
   - Click the **+** (plus) button
   - Add the following symbol:
     ```
     CMSIS_ARCH_CORTEXM
     ```

5. Click **Apply and Close**

---

## Step 8: Configure HeliOS Port Header

### Update MCU Include

1. In **Project Explorer**, expand the **Src** directory

2. Open the file **`port.h`**

3. Locate **line 52** (or search for **`stm32f429xx.h`**):
   ```c
   #include "stm32f429xx.h"
   ```

4. Change it to:
   ```c
   #include "stm32h723xx.h"
   ```

### Update System Core Clock Frequency

5. In the same file **`port.h`**, locate **line 58** (or search for **`SYSTEM_CORE_CLOCK_FREQUENCY`**):
   ```c
   #define SYSTEM_CORE_CLOCK_FREQUENCY 0xF42400u /* 16000000u */
   ```

6. Change it to:
   ```c
   #define SYSTEM_CORE_CLOCK_FREQUENCY 0x3D09000u /* 64000000u */
   ```

   This sets the system clock to 64 MHz (the default HSI frequency for STM32H723)

7. **Save** the file (**`Ctrl+S`** or **`Cmd+S`**)

---

## Step 9: Create Main Application Code

1. In **Project Explorer**, open **`main.c`** from the **`Src`** directory

2. Replace the entire contents with the following code:

```c
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : HeliOS LED Blink Example for NUCLEO-H723ZG
 ******************************************************************************
 * @details
 *
 *
 * This example demonstrates HeliOS (a lightweight RTOS) on the STM32H723ZG
 * microcontroller. A periodic task blinks the LED every 1 second.
 *
 * Hardware Configuration:
 * - Board: NUCLEO-H723ZG (Nucleo-144 form factor)
 * - MCU: STM32H723ZGT6 (Arm Cortex-M7, 550 MHz max, 1MB Flash, 564KB RAM)
 * - Clock: HSI at 64 MHz (default at reset)
 * - LED: LD1 (Green) on PB0
 *
 * HeliOS Configuration:
 * - System tick: 1 ms (SysTick at 1000 Hz)
 * - Task period: 1000 ticks = 1 second
 * - Scheduler: Cooperative multitasking
 *
 * Important Notes:
 * - Ensure port.h has SYSTEM_CORE_CLOCK_FREQUENCY = 64000000u (0x3D09000u)
 * - Ensure port.h has SYSTEM_CORE_CLOCK_PRESCALER = 1000u (0x3E8u)
 * - All HeliOS API calls include error checking with xSystemHalt() on failure
 *
 * @attention
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "HeliOS.h"
#include "stm32h723xx.h"

/* Private defines -----------------------------------------------------------*/
/**
 * @brief LED hardware configuration
 *
 * NUCLEO-H723ZG board LEDs:
 * - LD1 (Green):  PB0
 * - LD2 (Yellow): PE1
 * - LD3 (Red):    PB14
 */
#define LED_PORT        GPIOB
#define LED_PIN         0

/**
 * @brief Task period in milliseconds
 *
 * LED toggles every 1000 ms (1 second).
 * Change this value to adjust blink rate.
 */
#define BLINK_PERIOD_MS 1000

/* Private variables ---------------------------------------------------------*/
/**
 * @brief Task handle for LED blink task
 *
 * Used to reference the task for operations like period changes or state changes.
 */
static Task_t *ledTask = NULL;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void LED_Blink_Task(Task_t *task_, TaskParm_t *parm_);

/* Function implementations --------------------------------------------------*/

/**
 * @brief Configure system clock
 *
 * STM32H723ZG boots with HSI at 64 MHz by default.
 * No additional configuration needed for this example.
 */
static void SystemClock_Config(void)
{
    /* HSI at 64 MHz is enabled by default at reset */
    /* No configuration required */
}

/**
 * @brief Initialize GPIO for LED
 *
 * Configures PB0 as output for LED control.
 */
static void GPIO_Init(void)
{
    /* Enable GPIOB clock */
    RCC->AHB4ENR |= (1 << 1);

    /* Configure PB0 as output (MODER[1:0] = 01) */
    LED_PORT->MODER &= ~(3 << (LED_PIN * 2));  /* Clear mode bits */
    LED_PORT->MODER |=  (1 << (LED_PIN * 2));  /* Set output mode */

    /* Configure as push-pull output */
    LED_PORT->OTYPER &= ~(1 << LED_PIN);

    /* Set output speed to low */
    LED_PORT->OSPEEDR &= ~(3 << (LED_PIN * 2));

    /* Disable pull-up/pull-down resistors */
    LED_PORT->PUPDR &= ~(3 << (LED_PIN * 2));
}

/**
 * @brief LED blink task callback
 *
 * Called by HeliOS scheduler every 1 second to toggle the LED.
 *
 * @param task_ Pointer to task structure (provided by HeliOS)
 * @param parm_ Pointer to task parameters (NULL in this example)
 */
static void LED_Blink_Task(Task_t *task_, TaskParm_t *parm_)
{
    /* Toggle LED state using XOR */
    LED_PORT->ODR ^= (1 << LED_PIN);
}

/**
 * @brief Main program entry point
 *
 * Initializes hardware and HeliOS, then starts the scheduler.
 * This function never returns.
 *
 * @return Never returns
 */
int main(void)
{
    /* Initialize system clock (uses default HSI 64 MHz) */
    SystemClock_Config();

    /* Initialize GPIO for LED */
    GPIO_Init();

    /* Initialize HeliOS kernel (memory, scheduler, SysTick) */
    if(ERROR(xSystemInit())) {
        xSystemHalt();  /* Halt if initialization fails */
    }

    /* Create LED blink task
     * - &ledTask: Store task handle here
     * - "LED_Blink": Task name (max 8 bytes)
     * - LED_Blink_Task: Callback function
     * - NULL: No task parameters
     */
    if(ERROR(xTaskCreate(&ledTask, (Byte_t *)"LED_Blink", LED_Blink_Task, NULL))) {
        xSystemHalt();  /* Halt if task creation fails */
    }

    /* Set task period to 1000 ticks (1 second)
     * - ledTask: Task handle
     * - BLINK_PERIOD_MS: Period in milliseconds
     */
    if(ERROR(xTaskChangePeriod(ledTask, BLINK_PERIOD_MS))) {
        xSystemHalt();  /* Halt if period configuration fails */
    }

    /* Put task in waiting state for periodic execution
     * - ledTask: Task handle
     */
    if(ERROR(xTaskWait(ledTask))) {
        xSystemHalt();  /* Halt if state change fails */
    }

    /* Start HeliOS scheduler (never returns) */
    if(ERROR(xTaskStartScheduler())) {
        xSystemHalt();  /* Halt if scheduler fails to start */
    }

    /* Should never reach here */
    xSystemHalt();
}
```

3. **Save** the file (**`Ctrl+S`** or **`Cmd+S`**)

---

## Step 10: Build the Project

1. In STM32CubeIDE, click the **hammer icon** (Build) in the toolbar, or:
   - Press **`Ctrl+B`** (Windows/Linux)
   - Press **`Cmd+B`** (macOS)
   - Or select **Project** → **Build Project**

2. Monitor the **Console** tab for build output

3. Verify successful build:
   - Look for messages like:
     ```
     Finished building target: your_project_name.elf

     arm-none-eabi-size your_project_name.elf
        text    data     bss     dec     hex filename
       xxxxx    xxxx   xxxxx   xxxxx   xxxxx your_project_name.elf
     ```

4. If build errors occur:
   - Check that all CMSIS files were copied correctly
   - Verify **`CMSIS_ARCH_CORTEXM`** preprocessor symbol is defined
   - Ensure **`port.h`** modifications are correct
   - Review the Console output for specific error messages

---

## Step 11: Connect the Hardware

1. Locate the **ST-LINK** micro USB connector on the NUCLEO-H723ZG board
   - This is typically labeled "USB ST-LINK" or "CN1"
   - Located near the top of the board (opposite end from Arduino headers)

2. Connect the board to your computer using a USB cable
   - Use the ST-LINK micro USB port (not the user USB port)

3. Wait for the operating system to recognize the device:
   - **Windows**: Check Device Manager for "STMicroelectronics STLink Virtual COM Port"
   - **macOS**: The device should mount automatically
   - **Linux**: Check **`dmesg`** for USB device recognition

4. Verify the power LED on the board is lit

---

## Step 12: Flash and Run the Application

1. In STM32CubeIDE, select:
   - **Run** → **Run As** → **STM32 C/C++ Application**

2. If prompted to create a new launch configuration:
   - Select **STM32 Cortex-M C/C++ Application**
   - Click **OK**

3. The debugger will:
   - Flash the program to the microcontroller
   - Start execution automatically

4. Observe the output:
   - The **Debug** perspective will open
   - Execution will pause at the beginning of **`main()`**

5. Click the **Resume** button (green play icon with **`F8`**) to continue execution

---

## Project Structure

After completing all steps, your project structure should look like:

```
your_project_name/
├── Debug/                      # Build output directory
│   ├── your_project_name.elf   # Executable file
│   ├── your_project_name.bin   # Binary file
│   └── ...                     # Object files and dependencies
├── Inc/                        # Include directory
│   ├── stm32h723xx.h          # Device header
│   ├── stm32h7xx.h            # Family header
│   ├── core_cm7.h             # CMSIS Cortex-M7 header
│   └── ...                     # Other CMSIS headers
├── Src/                        # Source directory
│   ├── main.c                  # Application code
│   ├── HeliOS.h                # HeliOS main header
│   ├── config.h                # HeliOS configuration
│   ├── port.h                  # Platform port header
│   ├── port.c                  # Platform port implementation
│   ├── task.c / task.h         # Task management
│   ├── mem.c / mem.h           # Memory management
│   ├── device.c / device.h     # Device management
│   ├── fs.c / fs.h             # File system
│   ├── queue.c / queue.h       # Queue management
│   ├── streams.c / streams.h   # Stream I/O
│   ├── sys.c / sys.h           # System functions
│   ├── timer.c / timer.h       # Timer management
│   ├── types.h                 # Type definitions
│   ├── defines.h               # Macro definitions
│   ├── syscalls.c              # System call stubs
│   └── sysmem.c                # System memory allocation
├── Startup/                    # Startup code
│   └── startup_stm32h723zgtx.s # Startup assembly
├── .cproject                   # Eclipse project configuration
├── .project                    # Eclipse project file
└── STM32H723ZGTX_FLASH.ld     # Linker script
```