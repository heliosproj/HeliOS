#include "task.h"

#if defined(CONFIG_ENABLE_CONSOLE)

  #include "console.h"

#endif 

#if defined(CONFIG_ENABLE_IDLE_HOOK)

  extern void vApplicationIdleHook(void);

#endif 

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

          if((0x0u < task_->wdTimerPeriod) && (task_->lastRunTime > task_->wdTimerPeriod)) { \

            task_->state = TaskStateSuspended; \

          } \

          if(task_->totalRunTime < prev) { \

            __SetFlag__(OVERFLOW); \

          }

#else  

  #define __TaskRun__(task_) \

          prev = task_->totalRunTime; \

          start = __PortGetSysTicks__(); \

          (*task_->callback)(task_, task_->taskParameter); \

          task_->lastRunTime = __PortGetSysTicks__() - start; \

          task_->totalRunTime += task_->lastRunTime; \

          if(task_->totalRunTime < prev) { \

            __SetFlag__(OVERFLOW); \

          }

#endif 

Return_t xTaskCreate(Task_t **task_, const Byte_t *name_, void (*callback_)(Task_t *task_, TaskParm_t *parm_), TaskParm_t *taskParameter_) {

  FUNCTION_ENTER;

  Task_t *cursor = null;

  if(__PointerIsNotNull__(task_) && __PointerIsNotNull__(name_) && __PointerIsNotNull__(callback_) && __FlagIsNotSet__(RUNNING)) {

    if(__PointerIsNotNull__(tlist) || (__PointerIsNull__(tlist) && OK(__KernelAllocateMemory__((volatile Addr_t **) &tlist, sizeof(TaskList_t))))) {

      if(__PointerIsNotNull__(tlist) && !__ObjectIsValid__(tlist)) {

        tlist->valid = VALID;

        tlist->nextId = 0x0u;

        tlist->length = 0x0u;

        tlist->head = null;

      }

      if(OK(__KernelAllocateMemory__((volatile Addr_t **) task_, sizeof(Task_t)))) {

        if(__PointerIsNotNull__(*task_)) {

          if(OK(__memcpy__((*task_)->name, name_, CONFIG_TASK_NAME_BYTES))) {

            (*task_)->valid = VALID;

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(tlist) && __FlagIsNotSet__(RUNNING)) {

    if(OK(__TaskListFindTask__(task_))) {

      ((Task_t *) task_)->valid = INVALID;

      cursor = tlist->head;

      if(__PointerIsNotNull__(cursor) && (task_ == cursor)) {

        tlist->head = cursor->next;

        if(OK(__KernelFreeMemory__(cursor))) {

          tlist->length--;

          __ReturnOk__();

        } else {

          __AssertOnElse__();

        }

      } else if(__PointerIsNotNull__(cursor) && (task_ != cursor)) {

        while(__PointerIsNotNull__(cursor) && (task_ != cursor)) {

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

  if(__PointerIsNotNull__(task_) && (0x0u < id_) && __PointerIsNotNull__(tlist)) {

    cursor = tlist->head;

    while(__PointerIsNotNull__(cursor)) {

      if(cursor->id == id_) {

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

  Base_t task = 0x0u;

  Base_t tasks = 0x0u;

  Task_t *cursor = null;

  if(__PointerIsNotNull__(stats_) && __PointerIsNotNull__(tasks_) && __PointerIsNotNull__(tlist)) {

    cursor = tlist->head;

    while(__PointerIsNotNull__(cursor)) {

      tasks++;

      cursor = cursor->next;

    }

    if((0x0u < tasks) && (tlist->length == tasks)) {

      if(OK(__HeapAllocateMemory__((volatile Addr_t **) stats_, tasks * sizeof(TaskRunTimeStats_t)))) {

        if(__PointerIsNotNull__(*stats_)) {

          cursor = tlist->head;

          while(__PointerIsNotNull__(cursor)) {

            (*stats_)[task].valid = VALID;

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(stats_) && __PointerIsNotNull__(tlist)) {

    if(OK(__TaskListFindTask__(task_))) {

      if(OK(__HeapAllocateMemory__((volatile Addr_t **) stats_, sizeof(TaskRunTimeStats_t)))) {

        if(__PointerIsNotNull__(*stats_)) {

          (*stats_)->valid = VALID;

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

  Base_t tasks = 0x0u;

  Task_t *cursor = null;

  if(__PointerIsNotNull__(tasks_)) {

    if(__PointerIsNull__(tlist)) {

      *tasks_ = 0x0u;

      __ReturnOk__();

    } else {

      cursor = tlist->head;

      while(__PointerIsNotNull__(cursor)) {

        tasks++;

        cursor = cursor->next;

      }

      if(tlist->length == tasks) {

        *tasks_ = tasks;

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

    }

  } else {

    __AssertOnElse__();

  }

  FUNCTION_EXIT;

}

Return_t xTaskGetTaskInfo(const Task_t *task_, TaskInfo_t **info_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(info_) && __PointerIsNotNull__(tlist)) {

    if(OK(__TaskListFindTask__(task_))) {

      if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, sizeof(TaskInfo_t)))) {

        if(__PointerIsNotNull__(*info_)) {

          (*info_)->valid = VALID;

          if(OK(__memcpy__((*info_)->name, task_->name, CONFIG_TASK_NAME_BYTES))) {

            (*info_)->id = task_->id;

            (*info_)->state = task_->state;

            (*info_)->lastRunTime = task_->lastRunTime;

            (*info_)->totalRunTime = task_->totalRunTime;

            __ReturnOk__();

          } else {

            __AssertOnElse__();

            (*info_)->valid = INVALID;

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

  Base_t task = 0x0u;

  Base_t tasks = 0x0u;

  Task_t *cursor = null;

  if(__PointerIsNotNull__(info_) && __PointerIsNotNull__(tasks_) && __PointerIsNotNull__(tlist)) {

    cursor = tlist->head;

    while(__PointerIsNotNull__(cursor)) {

      tasks++;

      cursor = cursor->next;

    }

    if((0x0u < tasks) && (tlist->length == tasks)) {

      if(OK(__HeapAllocateMemory__((volatile Addr_t **) info_, tasks * sizeof(TaskInfo_t)))) {

        if(__PointerIsNotNull__(*info_)) {

          cursor = tlist->head;

          while(__PointerIsNotNull__(cursor)) {

            if(OK(__memcpy__((*info_)[task].name, cursor->name, CONFIG_TASK_NAME_BYTES))) {

              (*info_)[task].valid = VALID;

              (*info_)[task].id = cursor->id;

              (*info_)[task].state = cursor->state;

              (*info_)[task].lastRunTime = cursor->lastRunTime;

              (*info_)[task].totalRunTime = cursor->totalRunTime;

              cursor = cursor->next;

              task++;

            } else {

              __AssertOnElse__();

              (*info_)->valid = INVALID;

              __HeapFreeMemory__(*info_);

              FUNCTION_EXIT;

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(state_) && __PointerIsNotNull__(tlist)) {

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(name_) && __PointerIsNotNull__(tlist)) {

    if(OK(__TaskListFindTask__(task_))) {

      if(OK(__HeapAllocateMemory__((volatile Addr_t **) name_, CONFIG_TASK_NAME_BYTES))) {

        if(__PointerIsNotNull__(*name_)) {

          if(OK(__memcpy__(*name_, task_->name, CONFIG_TASK_NAME_BYTES))) {

            __ReturnOk__();

          } else {

            __AssertOnElse__();

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(id_) && __PointerIsNotNull__(tlist)) {

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(tlist)) {

    if(OK(__TaskListFindTask__(task_))) {

      if(0x0u < task_->notificationBytes) {

        if(OK(__memset__(task_->notificationValue, 0x0u, CONFIG_NOTIFICATION_VALUE_BYTES))) {

          task_->notificationBytes = 0x0u;

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(res_) && __PointerIsNotNull__(tlist)) {

    if(OK(__TaskListFindTask__(task_))) {

      if(0x0u < task_->notificationBytes) {

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

  if(__ObjectIsValid__(task_) && (0x0u < bytes_) && (CONFIG_NOTIFICATION_VALUE_BYTES >= bytes_) && __PointerIsNotNull__(value_) && __PointerIsNotNull__(tlist))

      {

    if(OK(__TaskListFindTask__(task_))) {

      if(0x0u == task_->notificationBytes) {

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(notification_) && __PointerIsNotNull__(tlist)) {

    if(OK(__TaskListFindTask__(task_))) {

      if(0x0u < task_->notificationBytes) {

        if(OK(__HeapAllocateMemory__((volatile Addr_t **) notification_, sizeof(TaskNotification_t)))) {

          if(__PointerIsNotNull__(*notification_)) {

            (*notification_)->valid = VALID;

            if(OK(__memcpy__((*notification_)->notificationValue, task_->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES))) {

              if(OK(__memset__(task_->notificationValue, 0x0u, CONFIG_NOTIFICATION_VALUE_BYTES))) {

                (*notification_)->notificationBytes = task_->notificationBytes;

                task_->notificationBytes = 0x0u;

                __ReturnOk__();

              } else {

                __AssertOnElse__();

                (*notification_)->valid = INVALID;

                __HeapFreeMemory__(*notification_);

              }

            } else {

              __AssertOnElse__();

              (*notification_)->valid = INVALID;

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(tlist)) {

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(tlist)) {

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(tlist)) {

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(tlist)) {

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

    if(__ObjectIsValid__(task_) && __PointerIsNotNull__(tlist)) {

      if(OK(__TaskListFindTask__(task_))) {

        task_->wdTimerPeriod = period_;

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

    } else {

      __AssertOnElse__();

    }

#endif 

  FUNCTION_EXIT;

}

Return_t xTaskGetPeriod(const Task_t *task_, Ticks_t *period_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(period_) && __PointerIsNotNull__(tlist)) {

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

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(tlist)) {

    cursor = tlist->head;

    while(__PointerIsNotNull__(cursor) && (task_ != cursor)) {

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

  FUNCTION_EXIT;

}

Return_t xTaskResetTimer(Task_t *task_) {

  FUNCTION_ENTER;

  if(__ObjectIsValid__(task_) && __PointerIsNotNull__(tlist)) {

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

  Ticks_t start = 0x0u;

  Ticks_t prev = 0x0u;

  Ticks_t least = -0x1;

  if(__FlagIsNotSet__(RUNNING) && __PointerIsNotNull__(tlist)) {

#if defined(CONFIG_ENABLE_CONSOLE)

      {

        Task_t *consoleTask = null;

        if(OK(xConsoleInit())) {

          if(OK(xTaskCreate(&consoleTask, "Console", vConsoleTask, null))) {

  #if (0x0u == CONFIG_CONSOLE_TASK_MODE)

              xTaskResume(consoleTask);

  #else 

              xTaskWait(consoleTask);

              xTaskChangePeriod(consoleTask, CONFIG_CONSOLE_TIMER_PERIOD_MS);

              xTaskResetTimer(consoleTask);

  #endif 

          }

        }

      }

#endif 

    __SetFlag__(RUNNING);

    while(SchedulerStateRunning == scheduler) {

      if(__FlagIsSet__(OVERFLOW)) {

        __RunTimeReset__();

      }

      cursor = tlist->head;

      while(__PointerIsNotNull__(cursor)) {

        if((TaskStateWaiting == cursor->state) && (0x0u < cursor->notificationBytes)) {

          __TaskRun__(cursor);

        } else if((TaskStateWaiting == cursor->state) && (0x0u < cursor->timerPeriod)) {

          Ticks_t elapsed = __PortGetSysTicks__() - cursor->timerStartTime;

          if(elapsed > cursor->timerPeriod) {

            __TaskRun__(cursor);

            cursor->timerStartTime = __PortGetSysTicks__();

          }

        } else if((TaskStateRunning == cursor->state) && (least > cursor->totalRunTime)) {

          least = cursor->totalRunTime;

          task = cursor;

        }

        cursor = cursor->next;

      }

      if(__PointerIsNotNull__(task)) {

        __TaskRun__(task);

        task = null;

      }

#if defined(CONFIG_ENABLE_IDLE_HOOK)

        else {

          vApplicationIdleHook();

        }

#endif 

      least = -0x1;

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

  if(__PointerIsNull__(tlist)) {

    return;

  }

  cursor = tlist->head;

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

  if(__PointerIsNotNull__(state_) && __PointerIsNotNull__(tlist)) {

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

    if(__ObjectIsValid__(task_) && __PointerIsNotNull__(period_) && __PointerIsNotNull__(tlist)) {

      if(OK(__TaskListFindTask__(task_))) {

        *period_ = task_->wdTimerPeriod;

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

    } else {

      __AssertOnElse__();

    }

#endif 

  FUNCTION_EXIT;

}

#if defined(POSIX_ARCH_OTHER)

  void __TaskStateClear__(void) {

    tlist = null;

    return;

  }

#endif 

