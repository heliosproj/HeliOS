/*UNCRUSTIFY-OFF*/
/**
 * @file Blink.ino
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


void blinkTask_main(xTask task_, xTaskParm parm_) {
  int ledState = DEREF_TASKPARM(int, parm_);


  if(ledState) {
    digitalWrite(LED_BUILTIN, HIGH);
    ledState = 0;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    ledState = 1;
  }

  DEREF_TASKPARM(int, parm_) = ledState;

  return;
}


void setup() {
  xTask blink;
  int ledState;


  pinMode(LED_BUILTIN, OUTPUT);

  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&blink, (const xByte *) "BLINKTSK", blinkTask_main, &ledState))) {
    xSystemHalt();
  }

  if(ERROR(xTaskWait(blink))) {
    xSystemHalt();
  }

  if(ERROR(xTaskChangePeriod(blink, 1000))) {
    xSystemHalt();
  }

  if(ERROR(xTaskStartScheduler())) {
    xSystemHalt();
  }

  xSystemHalt();
}


void loop() {


}