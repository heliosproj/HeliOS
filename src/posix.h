/**
 * @file posix.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief POSIX header file to use POSIX definitions in header files
 * @version 0.3.5
 * @date 2022-08-20
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
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

#ifndef POSIX_H_
#define POSIX_H_

#if defined(POSIX_ARCH_OTHER)
#if defined(_POSIX_C_SOURCE)
#undef _POSIX_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200809L
#endif


#endif