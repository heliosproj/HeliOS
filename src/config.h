/*UNCRUSTIFY-OFF*/
/**
 * @file config.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for build configuration
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#ifndef CONFIG_H_
  #define CONFIG_H_


/* The following configurable settings may be changed by the end-user to
 * customize the HeliOS kernel for their specific application. */
/**
 * @brief Define to enable the Arduino API C++ interface
 *
 * Because the HeliOS kernel is written in C, the Arduino API cannot be called
 * directly from the kernel. For example, assertions are unable to be written to
 * the serial bus in applications using the Arduino platform/tool-chain. The
 * CONFIG_ENABLE_ARDUINO_CPP_INTERFACE builds the included arduino.cpp file to
 * allow the kernel to call the Arduino API through wrapper functions such as
 * __ArduinoAssert__(). The arduino.cpp file can be found in the
 * /extras directory. It must be copied into the /src directory to be built.
 *
 * @note This setting is only relevant when building HeliOS for Arduino-based
 * platforms. For other platforms, this setting should remain undefined.
 *
 * @note When enabled, ensure arduino.cpp is present in the /src directory to
 * avoid linker errors.
 *
 * @sa CONFIG_SYSTEM_ASSERT_BEHAVIOR
 *
 */
  #if defined(DOXYGEN)
    #if !defined(CONFIG_ENABLE_ARDUINO_CPP_INTERFACE)
      #define CONFIG_ENABLE_ARDUINO_CPP_INTERFACE
    #endif /* if !defined(CONFIG_ENABLE_ARDUINO_CPP_INTERFACE) */
  #endif /* if defined(DOXYGEN) */


/**
 * @brief Define to enable system assertions
 *
 * The CONFIG_ENABLE_SYSTEM_ASSERT setting allows the end-user to enable system
 * assertions in HeliOS. Once enabled, the end-user must also define
 * CONFIG_SYSTEM_ASSERT_BEHAVIOR for there to be an effect. By default, the
 * CONFIG_ENABLE_SYSTEM_ASSERT setting is not defined.
 *
 * When enabled, the kernel will invoke assertion checks at critical points to
 * verify system integrity and catch programming errors. This adds runtime
 * overhead but is invaluable for debugging.
 *
 * @note For production builds, consider disabling this setting to reduce code
 * size and improve performance.
 *
 * @note Requires CONFIG_SYSTEM_ASSERT_BEHAVIOR to be defined to specify the
 * assertion handler behavior.
 *
 * @sa CONFIG_SYSTEM_ASSERT_BEHAVIOR
 * @sa CONFIG_ENABLE_ARDUINO_CPP_INTERFACE
 *
 */
  #if defined(DOXYGEN)
    #if !defined(CONFIG_ENABLE_SYSTEM_ASSERT)
      #define CONFIG_ENABLE_SYSTEM_ASSERT
    #endif /* if !defined(CONFIG_ENABLE_SYSTEM_ASSERT) */
  #endif /* if defined(DOXYGEN) */


/**
 * @brief Define the system assertion behavior
 *
 * The CONFIG_SYSTEM_ASSERT_BEHAVIOR setting allows the end-user to specify the
 * behavior (code) of the assertion which is called when
 * CONFIG_ENABLE_SYSTEM_ASSERT is defined. Typically some sort of output is
 * generated over a serial or other interface. By default the
 * CONFIG_SYSTEM_ASSERT_BEHAVIOR is not defined.
 *
 * This macro-function receives two parameters:
 * - @p f - The filename (const char*) where the assertion occurred
 * - @p l - The line number (int) where the assertion occurred
 *
 * Common implementations include writing to a serial port, toggling an LED,
 * writing to a log file, or entering an infinite loop for debugger attachment.
 *
 * @note In order to use the __ArduinoAssert__() functionality, the
 * CONFIG_ENABLE_ARDUINO_CPP_INTERFACE setting must be enabled.
 *
 * @note This setting has no effect unless CONFIG_ENABLE_SYSTEM_ASSERT is also
 * defined.
 *
 * @sa CONFIG_ENABLE_SYSTEM_ASSERT
 * @sa CONFIG_ENABLE_ARDUINO_CPP_INTERFACE
 *
 * @par Example (Arduino):
 * @code {.c}
 * #define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l) __ArduinoAssert__(f, l)
 * @endcode
 *
 */
  #if defined(DOXYGEN)
    #if !defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR)
      #define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l) __ArduinoAssert__(f, l)
    #endif /* if !defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR) */
  #endif /* if defined(DOXYGEN) */


