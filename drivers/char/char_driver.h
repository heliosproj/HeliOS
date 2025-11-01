#ifndef CHAR_DRIVER_H_
  #define CHAR_DRIVER_H_
  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)
    #include "console.h"
    #include "device.h"
    #include "fat.h"
    #include "fs.h"
  #endif
  #include "mem.h"
  #include "port.h"
  #include "posix.h"
  #include "queue.h"
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"
  #include "char_io_interface.h"
  #if defined(DEVICE_NAME)
    #undef DEVICE_NAME
  #endif
  #define DEVICE_NAME CHARDEV0
  #if defined(DEVICE_UID)
    #undef DEVICE_UID
  #endif
  #define DEVICE_UID 0x2000u
  #if defined(DEVICE_MODE)
    #undef DEVICE_MODE
  #endif
  #define DEVICE_MODE DeviceModeReadWrite
  #if defined(DEVICE_STATE)
    #undef DEVICE_STATE
  #endif
  #define DEVICE_STATE DeviceStateRunning
  #if defined(CHAR_CMD_CONFIG)
    #undef CHAR_CMD_CONFIG
  #endif
  #define CHAR_CMD_CONFIG 0x01u
  #if defined(CHAR_CMD_SET_PARAMS)
    #undef CHAR_CMD_SET_PARAMS
  #endif
  #define CHAR_CMD_SET_PARAMS 0x02u
  #if defined(CHAR_CMD_GET_INFO)
    #undef CHAR_CMD_GET_INFO
  #endif
  #define CHAR_CMD_GET_INFO 0x03u
  #if defined(CHAR_PROTOCOL_UART)
    #undef CHAR_PROTOCOL_UART
  #endif
  #define CHAR_PROTOCOL_UART 0x01u
  #if defined(CHAR_PROTOCOL_USART)
    #undef CHAR_PROTOCOL_USART
  #endif
  #define CHAR_PROTOCOL_USART 0x02u
  #if defined(CHAR_PROTOCOL_USB_CDC)
    #undef CHAR_PROTOCOL_USB_CDC
  #endif
  #define CHAR_PROTOCOL_USB_CDC 0x03u
  #if defined(CHAR_PROTOCOL_RAW)
    #undef CHAR_PROTOCOL_RAW
  #endif
  #define CHAR_PROTOCOL_RAW 0xFFu
  #if defined(CHAR_LINE_RAW)
    #undef CHAR_LINE_RAW
  #endif
  #define CHAR_LINE_RAW 0x00u
  #if defined(CHAR_LINE_COOKED)
    #undef CHAR_LINE_COOKED
  #endif
  #define CHAR_LINE_COOKED 0x01u
  #if defined(CHAR_DEFAULT_TIMEOUT_MS)
    #undef CHAR_DEFAULT_TIMEOUT_MS
  #endif
  #define CHAR_DEFAULT_TIMEOUT_MS 0x3E8u
  #if defined(CHAR_SINGLE_BYTE_COUNT)
    #undef CHAR_SINGLE_BYTE_COUNT
  #endif
  #define CHAR_SINGLE_BYTE_COUNT 0x1u
  typedef struct CharDeviceConfig_s {
    Byte_t command;
    HalfWord_t ioDriverUID;
    Byte_t protocol;
    Byte_t lineMode;
    Word_t baudRate;
    HalfWord_t rxBufferSize;
    HalfWord_t txBufferSize;
  } CharDeviceConfig_t;
  typedef struct CharDeviceCommand_s {
    Byte_t command;
    HalfWord_t byteCount;
    Byte_t transferMode;
  } CharDeviceCommand_t;
typedef struct CharDeviceState_s {
  HalfWord_t ioDriverUID;
  Byte_t protocol;
  Byte_t lineMode;
  Word_t baudRate;
  Base_t initialized;
  HalfWord_t rxBufferSize;
  HalfWord_t txBufferSize;
  Byte_t *rxBuffer;
  Byte_t *txBuffer;
  HalfWord_t rxHead;
  HalfWord_t rxTail;
  HalfWord_t txHead;
  HalfWord_t txTail;
  HalfWord_t currentByteCount;
  Byte_t currentTransferMode;
} CharDeviceState_t;
  typedef struct CharDeviceInfo_s {
    Byte_t command;
    Byte_t protocol;
    Byte_t lineMode;
    Word_t baudRate;
    HalfWord_t rxBytesAvailable;
    HalfWord_t txBytesFree;
    Base_t isInitialized;
    Base_t isConnected;
  } CharDeviceInfo_t;
  #ifdef __cplusplus
    extern "C" {
  #endif
  Return_t TO_FUNCTION(DEVICE_NAME, _self_register)(void);
  Return_t TO_FUNCTION(DEVICE_NAME, _init)(Device_t *device_);
  Return_t TO_FUNCTION(DEVICE_NAME, _config)(Device_t *device_, Size_t *size_, Addr_t *config_);
  Return_t TO_FUNCTION(DEVICE_NAME, _read)(Device_t *device_, Size_t *size_, Addr_t **data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _write)(Device_t *device_, Size_t *size_, Addr_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_read)(Device_t *device_, Byte_t *data_);
  Return_t TO_FUNCTION(DEVICE_NAME, _simple_write)(Device_t *device_, Byte_t data_);
  Return_t __PrepareCharIORequest__(const Byte_t operation_, CharIORequest_t **request_, Size_t *configSize_);
Return_t __CharDeviceReadRAW__(Byte_t **data_, Size_t *bytesRead_);
Return_t __CharDeviceWriteRAW__(const Byte_t *data_);
HalfWord_t __CircularBufferSpace__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
HalfWord_t __CircularBufferAvailable__(const HalfWord_t head_, const HalfWord_t tail_, const HalfWord_t size_);
  #if defined(POSIX_ARCH_OTHER)
    void __CharDeviceStateClear__(void);
  #endif
  #ifdef __cplusplus
    }
  #endif
#endif