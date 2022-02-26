/**
 * @file Timer.ino
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Example code to demonstrate event driven multitasking using a task timer
 * @version 0.3.2
 * @date 2022-02-14
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

#include <HeliOS.h>


void taskPrint_main(xTask task_, xTaskParm parm_) {

  String str = "taskPrint_main(): one second has passed.";

  Serial.println(str);
}

void setup() {

  Serial.begin(9600);

  /* Create a task to demonstrate event driven multitasking using
  a task timer. */
  xTask task = xTaskCreate("PRINT", taskPrint_main, NULL);

  /* Check to make sure the task was created by xTaskCreate() before
  attempting to use the task. */
  if (task) {

    /* Place the task in the waiting state. */
    xTaskWait(task);

    /* Set the task timer to one second. */
    xTaskChangePeriod(task, 1000000);

    /* Pass control to the HeliOS scheduler. The HeliOS scheduler will
    not relinquish control unless xTaskSuspendAll() is called. */
    xTaskStartScheduler();

    /* If the scheduler relinquishes control, do some clean-up by
    deleting the task. */
    xTaskDelete(task);
  }

  /* Halt the system. Once called, the system must be reset to
  recover. */
  xSystemHalt();
}

void loop() {
  /* The loop function is not used and should remain empty. */
}