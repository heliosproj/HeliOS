/*UNCRUSTIFY-OFF*/
/**
 * @file Coop.ino
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Example code for cooperative multitasking
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
void taskShort_main(xTask task_, xTaskParm parm_) {
  /* Arduino's delay() is only used here to demonstrate HeliOS functionality and
   * must not be used in a real application built on HeliOS. */
  delay(10);
  Serial.print("S");
}


/* Define the task's main function. This is the function that will be called by
 * the scheduler to run the task. The "task_" parameter contains the task object
 * for the task itself. The "parm_" parameter points to the memory containing
 * the task parameter(s). The task parameter must be dereferenced inside the
 * task's main function by using the DEREF_TASKPARM() C macro. */
void taskLong_main(xTask task_, xTaskParm parm_) {
  /* Arduino's delay() is only used here to demonstrate HeliOS functionality and
   * must not be used in a real application built on HeliOS. */
  delay(60);
  Serial.println("L");
}


void setup() {
  Serial.begin(9600);


  /* Call xSystemInit() to initialize any interrupt handlers and/or memory
   * required by HeliOS to execute on the target platform or architecture. The
   * xSystemInit() syscall must be called prior to calling any other syscall. */
  xSystemInit();


  /* Declare the task object (a.k.a., task handle) which will be used inside of
   * the Arduino setup() function to configure the task prior to handing over
   * control to the HeliOS scheduler. */
  xTask shortTask;
  xTask longTask;


  /* Call the xTaskCreate() syscall to create the task and update the task
   * object. The OK() C macro is a concise method for checking the return value
   * of the xTaskCreate() syscall. A consistent return type (xReturn) was
   * introduced in kernel 0.4.0. In this example the syscalls are heavily nested
   * for a single return point. If multiple return points are acceptable (e.g.,
   * not having to conform to MISRA C:2012) in your application, the syscalls
   * can be un-nested and paired with a return statement. */
  if(OK(xTaskCreate(&shortTask, "SHORT", taskShort_main, null))) {
    if(OK(xTaskCreate(&longTask, "LONG", taskLong_main, null))) {
      /* Because the tasks will be an cooperatively scheduled (i.e., scheduled
       * to always run), the task must be placed in the
       * "running" state by xTaskResume(). */
      if(OK(xTaskResume(shortTask))) {
        if(OK(xTaskResetTimer(longTask))) {
          /* Now that our task(s) are created and configured they way we want,
           * control must be passed to the HeliOS scheduler. Once this is done,
           * the only way to return control back to the Arduino setup() function
           * is by calling xTaskSuspendAll() which will cause the scheduler to
           * quit. */
          xTaskStartScheduler();
        }
      }
    }
  }


  /* While not required, it is advised to call xSystemHalt() at the end of the
   * Arduino setup() function. In this way, if there are errors that occur or if
   * the scheduler is forced to quick, the application will halt the execution
   * of further application code. */
  xSystemHalt();
}


void loop() {
  /* The Arduino loop() is not used in a HeliOS application and must remain
   * empty. */
}