/*UNCRUSTIFY-OFF*/
/**
 * @file stream_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit testing sources
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#include "stream_harness.h"


/* Test constants */
#define STREAM_BUFFER_SIZE 0x20u /* 32 bytes - stream buffer capacity */
#define STREAM_LAST_BYTE_INDEX 0x1Fu /* Index of last byte in buffer */


void stream_harness(void) {
  StreamBuffer_t *stream01 = null;
  HalfWord_t i = nil;
  HalfWord_t stream02 = nil;
  Byte_t *stream03 = null;
  HalfWord_t stream04 = nil;
  Base_t res;


  unit_begin("Stream buffer creation succeeds");
  unit_assert_ok(xStreamCreate(&stream01));
  unit_assert_not_null(stream01);
  unit_end();
  unit_begin("Stream accepts bytes until buffer is full");

  for(i = 0; i < CONFIG_STREAM_BUFFER_BYTES; i++) {
    unit_assert_ok(xStreamSend(stream01, i));
  }

  unit_assert_not_ok(xStreamSend(stream01, nil));
  unit_end();
  unit_begin("Stream full check correctly identifies full buffer");
  unit_assert_ok(xStreamIsFull(stream01, &res));
  unit_assert_true(res);
  unit_end();
  unit_begin("Stream empty check correctly identifies non-empty buffer");
  unit_assert_ok(xStreamIsEmpty(stream01, &res));
  unit_assert_false(res);
  unit_end();
  unit_begin("Stream bytes available returns buffer capacity");
  unit_assert_ok(xStreamBytesAvailable(stream01, &stream04));
  unit_assert_equal(stream04, STREAM_BUFFER_SIZE);
  unit_end();
  unit_begin("Stream receive returns all buffered bytes");
  unit_assert_ok(xStreamReceive(stream01, &stream02, &stream03));
  unit_assert_not_null(stream03);
  unit_assert_equal(stream02, STREAM_BUFFER_SIZE);
  unit_assert_equal(stream03[STREAM_LAST_BYTE_INDEX], STREAM_LAST_BYTE_INDEX);
  unit_end();
  unit_begin("Stream reset clears buffer contents");

  for(i = 0; i < CONFIG_STREAM_BUFFER_BYTES; i++) {
    unit_assert_ok(xStreamSend(stream01, i));
  }

  unit_assert_not_ok(xStreamSend(stream01, nil));
  unit_assert_ok(xStreamIsFull(stream01, &res));
  unit_assert_true(res);
  unit_assert_ok(xStreamReset(stream01));
  unit_assert_ok(xStreamIsEmpty(stream01, &res));
  unit_assert_true(res);
  unit_end();
  unit_begin("Stream delete invalidates stream handle");
  unit_assert_not_ok(xStreamReset(stream01));
  unit_assert_ok(xStreamDelete(stream01));
  unit_assert_not_ok(xStreamSend(stream01, nil));
  unit_end();

  return;
}