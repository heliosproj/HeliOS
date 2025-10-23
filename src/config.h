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
 * @sa QueueMessage_t
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
 * @sa TaskNotification_t
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
 * @sa TaskInfo_t
 * @sa xTaskCreate()
 *
 */
  #if !defined(CONFIG_TASK_NAME_BYTES)
    #define CONFIG_TASK_NAME_BYTES 0x8u /* 8 */
  #endif /* if !defined(CONFIG_TASK_NAME_BYTES) */


/**
 * @brief Define the memory region size in bytes for all memory regions
 *
 * The heap memory region is used by tasks, whereas the kernel memory region is
 * used solely by the kernel for kernel objects. CONFIG_MEMORY_REGION_SIZE
 * allows the end-user to define the size, in bytes, of each memory region (heap
 * and kernel). The size of all memory regions needs to be adjusted to fit the
 * memory requirements of the end-user's application.
 *
 * The memory implementation uses variable-sized blocks with headers, allowing
 * efficient allocation of different sized objects without fixed block overhead.
 * Each allocation includes a BlockHeader_t structure (typically 16-24 bytes
 * depending on architecture) for metadata and integrity checking.
 *
 * @par Memory Usage:
 * Since HeliOS maintains two separate memory regions (heap and kernel), the
 * total system memory usage is:
 * Total system memory = 2 * CONFIG_MEMORY_REGION_SIZE
 *
 * @par Tuning Guidelines:
 * - Embedded systems with limited RAM: 0x1000 - 0x4000 (4KB - 16KB)
 * - Small microcontrollers: 0x4000 - 0x10000 (16KB - 64KB)
 * - Larger embedded systems: 0x10000 - 0x40000 (64KB - 256KB)
 * - Systems with external RAM: 0x40000+ (256KB+)
 *
 * The default value is 0x10000 (64KB) which provides a good balance for typical
 * embedded applications.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x10000u for 64KB).
 *
 * @warning Reducing this value below the minimum requirements of your
 * application will lead to memory allocation failures.
 *
 * @note Increasing this value will increase the memory footprint of HeliOS,
 * which may be limited on resource-constrained systems.
 *
 * @note This value affects both heap and kernel memory regions equally.
 *
 * @sa CONFIG_MEMORY_MINIMUM_BLOCK_SIZE
 * @sa xMemAlloc()
 * @sa xMemFree()
 *
 */
  #if !defined(CONFIG_MEMORY_REGION_SIZE)
    #define CONFIG_MEMORY_REGION_SIZE 0x10000u /* 64KB default */
  #endif /* if !defined(CONFIG_MEMORY_REGION_SIZE) */


/**
 * @brief Define the minimum block size to prevent fragmentation
 *
 * Setting CONFIG_MEMORY_MINIMUM_BLOCK_SIZE determines the smallest memory
 * fragment that will be created during block splitting. When allocating memory,
 * if the remaining space after allocation would be smaller than this threshold
 * plus the block header size, the entire block is allocated instead of being
 * split. This prevents the creation of tiny, unusable memory fragments that
 * increase fragmentation and reduce allocator efficiency.
 *
 * The default value is 32 bytes, which provides a good balance between memory
 * utilization and fragmentation prevention. Increasing this value reduces
 * fragmentation but may waste more memory. Decreasing it allows tighter memory
 * packing but increases fragmentation overhead.
 *
 * @note This value should be at least as large as the smallest typical
 * allocation size in your application.
 *
 * @sa CONFIG_MEMORY_REGION_BLOCK_SIZE
 * @sa xMemAlloc()
 * @sa xMemFree()
 *
 */
  #if !defined(CONFIG_MEMORY_MINIMUM_BLOCK_SIZE)
    #define CONFIG_MEMORY_MINIMUM_BLOCK_SIZE 0x20u /* 32 bytes */
  #endif /* if !defined(CONFIG_MEMORY_MINIMUM_BLOCK_SIZE) */


/**
 * @brief Define memory alignment requirement for allocated memory
 *
 * Setting CONFIG_MEMORY_ALIGNMENT specifies the byte alignment requirement for
 * memory addresses returned by xMemAlloc(). This is critical for architectures
 * that require aligned memory access (ARM, RISC-V, MIPS, etc.) and for
 * optimizing cache performance.
 *
 * Common alignment requirements:
 * - 4 bytes: Minimum for 32-bit architectures
 * - 8 bytes: Standard for 64-bit architectures and double precision floats
 * - 16 bytes: Required for SIMD operations (SSE, NEON, etc.)
 *
 * The alignment must be a power of 2. The default value is 8 bytes, which
 * provides compatibility with most data types on both 32-bit and 64-bit
 * systems.
 *
 * @par Performance Impact:
 * Proper alignment can significantly improve memory access performance by:
 * - Avoiding unaligned access penalties (2-3x slower on some architectures)
 * - Preventing bus errors on strict alignment architectures
 * - Improving cache line utilization
 * - Enabling compiler vectorization optimizations
 *
 * @note On architectures with strict alignment requirements (e.g., ARM
 * Cortex-M0), misaligned access will cause a hard fault. Always use at least
 * 4-byte alignment.
 *
 * @warning The alignment value must be a power of 2 (4, 8, 16, etc.)
 *
 * @sa xMemAlloc()
 * @sa CONFIG_MEMORY_MINIMUM_BLOCK_SIZE
 *
 */
  #if !defined(CONFIG_MEMORY_ALIGNMENT)
    #define CONFIG_MEMORY_ALIGNMENT 0x8u /* 8-byte alignment */
  #endif /* if !defined(CONFIG_MEMORY_ALIGNMENT) */


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


/**
 * @brief Define the block device UID for filesystem operations
 *
 * Setting CONFIG_FS_BLOCK_DEVICE_UID specifies which block device the
 * filesystem will use for all operations. This UID must match a registered
 * block device driver in the system. The default is 0x1000u (BLOCKDEV).
 *
 * The filesystem uses this device for mounting, formatting, reading, and
 * writing file data. The block device must be properly initialized and
 * configured before filesystem operations can succeed.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x1000u).
 *
 * @note The block device must be registered and initialized before calling
 * xFSMount() or xFSFormat().
 *
 * @sa xFSMount()
 * @sa xFSFormat()
 *
 */
  #if !defined(CONFIG_FS_BLOCK_DEVICE_UID)
    #define CONFIG_FS_BLOCK_DEVICE_UID 0x1000u /* BLOCKDEV */
  #endif /* if !defined(CONFIG_FS_BLOCK_DEVICE_UID) */


/**
 * @brief Enable the console subsystem
 *
 * Defining CONFIG_ENABLE_CONSOLE will enable the interactive console feature.
 * When enabled, a console task will automatically start when the scheduler
 * begins, providing a UNIX-like command-line interface over a character device.
 * The default is disabled.
 *
 * The console provides interactive access to system diagnostics, filesystem
 * operations, task management, and memory statistics through a minimal shell.
 *
 * @note Enabling the console requires a properly configured character device
 * driver as specified by CONFIG_CONSOLE_DEVICE_UID.
 *
 * @note The console task will only start after xStartScheduler() is called.
 *
 * @note To disable this feature, comment out or remove the definition of
 * CONFIG_ENABLE_CONSOLE.
 *
 * @sa CONFIG_CONSOLE_DEVICE_UID
 * @sa CONFIG_CONSOLE_TASK_PRIORITY
 * @sa CONFIG_CONSOLE_TASK_MODE
 *
 */
  #if defined(DOXYGEN)
    #if !defined(CONFIG_ENABLE_CONSOLE)
      #define CONFIG_ENABLE_CONSOLE
    #endif /* if !defined(CONFIG_ENABLE_CONSOLE) */
  #endif /* if defined(DOXYGEN) */


/**
 * @brief Define the character device UID for console operations
 *
 * Setting CONFIG_CONSOLE_DEVICE_UID specifies which character device the
 * console will use for input and output. This UID must match a registered
 * character device driver in the system. The default is 0x2000u (CHARDEV0).
 *
 * The console requires a bidirectional character device, typically a UART,
 * USART, or USB CDC virtual COM port, to communicate with the user.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x2000u).
 *
 * @note The character device must be registered and initialized before the
 * scheduler starts for the console to function properly.
 *
 * @note This setting only has effect when CONFIG_ENABLE_CONSOLE is defined.
 *
 * @sa CONFIG_ENABLE_CONSOLE
 *
 */
  #if !defined(CONFIG_CONSOLE_DEVICE_UID)
    #define CONFIG_CONSOLE_DEVICE_UID 0x2000u /* CHARDEV0 */
  #endif /* if !defined(CONFIG_CONSOLE_DEVICE_UID) */


/**
 * @brief Define the console task priority
 *
 * Setting CONFIG_CONSOLE_TASK_PRIORITY specifies the scheduling priority for
 * the console task. Lower values indicate higher priority. The default is 5
 * (medium-low priority) as the console is typically an interactive,
 * non-critical task.
 *
 * The console task priority determines when it receives CPU time relative to
 * other tasks in the system. For interactive responsiveness, a moderate
 * priority is recommended. For systems where console is primarily diagnostic, a
 * lower priority may be appropriate.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x5u for priority 5).
 *
 * @note This setting only has effect when CONFIG_ENABLE_CONSOLE is defined.
 *
 * @sa CONFIG_ENABLE_CONSOLE
 * @sa CONFIG_CONSOLE_TASK_MODE
 *
 */
  #if !defined(CONFIG_CONSOLE_TASK_PRIORITY)
    #define CONFIG_CONSOLE_TASK_PRIORITY 0x5u /* 5 - medium-low priority */
  #endif /* if !defined(CONFIG_CONSOLE_TASK_PRIORITY) */


