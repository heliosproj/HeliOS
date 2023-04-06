/*UNCRUSTIFY-OFF*/
/**
 * @file loopback.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
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
#include "loopback.h"


/*UNCRUSTIFY-OFF*/


#define BUFFER_LENGTH 0xFFu
static Byte_t loopback_buffer[BUFFER_LENGTH];
static HalfWord_t loopback_buffer_size = nil;


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
}


Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t * device_) {
  FUNCTION_ENTER;

  if(OK(__memset__(loopback_buffer, nil, BUFFER_LENGTH))) {
    device_->available = false;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t * device_, Size_t *size_, Addr_t *config_) {
  FUNCTION_ENTER;


  /* INSERT DEVICE DRIVER CODE HERE CALL __ReturnOk__() IF SYSTEM CALL WAS
   * SUCCESSFUL BEFORE RETURNING. */
  FUNCTION_EXIT;
}


  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t * device_, Size_t *size_, Addr_t **data_) {
  FUNCTION_ENTER;

  __KernelAllocateMemory__((volatile Addr_t **) data_, loopback_buffer_size);

  __memcpy__(*data_, loopback_buffer, loopback_buffer_size);

  *size_ = loopback_buffer_size;
  device_->available = false;
  __ReturnOk__();


  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t * device_, Size_t *size_, Addr_t *data_) {
  FUNCTION_ENTER;

  if(OK(__memcpy__(loopback_buffer, data_, *size_))) {
    loopback_buffer_size = *size_;
    device_->available = true;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t * device_, Byte_t *data_) {
  FUNCTION_ENTER;


  *data_ = loopback_buffer[0];

  device_->available = false;
  __ReturnOk__();

  FUNCTION_EXIT;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t * device_, Byte_t data_) {
  FUNCTION_ENTER;

  loopback_buffer[0] = data_;
  __ReturnOk__();

  FUNCTION_EXIT;
}
/*UNCRUSTIFY-ON*/