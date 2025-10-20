/*UNCRUSTIFY-OFF*/
/**
 * @file mock_usart.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Mock USART device for console unit testing
 * @version 0.5.0
 * @date 2025-01-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef MOCK_USART_H_
  #define MOCK_USART_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "../../../drivers/char/char_io_interface.h"

  /* Mock USART device identifier and name */
  #define MOCK_USART_DEVICE_UID 0x0100u
  #define MOCK_USART_DEVICE_NAME "MOCKUSRT"

  /* Buffer sizes */
  #define MOCK_USART_RX_BUFFER_SIZE 2048u
  #define MOCK_USART_TX_BUFFER_SIZE 2048u

  /* Mock USART device state */
  typedef struct MockUSARTState_s {
    Byte_t rxBuffer[MOCK_USART_RX_BUFFER_SIZE];
    Byte_t txBuffer[MOCK_USART_TX_BUFFER_SIZE];
    HalfWord_t rxHead;
    HalfWord_t rxTail;
    HalfWord_t txHead;
    HalfWord_t txTail;
    CharIORequest_t currentRequest;
    Base_t initialized;
  } MockUSARTState_t;

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */

  /* Device interface functions */
  Return_t MOCKUSRT_self_register(void);
  Return_t MOCKUSRT_init(Device_t *device_);
  Return_t MOCKUSRT_config(Device_t *device_, Size_t *size_, Addr_t *config_);
  Return_t MOCKUSRT_read(Device_t *device_, Size_t *size_, Addr_t **data_);
  Return_t MOCKUSRT_write(Device_t *device_, Size_t *size_, Addr_t *data_);
  Return_t MOCKUSRT_simple_read(Device_t *device_, Byte_t *data_);
  Return_t MOCKUSRT_simple_write(Device_t *device_, Byte_t data_);

  /* Test helper functions */
  void xMockUSARTInjectInput(const Byte_t *input_);
  void xMockUSARTGetOutput(Byte_t *buffer_, HalfWord_t maxLen_, HalfWord_t *actualLen_);
  void xMockUSARTReset(void);
  HalfWord_t xMockUSARTGetRxAvailable(void);
  HalfWord_t xMockUSARTGetTxCount(void);
  void xMockUSARTClearOutput(void);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */

#endif /* ifndef MOCK_USART_H_ */
