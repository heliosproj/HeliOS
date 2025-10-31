/*UNCRUSTIFY-OFF*/
/**
 * @file arduino_helpers.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Arduino platform helper functions
 * @details
 * Defines Arduino-specific utility functions and macros for digital I/O, timing, and serial communication to simplify Arduino integration.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/


#ifndef ARDUINO_HELPERS_H_


  #define ARDUINO_HELPERS_H_


  #ifdef __cplusplus


    #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || \
    defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || \
    defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)


      #include <Arduino.h>


      #include "HeliOS.h"
      /**
       * @brief Converts byte array to Arduino String
       * @details Helper function to convert a byte array to an Arduino String object for easy display and manipulation.
       *
       * @param[in] size_  Size of byte array
       * @param[in] bytes_ Pointer to byte array
       *
       * @return           Arduino String object containing the byte data
       *
       * @note This function is only available on Arduino platforms with C++
       * support
       */
      String xByte2String(Size_t size_, Byte_t *bytes_);


    #endif /* if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || */


    /* defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || */


    /* defined(ARDUINO_ARCH_STM32) || */


  #endif /* ifdef __cplusplus */


#endif /* ifndef ARDUINO_HELPERS_H_ */