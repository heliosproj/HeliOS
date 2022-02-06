/**
 * @file config.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Header file for configurable settings in HelIOS
 * @version 0.3.0
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
to customize HeliOS for their specific use. */

/**
 * @brief Define the size in bytes of the message queue message value.
 * 
 * Setting the CONFIG_MESSAGE_VALUE_BYTES allows the end-user to define
 * the size of the message queue message value. The larger the size of the
 * message value, the greater impact there will be on system performance.
 * The default size is 16.
 * 
 * @sa xQueueMessage
 * 
 */
#define CONFIG_MESSAGE_VALUE_BYTES 16

/**
 * @brief Define the size in bytes of the direct to task notification value.
 * 
 * Setting the CONFIG_NOTIFICATION_VALUE_BYTES allows the end-user to define
 * the size of the direct to task notification value. The larger the size of the
 * notification value, the greater impact there will be on system performance.
 * The default size is 16.
 * 
 * @sa xTaskNotification
 */
#define CONFIG_NOTIFICATION_VALUE_BYTES 16

/**
 * @brief Define the size in bytes of the ASCII task name.
 * 
 * Setting the CONFIG_TASK_NAME_BYTES allows the end-user to define
 * the size of the ASCII task name. The larger the size of the task
 * name, the greater impact there will be on system performance.
 * The default size is 16.
 * 
 * @sa xTaskInfo
 * 
 */
#define CONFIG_TASK_NAME_BYTES 16

/**
 * @brief Define the number of entries in the dynamic memory allocation table.
 * 
 * Setting CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES allows the end-user to define
 * the size of the table that is used to track dynamic memory allocated and freed
 * by xMemAlloc() and xMemFree(). The larger the table, the greater the impact
 * there will be on system performance. However, if the table is too small,
 * xMemAlloc() may exhaust the available table entries and fail to allocate
 * the requested memory.
 * 
 * @sa xMemAlloc()
 * @sa xMemFree()
 * 
 */
#define CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES 100

/**
 * @brief Define the minimum value for a message queue limit.
 * 
 * Setting the CONFIG_QUEUE_MINIMUM_LIMIT allows the end-user to define
 * the MINIMUM value a message queue can be created with xQueueCreate().
 * When a message queue length equals its limit, the message queue will
 * be considered full and return true when xQueueIsQueueFull() is called.
 * A full queue will also not accept messages from xQueueSend().
 * 
 * @sa xQueueIsQueueFull()
 * @sa xQueueSend()
 * @sa xQueueCreate()
 * 
 */
#define CONFIG_QUEUE_MINIMUM_LIMIT 5
#endif