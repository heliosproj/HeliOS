/*UNCRUSTIFY-OFF*/
/**
 * @file stream.h
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for stream buffers
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
#ifndef STREAM_H_
  #define STREAM_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #include "port.h"
  #include "device.h"
  #include "mem.h"
  #include "queue.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  Return_t xStreamCreate(StreamBuffer_t **stream_);
  Return_t xStreamDelete(const StreamBuffer_t *stream_);
  Return_t xStreamSend(StreamBuffer_t *stream_, const Byte_t byte_);
  Return_t xStreamReceive(const StreamBuffer_t *stream_, HalfWord_t *bytes_, Byte_t **data_);
  Return_t xStreamBytesAvailable(const StreamBuffer_t *stream_, HalfWord_t *bytes_);
  Return_t xStreamReset(const StreamBuffer_t *stream_);
  Return_t xStreamIsEmpty(const StreamBuffer_t *stream_, Base_t *res_);
  Return_t xStreamIsFull(const StreamBuffer_t *stream_, Base_t *res_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef STREAM_H_ */