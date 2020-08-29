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
 * The task definition for taskShort() which
 * will perform 1,000 arbitrary floating point
 * operations.
 */
void taskShort(xTaskId id_) {
  volatile float a = 0.0f, b = 0.0f;

  for (int i = 0; i < 1000; i++)
    a += i;

  b = a;

  Serial.print("S");
}

/*
 * The task definition for taskLong() which
 * will perform 10,000 arbitrary floating point
 * operations.
 */
void taskLong(xTaskId id_) {
  volatile float a = 0.0f, b = 0.0f;

  for (int i = 0; i < 10000; i++)
    a += i;

  b = a;

  Serial.println("L");
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
   * Add the task taskShort() to HeliOS by passing
   * xTaskAdd() the friendly name of the task as well
   * as a callback pointer to the task function.
   */
  id = xTaskAdd("TASKSHORT", &taskShort);

  /*
   * Call xTaskStart() to start taskShort() by passing
   * xTaskStart() the id of the task to start.
   */
  xTaskStart(id);

  /*
   * Add the task taskShort() to HeliOS by passing
   * xTaskAdd() the friendly name of the task as well
   * as a callback pointer to the task function.
   */
  id = xTaskAdd("TASKLONG", &taskLong);

  /*
   * Call xTaskStart() to start taskLong() by passing
   * xTaskStart() the id of the task to start.
   */
  xTaskStart(id);
}

void loop() {
  /*
   * Momentarily pass control to HeliOS by calling the
   * xHeliOSLoop() function call. xHeliOSLoop() should be
   * the only code inside of the sketch's loop() function.
   */
  xHeliOSLoop();
}
