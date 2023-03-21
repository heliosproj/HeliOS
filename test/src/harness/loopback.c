/*UNCRUSTIFY-OFF*/
/**
 * @file loopback.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
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
#include "loopback.h"


/*UNCRUSTIFY-OFF*/


#define BUFFER_LENGTH 0xFFu
static Byte_t loopback_buffer[BUFFER_LENGTH];
static HalfWord_t loopback_buffer_size = zero;


Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void) {
  /* DO NOT MODIFY THIS FUNCTION */
  RET_DEFINE;

  if(OK(__RegisterDevice__(DEVICE_UID, (Byte_t *) TO_LITERAL(DEVICE_NAME), DEVICE_STATE, DEVICE_MODE, TO_FUNCTION(DEVICE_NAME, _init), TO_FUNCTION(
      DEVICE_NAME, _config), TO_FUNCTION(DEVICE_NAME, _read), TO_FUNCTION(DEVICE_NAME, _write), TO_FUNCTION(DEVICE_NAME, _simple_read), TO_FUNCTION(DEVICE_NAME,
    _simple_write)))) {
    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t * device_) {
  RET_DEFINE;

  if(OK(__memset__(loopback_buffer, zero, BUFFER_LENGTH))) {
    device_->available = false;
    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t * device_, Size_t *size_, Addr_t *config_) {
  RET_DEFINE;


  /* INSERT DEVICE DRIVER CODE HERE CALL RET_OK IF SYSTEM CALL WAS
   * SUCCESSFUL BEFORE RETURNING. */
  RET_RETURN;
}


  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t * device_, Size_t *size_, Addr_t **data_) {
  RET_DEFINE;

  __KernelAllocateMemory__((volatile Addr_t **) data_, loopback_buffer_size);

  __memcpy__(*data_, loopback_buffer, loopback_buffer_size);

  *size_ = loopback_buffer_size;
  device_->available = false;
  RET_OK;


  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t * device_, Size_t *size_, Addr_t *data_) {
  RET_DEFINE;

  if(OK(__memcpy__(loopback_buffer, data_, *size_))) {
    loopback_buffer_size = *size_;
    device_->available = true;
    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t * device_, Byte_t *data_) {
  RET_DEFINE;


  *data_ = loopback_buffer[0];

  device_->available = false;
  RET_OK;

  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t * device_, Byte_t data_) {
  RET_DEFINE;

  loopback_buffer[0] = data_;
  RET_OK;

  RET_RETURN;
}
/*UNCRUSTIFY-ON*/