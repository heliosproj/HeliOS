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
 * will perform 100,000 arbitrary floating point
 * operations.
 */
void taskShort(int id_) {
  float a = 0.0f;

  for (int i = 0; i < 100 000; i++)
    a *= 3.14f;
}

/*
 * The task definition for taskLong() which
 * will perform 1,000,000 arbitrary floating point
 * operations.
 */
void taskLong(int id_) {
  float a = 0.0f;

  for (int i = 0; i < 1 000 000; i++)
    a *= 3.14f;
}

void setup() {
  /*
   * Declare and initialize an int to hold the
   * task id.
   */
  int id = 0;

  /*
   * Call xHeliOSSetup() to initialize HeliOS and
   * its data structures. xHeliOSSetup() must be
   * called before any other HeliOS function call.
   */
  xHeliOSSetup();

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
