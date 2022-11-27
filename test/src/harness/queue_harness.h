/**
 * @file queue_harness.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.3.6
 * @date 2022-08-27
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
#ifndef QUEUE_HARNESS_H_
#define QUEUE_HARNESS_H_

#include "config.h"
#include "defines.h"
#include "types.h"
#include "port.h"
#include "device.h"
#include "mem.h"
#include "queue.h"
#include "stream.h"
#include "sys.h"
#include "task.h"
#include "timer.h"

#include "unit.h"

#ifdef __cplusplus
extern "C" {
#endif

void queue_harness(void);

#ifdef __cplusplus
}
#endif
#endif