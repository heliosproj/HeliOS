/*UNCRUSTIFY-OFF*/
/**
 * @file WaitNotify.ino
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


void taskSender_main(xTask task_, xTaskParm parm_) {
  xTask receiver;


  if(OK(xTaskGetHandleByName(&receiver, "RECEIVER"))) {
    xTaskNotifyGive(receiver, 5, "HELLO");
  }
}


void taskReceiver_main(xTask task_, xTaskParm parm_) {
  String str;
  xTaskNotification notif;


  if(OK(xTaskNotifyTake(task_, &notif))) {
    str += notif->notificationValue;
    Serial.println(str);

    if(ERROR(xMemFree(notif))) {
      xSystemHalt();
    }
  }
}


void setup() {
  xTask sender;
  xTask receiver;


  Serial.begin(9600);

  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&sender, "SENDER__", taskSender_main, null))) {
    xSystemHalt();
  }

  if(ERROR(xTaskCreate(&receiver, "RECEIVER", taskReceiver_main, null))) {
    xSystemHalt();
  }

  if(ERROR(xTaskWait(sender))) {
    xSystemHalt();
  }

  if(ERROR(xTaskWait(receiver))) {
    xSystemHalt();
  }

  if(ERROR(xTaskChangePeriod(sender, 1000))) {
    xSystemHalt();
  }

  if(ERROR(xTaskStartScheduler())) {
    xSystemHalt();
  }

  xSystemHalt();
}


void loop() {


}