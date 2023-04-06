/*UNCRUSTIFY-OFF*/
/**
 * @file driver.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief A template for kernel mode device drivers
 * @version 0.5.0
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
#include "driver.h"


/*UNCRUSTIFY-OFF*/
Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void) {
  /* DO NOT MODIFY THIS FUNCTION */
  FUNCTION_ENTER;

  if(OK(__RegisterDevice__(DEVICE_UID, (Byte_t *) TO_LITERAL(DEVICE_NAME), DEVICE_STATE, DEVICE_MODE, TO_FUNCTION(DEVICE_NAME, _init), TO_FUNCTION(
      DEVICE_NAME, _config), TO_FUNCTION(DEVICE_NAME, _read), TO_FUNCTION(DEVICE_NAME, _write), TO_FUNCTION(DEVICE_NAME, _simple_read), TO_FUNCTION(DEVICE_NAME,
    _simple_write)))) {
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
  /* DO NOT MODIFY THIS FUNCTION */
}


Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_) {
  FUNCTION_ENTER;

  /* INSERT DEVICE DRIVER CODE HERE CALL __ReturnOk__() IF SYSCALL WAS
   * SUCCESSFUL BEFORE RETURNING. */

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;

  /* INSERT DEVICE DRIVER CODE HERE CALL __ReturnOk__() IF SYSCALL WAS
   * SUCCESSFUL BEFORE RETURNING. */

  FUNCTION_EXIT;
}


  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;

  /* INSERT DEVICE DRIVER CODE HERE CALL __ReturnOk__() IF SYSCALL WAS
   * SUCCESSFUL BEFORE RETURNING. */

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_) {
  FUNCTION_ENTER;

  /* INSERT DEVICE DRIVER CODE HERE CALL __ReturnOk__() IF SYSCALL WAS
   * SUCCESSFUL BEFORE RETURNING. */

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_) {
  FUNCTION_ENTER;

  /* INSERT DEVICE DRIVER CODE HERE CALL __ReturnOk__() IF SYSCALL WAS
   * SUCCESSFUL BEFORE RETURNING. */

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_) {
  FUNCTION_ENTER;

  /* INSERT DEVICE DRIVER CODE HERE CALL __ReturnOk__() IF SYSCALL WAS
   * SUCCESSFUL BEFORE RETURNING. */

  FUNCTION_EXIT;
}
/*UNCRUSTIFY-ON*/