/**
 * @brief Define the size in bytes of the message queue message value
 *
 * Setting the CONFIG_MESSAGE_VALUE_BYTES allows the end-user to define the size
 * of the message queue message value. The larger the size of the message value,
 * the greater impact there will be on system performance and memory usage. The
 * default size is 8 bytes.
 *
 * This value determines the maximum payload size that can be sent through
 * message queues. Each message in a queue will allocate this amount of memory
 * for its value field. Consider the trade-off between message capacity and
 * memory efficiency when setting this value.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x8u for 8 bytes).
 *
 * @note Smaller values conserve memory but limit the data that can be passed in
 * a single message. Larger values increase flexibility but consume more memory
 * per message.
 *
 * @sa xQueueMessage
 * @sa xQueueSend()
 * @sa xQueueReceive()
 *
 */
  #if !defined(CONFIG_MESSAGE_VALUE_BYTES)
    #define CONFIG_MESSAGE_VALUE_BYTES 0x8u /* 8 */
  #endif /* if !defined(CONFIG_MESSAGE_VALUE_BYTES) */


/**
 * @brief Define the size in bytes of the direct to task notification value
 *
 * Setting the CONFIG_NOTIFICATION_VALUE_BYTES allows the end-user to define the
 * size of the direct to task notification value. The larger the size of the
 * notification value, the greater impact there will be on system performance
 * and memory usage. The default size is 8 bytes.
 *
 * This value determines the maximum payload size for direct-to-task
 * notifications. Direct-to-task notifications are a lightweight alternative to
 * message queues for simple inter-task communication. Each task that can
 * receive notifications will allocate this amount of memory for its
 * notification value.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x8u for 8 bytes).
 *
 * @note Direct-to-task notifications are faster than message queues but limited
 * to one pending notification per task.
 *
 * @sa xTaskNotification
 * @sa xTaskNotify()
 * @sa xTaskNotifyTake()
 *
 */
  #if !defined(CONFIG_NOTIFICATION_VALUE_BYTES)
    #define CONFIG_NOTIFICATION_VALUE_BYTES 0x8u /* 8 */
  #endif /* if !defined(CONFIG_NOTIFICATION_VALUE_BYTES) */


/**
 * @brief Define the size in bytes of the task name
 *
 * Setting the CONFIG_TASK_NAME_BYTES allows the end-user to define the size of
 * the task name. The larger the size of the task name, the greater impact there
 * will be on system performance and memory usage. The default size is 8 bytes.
 *
 * This value determines the maximum length of task names including the null
 * terminator. Each task in the system will allocate this amount of memory for
 * its name string. Task names are useful for debugging and identifying tasks
 * during runtime inspection.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x8u for 8 bytes).
 *
 * @note Task names longer than CONFIG_TASK_NAME_BYTES - 1 characters will be
 * truncated to fit, with the last byte reserved for the null terminator.
 *
 * @note Setting this value too small may make task names difficult to read in
 * debugging output. Setting it too large wastes memory on each task.
 *
 * @sa xTaskInfo
 * @sa xTaskCreate()
 *
 */
  #if !defined(CONFIG_TASK_NAME_BYTES)
    #define CONFIG_TASK_NAME_BYTES 0x8u /* 8 */
  #endif /* if !defined(CONFIG_TASK_NAME_BYTES) */


/**
 * @brief Define the number of memory blocks available in all memory regions
 *
 * The heap memory region is used by tasks, whereas the kernel memory region is
 * used solely by the kernel for kernel objects. The
 * CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS setting allows the end-user to define the
 * size, in blocks, of all memory regions thus affecting both the heap and
 * kernel memory regions. The size of a memory block is defined by the
 * CONFIG_MEMORY_REGION_BLOCK_SIZE setting. The size of all memory regions needs
 * to be adjusted to fit the memory requirements of the end-user's application.
 * The default value is 16 blocks.
 *
 * @par Total Memory Calculation:
 * Total memory per region = CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS *
 * CONFIG_MEMORY_REGION_BLOCK_SIZE
 *
 * Since HeliOS maintains two separate memory regions (heap and kernel), the
 * total system memory usage is:
 * Total system memory = 2 * (CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS *
 * CONFIG_MEMORY_REGION_BLOCK_SIZE)
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x10u for 16 blocks).
 *
 * @note Setting this value too small may result in memory allocation failures.
 * Setting it too large may exceed the available RAM on the target platform.
 *
 * @note This value affects both heap and kernel memory regions equally.
 *
 * @sa CONFIG_MEMORY_REGION_BLOCK_SIZE
 * @sa xMemAlloc()
 * @sa xMemFree()
 *
 */
  #if !defined(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS)
    #define CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS 0x10u /* 16 */
  #endif /* if !defined(CONFIG_MEMORY_REGION_SIZE_IN_BLOCKS) */


