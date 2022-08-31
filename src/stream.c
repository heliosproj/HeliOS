/**
 * @file stream.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for stream buffers
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
#include "stream.h"



/* The xStreamCreate() system call will create a new stream. Streams
are fast for sending streams of bytes between tasks. */
StreamBuffer_t *xStreamCreate() {


  return NULL;
}



/* The xStreamDelete() system call will delete a stream. Streams
are fast for sending streams of bytes between tasks. */
void xStreamDelete(StreamBuffer_t *stream_) {


  return;
}



/* The xStreamSend() system call will send one byte to
the designated stream. Sending to a stream must be done
one byte at a time. However, receiving from a stream
will return all available bytes in a stream. */
Base_t xStreamSend(StreamBuffer_t *stream_, Byte_t byte_) {


  return false;
}



/* The xStreamReceive() system call will receive all waiting bytes
in the stream. */
Byte_t *xStreamReceive(StreamBuffer_t *stream_, HWord_t *bytes_) {


  return NULL;
}



/* The xStreamBytesAvailable() system call will return the number
of waiting bytes in the stream. */
HWord_t xStreamBytesAvailable(StreamBuffer_t *stream_) {


  return zero;
}



/* The xStreamReset() system call will reset the stream and clear
its buffer of all waiting bytes. */
void xStreamReset(StreamBuffer_t *stream_) {

  return;
}



/* The xStreamIsEmpty() system call will return true if the stream
buffer is empty (i.e., zero in size). */
Base_t xStreamIsEmpty(StreamBuffer_t *stream_) {

  return false;
}



/* The xStreamIsFull() system call will return true if the stream
buffer is full (i.e., size is equal to CONFIG_STREAM_BUFFER_BYTES) */
Base_t xStreamIsFull(StreamBuffer_t *steam_) {


  return false;
}


/*

double queue_peek(void)
{
  return temps.arr[temps.head];
}

int queue_empty(void)
{
  int ret = false;

  if (0 == temps.size)
  {
    ret = true;
  }

  return ret;
}

int queue_full(void)
{
  int ret = false;

  if (MAX_QUEUE == temps.size)
  {
    ret = true;
  }

  return ret;
}

int queue_size(void)
{
  return temps.size;
}

void queue_add(double *tmp_)
{

  if (false == queue_full())
  {
    if ((MAX_QUEUE - 1) == temps.tail)
    {
      temps.tail = -1;
    }

    temps.arr[++temps.tail] = *tmp_;
    temps.size++;
  }

  return;
}

void queue_remove(double *tmp_)
{
  *tmp_ = temps.arr[temps.head++];

  if (MAX_QUEUE == temps.head)
  {
    temps.head = 0;
  }

  temps.size--;

  return;
}

*/