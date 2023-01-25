/**
 * @file Coop.ino
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Example code for cooperative multitasking
 * @version 0.4.0
 * @date 2022-02-14
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

#include <HeliOS.h>

/*
 * The task definition for short running task which
 * delay for 10 milliseconds. Do not actually use delay()
 * in your applications using HeliOS. This is strictly
 * for demonstration purposes.
 */
void taskShort_main(xTask task_, xTaskParm parm_) {
  delay(10);

  Serial.print("S");
}

/*
 * The task definition for short running task which
 * delay for 60 milliseconds. Do not actually use delay()
 * in your applications using HeliOS. This is strictly
 * for demonstration purposes.
 */
void taskLong_main(xTask task_, xTaskParm parm_) {
  delay(60);

  Serial.println("L");
}

void setup() {

  Serial.begin(9600);

  /* Call xSystemInit() to initialize any interrupt handlers and/or
  memory required by HeliOS to execute on the target platform/architecture. */
  xSystemInit();

  /* Create a new HeliOS task, give it an ASCII name, a reference to
  the task's main function. Since no task paramater is desired,
  simply set the task paramater argument to NULL. */
  xTask shortTask = xTaskCreate("SHORT", taskShort_main, NULL);
  xTask longTask = xTaskCreate("LONG", taskLong_main, NULL);


  /* Check to make sure the task was created by xTaskCreate() before
  attempting to use the task. */
  if (shortTask && longTask) {

    /* Start the short running and long running tasks. */
    xTaskResume(shortTask);

    xTaskResume(longTask);

    /* Pass control to the HeliOS scheduler. The HeliOS scheduler will
    not relinquish control unless xTaskSuspendAll() is called. */
    xTaskStartScheduler();


    /* If the scheduler relinquishes control, do some clean-up by
    deleting the task. */
    xTaskDelete(shortTask);

    xTaskDelete(longTask);
  }

  /* Halt the system. Once called, the system must be reset to
  recover. */
  xSystemHalt();
}

void loop() {
  /* The loop function is not used and should remain empty. */
}