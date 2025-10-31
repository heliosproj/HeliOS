#ifndef POSIX_H_

#define POSIX_H_

#if defined(POSIX_ARCH_OTHER)

  #if defined(_POSIX_C_SOURCE)

    #undef _POSIX_C_SOURCE

  #endif // if defined(_POSIX_C_SOURCE)

  #define _POSIX_C_SOURCE 200809L

#endif // if defined(POSIX_ARCH_OTHER)

#endif // ifndef POSIX_H_