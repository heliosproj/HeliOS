/*
 * HeliOS Embedded Operating System
 * Copyright (C) 2020 Manny Peterson <me@mannypeterson.com>
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

#include <stdio.h>
#include "HeliOS.h"
#include "list.h"
#include "mem.h"
#include "task.h"

void task(int id_) {
  printf("task() ran.\n");
}


int main(int argc, char *argv[]) {
  xHeliOSSetup();

  int id = 0;

  id = xTaskAdd("TASK", &task);
  xTaskWait(id);

  xTaskSetTimer(id, 1000000);

  while (1)
    xHeliOSLoop();

  return 0;
}
