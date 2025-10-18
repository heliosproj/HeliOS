#include "arduino_helpers.h"

#ifdef __cplusplus

  #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32) || \
  defined(ARDUINO_TEENSY_MICROMOD) || defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) || \
  defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC)


    String xByte2String(Size_t size_, Byte_t *bytes_) {
      String str = "";
      Size_t i = 0;
      char buf[size_ + 1];


      if((bytes_ != nullptr) && ((Size_t)0 < size_)) {
        for(i = 0; i < size_; i++) {
          buf[i] = (char) bytes_[i];
        }

        buf[size_] = '\0';
        str = String(buf);
      }

      return(str);
    }


  #endif /* if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) ||
          * defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266) ||
          * defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_TEENSY_MICROMOD) ||
          * defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) ||
          * defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY35) ||
          * defined(ARDUINO_TEENSY31) || defined(ARDUINO_TEENSY32) ||
          * defined(ARDUINO_TEENSY30) || defined(ARDUINO_TEENSYLC) */

#endif /* ifdef __cplusplus */