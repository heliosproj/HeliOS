#include "sys.h"
Flags_t flag = {
  VALID,
  0,
  0,
  0,
  0
};
Return_t xSystemAssert(const char *file_, const int line_) {
  FUNCTION_ENTER;
#if defined(CONFIG_SYSTEM_ASSERT_BEHAVIOR)
    CONFIG_SYSTEM_ASSERT_BEHAVIOR(file_, line_);
    __ReturnOk__();
#endif
  FUNCTION_EXIT;
}
Return_t xSystemInit(void) {
  FUNCTION_ENTER;
  if(OK(__MemoryInit__())) {
    if(OK(__PortInit__())) {
      __UnsetFlag__(OVERFLOW);
      __UnsetFlag__(RUNNING);
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
Return_t xSystemHalt(void) {
  FUNCTION_ENTER;
  __DisableInterrupts__();
  for(;;) {
  }
  FUNCTION_EXIT;
}
Return_t xSystemGetSystemInfo(SystemInfo_t **info_) {
  FUNCTION_ENTER;
  if(__PointerIsNotNull__(info_)) {
    if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, sizeof(SystemInfo_t)))) {
      if(__PointerIsNotNull__(*info_)) {
        (*info_)->valid = VALID;
        if(OK(__memcpy__((*info_)->productName, OS_PRODUCT_NAME, OS_PRODUCT_NAME_SIZE))) {
          (*info_)->majorVersion = OS_MAJOR_VERSION_NO;
          (*info_)->minorVersion = OS_MINOR_VERSION_NO;
          (*info_)->patchVersion = OS_PATCH_VERSION_NO;
          if(__FlagIsSet__(LITTLEEND)) {
            (*info_)->littleEndian = true;
          } else {
            (*info_)->littleEndian = false;
          }
          if(OK(xTaskGetNumberOfTasks(&(*info_)->numberOfTasks))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();
            (*info_)->valid = INVALID;
            __HeapFreeMemory__(*info_);
          }
        } else {
          __AssertOnElse__();
          (*info_)->valid = INVALID;
          __HeapFreeMemory__(*info_);
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }
  FUNCTION_EXIT;
}
#if defined(POSIX_ARCH_OTHER)
  void __SysStateClear__(void) {
    __memset__(&flag, 0x0u, sizeof(Flags_t));
    return;
  }
#endif