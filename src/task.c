/*UNCRUSTIFY-OFF*/
/**
 * @file task.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for tasks and task management
 * @version 0.5.0
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
static Return_t __TaskListFindTask__(const Task_t *task_);



static SchedulerState_t scheduler = SchedulerStateRunning;


#if defined(CONFIG_TASK_WD_TIMER_ENABLE)
  #define __TaskRun__(task_) \
          prev = task_->totalRunTime; \
          start = __PortGetSysTicks__(); \
          (*task_->callback)(task_, task_->taskParameter); \
          task_->lastRunTime = __PortGetSysTicks__() - start; \
          task_->totalRunTime += task_->lastRunTime; \
          if((nil < task_->wdTimerPeriod) && (task_->lastRunTime > task_->wdTimerPeriod)) { \
            task_->state = TaskStateSuspended; \
          } \
          if(task_->totalRunTime < prev) { \
            __SetFlag__(OVERFLOW); \
          }
#elif  /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */
  #define __TaskRun__(task_) \
          prev = task_->totalRunTime; \
          start = __PortGetSysTicks__(); \
          (*task_->callback)(task_, task_->taskParameter); \
          task_->lastRunTime = __PortGetSysTicks__() - start; \
          task_->totalRunTime += task_->lastRunTime; \
          if(task_->totalRunTime < prev) { \
            __SetFlag__(OVERFLOW); \
          }
#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */


