/**
 * @file test.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.4.0
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
#ifndef TEST_H_
#define TEST_H_

#include "unit.h"
#include "memory_harness.h"
#include "queue_harness.h"
#include "task_harness.h"
#include "timer_harness.h"
#include "stream_harness.h"
#include "device_harness.h"

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char **argv);
void reset(void);

#ifdef __cplusplus
}
#endif
#endif