#ifndef SYS_H_
  #define SYS_H_
  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)
    #include "console.h"
    #include "device.h"
    #include "fat.h"
    #include "fs.h"
  #endif
  #include "mem.h"
  #include "port.h"
  #include "posix.h"
  #include "queue.h"
  #include "streams.h"
  #include "task.h"
  #include "timer.h"
  #if defined(OS_PRODUCT_NAME)
    #undef OS_PRODUCT_NAME
  #endif
  #define OS_PRODUCT_NAME "HeliOS"
  #if defined(RUNNING)
    #undef RUNNING
  #endif
  #define RUNNING flag.running
  #if defined(OVERFLOW)
    #undef OVERFLOW
  #endif
  #define OVERFLOW flag.overflow
  #if defined(MEMFAULT)
    #undef MEMFAULT
  #endif
  #define MEMFAULT flag.memfault
  #if defined(LITTLEEND)
    #undef LITTLEEND
  #endif
  #define LITTLEEND flag.littleend
  #if defined(__SetFlag__)
    #undef __SetFlag__
  #endif
  #define __SetFlag__(flag_) flag_ = 0xFFu
  #if defined(__UnsetFlag__)
    #undef __UnsetFlag__
  #endif
  #define __UnsetFlag__(flag_) flag_ = 0x00u
  #if defined(__FlagIsSet__)
    #undef __FlagIsSet__
  #endif
  #define __FlagIsSet__(flag_) (0xFFu == (flag_))
  #if defined(__FlagIsNotSet__)
    #undef __FlagIsNotSet__
  #endif
  #define __FlagIsNotSet__(flag_) (0x0u == (flag_))
  #ifdef __cplusplus
    extern "C" {
  #endif
  extern Flags_t flag;
  Return_t xSystemAssert(const char *file_, const int line_);
  Return_t xSystemInit(void);
  Return_t xSystemHalt(void);
  Return_t xSystemGetSystemInfo(SystemInfo_t **info_);
  #if defined(CONFIG_ENABLE_ARDUINO_CPP_INTERFACE)
    void __ArduinoAssert__(const char *file_, int line_);
  #endif
  #if defined(POSIX_ARCH_OTHER)
    void __SysStateClear__(void);
  #endif
  #ifdef __cplusplus
    }
  #endif
#endif