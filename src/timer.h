#ifndef TIMER_H_

#define TIMER_H_

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

#include "queue.h"

#include "streams.h"

#include "sys.h"

#include "task.h"

#ifdef __cplusplus

  extern "C" {

#endif /* ifdef __cplusplus */
Return_t xTimerCreate(Timer_t **timer_, const Ticks_t period_);
Return_t xTimerDelete(const Timer_t *timer_);
Return_t xTimerChangePeriod(Timer_t *timer_, const Ticks_t period_);
Return_t xTimerGetPeriod(const Timer_t *timer_, Ticks_t *period_);
Return_t xTimerIsTimerActive(const Timer_t *timer_, Base_t *res_);
Return_t xTimerHasTimerExpired(const Timer_t *timer_, Base_t *res_);
Return_t xTimerReset(Timer_t *timer_);
Return_t xTimerStart(Timer_t *timer_);
Return_t xTimerStop(Timer_t *timer_);

#ifdef __cplusplus

  }

#endif /* ifdef __cplusplus */

#endif /* ifndef TIMER_H_ */