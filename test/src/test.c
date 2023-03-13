/*UNCRUSTIFY-OFF*/
/**
 * @file test.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code for HeliOS unit testing
 * @version 0.4.0
 * @date 2022-08-23
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
#include "test.h"


int main(int argc, char **argv) {
  unit_init();
  reset();
  memory_1_harness();
  reset();
  memory_2_harness();
  reset();
  queue_harness();
  reset();
  timer_harness();
  reset();
  task_harness();
  reset();
  stream_harness();
  reset();
  device_harness();
  unit_exit();

  return(0);
}


void reset(void) {
  __MemoryClear__();
  __SysStateClear__();
  __TaskStateClear__();
  __DeviceStateClear__();

  return;
}