/*UNCRUSTIFY-OFF*/
/**
 * @file config.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel header file for user definable settings
 * @version 0.4.0
 * @date 2022-01-31
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef CONFIG_H_
  #define CONFIG_H_
/*UNCRUSTIFY-OFF*/


  /* The following configurable settings may be changed by the end-user to
   * customize the HeliOS kernel for their specific application. */
  /**
   * @brief Define to enable the Arduino API C++ interface
   *
   * Because HeliOS kernel is written in C, the Arduino API cannot be called
   * directly from the kernel. For example, assertions are unable to be written
   * to the serial bus in applications using the Arduino platform/tool-chain.
   * The CONFIG_ENABLE_ARDUINO_CPP_INTERFACE builds the included arduino.cpp
   * file to allow the kernel to call the Arduino API through wrapper functions
   * such as __ArduinoAssert__(). The arduino.cpp file can be found in the
   * /extras directory. It must be copied into the /src directory to be built.
   *
   */
  /*
   * #define CONFIG_ENABLE_ARDUINO_CPP_INTERFACE
   */


  /**
   * @brief Define to enable system assertions
   *
   * The CONFIG_ENABLE_SYSTEM_ASSERT setting allows the end-user to enable
   * system assertions in HeliOS. Once enabled, the end-user must define
   * CONFIG_SYSTEM_ASSERT_BEHAVIOR for there to be an effect. By default the
   * CONFIG_ENABLE_SYSTEM_ASSERT setting is not defined.
   *
   * @sa CONFIG_SYSTEM_ASSERT_BEHAVIOR
   *
   */
  /*
   * #define CONFIG_ENABLE_SYSTEM_ASSERT
   */


  /**
   * @brief Define the system assertion behavior
   *
   * The CONFIG_SYSTEM_ASSERT_BEHAVIOR setting allows the end-user to specify
   * the behavior (code) of the assertion which is called when
   * CONFIG_ENABLE_SYSTEM_ASSERT is defined. Typically some sort of output is
   * generated over a serial or other interface. By default the
   * CONFIG_SYSTEM_ASSERT_BEHAVIOR is not defined.
   *
   * @note In order to use the __ArduinoAssert__() functionality, the
   * CONFIG_ENABLE_ARDUINO_CPP_INTERFACE setting must be enabled.
   *
   * @sa CONFIG_ENABLE_SYSTEM_ASSERT
   * @sa CONFIG_ENABLE_ARDUINO_CPP_INTERFACE
   *
   * @code {.c}
   * #define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l) __ArduinoAssert__( f , l )
   * @endcode
   *
   */
  /*
   * #define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l) __ArduinoAssert__( f , l )
   */


