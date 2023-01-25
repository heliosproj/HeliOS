/**
 * @file Blink.ino
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Blink example code for Arduino
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


/* Include the HeliOS header, do not include
any other HeliOS header. */
#include <HeliOS.h>

/* Define the task's main function. The definition must
include the xTask and xTaskParm parameters. */
void blinkTask_main(xTask task_, xTaskParm parm_) {

  /* Dereference the task parameter and store its value
  in a local integer. This integer contains the state
  of the LED (i.e., on or off). */
  int ledState = DEREF_TASKPARM(int, parm_);

  if (ledState) {
    digitalWrite(LED_BUILTIN, HIGH);

    ledState = 0;
  } else {
    digitalWrite(LED_BUILTIN, LOW);

    ledState = 1;
  }

  /* Dereference the task parameter to update its
  value. The task's main function will receive this
  value next time the task's main function is called
  by the scheduler. */
  DEREF_TASKPARM(int, parm_) = ledState;

  return;
}

void setup() {

  int ledState = 0;

  pinMode(LED_BUILTIN, OUTPUT);

  /* Call xSystemInit() to initialize any interrupt handlers and/or
  memory required by HeliOS to execute on the target platform/architecture. */
  xSystemInit();

  /* Create a new HeliOS task, give it an ASCII name, a reference to
  the task's main function and a reference to the task's parameter - in
  this case the state of the LED. */
  xTask blink = xTaskCreate("BLINK", blinkTask_main, &ledState);

  /* Check to make sure the task was created by xTaskCreate() before
  attempting to use the task. */
  if (blink) {

    /* Place the task in the "waiting" state so it will respond to task
    events. */
    xTaskWait(blink);

    /* Set the task timer period to 1,000 ticks. The HeliOS scheduler
    will execute the task every 1,000 ticks until the task is either suspended,
    its task timer period is changed or the task is deleted. */
    xTaskChangePeriod(blink, 1000);

    /* Pass control to the HeliOS scheduler. The HeliOS scheduler will
    not relinquish control unless xTaskSuspendAll() is called. */
    xTaskStartScheduler();


    /* If the scheduler relinquishes control, do some clean-up by
    deleting the task. */
    xTaskDelete(blink);
  }

  /* Halt the system. Once called, the system must be reset to
  recover. */
  xSystemHalt();
}

void loop() {
  /* The loop function is not used and should remain empty. */
}