Return_t xTaskCreate(Task_t **task_, const Byte_t *name_, void (*callback_)(Task_t *task_, TaskParm_t *parm_), TaskParm_t *taskParameter_) {
  FUNCTION_ENTER;


  Task_t *cursor = null;


  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(name_) && __PointerIsNotNull__(callback_) && __FlagIsNotSet__(RUNNING)) {
    /* NOTE: There is a __KernelAllocateMemory__() syscall buried in this if()
     * statement. */
    if(__PointerIsNotNull__(tlist) || (__PointerIsNull__(tlist) && OK(__KernelAllocateMemory__((volatile Addr_t **) &tlist, sizeof(TaskList_t))))) {
      if(OK(__KernelAllocateMemory__((volatile Addr_t **) task_, sizeof(Task_t)))) {
        if(__PointerIsNotNull__(*task_)) {
          if(OK(__memcpy__((*task_)->name, name_, CONFIG_TASK_NAME_BYTES))) {
            tlist->nextId++;
            (*task_)->id = tlist->nextId;
            (*task_)->state = TaskStateSuspended;
            (*task_)->callback = callback_;
            (*task_)->taskParameter = taskParameter_;
            (*task_)->next = null;
            cursor = tlist->head;

            if(__PointerIsNotNull__(tlist->head)) {
              while(__PointerIsNotNull__(cursor->next)) {
                cursor = cursor->next;
              }

              cursor->next = *task_;
            } else {
              tlist->head = *task_;
            }

            tlist->length++;
            __ReturnOk__();
          } else {
            __AssertOnElse__();


            /* Free kernel memory because __memcpy__() failed.*/
            __KernelFreeMemory__(*task_);
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskDelete(const Task_t *task_) {
  FUNCTION_ENTER;


  Task_t *cursor = null;
  Task_t *previous = null;


  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(tlist) && __FlagIsNotSet__(RUNNING)) {
    if(OK(__TaskListFindTask__(task_))) {
      cursor = tlist->head;

      if(__PointerIsNotNull__(cursor) && (cursor == task_)) {
        tlist->head = cursor->next;

        if(OK(__KernelFreeMemory__(cursor))) {
          tlist->length--;
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else if(__PointerIsNotNull__(cursor) && (cursor != task_)) {
        while(__PointerIsNotNull__(cursor) && (cursor != task_)) {
          previous = cursor;
          cursor = cursor->next;
        }

        if(__PointerIsNotNull__(cursor)) {
          previous->next = cursor->next;

          if(OK(__KernelFreeMemory__(cursor))) {
            tlist->length--;
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetHandleByName(Task_t **task_, const Byte_t *name_) {
  FUNCTION_ENTER;


  Task_t *cursor = null;
  Base_t res = false;


  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(name_) && __PointerIsNotNull__(tlist)) {
    cursor = tlist->head;

    while(__PointerIsNotNull__(cursor)) {
      if(OK(__memcmp__(cursor->name, name_, CONFIG_TASK_NAME_BYTES, &res))) {
        if(true == res) {
          *task_ = cursor;
          __ReturnOk__();
          break;
        }
      } else {
        __AssertOnElse__();
        break;
      }

      cursor = cursor->next;
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetHandleById(Task_t **task_, const Base_t id_) {
  FUNCTION_ENTER;


  Task_t *cursor = null;


  if(__PointerIsNotNull__(task_) && (nil < id_) && __PointerIsNotNull__(tlist)) {
    cursor = tlist->head;

    while(__PointerIsNotNull__(cursor)) {
      if(id_ == cursor->id) {
        *task_ = cursor;
        __ReturnOk__();
        break;
      }

      cursor = cursor->next;
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetAllRunTimeStats(TaskRunTimeStats_t **stats_, Base_t *tasks_) {
  FUNCTION_ENTER;


  Base_t task = nil;
  Base_t tasks = nil;
  Task_t *cursor = null;


  if(__PointerIsNotNull__(stats_) && __PointerIsNotNull__(tasks_) && __PointerIsNotNull__(tlist)) {
    cursor = tlist->head;

    while(__PointerIsNotNull__(cursor)) {
      tasks++;
      cursor = cursor->next;
    }

    if((nil < tasks) && (tasks == tlist->length)) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) stats_, tasks * sizeof(TaskRunTimeStats_t)))) {
        if(__PointerIsNotNull__(*stats_)) {
          cursor = tlist->head;

          while(__PointerIsNotNull__(cursor)) {
            (*stats_)[task].id = cursor->id;
            (*stats_)[task].lastRunTime = cursor->lastRunTime;
            (*stats_)[task].totalRunTime = cursor->totalRunTime;
            cursor = cursor->next;
            task++;
          }

          *tasks_ = tasks;
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetTaskRunTimeStats(const Task_t *task_, TaskRunTimeStats_t **stats_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(stats_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) stats_, sizeof(TaskRunTimeStats_t)))) {
        if(__PointerIsNotNull__(*stats_)) {
          (*stats_)->id = task_->id;
          (*stats_)->lastRunTime = task_->lastRunTime;
          (*stats_)->totalRunTime = task_->totalRunTime;
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetNumberOfTasks(Base_t *tasks_) {
  FUNCTION_ENTER;


  Base_t tasks = nil;
  Task_t *cursor = null;


  if(__PointerIsNotNull__(tasks_) && __PointerIsNotNull__(tlist)) {
    cursor = tlist->head;

    while(__PointerIsNotNull__(cursor)) {
      tasks++;
      cursor = cursor->next;
    }

    if(tasks == tlist->length) {
      *tasks_ = tasks;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetTaskInfo(const Task_t *task_, TaskInfo_t **info_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(info_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, sizeof(TaskInfo_t)))) {
        if(__PointerIsNotNull__(*info_)) {
          if(OK(__memcpy__((*info_)->name, task_->name, CONFIG_TASK_NAME_BYTES))) {
            (*info_)->id = task_->id;
            (*info_)->state = task_->state;
            (*info_)->lastRunTime = task_->lastRunTime;
            (*info_)->totalRunTime = task_->totalRunTime;
            __ReturnOk__();
          } else {
            __AssertOnElse__();


            /* Free heap memory because __memcpy__() failed. */
            __HeapFreeMemory__(*info_);
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetAllTaskInfo(TaskInfo_t **info_, Base_t *tasks_) {
  FUNCTION_ENTER;


  Base_t task = nil;
  Base_t tasks = nil;
  Task_t *cursor = null;


  if(__PointerIsNotNull__(info_) && __PointerIsNotNull__(tasks_) && __PointerIsNotNull__(tlist)) {
    cursor = tlist->head;

    while(__PointerIsNotNull__(cursor)) {
      tasks++;
      cursor = cursor->next;
    }

    if((nil < tasks) && (tasks == tlist->length)) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, tasks * sizeof(TaskInfo_t)))) {
        if(__PointerIsNotNull__(*info_)) {
          cursor = tlist->head;

          while(__PointerIsNotNull__(cursor)) {
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
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetTaskState(const Task_t *task_, TaskState_t *state_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(state_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      *state_ = task_->state;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetName(const Task_t *task_, Byte_t **name_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(name_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(OK(__HeapAllocateMemory__((volatile Addr_t **) name_, CONFIG_TASK_NAME_BYTES))) {
        if(__PointerIsNotNull__(*name_)) {
          if(OK(__memcpy__(*name_, task_->name, CONFIG_TASK_NAME_BYTES))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();


            /* Free heap memory because __memcpy__() failed. */
            __HeapFreeMemory__(*name_);
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetId(const Task_t *task_, Base_t *id_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(id_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      *id_ = task_->id;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskNotifyStateClear(Task_t *task_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(nil < task_->notificationBytes) {
        if(OK(__memset__(task_->notificationValue, nil, CONFIG_NOTIFICATION_VALUE_BYTES))) {
          task_->notificationBytes = nil;
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskNotificationIsWaiting(const Task_t *task_, Base_t *res_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(res_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(nil < task_->notificationBytes) {
        *res_ = true;
        __ReturnOk__();
      } else {
        *res_ = false;
        __ReturnOk__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskNotifyGive(Task_t *task_, const Base_t bytes_, const Byte_t *value_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && (nil < bytes_) && (CONFIG_NOTIFICATION_VALUE_BYTES >= bytes_) && __PointerIsNotNull__(value_) && __PointerIsNotNull__(
      tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(nil == task_->notificationBytes) {
        if(OK(__memcpy__(task_->notificationValue, value_, CONFIG_NOTIFICATION_VALUE_BYTES))) {
          task_->notificationBytes = bytes_;
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskNotifyTake(Task_t *task_, TaskNotification_t **notification_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(notification_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      if(nil < task_->notificationBytes) {
        if(OK(__HeapAllocateMemory__((volatile Addr_t **) notification_, sizeof(TaskNotification_t)))) {
          if(__PointerIsNotNull__(*notification_)) {
            if(OK(__memcpy__((*notification_)->notificationValue, task_->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES))) {
              if(OK(__memset__(task_->notificationValue, nil, CONFIG_NOTIFICATION_VALUE_BYTES))) {
                (*notification_)->notificationBytes = task_->notificationBytes;
                task_->notificationBytes = nil;
                __ReturnOk__();
              } else {
                __AssertOnElse__();


                /* Free heap memory because __memset__() failed. */
                __HeapFreeMemory__(*notification_);
              }
            } else {
              __AssertOnElse__();


              /* Free heap memory because __memcpy__() failed. */
              __HeapFreeMemory__(*notification_);
            }
          } else {
            __AssertOnElse__();
          }
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskResume(Task_t *task_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->state = TaskStateRunning;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskSuspend(Task_t *task_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->state = TaskStateSuspended;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskWait(Task_t *task_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->state = TaskStateWaiting;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskChangePeriod(Task_t *task_, const Ticks_t period_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->timerPeriod = period_;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskChangeWDPeriod(Task_t *task_, const Ticks_t period_) {
  FUNCTION_ENTER;
#if defined(CONFIG_TASK_WD_TIMER_ENABLE)

    if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(tlist)) {
      if(OK(__TaskListFindTask__(task_))) {
        task_->wdTimerPeriod = period_;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }

#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */
  FUNCTION_EXIT;
}


Return_t xTaskGetPeriod(const Task_t *task_, Ticks_t *period_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(period_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      *period_ = task_->timerPeriod;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static Return_t __TaskListFindTask__(const Task_t *task_) {
  FUNCTION_ENTER;


  Task_t *cursor = null;


  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(tlist)) {
    if(OK(__MemoryRegionCheckKernel__(task_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      cursor = tlist->head;

      while(__PointerIsNotNull__(cursor) && (cursor != task_)) {
        cursor = cursor->next;
      }

      if(__PointerIsNotNull__(cursor)) {
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskResetTimer(Task_t *task_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(tlist)) {
    if(OK(__TaskListFindTask__(task_))) {
      task_->timerStartTime = __PortGetSysTicks__();
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskStartScheduler(void) {
  FUNCTION_ENTER;


  Task_t *task = null;
  Task_t *cursor = null;
  Ticks_t start = nil;
  Ticks_t prev = nil;


  /* Intentionally underflow to get the maximum value of Ticks_t. */
  Ticks_t least = -1;


  if(__FlagIsNotSet__(RUNNING) && __PointerIsNotNull__(tlist)) {
    while(SchedulerStateRunning == scheduler) {
      /* If the total runtime on a task has overflowed, reset the total runtime
       * for all tasks to their last runtime. */
      if(__FlagIsSet__(OVERFLOW)) {
        __RunTimeReset__();
      }

      cursor = tlist->head;

      while(__PointerIsNotNull__(cursor)) {
        /* If the task is in a waiting state *AND* has a waiting notification,
         * then run the task. */
        if((TaskStateWaiting == cursor->state) && (nil < cursor->notificationBytes)) {
          __TaskRun__(cursor);


          /* If the task is in a waiting state *AND* the task timer has elapsed,
           * then run the task. */
        } else if((TaskStateWaiting == cursor->state) && (nil < cursor->timerPeriod) && ((__PortGetSysTicks__() - cursor->timerStartTime) >
          cursor->timerPeriod)) {
          __TaskRun__(cursor);
          cursor->timerStartTime = __PortGetSysTicks__();


          /* If the task is in the running state *AND* its total runtime is less
           * than the least runtime of the tasks thus far, then update lest
           * runtime and remember the task because it will get ran later if it
           * is in fact the task with the least runtime. */
        } else if((TaskStateRunning == cursor->state) && (least > cursor->totalRunTime)) {
          least = cursor->totalRunTime;
          task = cursor;
        }

        cursor = cursor->next;
      }

      /* If a running task was found with the least runtime of all of the
       * running tasks then we run it here, otherwise we ignore this step. */
      if(__PointerIsNotNull__(task)) {
        __TaskRun__(task);
        task = null;
      }

      /* Intentionally underflow to get the maximum value of Ticks_t. */
      least = -1;
    }

    __UnsetFlag__(RUNNING);
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


static void __RunTimeReset__(void) {
  Task_t *cursor = null;


  cursor = tlist->head;

  /* Go through all of the tasks and set their total runtime to their last
   * runtime. */
  while(__PointerIsNotNull__(cursor)) {
    cursor->totalRunTime = cursor->lastRunTime;
    cursor = cursor->next;
  }

  __UnsetFlag__(OVERFLOW);

  return;
}


Return_t xTaskResumeAll(void) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(tlist)) {
    scheduler = SchedulerStateRunning;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskSuspendAll(void) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(tlist)) {
    scheduler = SchedulerStateSuspended;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetSchedulerState(SchedulerState_t *state_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(tlist)) {
    *state_ = scheduler;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xTaskGetWDPeriod(const Task_t *task_, Ticks_t *period_) {
  FUNCTION_ENTER;
#if defined(CONFIG_TASK_WD_TIMER_ENABLE)

    if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(period_) && __PointerIsNotNull__(tlist)) {
      if(OK(__TaskListFindTask__(task_))) {
        *period_ = task_->wdTimerPeriod;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }

#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */
  FUNCTION_EXIT;
}


#if defined(POSIX_ARCH_OTHER)


  /* For unit testing only! */
  void __TaskStateClear__(void) {
    tlist = null;

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */