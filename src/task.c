/*UNCRUSTIFY-OFF*/
/**
 * @file task.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for task management
 * @version 0.4.0
 * @date 2022-01-31
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
#include "task.h"



static TaskList_t *taskList = null;
static void __RunTimeReset__(void);
static void __TaskRun__(Task_t *task_);
static Return_t __TaskListFindTask__(const Task_t *task_);



static SchedulerState_t schedulerState = SchedulerStateRunning;


Return_t xTaskCreate(Task_t **task_, const Byte_t *name_, void (*callback_)(Task_t *task_, TaskParm_t *parm_), TaskParm_t *taskParameter_) {
  RET_DEFINE;


  Task_t *cursor = null;


  if(ISNOTNULLPTR(task_) && (ISNOTNULLPTR(name_)) && (ISNOTNULLPTR(callback_)) && (false == SYSFLAG_RUNNING())) {
    if(ISNOTNULLPTR(taskList) || (ISNULLPTR(taskList) && ISOK(__KernelAllocateMemory__((volatile Addr_t **) &taskList, sizeof(TaskList_t))))) {
      if(ISOK(__KernelAllocateMemory__((volatile Addr_t **) task_, sizeof(Task_t)))) {
        if(ISNOTNULLPTR(*task_)) {
          if(ISOK(__memcpy__((*task_)->name, name_, CONFIG_TASK_NAME_BYTES))) {
            taskList->nextId++;
            (*task_)->id = taskList->nextId;
            (*task_)->state = TaskStateSuspended;
            (*task_)->callback = callback_;
            (*task_)->taskParameter = taskParameter_;
            (*task_)->next = null;
            cursor = taskList->head;

            if(ISNOTNULLPTR(taskList->head)) {
              while(ISNOTNULLPTR(cursor->next)) {
                cursor = cursor->next;
              }

              cursor->next = *task_;
            } else {
              taskList->head = *task_;
            }

            taskList->length++;
            RET_SUCCESS;
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


Return_t xTaskDelete(const Task_t *task_) {
  RET_DEFINE;


  Task_t *cursor = null;
  Task_t *taskPrevious = null;


  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(taskList) && (false == SYSFLAG_RUNNING())) {
    if(ISOK(__TaskListFindTask__(task_))) {
      cursor = taskList->head;

      if((ISNOTNULLPTR(cursor)) && (cursor == task_)) {
        taskList->head = cursor->next;

        if(ISOK(__KernelFreeMemory__(cursor))) {
          taskList->length--;
          RET_SUCCESS;
        } else {
          ASSERT;
        }
      } else if((ISNOTNULLPTR(cursor)) && (cursor != task_)) {
        while((ISNOTNULLPTR(cursor)) && (cursor != task_)) {
          taskPrevious = cursor;
          cursor = cursor->next;
        }

        if(ISNOTNULLPTR(cursor)) {
          taskPrevious->next = cursor->next;

          if(ISOK(__KernelFreeMemory__(cursor))) {
            taskList->length--;
            RET_SUCCESS;
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


  if((ISNOTNULLPTR(task_)) && (ISNOTNULLPTR(name_)) && (ISNOTNULLPTR(taskList))) {
    cursor = taskList->head;

    while(ISNOTNULLPTR(cursor)) {
      if(ISOK(__memcmp__(cursor->name, name_, CONFIG_TASK_NAME_BYTES, &res))) {
        if(true == res) {
          *task_ = cursor;
          RET_SUCCESS;
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


  if((ISNOTNULLPTR(task_)) && (zero < id_) && (ISNOTNULLPTR(taskList))) {
    cursor = taskList->head;

    while(ISNOTNULLPTR(cursor)) {
      if(id_ == cursor->id) {
        *task_ = cursor;
        RET_SUCCESS;
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


  if((ISNOTNULLPTR(stats_)) && (ISNOTNULLPTR(tasks_)) && (ISNOTNULLPTR(taskList))) {
    cursor = taskList->head;

    while(ISNOTNULLPTR(cursor)) {
      tasks++;
      cursor = cursor->next;
    }

    if((zero < tasks) && (tasks == taskList->length)) {
      if(ISOK(__HeapAllocateMemory__((volatile Addr_t **) stats_, tasks * sizeof(TaskRunTimeStats_t)))) {
        if(ISNOTNULLPTR(*stats_)) {
          cursor = taskList->head;

          while(ISNOTNULLPTR(cursor)) {
            (*stats_)[task].id = cursor->id;
            (*stats_)[task].lastRunTime = cursor->lastRunTime;
            (*stats_)[task].totalRunTime = cursor->totalRunTime;
            cursor = cursor->next;
            task++;
          }

          *tasks_ = tasks;
          RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(stats_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      if(ISOK(__HeapAllocateMemory__((volatile Addr_t **) stats_, sizeof(TaskRunTimeStats_t)))) {
        if(ISNOTNULLPTR(*stats_)) {
          (*stats_)->id = task_->id;
          (*stats_)->lastRunTime = task_->lastRunTime;
          (*stats_)->totalRunTime = task_->totalRunTime;
          RET_SUCCESS;
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


  if(ISNOTNULLPTR(tasks_) && ISNOTNULLPTR(taskList)) {
    cursor = taskList->head;

    while(ISNOTNULLPTR(cursor)) {
      tasks++;
      cursor = cursor->next;
    }

    if(tasks == taskList->length) {
      *tasks_ = tasks;
      RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(info_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      if(ISOK(__HeapAllocateMemory__((volatile Addr_t **) info_, sizeof(TaskInfo_t)))) {
        if(ISNOTNULLPTR(*info_)) {
          if(ISOK(__memcpy__((*info_)->name, task_->name, CONFIG_TASK_NAME_BYTES))) {
            (*info_)->id = task_->id;
            (*info_)->state = task_->state;
            (*info_)->lastRunTime = task_->lastRunTime;
            (*info_)->totalRunTime = task_->totalRunTime;
            RET_SUCCESS;
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


Return_t xTaskGetAllTaskInfo(TaskInfo_t **info_, Base_t *tasks_) {
  RET_DEFINE;


  Base_t task = zero;
  Base_t tasks = zero;
  Task_t *cursor = null;


  if(ISNOTNULLPTR(info_) && (ISNOTNULLPTR(tasks_)) && ISNOTNULLPTR(taskList)) {
    cursor = taskList->head;

    while(ISNOTNULLPTR(cursor)) {
      tasks++;
      cursor = cursor->next;
    }

    if((zero < tasks) && (tasks == taskList->length)) {
      if(ISOK(__HeapAllocateMemory__((volatile Addr_t **) info_, tasks * sizeof(TaskInfo_t)))) {
        if(ISNOTNULLPTR(*info_)) {
          cursor = taskList->head;

          while(ISNOTNULLPTR(cursor)) {
            if(ISOK(__memcpy__((*info_)[task].name, cursor->name, CONFIG_TASK_NAME_BYTES))) {
              (*info_)[task].id = cursor->id;
              (*info_)[task].state = cursor->state;
              (*info_)[task].lastRunTime = cursor->lastRunTime;
              (*info_)[task].totalRunTime = cursor->totalRunTime;
              cursor = cursor->next;
              task++;
            }
          }

          *tasks_ = tasks;
          RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(state_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      *state_ = task_->state;
      RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(name_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      if(ISOK(__HeapAllocateMemory__((volatile Addr_t **) name_, CONFIG_TASK_NAME_BYTES))) {
        if(ISNOTNULLPTR(*name_)) {
          if(ISOK(__memcpy__(*name_, task_->name, CONFIG_TASK_NAME_BYTES))) {
            RET_SUCCESS;
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


Return_t xTaskGetId(const Task_t *task_, Base_t *id_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(id_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      *id_ = task_->id;
      RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      if(zero < task_->notificationBytes) {
        if(ISOK(__memset__(task_->notificationValue, zero, CONFIG_NOTIFICATION_VALUE_BYTES))) {
          task_->notificationBytes = zero;
          RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(res_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      if(zero < task_->notificationBytes) {
        *res_ = true;
        RET_SUCCESS;
      } else {
        *res_ = false;
        RET_SUCCESS;
      }
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskNotifyGive(Task_t *task_, const Base_t notificationBytes_, const Byte_t *notificationValue_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(task_) && (zero < notificationBytes_) && (CONFIG_NOTIFICATION_VALUE_BYTES >= notificationBytes_) && (ISNOTNULLPTR(notificationValue_)) &&
    ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      if(zero == task_->notificationBytes) {
        if(ISOK(__memcpy__(task_->notificationValue, notificationValue_, CONFIG_NOTIFICATION_VALUE_BYTES))) {
          task_->notificationBytes = notificationBytes_;
          RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(notification_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      if(zero < task_->notificationBytes) {
        if(ISOK(__HeapAllocateMemory__((volatile Addr_t **) notification_, sizeof(TaskNotification_t)))) {
          if(ISNOTNULLPTR(*notification_)) {
            if(ISOK(__memcpy__((*notification_)->notificationValue, task_->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES))) {
              if(ISOK(__memset__(task_->notificationValue, zero, CONFIG_NOTIFICATION_VALUE_BYTES))) {
                (*notification_)->notificationBytes = task_->notificationBytes;
                task_->notificationBytes = zero;
                RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      task_->state = TaskStateRunning;
      RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      task_->state = TaskStateSuspended;
      RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      task_->state = TaskStateWaiting;
      RET_SUCCESS;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskChangePeriod(Task_t *task_, const Ticks_t timerPeriod_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      task_->timerPeriod = timerPeriod_;
      RET_SUCCESS;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskChangeWDPeriod(Task_t *task_, const Ticks_t wdTimerPeriod_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      task_->wdTimerPeriod = wdTimerPeriod_;
      RET_SUCCESS;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetPeriod(const Task_t *task_, Ticks_t *timerPeriod_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(timerPeriod_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      *timerPeriod_ = task_->timerPeriod;
      RET_SUCCESS;
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


  if((ISNOTNULLPTR(task_)) && (ISNOTNULLPTR(taskList))) {
    if(ISOK(__MemoryRegionCheckKernel__(task_, MEMORY_REGION_CHECK_OPTION_W_ADDR))) {
      cursor = taskList->head;

      while((ISNOTNULLPTR(cursor)) && (cursor != task_)) {
        cursor = cursor->next;
      }

      if(ISNOTNULLPTR(cursor)) {
        RET_SUCCESS;
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

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      task_->timerStartTime = __PortGetSysTicks__();
      RET_SUCCESS;
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
  Ticks_t leastRunTime = -1;


  if((false == SYSFLAG_RUNNING()) && (ISNOTNULLPTR(taskList))) {
    while(SchedulerStateRunning == schedulerState) {
      if(SYSFLAG_OVERFLOW()) {
        __RunTimeReset__();
      }

      cursor = taskList->head;

      while(ISNOTNULLPTR(cursor)) {
        if((TaskStateWaiting == cursor->state) && (zero < cursor->notificationBytes)) {
          __TaskRun__(cursor);
        } else if((TaskStateWaiting == cursor->state) && (zero < cursor->timerPeriod) && ((__PortGetSysTicks__() - cursor->timerStartTime) >
          cursor->timerPeriod)) {
          __TaskRun__(cursor);
          cursor->timerStartTime = __PortGetSysTicks__();
        } else if((TaskStateRunning == cursor->state) && (leastRunTime > cursor->totalRunTime)) {
          leastRunTime = cursor->totalRunTime;
          runTask = cursor;
        }

        cursor = cursor->next;
      }

      if(ISNOTNULLPTR(runTask)) {
        __TaskRun__(runTask);
        runTask = null;
      }

      leastRunTime = -1;
    }

    SYSFLAG_RUNNING() = false;
    RET_SUCCESS;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


static void __RunTimeReset__(void) {
  Task_t *cursor = null;


  cursor = taskList->head;

  while(ISNOTNULLPTR(cursor)) {
    cursor->totalRunTime = cursor->lastRunTime;
    cursor = cursor->next;
  }

  SYSFLAG_OVERFLOW() = false;

  return;
}


static void __TaskRun__(Task_t *task_) {
  Ticks_t taskStartTime = zero;
  Ticks_t prevTotalRunTime = zero;


  prevTotalRunTime = task_->totalRunTime;
  taskStartTime = __PortGetSysTicks__();
  (*task_->callback)(task_, task_->taskParameter);
  task_->lastRunTime = __PortGetSysTicks__() - taskStartTime;
  task_->totalRunTime += task_->lastRunTime;

#if defined(CONFIG_TASK_WD_TIMER_ENABLE)

    if((zero != task_->wdTimerPeriod) && (task_->lastRunTime > task_->wdTimerPeriod)) {
      task_->state = TaskStateSuspended;
    }

#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */

  if(task_->totalRunTime < prevTotalRunTime) {
    SYSFLAG_OVERFLOW() = true;
  }

  return;
}


