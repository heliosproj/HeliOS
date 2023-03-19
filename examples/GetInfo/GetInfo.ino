/*UNCRUSTIFY-OFF*/
/**
 * @file GetInfo.ino
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
#include <Arduino.h>
#include <HeliOS.h>


void taskPrint_main(xTask task_, xTaskParm parm_) {
  String str;
  xTaskInfo tinfo;
  xSystemInfo sinfo;


  if(OK(xTaskGetTaskInfo(task_, &tinfo))) {
    str = "taskPrint_main(): id = ";
    str += tinfo->id;
    str += ", name = ";
    str += (char *) tinfo->name; /* Technically this is not a good practice as
                                  * tinfo->name is not a null terminated char
                                  * array so the String "+" operated may read
                                  * past the eight byte foundry of tinfo->name.
                                  */
    str += ", state = ";
    str += tinfo->state;
    str += ", ltime = ";
    str += tinfo->lastRunTime;
    str += ", ttime = ";
    str += tinfo->totalRunTime;
    Serial.println(str);

    if(ERROR(xMemFree(tinfo))) {
      xSystemHalt();
    }
  }

  if(OK(xSystemGetSystemInfo(&sinfo))) {
    str = "taskPrint_main(): ";
    str += (char *) sinfo->productName; /* Technically this is not a good
                                         * practice as sinfo->productName is not
                                         * a null terminated char array so the
                                         * String "+" operated may read past the
                                         * eight byte boundary of
                                         * sinfo->productName. */
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

    if(ERROR(xMemFree(sinfo))) {
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