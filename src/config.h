/*
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
 */
#ifndef CONFIG_H_
#define CONFIG_H_

#define TASKNAME_SIZE 16
#define TNOTIFYVALUE_SIZE 16
#define MEMALLOCTABLE_SIZE 100
#define QUEUE_MINIMUM_LIMIT 5
#define OTHER_ARCH_LINUX
/*
 * Un-comment to compile for Linux or Microsoft
 * Windows.
 * #define OTHER_ARCH_LINUX
 * #define OTHER_ARCH_WINDOWS
 */

#endif