Return_t xTaskResumeAll(void) {
  RET_DEFINE;

  if(ISNOTNULLPTR(taskList)) {
    schedulerState = SchedulerStateRunning;
    RET_SUCCESS;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskSuspendAll(void) {
  RET_DEFINE;

  if(ISNOTNULLPTR(taskList)) {
    schedulerState = SchedulerStateSuspended;
    RET_SUCCESS;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetSchedulerState(SchedulerState_t *state_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(taskList)) {
    *state_ = schedulerState;
    RET_SUCCESS;
  } else {
    ASSERT;
  }

  RET_RETURN;
}


Return_t xTaskGetWDPeriod(const Task_t *task_, Ticks_t *wdTimerPeriod_) {
  RET_DEFINE;

  if(ISNOTNULLPTR(task_) && ISNOTNULLPTR(wdTimerPeriod_) && ISNOTNULLPTR(taskList)) {
    if(ISOK(__TaskListFindTask__(task_))) {
      *wdTimerPeriod_ = task_->wdTimerPeriod;
      RET_SUCCESS;
    } else {
      ASSERT;
    }
  } else {
    ASSERT;
  }

  RET_RETURN;
}


#if defined(POSIX_ARCH_OTHER)


  void __TaskStateClear__(void) {
    taskList = null;

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */