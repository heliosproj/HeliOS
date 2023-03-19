/*UNCRUSTIFY-OFF*/
/**
 * @file ListTasks.ino
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief An example Arduino sketch
 * @version 0.4.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#include <HeliOS.h>


/*
 * The task definition for the task to print all task informaiton every one
 * second.
 */
void taskPrint_main(xTask task_, xTaskParm parm_) {
  String str = "";
  xBase tasks = 0;


  /* Get information about the this task by calling xTaskGetAllRunTimeStats() on
   * the task handle. */
  xTaskRunTimeStats stats = xTaskGetAllRunTimeStats(&tasks);


  /* Check to make sure the task information was returned by
   * xTaskGetAllRunTimeStats() before attempting to access it. */
  if(stats) {
    for(int i = 0; i < tasks; i++) {
      str += "taskPrint_main(): ltime = ";
      str += stats->lastRunTime;
      str += ", ttime = ";
      str += stats->totalRunTime;
    }

    Serial.println(str);
  }


  /* Free the memory allocated to the task information structure. */
  xMemFree(stats);
}


void setup() {
  Serial.begin(9600);


  /* Call xSystemInit() to initialize any interrupt handlers and/or memory
   * required by HeliOS to execute on the target platform/architecture. */
  xSystemInit();


  /* Create a task to print all task information every second. */
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