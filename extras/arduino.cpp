/*UNCRUSTIFY-OFF*/
/**
 * @file arduino.cpp
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source to allow kernel to interface with Arduino C++ API
 * @version 0.4.1
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2023 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/

#include "config.h"

#if defined(CONFIG_ENABLE_ARDUINO_CPP_INTERFACE)

#ifdef __cplusplus

#include <Arduino.h>


extern "C" void __ArduinoAssert__(const char *file_, int line_);


void __ArduinoAssert__(const char *file_, int line_)
{

    Serial.println("kernel: assert at " + String(file_) + ":" + String(line_, DEC));

    return;
}

#endif

#endif