/**
 * @brief Define the console task scheduling mode
 *
 * Setting CONFIG_CONSOLE_TASK_MODE specifies whether the console task runs
 * continuously with normal task scheduling or event-driven via a timer. Valid
 * values:
 * - 0: Continuous mode - task runs every scheduler iteration
 * - 1: Event-driven mode - task runs on timer events only
 *
 * The default is 1 (event-driven) to reduce CPU usage when the console is idle.
 *
 * @note The value should be 0 or 1.
 *
 * @note In event-driven mode, the timer period is set by
 * CONFIG_CONSOLE_TIMER_PERIOD_MS.
 *
 * @note This setting only has effect when CONFIG_ENABLE_CONSOLE is defined.
 *
 * @sa CONFIG_ENABLE_CONSOLE
 * @sa CONFIG_CONSOLE_TIMER_PERIOD_MS
 *
 */
  #if !defined(CONFIG_CONSOLE_TASK_MODE)
    #define CONFIG_CONSOLE_TASK_MODE 0x1u /* 1 - event-driven */
  #endif /* if !defined(CONFIG_CONSOLE_TASK_MODE) */


/**
 * @brief Define the console timer period in milliseconds
 *
 * Setting CONFIG_CONSOLE_TIMER_PERIOD_MS specifies how often the console task
 * runs when in event-driven mode (CONFIG_CONSOLE_TASK_MODE = 1). The default is
 * 50ms, providing responsive interactive performance while minimizing CPU
 * overhead. The default is 50 milliseconds.
 *
 * This value represents the polling interval for checking user input. Lower
 * values provide more responsive input but increase CPU usage. Higher values
 * reduce overhead but may make the console feel sluggish.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x32u for 50 milliseconds).
 *
 * @note This setting only has effect when CONFIG_ENABLE_CONSOLE is defined and
 * CONFIG_CONSOLE_TASK_MODE is set to 1 (event-driven).
 *
 * @note Typical values range from 10ms (very responsive) to 100ms (low
 * overhead).
 *
 * @sa CONFIG_ENABLE_CONSOLE
 * @sa CONFIG_CONSOLE_TASK_MODE
 *
 */
  #if !defined(CONFIG_CONSOLE_TIMER_PERIOD_MS)
    #define CONFIG_CONSOLE_TIMER_PERIOD_MS 0x32u /* 50 milliseconds */
  #endif /* if !defined(CONFIG_CONSOLE_TIMER_PERIOD_MS) */


/**
 * @brief Define the maximum command line length
 *
 * Setting CONFIG_CONSOLE_MAX_COMMAND_LENGTH specifies the maximum number of
 * characters that can be entered in a single command line, including arguments.
 * The default is 80 characters, matching traditional terminal line lengths.
 *
 * This buffer size limits the total input line. Commands exceeding this length
 * will be rejected or truncated. Consider increasing this value for scripts or
 * commands with long file paths.
 *
 * @note The value should be set as a hexadecimal constant with the 'u' suffix
 * (e.g., 0x50u for 80 characters).
 *
 * @note Larger values increase RAM usage for the input buffer.
 *
 * @note This setting only has effect when CONFIG_ENABLE_CONSOLE is defined.
 *
 * @sa CONFIG_ENABLE_CONSOLE
 *
 */
  #if !defined(CONFIG_CONSOLE_MAX_COMMAND_LENGTH)
    #define CONFIG_CONSOLE_MAX_COMMAND_LENGTH 0x50u /* 80 characters */
  #endif /* if !defined(CONFIG_CONSOLE_MAX_COMMAND_LENGTH) */


/**
 * @brief Enable character echo by default
 *
 * Defining CONFIG_CONSOLE_ECHO_ENABLED will cause the console to echo typed
 * characters back to the user terminal by default. This can be toggled at
 * runtime through the console. The default is enabled for typical interactive
 * terminal use.
 *
 * Echo allows users to see what they are typing. This is standard behavior for
 * interactive terminals but may be undesirable for password entry or automated
 * input.
 *
 * @note This setting only defines the initial/default state. Echo can be
 * toggled on and off through console commands during runtime.
 *
 * @note This setting only has effect when CONFIG_ENABLE_CONSOLE is defined.
 *
 * @note To disable echo by default, comment out or remove the definition of
 * CONFIG_CONSOLE_ECHO_ENABLED.
 *
 * @sa CONFIG_ENABLE_CONSOLE
 *
 */
  #if !defined(CONFIG_CONSOLE_ECHO_ENABLED)
    #define CONFIG_CONSOLE_ECHO_ENABLED
  #endif /* if !defined(CONFIG_CONSOLE_ECHO_ENABLED) */


/**
 * @brief Define the console command prompt
 *
 * Setting CONFIG_CONSOLE_PROMPT specifies the string displayed as the command
 * prompt. The default is "$" to match UNIX shell conventions.
 *
 * The prompt is displayed after the console banner and after each command
 * completes, indicating the console is ready for input.
 *
 * @note The value should be a string literal (e.g., "$ " or "HeliOS> ").
 *
 * @note Keep the prompt short to conserve screen space and bandwidth.
 *
 * @note This setting only has effect when CONFIG_ENABLE_CONSOLE is defined.
 *
 * @sa CONFIG_ENABLE_CONSOLE
 *
 */
  #if !defined(CONFIG_CONSOLE_PROMPT)
    #define CONFIG_CONSOLE_PROMPT "$ "
  #endif /* if !defined(CONFIG_CONSOLE_PROMPT) */
#endif /* ifndef CONFIG_H_ */