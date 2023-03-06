/*UNCRUSTIFY-OFF*/
/**
 * @file GetInfo.ino
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Example code for getting task and system information
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
/*UNCRUSTIFY-ON*/

#include <HeliOS.h>


/*
 * The task definition for the task to print the task and system informaiton
 * every one second.
 */
void taskPrint_main(xTask task_, xTaskParm parm_) {
  String str = "";


  /* Get information about the this task by calling xTaskGetTaskInfo() on the
   * task handle. */
  xTaskInfo tinfo = xTaskGetTaskInfo(task_);


  /* Check to make sure the task information was returned by xTaskGetTaskInfo()
   * before attempting to access it. */
  if(tinfo) {
    str += "taskPrint_main(): id = ";
    str += tinfo->id;
    str += ", name = ";
    str += tinfo->name;
    str += ", state = ";
    str += tinfo->state;
    str += ", ltime = ";
    str += tinfo->lastRunTime;
    str += ", ttime = ";
    str += tinfo->totalRunTime;
    Serial.println(str);
  }


  /* Free the memory allocated to the task information structure. */
  xMemFree(tinfo);
  str = "";


  /* Get information about the this system by calling xSystemGetSystemInfo(). */
  xSystemInfo sinfo = xSystemGetSystemInfo();


  /* Check to make sure the system information was returned by
   * xSystemGetSystemInfo() before attempting to access it. */
  if(sinfo) {
    str += "taskPrint_main(): ";
    str += sinfo->productName;
    str += " ";
    str += sinfo->majorVersion;
    str += ".";
    str += sinfo->minorVersion;
    str += ".";
    str += sinfo->patchVersion;
    str += " has ";
    str += sinfo->numberOfTasks;
    str += " task.";
    Serial.println(str);
  }


  /* Free the memory allocated to the system information structure. */
  xMemFree(sinfo);
}


void setup() {
  Serial.begin(9600);


  /* Call xSystemInit() to initialize any interrupt handlers and/or memory
   * required by HeliOS to execute on the target platform/architecture. */
  xSystemInit();


  /* Create a task to print the task and system information every second. */
  xTask task = xTaskCreate("PRINT", taskPrint_main, null);


  /* Check to make sure the task was created by xTaskCreate() before attempting
   * to use the task. */
  if(task) {
    /* Place the task in the waiting state. */
    xTaskWait(task);


    /* Set the task timer to one second. */
    xTaskChangePeriod(task, 1000000);


    /* Pass control to the HeliOS scheduler. The HeliOS scheduler will not
     * relinquish control unless xTaskSuspendAll() is called. */
    xTaskStartScheduler();


    /* If the scheduler relinquishes control, do some clean-up by deleting the
     * task. */
    xTaskDelete(task);
  }


  /* Halt the system. Once called, the system must be reset to recover. */
  xSystemHalt();
}


void loop() {
  /* The loop function is not used and should remain empty. */
}