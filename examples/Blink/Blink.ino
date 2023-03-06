/*UNCRUSTIFY-OFF*/
/**
 * @file Blink.ino
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Blink example code for Arduino
 * @version 0.4.0
 * @date 2022-02-14
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
/*UNCRUSTIFY-ON*/
/* Include the HeliOS header. Do not include any other HeliOS headers. */
#include <HeliOS.h>


/* Define the task's main function. This is the function that will be called by
 * the scheduler to run the task. The "task_" parameter contains the task object
 * for the task itself. The "parm_" parameter points to the memory containing
 * the task parameter(s). The task parameter must be dereferenced inside the
 * task's main function by using the DEREF_TASKPARM() C macro. */
void blinkTask_main(xTask task_, xTaskParm parm_) {
  /* Dereference the task parameter and store its value in the local integer
   * "ledState". This integer contains the state of the LED (i.e., 1 (on) or 2
   * (off)). This method is recommended over using variables with a global
   * scope.*/
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
   * receive the same value the next time the task's main function is called by
   * the scheduler. Task parameters are also the preffered method for sharing
   * messages queues, stream buffers, etc. between tasks. */
  DEREF_TASKPARM(int, parm_) = ledState;

  return;
}


void setup() {
  int ledState = 0;


  pinMode(LED_BUILTIN, OUTPUT);


  /* Call xSystemInit() to initialize any interrupt handlers and/or memory
   * required by HeliOS to execute on the target platform or architecture. The
   * xSystemInit() syscall must be called prior to calling any other syscall.
   * The ERROR() and OK() C macros are a concise method for checking the return
   * value of the xTaskCreate() syscall. A consistent return type (xReturn) was
   * introduced in kernel 0.4.0.  If the syscall fails, call xSystemHalt().*/
  if(ERROR(xSystemInit())) {
    xSystemHalt();
  }


  /* Declare the task object (a.k.a., task handle) which will be used inside of
   * the Arduino setup() function to configure the task prior to handing over
   * control to the HeliOS scheduler. */
  xTask blink;


  /* Call the xTaskCreate() syscall to create the task and pass back the task
   * object. The xTaskCreate() syscall prototype and parameters are as follows.
   *
   * xReturn xTaskCreate(xTask *task_, const xByte *name_, void
   * (*callback_)(xTask task_, xTaskParm parm_), xTaskParm taskParameter_)
   *
   * task_ A pointer to the task object (a.k.a., task handle). To pass a pointer
   * to the task objects the address-of ("&") operator must be used (e.g.,
   * &blink).
   *
   * name_ A pointer to a byte array containing the ASCII name of the task which
   * can be used later to obtain the task handle by using the xTaskGetName()
   * syscall. The length of the name cannot exceed CONFIG_TASK_NAME_BYTES
   * (default is 8 bytes).
   *
   * callBack_ A pointer to the task's main function. The task's main function's
   * prototype must be as follows.
   *
   *    void <taskname>(xTask task_, xTaskParm parm_)
   *
   * If the syscall fails, call xSystemHalt(). */
  if(ERROR(xTaskCreate(&blink, "BLINK", blinkTask_main, &ledState))) {
    xSystemHalt();
  }


  /* Because the blink task will be an event-driven task (i.e., scheduled for
   * execution only when a task event occurs), the task must be placed in the
   * "waiting" state by xTaskWait(). There are two types of task events,
   * direct-to-task notifications and task timers. In this example we will be
   * using a task timer. If the syscall fails, call xSystemHalt(). */
  if(ERROR(xTaskWait(blink))) {
    xSystemHalt();
  }


  /* In order to use the task timer, the task time period must be set to a
   * non-zero positive value. In this example we are setting the task timer to
   * 1,000 ticks. This way the HeliOS scheduler will schedule the blink task for
   * execution every 1,000 ticks. The length of a tick is platform and/or
   * architecture dependent though on most a tick will occur every one
   * millisecond. If the syscall fails, call xSystemHalt().*/
  if(ERROR(xTaskChangePeriod(blink, 1000))) {
    xSystemHalt();
  }


  /* Now that our task(s) are created and configured they way we want, control
   * must be passed to the HeliOS scheduler. Once this is done, the only way to
   * return control back to the Arduino setup() function is by calling
   * xTaskSuspendAll() which will cause the scheduler to quit. If the syscall
   * fails, call xSystemHalt(). */
  if(ERROR(xTaskStartScheduler())) {
    xSystemHalt();
  }


  /* While not required, it is advised to call xSystemHalt() at the end of the
   * Arduino setup() function. In this way,  if the scheduler is forced to quit,
   * the application will halt the execution of further application code. */
  xSystemHalt();
}


void loop() {
  /* The Arduino loop() is not used in a HeliOS application and must remain
   * empty. */
}