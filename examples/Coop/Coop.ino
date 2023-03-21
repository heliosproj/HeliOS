/*UNCRUSTIFY-OFF*/
/**
 * @file Coop.ino
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


void taskShort_main(xTask task_, xTaskParm parm_) {
  /* Arduino's delay() is only used here to demonstrate HeliOS functionality and
   * must not be used in a real application built on HeliOS. */
  delay(10);
  Serial.print("S");
}


void taskLong_main(xTask task_, xTaskParm parm_) {
  /* Arduino's delay() is only used here to demonstrate HeliOS functionality and
   * must not be used in a real application built on HeliOS. */
  delay(60);
  Serial.println("L");
}


void setup() {
  xTask shortTask;
  xTask longTask;


  Serial.begin(9600);

  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&shortTask, (const xByte *) "SHORTTSK", taskShort_main, null))) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&longTask, (const xByte *) "LONGTSK ", taskLong_main, null))) {
    xSystemHalt();
  }

  if(ERROR(xTaskResume(shortTask))) {
    xSystemHalt();
  }

  if(ERROR(xTaskResume(longTask))) {
    xSystemHalt();
  }

  if(ERROR(xTaskStartScheduler())) {
    xSystemHalt();
  }

  xSystemHalt();
}


void loop() {


}