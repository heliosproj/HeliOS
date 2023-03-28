/*UNCRUSTIFY-OFF*/
/**
 * @file task.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for tasks and task management
 * @version 0.4.1
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
#include "task.h"



static TaskList_t *tlist = null;
static void __RunTimeReset__(void);
static void __TaskRun__(Task_t *task_);
static Return_t __TaskListFindTask__(const Task_t *task_);



static SchedulerState_t schedulerState = SchedulerStateRunning;


Return_t xTaskCreate(Task_t **task_, const Byte_t *name_, void (*callback_)(Task_t *task_, TaskParm_t *parm_), TaskParm_t *taskParameter_) {
  RET_DEFINE;


  Task_t *cursor = null;


  if(NOTNULLPTR(task_) && (NOTNULLPTR(name_)) && (NOTNULLPTR(callback_)) && (false == FLAG_RUNNING)) {
    /* NOTE: There is a __KernelAllocateMemory__() syscall buried in this if()
     * statement. */
    if(NOTNULLPTR(tlist) || (NULLPTR(tlist) && OK(__KernelAllocateMemory__((volatile Addr_t **) &tlist, sizeof(TaskList_t))))) {
      if(OK(__KernelAllocateMemory__((volatile Addr_t **) task_, sizeof(Task_t)))) {
        if(NOTNULLPTR(*task_)) {
          if(OK(__memcpy__((*task_)->name, name_, CONFIG_TASK_NAME_BYTES))) {
            tlist->nextId++;
            (*task_)->id = tlist->nextId;
            (*task_)->state = TaskStateSuspended;
            (*task_)->callback = callback_;
            (*task_)->taskParameter = taskParameter_;
            (*task_)->next = null;
            cursor = tlist->head;

            if(NOTNULLPTR(tlist->head)) {
              while(NOTNULLPTR(cursor->next)) {
                cursor = cursor->next;
              }

              cursor->next = *task_;
            } else {
              tlist->head = *task_;
            }

            tlist->length++;
            RET_OK;
          } else {
            ASSERT;


            /* Free kernel memory because __memcpy__() failed.*/
            __KernelFreeMemory__(*task_);
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskDelete(const Task_t *task_) {
  RET_DEFINE;


  Task_t *cursor = null;
  Task_t *previous = null;


  if(NOTNULLPTR(task_) && NOTNULLPTR(tlist) && (false == FLAG_RUNNING)) {
    if(OK(__TaskListFindTask__(task_))) {
      cursor = tlist->head;

      if((NOTNULLPTR(cursor)) && (cursor == task_)) {
        tlist->head = cursor->next;

        if(OK(__KernelFreeMemory__(cursor))) {
          tlist->length--;
          RET_OK;
        } else {
          ASSERT;
        }
      } else if((NOTNULLPTR(cursor)) && (cursor != task_)) {
        while((NOTNULLPTR(cursor)) && (cursor != task_)) {
          previous = cursor;
          cursor = cursor->next;
        }

        if(NOTNULLPTR(cursor)) {
          previous->next = cursor->next;

          if(OK(__KernelFreeMemory__(cursor))) {
            tlist->length--;
            RET_OK;
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetHandleByName(Task_t **task_, const Byte_t *name_) {
  RET_DEFINE;


  Task_t *cursor = null;
  Base_t res = false;


  if((NOTNULLPTR(task_)) && (NOTNULLPTR(name_)) && (NOTNULLPTR(tlist))) {
    cursor = tlist->head;

    while(NOTNULLPTR(cursor)) {
      if(OK(__memcmp__(cursor->name, name_, CONFIG_TASK_NAME_BYTES, &res))) {
        if(true == res) {
          *task_ = cursor;
          RET_OK;
          break;
        }
      } else {
        ASSERT;
        break;
      }

      cursor = cursor->next;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetHandleById(Task_t **task_, const Base_t id_) {
  RET_DEFINE;


  Task_t *cursor = null;


  if((NOTNULLPTR(task_)) && (zero < id_) && (NOTNULLPTR(tlist))) {
    cursor = tlist->head;

    while(NOTNULLPTR(cursor)) {
      if(id_ == cursor->id) {
        *task_ = cursor;
        RET_OK;
        break;
      }

      cursor = cursor->next;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetAllRunTimeStats(TaskRunTimeStats_t **stats_, Base_t *tasks_) {
  RET_DEFINE;


  Base_t task = zero;
  Base_t tasks = zero;
  Task_t *cursor = null;


  if((NOTNULLPTR(stats_)) && (NOTNULLPTR(tasks_)) && (NOTNULLPTR(tlist))) {
    cursor = tlist->head;

    while(NOTNULLPTR(cursor)) {
      tasks++;
      cursor = cursor->next;
    }

    if((zero < tasks) && (tasks == tlist->length)) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) stats_, tasks * sizeof(TaskRunTimeStats_t)))) {
        if(NOTNULLPTR(*stats_)) {
          cursor = tlist->head;

          while(NOTNULLPTR(cursor)) {
            (*stats_)[task].id = cursor->id;
            (*stats_)[task].lastRunTime = cursor->lastRunTime;
            (*stats_)[task].totalRunTime = cursor->totalRunTime;
            cursor = cursor->next;
            task++;
          }

          *tasks_ = tasks;
          RET_OK;
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetTaskRunTimeStats(const Task_t *task_, TaskRunTimeStats_t **stats_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(stats_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) stats_, sizeof(TaskRunTimeStats_t)))) {
        if(NOTNULLPTR(*stats_)) {
          (*stats_)->id = task_->id;
          (*stats_)->lastRunTime = task_->lastRunTime;
          (*stats_)->totalRunTime = task_->totalRunTime;
          RET_OK;
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetNumberOfTasks(Base_t *tasks_) {
  RET_DEFINE;


  Base_t tasks = zero;
  Task_t *cursor = null;


  if(NOTNULLPTR(tasks_) && NOTNULLPTR(tlist)) {
    cursor = tlist->head;

    while(NOTNULLPTR(cursor)) {
      tasks++;
      cursor = cursor->next;
    }

    if(tasks == tlist->length) {
      *tasks_ = tasks;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetTaskInfo(const Task_t *task_, TaskInfo_t **info_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(info_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, sizeof(TaskInfo_t)))) {
        if(NOTNULLPTR(*info_)) {
          if(OK(__memcpy__((*info_)->name, task_->name, CONFIG_TASK_NAME_BYTES))) {
            (*info_)->id = task_->id;
            (*info_)->state = task_->state;
            (*info_)->lastRunTime = task_->lastRunTime;
            (*info_)->totalRunTime = task_->totalRunTime;
            RET_OK;
          } else {
            ASSERT;


            /* Free heap memory because __memcpy__() failed. */
            __HeapFreeMemory__(*info_);
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetAllTaskInfo(TaskInfo_t **info_, Base_t *tasks_) {
  RET_DEFINE;


  Base_t task = zero;
  Base_t tasks = zero;
  Task_t *cursor = null;


  if(NOTNULLPTR(info_) && (NOTNULLPTR(tasks_)) && NOTNULLPTR(tlist)) {
    cursor = tlist->head;

    while(NOTNULLPTR(cursor)) {
      tasks++;
      cursor = cursor->next;
    }

    if((zero < tasks) && (tasks == tlist->length)) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, tasks * sizeof(TaskInfo_t)))) {
        if(NOTNULLPTR(*info_)) {
          cursor = tlist->head;

          while(NOTNULLPTR(cursor)) {
            if(OK(__memcpy__((*info_)[task].name, cursor->name, CONFIG_TASK_NAME_BYTES))) {
              (*info_)[task].id = cursor->id;
              (*info_)[task].state = cursor->state;
              (*info_)[task].lastRunTime = cursor->lastRunTime;
              (*info_)[task].totalRunTime = cursor->totalRunTime;
              cursor = cursor->next;
              task++;
            }
          }

          *tasks_ = tasks;
          RET_OK;
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetTaskState(const Task_t *task_, TaskState_t *state_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(state_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      *state_ = task_->state;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetName(const Task_t *task_, Byte_t **name_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(name_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) name_, CONFIG_TASK_NAME_BYTES))) {
        if(NOTNULLPTR(*name_)) {
          if(OK(__memcpy__(*name_, task_->name, CONFIG_TASK_NAME_BYTES))) {
            RET_OK;
          } else {
            ASSERT;


            /* Free heap memory because __memcpy__() failed. */
            __HeapFreeMemory__(*name_);
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetId(const Task_t *task_, Base_t *id_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(id_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      *id_ = task_->id;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskNotifyStateClear(Task_t *task_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(zero < task_->notificationBytes) {
        if(OK(__memset__(task_->notificationValue, zero, CONFIG_NOTIFICATION_VALUE_BYTES))) {
          task_->notificationBytes = zero;
          RET_OK;
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskNotificationIsWaiting(const Task_t *task_, Base_t *res_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(res_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(zero < task_->notificationBytes) {
        *res_ = true;
        RET_OK;
      } else {
        *res_ = false;
        RET_OK;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskNotifyGive(Task_t *task_, const Base_t bytes_, const Byte_t *value_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && (zero < bytes_) && (CONFIG_NOTIFICATION_VALUE_BYTES >= bytes_) && (NOTNULLPTR(value_)) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(zero == task_->notificationBytes) {
        if(OK(__memcpy__(task_->notificationValue, value_, CONFIG_NOTIFICATION_VALUE_BYTES))) {
          task_->notificationBytes = bytes_;
          RET_OK;
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskNotifyTake(Task_t *task_, TaskNotification_t **notification_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(notification_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(zero < task_->notificationBytes) {
        if(OK(__HeapAllocateMemory__((volatile Addr_t **) notification_, sizeof(TaskNotification_t)))) {
          if(NOTNULLPTR(*notification_)) {
            if(OK(__memcpy__((*notification_)->notificationValue, task_->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES))) {
              if(OK(__memset__(task_->notificationValue, zero, CONFIG_NOTIFICATION_VALUE_BYTES))) {
                (*notification_)->notificationBytes = task_->notificationBytes;
                task_->notificationBytes = zero;
                RET_OK;
              } else {
                ASSERT;


                /* Free heap memory because __memset__() failed. */
                __HeapFreeMemory__(*notification_);
              }
            } else {
              ASSERT;


              /* Free heap memory because __memcpy__() failed. */
              __HeapFreeMemory__(*notification_);
            }
          } else {
            ASSERT;
          }
        } else {
          ASSERT;
        }
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskResume(Task_t *task_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->state = TaskStateRunning;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskSuspend(Task_t *task_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->state = TaskStateSuspended;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskWait(Task_t *task_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->state = TaskStateWaiting;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskChangePeriod(Task_t *task_, const Ticks_t period_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->timerPeriod = period_;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskChangeWDPeriod(Task_t *task_, const Ticks_t period_) {
  RET_DEFINE;
#if defined(CONFIG_TASK_WD_TIMER_ENABLE)

    if(NOTNULLPTR(task_) && NOTNULLPTR(tlist)) {
      if(OK(__TaskListFindTask__(task_))) {
        task_->wdTimerPeriod = period_;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }

#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */
  RET_RETURN;
}


Return_t xTaskGetPeriod(const Task_t *task_, Ticks_t *period_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(period_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      *period_ = task_->timerPeriod;
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static Return_t __TaskListFindTask__(const Task_t *task_) {
  RET_DEFINE;


  Task_t *cursor = null;


  if((NOTNULLPTR(task_)) && (NOTNULLPTR(tlist))) {
    if(OK(__MemoryRegionCheckKernel__(task_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      cursor = tlist->head;

      while((NOTNULLPTR(cursor)) && (cursor != task_)) {
        cursor = cursor->next;
      }

      if(NOTNULLPTR(cursor)) {
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskResetTimer(Task_t *task_) {
  RET_DEFINE;

  if(NOTNULLPTR(task_) && NOTNULLPTR(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->timerStartTime = __PortGetSysTicks__();
      RET_OK;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskStartScheduler(void) {
  RET_DEFINE;


  Task_t *runTask = null;
  Task_t *cursor = null;


  /* Intentionally underflow to get the maximum value of Ticks_t. */
  Ticks_t least = -1;


  if((false == FLAG_RUNNING) && (NOTNULLPTR(tlist))) {
    while(SchedulerStateRunning == schedulerState) {
      /* If the total runtime on a task has overflowed, reset the total runtime
       * for all tasks to their last runtime. */
      if(true == FLAG_OVERFLOW) {
        __RunTimeReset__();
      }

      cursor = tlist->head;

      while(NOTNULLPTR(cursor)) {
        /* If the task is in a waiting state *AND* has a waiting notification,
         * then run the task. */
        if((TaskStateWaiting == cursor->state) && (zero < cursor->notificationBytes)) {
          __TaskRun__(cursor);


          /* If the task is in a waiting state *AND* the task timer has elapsed,
           * then run the task. */
        } else if((TaskStateWaiting == cursor->state) && (zero < cursor->timerPeriod) && ((__PortGetSysTicks__() - cursor->timerStartTime) >
          cursor->timerPeriod)) {
          __TaskRun__(cursor);
          cursor->timerStartTime = __PortGetSysTicks__();


          /* If the task is in the running state *AND* its total runtime is less
           * than the least runtime of the tasks thus far, then update lest
           * runtime and remember the task because it will get ran later if it
           * is in fact the task with the least runtime. */
        } else if((TaskStateRunning == cursor->state) && (least > cursor->totalRunTime)) {
          least = cursor->totalRunTime;
          runTask = cursor;
        }

        cursor = cursor->next;
      }

      /* If a running task was found with the least runtime of all of the
       * running tasks then we run it here, otherwise we ignore this step. */
      if(NOTNULLPTR(runTask)) {
        __TaskRun__(runTask);
        runTask = null;
      }

      /* Intentionally underflow to get the maximum value of Ticks_t. */
      least = -1;
    }

    FLAG_RUNNING = false;
    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static void __RunTimeReset__(void) {
  Task_t *cursor = null;


  cursor = tlist->head;

  /* Go through all of the tasks and set their total runtime to their last
   * runtime. */
  while(NOTNULLPTR(cursor)) {
    cursor->totalRunTime = cursor->lastRunTime;
    cursor = cursor->next;
  }

  FLAG_OVERFLOW = false;

  return;
}


static void __TaskRun__(Task_t *task_) {
  Ticks_t start = zero;
  Ticks_t prev = zero;


  /* Store the previous total runtime to detect for overflow later. */
  prev = task_->totalRunTime;


  /* Capture the start time of the task in ticks. */
  start = __PortGetSysTicks__();


  /* Call the task main function through it's callback. */
  (*task_->callback)(task_, task_->taskParameter);


  /* Capture the task runtime by subtracting the start time from the end time.
   */
  task_->lastRunTime = __PortGetSysTicks__() - start;


  /* Add the last runtime to the total runtime. */
  task_->totalRunTime += task_->lastRunTime;

#if defined(CONFIG_TASK_WD_TIMER_ENABLE)

    /* If the task WD timer feature is enabled, if the WD timer period is
     * greater than zero *AND*
     *  the last runtime exceeded the WD timer period, then suspend the task. */
    if((zero < task_->wdTimerPeriod) && (task_->lastRunTime > task_->wdTimerPeriod)) {
      task_->state = TaskStateSuspended;
    }

#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */

  /* Detect overflow of total runtime. If an overflow occurs, then set the
   * overflow flag to true. */
  if(task_->totalRunTime < prev) {
    FLAG_OVERFLOW = true;
  }

  return;
}


Return_t xTaskResumeAll(void) {
  RET_DEFINE;

  if(NOTNULLPTR(tlist)) {
    schedulerState = SchedulerStateRunning;
    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskSuspendAll(void) {
  RET_DEFINE;

  if(NOTNULLPTR(tlist)) {
    schedulerState = SchedulerStateSuspended;
    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetSchedulerState(SchedulerState_t *state_) {
  RET_DEFINE;

  if(NOTNULLPTR(tlist)) {
    *state_ = schedulerState;
    RET_OK;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetWDPeriod(const Task_t *task_, Ticks_t *period_) {
  RET_DEFINE;
#if defined(CONFIG_TASK_WD_TIMER_ENABLE)

    if(NOTNULLPTR(task_) && NOTNULLPTR(period_) && NOTNULLPTR(tlist)) {
      if(OK(__TaskListFindTask__(task_))) {
        *period_ = task_->wdTimerPeriod;
        RET_OK;
      } else {
        ASSERT;
      }
    } else {
      ASSERT;
    }

#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */
  RET_RETURN;
}


#if defined(POSIX_ARCH_OTHER)


  /* For unit testing only! */
  void __TaskStateClear__(void) {
    tlist = null;

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */