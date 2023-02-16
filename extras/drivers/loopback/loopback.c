/*UNCRUSTIFY-OFF*/
/**
 * @file driver.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief A template for HeliOS device drivers
 * @version 0.4.0
 * @date 2022-09-02
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
#include "loopback.h"


/*UNCRUSTIFY-OFF*/


#define BUFFER_LENGTH 0xFFu
static Byte_t loopback_buffer[BUFFER_LENGTH];
static HalfWord_t loopback_buffer_size = zero;


Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void) {
  /* DO NOT MODIFY THIS FUNCTION */
  RET_DEFINE;

  if(ISSUCCESSFUL(__RegisterDevice__(DEVICE_UID, (Byte_t *) TO_LITERAL(DEVICE_NAME), DEVICE_STATE, DEVICE_MODE, TO_FUNCTION(DEVICE_NAME, _init), TO_FUNCTION(
      DEVICE_NAME, _config), TO_FUNCTION(DEVICE_NAME, _read), TO_FUNCTION(DEVICE_NAME, _write), TO_FUNCTION(DEVICE_NAME, _simple_read), TO_FUNCTION(DEVICE_NAME,
    _simple_write)))) {
    RET_SUCCESS;
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t * device_) {
  RET_DEFINE;

  if(ISSUCCESSFUL(__memset__(loopback_buffer, zero, BUFFER_LENGTH))) {
    device_->available = false;
    RET_SUCCESS;
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t * device_, Size_t *size_, Addr_t *config_) {
  RET_DEFINE;


  /* INSERT DEVICE DRIVER CODE HERE CALL RET_SUCCESS IF SYSTEM CALL WAS
   * SUCCESSFUL BEFORE RETURNING. */
  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t * device_, Size_t *size_, Addr_t *data_) {
  RET_DEFINE;

  if(ISSUCCESSFUL(__memcpy__(data_, loopback_buffer, *size_))) {
    *size_ = loopback_buffer_size;
    device_->available = false;
    RET_SUCCESS;
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t * device_, Size_t *size_, Addr_t *data_) {
  RET_DEFINE;

  if(ISSUCCESSFUL(__memcpy__(loopback_buffer, data_, *size_))) {
    loopback_buffer_size = *size_;
    device_->available = true;
    RET_SUCCESS;
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t * device_, Word_t *data_) {
  RET_DEFINE;

  if(ISSUCCESSFUL(__memcpy__(data_, loopback_buffer, sizeof(Word_t)))) {
    device_->available = false;
    RET_SUCCESS;
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}


Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t * device_, Word_t *data_) {
  RET_DEFINE;

  if(ISSUCCESSFUL(__memcpy__(loopback_buffer, data_, sizeof(Word_t)))) {
    device_->available = true;
    RET_SUCCESS;
  } else {
    SYSASSERT(false);
  }

  RET_RETURN;
}
/*UNCRUSTIFY-ON*/