/*UNCRUSTIFY-OFF*/
/**
 * @file Coop.ino
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief An example Arduino sketch
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#include <Arduino.h>
#include <HeliOS.h>


void taskShort_main(Task_t *task_, TaskParm_t *parm_) {
  /* Arduino's delay() is only used here to demonstrate HeliOS functionality and
   * must not be used in a real application built on HeliOS. */
  delay(10);
  Serial.print("S");
}


void taskLong_main(Task_t *task_, TaskParm_t *parm_) {
  /* Arduino's delay() is only used here to demonstrate HeliOS functionality and
   * must not be used in a real application built on HeliOS. */
  delay(60);
  Serial.println("L");
}


void setup() {
  Task_t *shortTask;
  Task_t *longTask;


  Serial.begin(9600);

  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&shortTask, (const Byte_t *) "SHORTTSK", taskShort_main, null))) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&longTask, (const Byte_t *) "LONGTSK ", taskLong_main, null))) {
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