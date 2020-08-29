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

/*
 * Additional documentation on HeliOS and its Application
 * Programming Interface (API) is available in the
 * HeliOS Programmer's Guide which can be found here:
 *
 * https://github.com/MannyPeterson/HeliOS/blob/master/extras/HeliOS_Programmers_Guide.md
 */

/*
 * Include the standard HeliOS header for Arduino sketches. This header
 * includes the required HeliOS header files automatically.
 */
#include <HeliOS_Arduino.h>

/*
 * The task definition for taskSerial() which will
 * be executed by HeliOS when digital GPIO pin 2
 * or 3 change state to HIGH.
 */
void taskSerial(xTaskId id_) {
  /*
   * Call xTaskGetNotif() to obtain the task notification
   * bytes and value by passing xTaskGetNotif() the task
   * id of the active task.
   */
  xTaskGetNotifResult nres = xTaskGetNotif(id_);

  /*
   * Check the pointer to the xTaskGetNotifResult
   * structure before accessing any of its members
   * since xTaskGetNotif() can return null if the
   * task id does not exist or HeliOS is unable
   * to reserve the required managed memory.
   */
  if (nres) {
    /*
     * Declare and initialize a string object to
     * hold the text which will be written
     * to the serial bus when digital GPIO
     * pin 2 or 3 changes state to HIGH.
     */
    String str = "taskSerial(): GPIO pin ";

    /*
     * Append the notification value to the
     * string. In this example, the notifiation
     * value will contain either "D2" or "D3"
     * depending on which digital GPIO pin
     * changes state to HIGH.
     */
    str += nres->notifyValue;

    /* Close the string which will be written
     * to the serial bus.
     */
    str += " changed state to HIGH.";

    /*
     * Print the string to the serial bus.
     */
    Serial.println(str);
  }

  /*
   * Free the managed memory allocated by the xTaskGetNotif()
   * function call. If xMemFree() is not called, HeliOS
   * may exhaust its available managed memory through
   * subsequent calls to xTaskGetNotif().
   */
  xMemFree(nres);

  /*
   * HeliOS will automatically set the notification bytes
   * back to zero after the task returns control to HeliOS.
   * However, you can optionally call xTaskNotifyClear() to
   * ensure the notifiation value itself is also cleared. The
   * xTaskNotifyClear() function call sets all bytes of the
   * notifiation value to zero.
   */
  xTaskNotifyClear(id_);
}

/*
 * The interrupt service routine (ISR) definition
 * for digital GPIO pin 2.
 */
void D2ISR() {
  /*
   * Send 2 byte notification value of "D2" to
   * taskSerial() by first calling xTaskGetId()
   * to resolve the task's friendly name to a
   * task id (do not assume a task's id, always
   * reference a task by its friendly name). Then
   * pass the task id, the number of notification
   * bytes in the notification value and the
   * notification value itself to xTaskNotify().
   * Please note: it is not neccessary to cast
   * the notification value to (char*). This is
   * only done to avoid compiler warnings in C++.
   */
  xTaskNotify(xTaskGetId("TASKSERIAL"), 2, (char *)"D2");
}

/*
 * The interrupt service routine (ISR) definition
 * for digital GPIO pin 3.
 */
void D3ISR() {
  /*
   * Send 2 byte notification value of "D3" to
   * taskSerial() by first calling xTaskGetId()
   * to resolve the task's friendly name to a
   * task id (do not assume a task's id, always
   * reference a task by its friendly name). Then
   * pass the task id, the number of notification
   * bytes in the notification value and the
   * notification value itself to xTaskNotify().
   * Please note: it is not neccessary to cast
   * the notification value to (char*). This is
   * only done to avoid compiler warnings in C++.
   */
  xTaskNotify(xTaskGetId("TASKSERIAL"), 2, (char *)"D3");
}

void setup() {
  /*
   * Declare an xTaskId to hold the the task id
   * and initialize.
   */
  xTaskId id = 0;

  /*
   * Call xHeliOSSetup() to initialize HeliOS and
   * its data structures. xHeliOSSetup() must be
   * called before any other HeliOS function call.
   */
  xHeliOSSetup();

  /*
   * Set the serial data rate and begin serial
   * communication.
   */
  Serial.begin(9600);

  /*
   * Set digital GPIO pins 2 and 3 to input only.
   */
  pinMode(2, INPUT);
  pinMode(3, INPUT);

  /*
   * Attach the interrupt service routines D2ISR()
   * and D3ISR() to the interrupts for digital GPIO
   * pins 2 and 3.
   */
  attachInterrupt(digitalPinToInterrupt(2), D2ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(3), D3ISR, RISING);

  /*
   * Add the task taskSerial() to HeliOS by passing
   * xTaskAdd() the friendly name of the task as well
   * as a callback pointer to the task function.
   */
  id = xTaskAdd("TASKSERIAL", &taskSerial);

  /*
   * Call xTaskWait() to place taskSerial() into a wait
   * state by passing xTaskWait() the task id. A task
   * must be in a wait state to receive notifications.
   */
  xTaskWait(id);
}

void loop() {
  /*
   * Momentarily pass control to HeliOS by calling the
   * xHeliOSLoop() function call. xHeliOSLoop() should be
   * the only code inside of the sketch's loop() function.
   */
  xHeliOSLoop();
}
