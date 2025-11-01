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
  #endif
  #include "mem.h"
  #include "port.h"
  #include "posix.h"
  #include "streams.h"
  #include "sys.h"
  #include "task.h"
  #include "timer.h"
  #ifdef __cplusplus
    extern "C" {
  #endif
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
  Return_t __QueueDropmessage__(Queue_t *queue_);
  Return_t __QueuePeek__(const Queue_t *queue_, QueueMessage_t **message_);
  #ifdef __cplusplus
    }
  #endif
#endif