/**
 * @brief Define the memory block size in bytes for all memory regions
 *
 * Setting CONFIG_MEMORY_REGION_BLOCK_SIZE allows the end-user to define the
 * size of a memory region block in bytes. The memory region block size should
 * be set to achieve the best possible utilization of the available memory. The
 * CONFIG_MEMORY_REGION_BLOCK_SIZE setting affects both the heap and kernel
 * memory regions. The default value is 32 bytes.
 *
 * This value represents the granularity of memory allocation. All memory
 * allocations will be rounded up to the nearest multiple of this block size.
 * Choosing an appropriate block size is crucial for minimizing internal
 * fragmentation while maintaining efficient memory utilization.
 *
 * @par Tuning Guidelines:
 * - Smaller block sizes (8-16 bytes): Better for many small allocations, less
 * internal fragmentation, but higher overhead from block management
 * - Medium block sizes (32-64 bytes): Good general-purpose balance
 * - Larger block sizes (128+ bytes): Better for large allocations, lower
 * management overhead, but more internal fragmentation for small allocations
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x20u for 32 bytes).
 *
 * @note Consider using a power-of-2 value for optimal memory alignment on most
 * architectures.
 *
 * @note This value should be chosen based on the typical size of objects
 * allocated by your application.
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
 * MINIMUM length limit a message queue can be created with using
 * xQueueCreate(). When a message queue length equals its limit, the message
 * queue will be considered full and return true when xQueueIsQueueFull() is
 * called. A full queue will also not accept messages from xQueueSend(). The
 * default value is 5.
 *
 * This setting enforces a minimum queue depth to prevent creating queues that
 * are too small to be useful. Attempts to create queues with a limit smaller
 * than this value will result in the queue being created with this minimum
 * limit instead.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x5u for 5 messages).
 *
 * @note Each message in a queue consumes CONFIG_MESSAGE_VALUE_BYTES of memory,
 * so larger minimum limits increase the minimum memory cost per queue.
 *
 * @sa xQueueIsQueueFull()
 * @sa xQueueSend()
 * @sa xQueueCreate()
 * @sa CONFIG_MESSAGE_VALUE_BYTES
 *
 */
  #if !defined(CONFIG_QUEUE_MINIMUM_LIMIT)
    #define CONFIG_QUEUE_MINIMUM_LIMIT 0x5u /* 5 */
  #endif /* if !defined(CONFIG_QUEUE_MINIMUM_LIMIT) */


/**
 * @brief Define the length of the stream buffer
 *
 * Setting CONFIG_STREAM_BUFFER_BYTES will define the length of stream buffers
 * created by xStreamCreate(). When the length of the stream buffer reaches this
 * value, it is considered full and can no longer be written to by calling
 * xStreamSend(). The default value is 32 bytes.
 *
 * Stream buffers provide a byte-oriented data transfer mechanism, suitable for
 * streaming data between tasks or from interrupt handlers to tasks. Unlike
 * message queues which handle discrete messages, stream buffers handle
 * continuous byte streams.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x20u for 32 bytes).
 *
 * @note Each stream buffer instance will allocate this amount of memory for its
 * internal buffer storage.
 *
 * @note Stream buffers are ideal for serial port data, sensor readings, or any
 * continuous data flow where message boundaries are not important.
 *
 * @sa xStreamCreate()
 * @sa xStreamSend()
 * @sa xStreamReceive()
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
 * Task watchdog timers provide a mechanism to detect and respond to tasks that
 * have stopped executing or become unresponsive. When enabled, tasks can be
 * monitored to ensure they execute within expected time bounds. This is
 * particularly useful for safety-critical applications or systems that require
 * high reliability.
 *
 * @note Enabling watchdog timers adds a small amount of runtime overhead for
 * timer management and checking.
 *
 * @note When disabled (by not defining this macro), watchdog timer functions
 * will not be available, reducing code size and eliminating the associated
 * overhead.
 *
 * @note To disable this feature, comment out or remove the definition of
 * CONFIG_TASK_WD_TIMER_ENABLE.
 *
 * @sa xTaskStartWatchdog()
 * @sa xTaskResetWatchdog()
 * @sa xTaskStopWatchdog()
 *
 */
  #if !defined(CONFIG_TASK_WD_TIMER_ENABLE)
    #define CONFIG_TASK_WD_TIMER_ENABLE
  #endif /* if !defined(CONFIG_TASK_WD_TIMER_ENABLE) */


/**
 * @brief Define the length of a device driver name
 *
 * Setting CONFIG_DEVICE_NAME_BYTES will define the length of a device driver
 * name. The name of device drivers should be exactly this length. There really
 * isn't a reason to change this and doing so may break existing device drivers.
 * The default length is 8 bytes.
 *
 * Device driver names are used to identify and access device drivers within the
 * system. This value determines the maximum length of these names including the
 * null terminator.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x8u for 8 bytes).
 *
 * @warning Changing this value may cause compatibility issues with existing
 * device drivers that expect the standard 8-byte name length. Only modify this
 * setting if you are also updating all device drivers in your system.
 *
 * @warning Device driver names in the HeliOS ecosystem are standardized to 8
 * bytes. Changing this value may break third-party or built-in drivers.
 *
 * @note It is strongly recommended to keep this value at its default of 8 bytes
 * unless you have a specific requirement and are maintaining all device drivers
 * yourself.
 *
 */
  #if !defined(CONFIG_DEVICE_NAME_BYTES)
    #define CONFIG_DEVICE_NAME_BYTES 0x8u /* 8 */
  #endif /* if !defined(CONFIG_DEVICE_NAME_BYTES) */
#endif /* ifndef CONFIG_H_ */