#include "driver.h"
Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void) {

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
Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_) {

  FUNCTION_ENTER;

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_) {

  FUNCTION_ENTER;

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_) {

  FUNCTION_ENTER;

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_) {

  FUNCTION_ENTER;

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_) {

  FUNCTION_ENTER;

  FUNCTION_EXIT;

}
Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_) {

  FUNCTION_ENTER;

  FUNCTION_EXIT;

}