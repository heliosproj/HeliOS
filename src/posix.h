#ifndef POSIX_H_
  #define POSIX_H_
  #if defined(POSIX_ARCH_OTHER)
    #if defined(_POSIX_C_SOURCE)
      #undef _POSIX_C_SOURCE
    #endif
    #define _POSIX_C_SOURCE 0x31069u
  #endif
#endif