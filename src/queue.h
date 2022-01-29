/*
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
 */
#ifndef QUEUE_H_
#define QUEUE_H_

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "defines.h"
#include "types.h"
#include "mem.h"
#include "sched.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

Queue_t *xQueueCreate(int16_t);
void xQueueDelete(Queue_t *);
int16_t xQueueGetLength(Queue_t *);
int16_t xQueueIsQueueEmpty(Queue_t *);
int16_t xQueueIsQueueFull(Queue_t *);
int16_t xQueueMessagesWaiting(Queue_t *);
int16_t xQueueSend(Queue_t *, int16_t, char *);
QueueMessage_t *xQueuePeek(Queue_t *);
void xQueueDropMessage(Queue_t *);
QueueMessage_t *xQueueReceive(Queue_t *); 

#ifdef __cplusplus
}  // extern "C" {
#endif
#endif