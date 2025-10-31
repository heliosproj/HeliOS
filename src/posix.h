/*UNCRUSTIFY-OFF*/


/**
 * @file posix.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief POSIX compatibility layer header
 * @details
 * Defines POSIX-compatible function prototypes for memory operations, string handling, and system utilities to support testing on host systems.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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