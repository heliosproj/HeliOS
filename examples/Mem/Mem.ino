/*UNCRUSTIFY-OFF*/
/**
 * @file Mem.ino
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Example code to demonstrate how heap memory is managed in HeliOS
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


void taskPrint_main(xTask task_, xTaskParm parm_) {
  String str = "";


  /* Get how much heap memory is being used to start with. */
  str += "taskPrint_main(): ";
  str += xMemGetUsed();
  str += " bytes of heap memory in use.";
  Serial.println(str);


  /* Call xTaskGetInfo() a few times to allocate some memory. */
  xTaskInfo tinfo1 = xTaskGetTaskInfo(task_);
  xTaskInfo tinfo2 = xTaskGetTaskInfo(task_);
  xTaskInfo tinfo3 = xTaskGetTaskInfo(task_);


  str = "";


  /* Call xMemGetSize() to find out how much heap memory is being used by
   * xTaskInfo. */
  str += "taskPrint_main(): xTaskInfo is using ";
  str += xMemGetSize(tinfo1);
  str += " bytes of heap memory.";
  Serial.println(str);
  str = "";


  /* Now see how much heap memory is in use. */
  str += "taskPrint_main(): ";
  str += xMemGetUsed();
  str += " bytes of heap memory in use.";
  Serial.println(str);


  /* Free all of the xTaskInfo memory. */
  xMemFree(tinfo1);
  xMemFree(tinfo2);
  xMemFree(tinfo3);
  str = "";


  /* Check how much heap memory is in use one more time. It should be the same
   * as what we started with. */
  str += "taskPrint_main(): ";
  str += xMemGetUsed();
  str += " bytes of heap memory in use.";
  Serial.println(str);
}


void setup() {
  Serial.begin(9600);


  /* Call xSystemInit() to initialize any interrupt handlers and/or memory
   * required by HeliOS to execute on the target platform/architecture. */
  xSystemInit();


  /* Create a task to demonstrate how heap memory is managed in HeliOS. */
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