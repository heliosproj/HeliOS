/*UNCRUSTIFY-OFF*/
/**
 * @file stream_harness.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.4.0
 * @date 2022-08-30
 *
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2023 Manny Peterson <mannymsp@gmail.com>
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
/*UNCRUSTIFY-ON*/
#include "stream_harness.h"


void stream_harness(void) {
  StreamBuffer_t *stream01 = null;
  HalfWord_t i = zero;
  HalfWord_t stream02 = zero;
  Byte_t *stream03 = null;
  HalfWord_t stream04 = zero;
  Base_t res;


  unit_begin("xStreamCreate()");
  unit_try(ISSUCCESSFUL(xStreamCreate(&stream01)));
  unit_try(null != stream01);
  unit_end();
  unit_begin("xStreamSend()");

  for(i = 0; i < CONFIG_STREAM_BUFFER_BYTES; i++) {
    unit_try(ISSUCCESSFUL(xStreamSend(stream01, i)));
  }

  unit_try(!ISSUCCESSFUL(xStreamSend(stream01, zero)));
  unit_end();
  unit_begin("xStreamIsFull()");
  unit_try(ISSUCCESSFUL(xStreamIsFull(stream01, &res)));
  unit_try(true == res);
  unit_end();
  unit_begin("xStreamIsEmpty()");
  unit_try(ISSUCCESSFUL(xStreamIsEmpty(stream01, &res)));
  unit_try(false == res);
  unit_end();
  unit_begin("xStreamBytesAvailable()");
  unit_try(ISSUCCESSFUL(xStreamBytesAvailable(stream01, &stream04)));
  unit_try(0x20u == stream04);
  unit_end();
  unit_begin("xStreamReceive()");
  unit_try(ISSUCCESSFUL(xStreamReceive(stream01, &stream02, &stream03)));
  unit_try(null != stream03);
  unit_try(0x20u == stream02);
  unit_try(0x1Fu == stream03[0x1Fu]);
  unit_end();
  unit_begin("xStreamReset()");

  for(i = 0; i < CONFIG_STREAM_BUFFER_BYTES; i++) {
    unit_try(ISSUCCESSFUL(xStreamSend(stream01, i)));
  }

  unit_try(!ISSUCCESSFUL(xStreamSend(stream01, zero)));
  unit_try(ISSUCCESSFUL(xStreamIsFull(stream01, &res)));
  unit_try(true == res);
  unit_try(ISSUCCESSFUL(xStreamReset(stream01)));
  unit_try(ISSUCCESSFUL(xStreamIsEmpty(stream01, &res)));
  unit_try(true == res);
  unit_end();
  unit_begin("xStreamDelete()");

  unit_try(ISSUCCESSFUL(xStreamReset(stream01)));

  unit_try(ISSUCCESSFUL(xStreamDelete(stream01)));

  unit_try(!ISSUCCESSFUL(xStreamSend(stream01, zero)));
  unit_end();

  return;
}