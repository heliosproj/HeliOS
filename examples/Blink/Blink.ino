/*UNCRUSTIFY-OFF*/
/**
 * @file Blink.ino
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


/* Define the task's main function. This function is the entry point for the
 * task when executed by the scheduler. The "task_" parameter contains the task
 * itself and may be used to perform operations against the task such as
 * suspending it with xTaskSuspend(task_). The "parm_" parameter points to
 * memory containing the task parameter(s). This memory can be allocated by
 * xMemAlloc() if needed. The task parameter must be dereferenced inside the
 * task's main function. A convenient C macro, DEREF_TASKPARM(), is available to
 * simplify the task of dereferencing the task parameter. */
void blinkTask_main(xTask task_, xTaskParm parm_) {
  /* Dereference the task parameter and store its value in the local integer
   * "ledState". This integer contains the state of the LED (i.e., 1 (on) or 0
   * (off)). Global variables are discouraged in favor of task parameters when
   * sharing or persisting a value is required*/
  int ledState = DEREF_TASKPARM(int, parm_);


  /* Once inside the task's main function, do not call functions like Arduino's
   * delay(). HeliOS tasks should implement a state machine model like the one
   * used here to ensure control is returned to the scheduler as quickly as
   * possible so other tasks may run. */
  if(ledState) {
    digitalWrite(LED_BUILTIN, HIGH);
    ledState = 0;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    ledState = 1;
  }


  /* Because the value of "ledState" has changed, the task parameter must be
   * dereferenced again so that it may be updated. The task's main function will
   * receive the same value the next time the task is executed by the scheduler.
   * Task parameters are also the preferred method for sharing message queues,
   * stream buffers, etc. between tasks. */
  DEREF_TASKPARM(int, parm_) = ledState;

  return;
}


void setup() {
  int ledState = 0;


  pinMode(LED_BUILTIN, OUTPUT);


  /* Call xSystemInit() to initialize memory and call initialization functions
   * in the port layer. The xSystemInit() syscall must be made prior to making
   * any other syscall. The ERROR() and OK() C macros are a concise method for
   * checking the return value of the xSystemInit() syscall. A consistent return
   * type (xReturn) was introduced in kernel 0.4.0.  If the syscall fails,
   * xSystemHalt() will be called to halt the system.*/
  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }


  /* Declare the task which will be used inside of the Arduino setup() function
   * to configure the task prior to handing over control to the HeliOS
   * scheduler. */
  xTask blink;


  /* Call the xTaskCreate() syscall to create the task. The xTaskCreate()
   * syscall prototype and parameters are as follows.
   *
   * PROTOTYPE
   * ~~~~~~~~~
   *   xReturn xTaskCreate(xTask *task_,const xByte *name_, void
   * (*callback_)(xTask task_, xTaskParm parm_), xTaskParm taskParameter_)
   *
   * PARAMETERS
   * ~~~~~~~~~~
   *   task_     A reference to the task. To pass the task by reference, the
   * address-of ("&") operator must be used (e.g., &blink).
   *
   *   name_     A reference to the first byte of a byte array containing the
   * ASCII name of the task. The task name is not a null terminated C char array
   * (sometimes called a "string"). The length of the byte array must be
   * precisely CONFIG_TASK_NAME_BYTES (default is 8) bytes. If the task name is
   * shorter, then it must be padded to meet the precise length requirement. To
   * avoid compiler warnings when using a literal (e.g., "BLINKTSK"), the
   * argument must be cast as "const xByte *".
   *
   *   callBack_ A reference to the task's main function. The task's main
   * function's prototype must be as follows. The name of the task's main
   * function does *NOT* need to match the name given to the task through the
   *             "name_" parameter.
   *
   *                 void <taskname>(xTask task_, xTaskParm parm_)
   *
   * If the syscall fails, xSystemHalt() will be called to halt the system.*/
  if(ERROR(xTaskCreate(&blink, (const xByte *) "BLINKTSK", blinkTask_main, &ledState))) {
    xSystemHalt();
  }


  /* Because the blink task will be an event-driven task (i.e., scheduled for
   * execution only when a task event occurs), the task must be placed in the
   * "waiting" state by xTaskWait(). There are two types of task events,
   * direct-to-task notifications and task timers. In this example we will be
   * using a task timer. If the syscall fails, xSystemHalt() will be called to
   * halt the system. */
  if(ERROR(xTaskWait(blink))) {
    xSystemHalt();
  }


  /* In order to use the task timer, the task timer period must be set to a
   * positive non-zero value. In this example we are setting the task timer to
   * 1,000 ticks. This way the HeliOS scheduler will schedule the blink task for
   * execution every 1,000 ticks. The length of a tick is platform and/or
   * architecture dependent though on most platforms a tick will occur every one
   * millisecond. If the syscall fails, xSystemHalt() will be called to halt the
   * system. */
  if(ERROR(xTaskChangePeriod(blink, 1000))) {
    xSystemHalt();
  }


  /* Now that the task(s) are created and configured they way we want, control
   * must be passed to the HeliOS scheduler. Once this is done, the only way to
   * return control back to the Arduino setup() function is by calling
   * xTaskSuspendAll() which will cause the scheduler to quit. If the syscall
   * fails, xSystemHalt() will be called to halt the system. */
  if(ERROR(xTaskStartScheduler())) {
    xSystemHalt();
  }


  /* While not required, it is advised to call xSystemHalt() at the end of the
   * Arduino setup() function. In this way, if the scheduler is forced to quit,
   * the application will halt and no further code will be executed. */
  xSystemHalt();
}


void loop() {
  /* The Arduino loop() function is not used in a HeliOS application and must
   * remain empty. */
}