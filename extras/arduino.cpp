/**
 * @file arduino.cpp
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Source code to allow the HeliOS kernel to interface with the C++ Arduino API.
 * @version 0.3.3
 * @date 2022-02-25
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


#include "config.h"

#if defined(CONFIG_ENABLE_ARDUINO_CPP_INTERFACE)

#ifdef __cplusplus

#include <Arduino.h>


extern "C" void ArduinoAssert(const char *file_, int line_);


void ArduinoAssert(const char *file_, int line_)
{

    Serial.println("assert: " + String(file_) + ":" + String(line_, DEC));

    return;
}

#endif

#endif