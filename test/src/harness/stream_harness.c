/**
 * @file stream_harness.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief
 * @version 0.3.5
 * @date 2022-08-30
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

#include "stream_harness.h"

void stream_harness(void) {

  StreamBuffer_t *stream01 = NULL;

  HalfWord_t i = zero;

  HalfWord_t stream02 = zero;

  Byte_t *stream03 = NULL;



  unit_begin("xStreamCreate()");

  stream01 = xStreamCreate();

  unit_try(NULL != stream01);

  unit_end();



  unit_begin("xStreamSend()");

  for (i = 0; i < CONFIG_STREAM_BUFFER_BYTES; i++) {

    unit_try(RETURN_SUCCESS == xStreamSend(stream01, i));
  }

  unit_try(RETURN_FAILURE == xStreamSend(stream01, zero));

  unit_end();



  unit_begin("xStreamIsFull()");

  unit_try(true == xStreamIsFull(stream01));

  unit_end();




  unit_begin("xStreamIsEmpty()");

  unit_try(false == xStreamIsEmpty(stream01));

  unit_end();



  unit_begin("xStreamBytesAvailable()");

  unit_try(0x20u == xStreamBytesAvailable(stream01));

  unit_end();



  unit_begin("xStreamReceive()");

  stream03 = xStreamReceive(stream01, &stream02);

  unit_try(NULL != stream03);

  unit_try(0x20u == stream02);

  unit_try(0x1Fu == stream03[0x1Fu]);

  unit_end();



  unit_begin("xStreamReset()");

  for (i = 0; i < CONFIG_STREAM_BUFFER_BYTES; i++) {

    unit_try(RETURN_SUCCESS == xStreamSend(stream01, i));
  }

  unit_try(RETURN_FAILURE == xStreamSend(stream01, zero));

  unit_try(true == xStreamIsFull(stream01));

  xStreamReset(stream01);

  unit_try(true == xStreamIsEmpty(stream01));

  unit_end();



  unit_begin("xStreamDelete()");

  xStreamReset(stream01);

  xStreamDelete(stream01);

  unit_try(RETURN_FAILURE == xStreamSend(stream01, zero));

  unit_end();



  return;
}
