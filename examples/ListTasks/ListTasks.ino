/*UNCRUSTIFY-OFF*/
/**
 * @file ListTasks.ino
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief An example Arduino sketch
 * @version 0.4.1
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
#include <Arduino.h>
#include <HeliOS.h>


void taskPrint_main(xTask task_, xTaskParm parm_) {
  String str;
  xBase tasks;
  xTaskRunTimeStats stats;


  if(OK(xTaskGetAllRunTimeStats(&stats, &tasks))) {
    for(int i = 0; i < tasks; i++) {
      str = "taskPrint_main(): ltime = ";
      str += stats->lastRunTime;
      str += ", ttime = ";
      str += stats->totalRunTime;
      Serial.println(str);
    }

    if(ERROR(xMemFree(stats))) {
      xSystemHalt();
    }
  }
}


void setup() {
  xTask task;


  Serial.begin(9600);

  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&task, (const xByte *) "PRINTTSK", taskPrint_main, null))) {
    xSystemHalt();
  }

  if(ERROR(xTaskWait(task))) {
    xSystemHalt();
  }

  if(ERROR(xTaskChangePeriod(task, 1000))) {
    xSystemHalt();
  }

  if(ERROR(xTaskStartScheduler())) {
    xSystemHalt();
  }

  xSystemHalt();
}


void loop() {


}