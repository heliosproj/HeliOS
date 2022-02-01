/**
 * @file config.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Header file for configurable settings in HelIOS
 * @version 0.3.0
 * @date 2022-01-31
 * 
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

/* Define the size in bytes of the queue message value. */
#define CONFIG_MESSAGE_VALUE_BYTES 16

/* Define the size in bytes of the task notification value. */
#define CONFIG_NOTIFICATION_VALUE_BYTES 16

/* Define the size in bytes of the task name. */
#define CONFIG_TASK_NAME_BYTES 16

/* Define the number of entries in the dynamic memory allocation table. */
#define CONFIG_DYNAMIC_MEMORY_ALLOC_TABLE_ENTRIES 100

/* When the messages in a message queue reach the limit set when the queue is
created, the queue is considered full. This defines the minimum value for
"limit" when a queue is created. */
#define CONFIG_QUEUE_MININUM_LIMIT 5
#endif