/*UNCRUSTIFY-ON*/
  /**
   * @brief Define the size in bytes of the message queue message value
   *
   * Setting the CONFIG_MESSAGE_VALUE_BYTES allows the end-user to define the
   * size of the message queue message value. The larger the size of the message
   * value, the greater impact there will be on system performance. The default
   * size is 8 bytes.
   *
   * @sa xQueueMessage
   *
   */
  #if !defined(CONFIG_MESSAGE_VALUE_BYTES)
    #define CONFIG_MESSAGE_VALUE_BYTES 0x8u /* 8 */
  #endif /* if !defined(CONFIG_MESSAGE_VALUE_BYTES) */


  /**
   * @brief Define the size in bytes of the direct to task notification value
   *
   * Setting the CONFIG_NOTIFICATION_VALUE_BYTES allows the end-user to define
   * the size of the direct to task notification value. The larger the size of
   * the notification value, the greater impact there will be on system
   * performance. The default size is 8 bytes.
   *
   * @sa xTaskNotification
   */
  #if !defined(CONFIG_NOTIFICATION_VALUE_BYTES)
    #define CONFIG_NOTIFICATION_VALUE_BYTES 0x8u /* 8 */
  #endif /* if !defined(CONFIG_NOTIFICATION_VALUE_BYTES) */


  /**
   * @brief Define the size in bytes of the task name
   *
   * Setting the CONFIG_TASK_NAME_BYTES allows the end-user to define the size
   * of the task name. The larger the size of the task name, the greater impact
   * there will be on system performance. The default size is 8 bytes.
   *
   * @sa xTaskInfo
   *
   */
  #if !defined(CONFIG_TASK_NAME_BYTES)
    #define CONFIG_TASK_NAME_BYTES 0x8u /* 8 */
  #endif /* if !defined(CONFIG_TASK_NAME_BYTES) */


  /**
   * @brief Define the number of memory blocks available in all memory regions
   *
   * The heap memory region is used by tasks. Whereas the kernel memory region
   * is used solely by the kernel for kernel objects. The
   * CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS setting allows the end-user to define
   * the size, in blocks, of all memory regions thus effecting both the heap and
   * kernel memory regions. The size of a memory block is defined by the
   * CONFIG_MEMORY_REGION_BLOCK_SIZE setting. The size of all memory regions
   * needs to be adjusted to fit the memory requirements of the end-user's
   * application. The default value is 24 blocks.
   *
   */
  #if !defined(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS)
    #define CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS 0x18u /* 24 */
  #endif /* if !defined(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS) */


  /**
   * @brief Define the memory block size in bytes for all memory regions
   *
   * Setting CONFIG_MEMORY_REGION_BLOCK_SIZE allows the end-user to define the
   * size of a memory region block in bytes. The memory region block size should
   * be set to achieve the best possible utilization of the available memory.
   * The CONFIG_MEMORY_REGION_BLOCK_SIZE setting effects both the heap and
   * kernel memory regions. The default value is 32 bytes.
   *
   * @sa xMemAlloc()
   * @sa xMemFree()
   * @sa CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS
   *
   */
  #if !defined(CONFIG_MEMORY_REGION_BLOCK_SIZE)
    #define CONFIG_MEMORY_REGION_BLOCK_SIZE 0x20u /* 32 */
  #endif /* if !defined(CONFIG_MEMORY_REGION_BLOCK_SIZE) */


  /**
   * @brief Define the minimum value for a message queue limit
   *
   * Setting the CONFIG_QUEUE_MINIMUM_LIMIT allows the end-user to define the
   * MINIMUM length limit a message queue can be created with xQueueCreate().
   * When a message queue length equals its limit, the message queue will be
   * considered full and return true when xQueueIsQueueFull() is called. A full
   * queue will also not accept messages from xQueueSend(). The default value is
   * 5.
   *
   * @sa xQueueIsQueueFull()
   * @sa xQueueSend()
   * @sa xQueueCreate()
   *
   */
  #if !defined(CONFIG_QUEUE_MINIMUM_LIMIT)
    #define CONFIG_QUEUE_MINIMUM_LIMIT 0x5u /* 5 */
  #endif /* if !defined(CONFIG_QUEUE_MINIMUM_LIMIT) */


  /**
   * @brief Define the length of the stream buffer
   *
   * Setting CONFIG_STREAM_BUFFER_BYTES will define the length of stream buffers
   * created by xStreamCreate(). When the length of the stream buffer reaches
   * this value, it is considered full and can no longer be written to by
   * calling xStreamSend(). The default value is 32.
   *
   */
  #if !defined(CONFIG_STREAM_BUFFER_BYTES)
    #define CONFIG_STREAM_BUFFER_BYTES 0x20u /* 32 */
  #endif /* if !defined(CONFIG_STREAM_BUFFER_BYTES) */


  /**
   * @brief Enable task watchdog timers
   *
   * Defining CONFIG_TASK_WD_TIMER_ENABLE will enable the task watchdog timer
   * feature. The default is enabled.
   *
   */
  #if !defined(CONFIG_TASK_WD_TIMER_ENABLE)
    #define CONFIG_TASK_WD_TIMER_ENABLE
  #endif /* if !defined(CONFIG_TASK_WD_TIMER_ENABLE) */


  /**
   * @brief Define the length of a device driver name
   *
   * Setting CONFIG_DEVICE_NAME_BYTES will define the length of a device driver
   * name. The name of device drivers should be exactly this length. There
   * really isn't a reason to change this and doing so may break existing device
   * drivers. The default length is 8 bytes.
   *
   */
  #if !defined(CONFIG_DEVICE_NAME_BYTES)
    #define CONFIG_DEVICE_NAME_BYTES 0x8u /* 8 */
  #endif /* if !defined(CONFIG_DEVICE_NAME_BYTES) */
#endif /* ifndef CONFIG_H_ */