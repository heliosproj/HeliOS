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
 * be executed by HeliOS every 1,000,000 microseconds
 * (1 second).
 */
void taskSerial(xTaskId id_) {
  /*
   * Declare and initialize a string object to
   * hold the text which will be written to the
   * serial bus every 1,000,000 microseconds
   * (1 second).
   */
  String str = "";

  /*
   * Call xTaskGetInfo() to obtain the task information
   * by passing xTaskGetInfo() the task id of the active
   * task.
   */
  xTaskGetInfoResult tres = xTaskGetInfo(id_);

  /*
   * Check the pointer to the xTaskGetInfoResult
   * structure before accessing any of its members
   * since xTaskGetInfo() can return null if the
   * task id does not exist or HeliOS is unable
   * to reserve the required managed memory.
   */
  if (tres) {
    /*
     * Append all of the members of the xTaskGetInfoResult
     * structure to the string.
     */
    str += "taskSerial(): id = ";
    str += tres->id;
    str += ", name = ";
    str += tres->name;
    str += ", state = ";
    str += tres->state;
    str += ", nbytes = ";
    str += tres->notifyBytes;
    str += ", nvalue = ";
    str += tres->notifyValue;
    str += ", ltime = ";
    str += tres->lastRuntime;
    str += ", ttime = ";
    str += tres->totalRuntime;
    str += ", tinterval = ";
    str += tres->timerInterval;
    str += ", tstart = ";
    str += tres->timerStartTime;

    /*
     * Print the string to the serial bus.
     */
    Serial.println(str);
  }

  /*
   * Free the managed memory allocated by the xTaskGetInfo()
   * function call. If xMemFree() is not called, HeliOS
   * may exhaust its available managed memory through
   * subsequent calls to xTaskGetInfo().
   */
  xMemFree(tres);

  /*
   * Clear the string.
   */
  str = "";

  /*
   * Call xHeliOSGetInfo() to obtain the system information.
   */
  xHeliOSGetInfoResult hres = xHeliOSGetInfo();

  /*
   * Check the pointer to the xHeliOSGetInfoResult
   * structure before accessing any of its members
   * since xHeliOSGetInfo() can return null if the
   * task id does not exist or HeliOS is unable
   * to reserve the required managed memory.
   */
  if (hres) {
    str += "taskSerial(): ";
    str += hres->productName;
    str += " ";
    str += hres->majorVersion;
    str += ".";
    str += hres->minorVersion;
    str += ".";
    str += hres->patchVersion;
    str += " has ";
    str += hres->tasks;
    str += " task.";

    /*
     * Print the string to the serial bus.
     */
    Serial.println(str);
  }

  /*
   * Free the managed memory allocated by the xHeliOSGetInfo()
   * function call. If xMemFree() is not called, HeliOS
   * may exhaust its available managed memory through
   * subsequent calls to xHeliOSGetInfo().
   */
  xMemFree(hres);
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
   * Add the task taskSerial() to HeliOS by passing
   * xTaskAdd() the friendly name of the task as well
   * as a callback pointer to the task function.
   */
  id = xTaskAdd("TASKSERIAL", &taskSerial);

  /*
   * Call xTaskWait() to place taskSerial() into a wait
   * state by passing xTaskWait() the task id. A task
   * must be in a wait state to respond to timer events.
   */
  xTaskWait(id);

  /*
   * Set the timer interval for taskSerial() to 1,000,000 microseconds
   * (1 second). HeliOS automatically begins incrementing
   * the timer for the task once the timer interval is set.
   */
  xTaskSetTimer(id, 1000000);
}

void loop() {
  /*
   * Momentarily pass control to HeliOS by calling the
   * xHeliOSLoop() function call. xHeliOSLoop() should be
   * the only code inside of the sketch's loop() function.
   */
  xHeliOSLoop();
}
