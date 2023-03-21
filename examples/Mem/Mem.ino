/*UNCRUSTIFY-OFF*/
/**
 * @file Mem.ino
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
  xSize size;
  xTaskInfo tinfo1;
  xTaskInfo tinfo2;
  xTaskInfo tinfo3;


  if(OK(xMemGetUsed(&size))) {
    str = "taskPrint_main(): ";
    str += size;
    str += " bytes of heap memory in use.";
    Serial.println(str);
  }

  if(OK(xTaskGetTaskInfo(task_, &tinfo1)) && OK(xTaskGetTaskInfo(task_, &tinfo2)) && OK(xTaskGetTaskInfo(task_, &tinfo3))) {
    if(OK(xMemGetSize((const volatile xAddr) tinfo1, &size))) {
      str = "taskPrint_main(): xTaskInfo is using ";
      str += size;
      str += " bytes of heap memory.";
      Serial.println(str);
    }

    if(OK(xMemGetUsed(&size))) {
      str = "taskPrint_main(): ";
      str += size;
      str += " bytes of heap memory in use.";
      Serial.println(str);
    }

    if(ERROR(xMemFree(tinfo1)) || ERROR(xMemFree(tinfo2)) || ERROR(xMemFree(tinfo3))) {
      xSystemHalt();
    }

    if(OK(xMemGetUsed(&size))) {
      str = "taskPrint_main(): ";
      str += size;
      str += " bytes of heap memory in use.";
      Serial.println(str);
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