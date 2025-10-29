/*UNCRUSTIFY-OFF*/
/**
 * @file GetInfo.ino
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief An example Arduino sketch
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#include <Arduino.h>
#include <HeliOS.h>
#include <arduino_helpers.h>


void taskPrint_main(Task_t *task_, TaskParm_t *parm_) {
  String str;
  TaskInfo_t *tinfo;
  SystemInfo_t *sinfo;


  if(OK(xTaskGetTaskInfo(task_, &tinfo))) {
    str = "taskPrint_main(): id = ";
    str += tinfo->id;
    str += ", name = ";
    str += xByte2String(CONFIG_TASK_NAME_BYTES, tinfo->name);
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
    str += xByte2String(OS_PRODUCT_NAME_SIZE, sinfo->productName);
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
  Task_t *task;


  Serial.begin(9600);

  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&task, (const Byte_t *) "PRINTTSK", taskPrint_main, null))) {
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