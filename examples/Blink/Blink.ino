/*UNCRUSTIFY-OFF*/
/**
 * @file Blink.ino
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief An example Arduino sketch
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


void blinkTask_main(Task_t *task_, TaskParm_t *parm_) {
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
  Task_t *blink;
  int ledState;


  pinMode(LED_BUILTIN, OUTPUT);

  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&blink, (const Byte_t *) "BLINKTSK", blinkTask_main, &ledState))) {
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