/*UNCRUSTIFY-OFF*/
/**
 * @file queue.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for message queue inter-task communication
 * @version 0.5.0
 * @date 2023-03-19
 * 
 * @copyright
 * (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *  
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  
 * 
 */
/*UNCRUSTIFY-ON*/
#ifndef QUEUE_H_
  #define QUEUE_H_

  #include "config.h"
  #include "defines.h"
  #include "types.h"
  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)
    #include "console.h"
    #include "device.h"
    #include "fat.h"
    #include "fs.h"
  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */
  #include "mem.h"
  #include "port.h"
  #include "posix.h"
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"

  #ifdef __cplusplus
    extern "C" {
  #endif /* ifdef __cplusplus */
  Return_t xQueueCreate(Queue_t **queue_, const Base_t limit_);
  Return_t xQueueDelete(Queue_t *queue_);
  Return_t xQueueGetLength(const Queue_t *queue_, Base_t *res_);
  Return_t xQueueIsQueueEmpty(const Queue_t *queue_, Base_t *res_);
  Return_t xQueueIsQueueFull(const Queue_t *queue_, Base_t *res_);
  Return_t xQueueMessagesWaiting(const Queue_t *queue_, Base_t *res_);
  Return_t xQueueSend(Queue_t *queue_, const Base_t bytes_, const Byte_t *value_);
  Return_t xQueuePeek(const Queue_t *queue_, QueueMessage_t **message_);
  Return_t xQueueDropMessage(Queue_t *queue_);
  Return_t xQueueReceive(Queue_t *queue_, QueueMessage_t **message_);
  Return_t xQueueLockQueue(Queue_t *queue_);
  Return_t xQueueUnLockQueue(Queue_t *queue_);

  #ifdef __cplusplus
    }
  #endif /* ifdef __cplusplus */
#endif /* ifndef QUEUE_H_ */