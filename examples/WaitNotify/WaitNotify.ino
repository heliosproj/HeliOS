/**
 * @file WaitNotify.ino
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Example code to demonstrate direct-to-task notifications used in event-driven multitasking
 * @version 0.3.5
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

/* Define a task that will send the direct-to-task notification. This task
will be scheduled as event driven using a task timer to execute
the task every one second. */
void taskSender_main(xTask task_, xTaskParm parm_) {

  /* Get the task handle for the task that will receive the
  direct-to-task notification. */
  xTask receiver = xTaskGetHandleByName("RECEIVER");

  /* Check to make sure the handle was found. */
  if (receiver) {

    /* Send the direct-to-task notification of "HELLO"
    which is five bytes. */
    xTaskNotifyGive(receiver, 5, "HELLO");
  }
}

/* Create a task that will receive the direct-to-task notification
from the sender task. */
void taskReceiver_main(xTask task_, xTaskParm parm_) {

  String str = "";

  /* Receive the direct-to-task notification if one
  is waiting. */
  xTaskNotification notif = xTaskNotifyTake(task_);

  /* Check if xTaskNotifyTake() returned a direct-to-task
  notification. If it is NULl then there likely isn't
  a waiting notification. */
  if (notif) {

    /* Grab the notification value. It is important
    to note that the notification value is not
    a NULL terminated string. So... */
    str += notif->notificationValue;

    Serial.println(str);

    /* Free the heap memory allocated for the direct-to-task
    notification. */
    xMemFree(notif);
  }
}

void setup() {

  Serial.begin(9600);

  /* Call xSystemInit() to initialize any interrupt handlers and/or
  memory required by HeliOS to execute on the target platform/architecture. */
  xSystemInit();

  /* Create two tasks, one to send the direct-to-task notification, the
  other to receive it. */
  xTask sender = xTaskCreate("SENDER", taskSender_main, NULL);
  xTask receiver = xTaskCreate("RECEIVER", taskReceiver_main, NULL);

  /* Check to make sure both tasks were created. */
  if (sender && receiver) {

    /* Set both tasks to waiting so they respond to direct-to-task notifactions
    and task timer events. */
    xTaskWait(sender);
    xTaskWait(receiver);

    /* The sender task we want to execute every one second. */
    xTaskChangePeriod(sender, 1000000);

    /* Pass control to the HeliOS scheduler. */
    xTaskStartScheduler();


    /* Do some clean-up. */
    xTaskDelete(sender);
    xTaskDelete(receiver);
  }


  /* Halt the system. */
  xSystemHalt();
}

void loop() {
  /* The loop function is not used and should remain empty. */
}