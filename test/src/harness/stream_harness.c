/*UNCRUSTIFY-OFF*/
/**
 * @file stream_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.5.0
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
#include "stream_harness.h"


void stream_harness(void) {
  StreamBuffer_t *stream01 = null;
  HalfWord_t i = nil;
  HalfWord_t stream02 = nil;
  Byte_t *stream03 = null;
  HalfWord_t stream04 = nil;
  Base_t res;


  unit_begin("Stream buffer creation succeeds");
  unit_try(OK(xStreamCreate(&stream01)));
  unit_try(null != stream01);
  unit_end();
  unit_begin("Stream accepts bytes until buffer is full");

  for(i = 0; i < CONFIG_STREAM_BUFFER_BYTES; i++) {
    unit_try(OK(xStreamSend(stream01, i)));
  }

  unit_try(!OK(xStreamSend(stream01, nil)));
  unit_end();
  unit_begin("Stream full check correctly identifies full buffer");
  unit_try(OK(xStreamIsFull(stream01, &res)));
  unit_try(true == res);
  unit_end();
  unit_begin("Stream empty check correctly identifies non-empty buffer");
  unit_try(OK(xStreamIsEmpty(stream01, &res)));
  unit_try(false == res);
  unit_end();
  unit_begin("Stream bytes available returns buffer capacity");
  unit_try(OK(xStreamBytesAvailable(stream01, &stream04)));
  unit_try(0x20u == stream04);
  unit_end();
  unit_begin("Stream receive returns all buffered bytes");
  unit_try(OK(xStreamReceive(stream01, &stream02, &stream03)));
  unit_try(null != stream03);
  unit_try(0x20u == stream02);
  unit_try(0x1Fu == stream03[0x1Fu]);
  unit_end();
  unit_begin("Stream reset clears buffer contents");

  for(i = 0; i < CONFIG_STREAM_BUFFER_BYTES; i++) {
    unit_try(OK(xStreamSend(stream01, i)));
  }

  unit_try(!OK(xStreamSend(stream01, nil)));
  unit_try(OK(xStreamIsFull(stream01, &res)));
  unit_try(true == res);
  unit_try(OK(xStreamReset(stream01)));
  unit_try(OK(xStreamIsEmpty(stream01, &res)));
  unit_try(true == res);
  unit_end();
  unit_begin("Stream delete invalidates stream handle");
  unit_try(!OK(xStreamReset(stream01)));
  unit_try(OK(xStreamDelete(stream01)));
  unit_try(!OK(xStreamSend(stream01, nil)));
  unit_end();

  return;
}