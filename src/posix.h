/*UNCRUSTIFY-OFF*/
/**
 * @file posix.h
 * @author Manny Peterson <manny@heliosproject.org>
 * @brief Kernel source for setting POSIX standard used during compilation of unit tests
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * (C) 2020-2026 Manny Peterson <manny@heliosproject.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#ifndef POSIX_H_
  #define POSIX_H_

  #if defined(POSIX_ARCH_OTHER)
    #if defined(_POSIX_C_SOURCE)
      #undef _POSIX_C_SOURCE
    #endif /* if defined(_POSIX_C_SOURCE) */
    #define _POSIX_C_SOURCE 200809L
  #endif /* if defined(POSIX_ARCH_OTHER) */
#endif /* ifndef POSIX_H_ */