/*
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
 * Define Serial speed and timers
 */
#define SERIAL_SPEED 9600
#define TASKSERIAL_TIMER 5000
#define OTHERTASK_TIMER 2000

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
   * Declare and initialize and int to store the number
   * of tasks in the task list. This will get populated
   * by xTaskGetList().
   */
  int16_t tasks = 0;

  /*
   * Call xTaskGetInfo() to obtain the task information
   * by passing xTaskGetInfo() the task id of the active
   * task.
   */
  xTaskGetListResult tres = xTaskGetList(&tasks);

  /*
   * Check the pointer to the xTaskGetListResult
   * structure before accessing any of its members
   * since xTaskGetList() can return null if the
   * task id does not exist or HeliOS is unable
   * to reserve the required managed memory.
   */
  if (tres) {
    /*
     * Loop through the xTaskGetListResult array
     * for the number of times specified by
     * the int task.
     */
    for (int i = 0; i < tasks; i++) {
      /*
       * Clear the string.
       */
      str = "";

      /*
       * Append all of the members of the xTaskGetListResult
       * structure to the string.
       */
      str += "task : id = ";
      str += tres[i].id;
      str += ", name = ";
      str += tres[i].name;
      str += ", state = ";
      str += tres[i].state;
      str += ", ltime = ";
      str += tres[i].lastRuntime;
      str += ", ttime = ";
      str += tres[i].totalRuntime;

      /*
       * Print the string to the serial bus.
       */
      Serial.println(str);
    }
  }

  /*
   * Free the managed memory allocated by the xTaskGetInfo()
   * function call. If xMemFree() is not called, HeliOS
   * may exhaust its available managed memory through
   * subsequent calls to xTaskGetInfo().
   */
  xMemFree(tres);
}

/*
 * The task definition for otherTask() which will
 * be executed by HeliOS every 2,000,000 microseconds
 * (2 second).
 */
void otherTask(xTaskId id_) {
  xTaskGetInfoResult tres = xTaskGetInfo(id_);
  xMemFree(tres);
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
  Serial.begin(SERIAL_SPEED);

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
   * Set the timer interval for taskSerial() to 5,000,000 microseconds
   * (5 second). HeliOS automatically begins incrementing
   * the timer for the task once the timer interval is set.
   */
  xTaskSetTimer(id, TASKSERIAL_TIMER);
  
  /*
   * Add the second task otherTask() to HeliOS by passing
   * xTaskAdd() the friendly name of the task as well
   * as a callback pointer to the task function.
   */
  id = xTaskAdd("OTHERTASK", &otherTask);

  /*
   * Call xTaskWait() to place taskSerial2() into a wait
   * state by passing xTaskWait() the task id. A task
   * must be in a wait state to respond to timer events.
   */
  xTaskWait(id);

  /*
   * Set the timer interval for otherTask() to 2,000,000 microseconds
   * (2 second). HeliOS automatically begins incrementing
   * the timer for the task once the timer interval is set.
   */
  xTaskSetTimer(id, OTHERTASK_TIMER);
}

void loop() {
  /*
   * Momentarily pass control to HeliOS by calling the
   * xHeliOSLoop() function call. xHeliOSLoop() should be
   * the only code inside of the sketch's loop() function.
   */
  xHeliOSLoop();
}
