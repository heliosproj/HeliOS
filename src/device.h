#ifndef DEVICE_H_

  #define DEVICE_H_

  #include "config.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "defines.h"

    #include "types.h"

    #include "console.h"

    #include "fat.h"

    #include "fs.h"

    #include "mem.h"

    #include "port.h"

    #include "posix.h"

    #include "queue.h"

    #include "streams.h"

    #include "sys.h"

    #include "task.h"

    #include "timer.h"

    #if defined(CONCAT)

      #undef CONCAT

    #endif 

    #define CONCAT(a_, b_) a_ ## b_

    #if defined(QUOTE)

      #undef QUOTE

    #endif 

    #define QUOTE(a_) #a_

    #if defined(TO_FUNCTION)

      #undef TO_FUNCTION

    #endif 

    #define TO_FUNCTION(a_, b_) CONCAT(a_, b_)

    #if defined(TO_LITERAL)

      #undef TO_LITERAL

    #endif 

    #define TO_LITERAL(a_) QUOTE(a_)

    #ifdef __cplusplus

      extern "C" {

    #endif 

    Return_t xDeviceRegisterDevice(Return_t (*device_self_register_)());

    Return_t __RegisterDevice__(const HalfWord_t uid_, const Byte_t *name_, const DeviceState_t state_, const DeviceMode_t mode_, Return_t (*init_)(Device_t *

      device_), Return_t (*config_)(Device_t *device_, Size_t *size_, Addr_t *config_), Return_t (*read_)(Device_t *device_, Size_t *size_, Addr_t **data_),

      Return_t (*write_)(Device_t *device_, Size_t *size_, Addr_t *data_), Return_t (*simple_read_)(Device_t *device_, Byte_t *data_), Return_t (*simple_write_)

      (Device_t *device_, Byte_t data_));

    Return_t xDeviceIsAvailable(const HalfWord_t uid_, Base_t *res_);

    Return_t xDeviceSimpleWrite(const HalfWord_t uid_, Byte_t data_);

    Return_t xDeviceWrite(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);

    Return_t xDeviceSimpleRead(const HalfWord_t uid_, Byte_t *data_);

    Return_t xDeviceRead(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);

    Return_t xDeviceInitDevice(const HalfWord_t uid_);

    Return_t xDeviceConfigDevice(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);

    Return_t __DeviceListFind__(const HalfWord_t uid_, Device_t **device_);

    Return_t __DeviceWrite__(const HalfWord_t uid_, Size_t *size_, Addr_t *data_);

    Return_t __DeviceRead__(const HalfWord_t uid_, Size_t *size_, Addr_t **data_);

    Return_t __DeviceConfigDevice__(const HalfWord_t uid_, Size_t *size_, Addr_t *config_);

    #if defined(POSIX_ARCH_OTHER)

      void __DeviceStateClear__(void);

    #endif 

    #ifdef __cplusplus

      }

    #endif 

  #endif 

#endif 

