/*UNCRUSTIFY-OFF*/
/**
 * @file Timer.ino
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
  Serial.println("taskPrint_main(): one second has passed.");
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