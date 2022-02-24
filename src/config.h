/**
 * @file config.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel header file for user definable settings
 * @version 0.3.1
 * @date 2022-01-31
 * 
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
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
#ifndef CONFIG_H_
#define CONFIG_H_

/* The following configurable settings may be changed by the end-user
to customize the HeliOS kernel for their specific application. */

/**
 * @brief Define to enable system assertions.
 * 
 * The CONFIG_ENABLE_SYSTEM_ASSERT setting allows the end-user to
 * enable system assertions in HeliOS. Once enabled, the end-user
 * must define CONFIG_SYSTEM_ASSERT_BEHAVIOR for there
 * to be an effect.
 * 
 * @sa xSystemAssert()
 * 
 */
#define CONFIG_ENABLE_SYSTEM_ASSERT

/**
 * @brief Define the system assertion behavior.
 * 
 * The CONFIG_SYSTEM_ASSERT_BEHAVIOR setting allows the end-user
 * to specify the behavior (code) of xSystemAssert() which is called
 * when CONFIG_ENABLE_SYSTEM_ASSERT is defined. Typically some sort
 * of output is generated over a serial or other interface.
 * 
 * @sa CONFIG_ENABLE_SYSTEM_ASSERT
 * @sa xSystemAssert()
 * 
 * @code {.c}
 * #define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l) printf("assert %s:%d\n", f, l)
 * @endcode
 * 
 * 
 */
#define CONFIG_SYSTEM_ASSERT_BEHAVIOR(f, l)

/**
 * @brief Define the size in bytes of the message queue message value.
 * 
 * Setting the CONFIG_MESSAGE_VALUE_BYTES allows the end-user to define
 * the size of the message queue message value. The larger the size of the
 * message value, the greater impact there will be on system performance.
 * The default size is 16 bytes. The literal must be appended with "u" to
 * maintain MISRA C:2012 compliance.
 * 
 * @sa xQueueMessage
 * 
 */
#define CONFIG_MESSAGE_VALUE_BYTES 16u

/**
 * @brief Define the size in bytes of the direct to task notification value.
 * 
 * Setting the CONFIG_NOTIFICATION_VALUE_BYTES allows the end-user to define
 * the size of the direct to task notification value. The larger the size of the
 * notification value, the greater impact there will be on system performance.
 * The default size is 16 bytes. The literal must be appended with "u" to
 * maintain MISRA C:2012 compliance.
 * 
 * @sa xTaskNotification
 */
#define CONFIG_NOTIFICATION_VALUE_BYTES 16u

/**
 * @brief Define the size in bytes of the ASCII task name.
 * 
 * Setting the CONFIG_TASK_NAME_BYTES allows the end-user to define
 * the size of the ASCII task name. The larger the size of the task
 * name, the greater impact there will be on system performance.
 * The default size is 16 bytes. The literal must be appended with "u"
 * to maintain MISRA C:2012 compliance.
 * 
 * @sa xTaskInfo
 * 
 */
#define CONFIG_TASK_NAME_BYTES 16u

/**
 * @brief Define the number of blocks in the heap.
 * 
 * Setting CONFIG_HEAP_SIZE_IN_BLOCKS allows the end-user to
 * define the size of the heap in blocks. The size of a block
 * in the heap is determined by the CONFIG_HEAP_BLOCK_SIZE which
 * is represented in bytes. The size of the heap needs to be
 * adjusted to fit the memory requirements of the end-user's
 * application. The default value is 512 blocks. The literal
 * must be appended with "u" to maintain MISRA C:2012 compliance.
 * 
 * @sa xMemAlloc()
 * @sa xMemFree()
 * @sa CONFIG_HEAP_BLOCK_SIZE
 * 
 * @note To use the platform and/or tool-chain defaults,
 * leave CONFIG_HEAP_SIZE_IN_BLOCKS undefined.
 * 
 */
#define CONFIG_HEAP_SIZE_IN_BLOCKS 512u

/**
 * @brief Define the heap block size in bytes.
 * 
 * Setting CONFIG_HEAP_BLOCK_SIZE allows the end-user to
 * define the size of a heap block in bytes. The block size
 * should be set to achieve the best possible utilization
 * of the heap. A block size that is too large will waste the
 * heap for smaller requests for heap. A block size that is too small
 * will waste heap on entries. The default value is 32 bytes. The
 * literal must be appended with "u" to maintain MISRA C:2012 compliance.
 * 
 * @sa xMemAlloc()
 * @sa xMemFree()
 * @sa CONFIG_HEAP_SIZE_IN_BLOCKS
 * 
 */
#define CONFIG_HEAP_BLOCK_SIZE 32u

/**
 * @brief Define the minimum value for a message queue limit.
 * 
 * Setting the CONFIG_QUEUE_MINIMUM_LIMIT allows the end-user to define
 * the MINIMUM length limit a message queue can be created with xQueueCreate().
 * When a message queue length equals its limit, the message queue will
 * be considered full and return true when xQueueIsQueueFull() is called.
 * A full queue will also not accept messages from xQueueSend(). The default
 * value is 5. The literal must be appended with "u" to maintain MISRA C:2012
 * compliance.
 * 
 * @sa xQueueIsQueueFull()
 * @sa xQueueSend()
 * @sa xQueueCreate()
 * 
 */
#define CONFIG_QUEUE_MINIMUM_LIMIT 5u
#endif