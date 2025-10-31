/*UNCRUSTIFY-OFF*/


/**
 * @file task.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Task scheduler and management implementation
 * @details
 * Implements the cooperative task scheduler and task management functions including task creation, deletion, suspension, resumption, and runtime statistics.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */



/*UNCRUSTIFY-ON*/


#include "task.h"


#if defined(CONFIG_ENABLE_CONSOLE)


  #include "console.h"


#endif /* if defined(CONFIG_ENABLE_CONSOLE) */


#if defined(CONFIG_ENABLE_IDLE_HOOK)
  extern void vApplicationIdleHook(void);


#endif /* if defined(CONFIG_ENABLE_IDLE_HOOK) */


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


#else  /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */


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
/**
 * @brief Deletes a task
 * @details Removes the task from the scheduler and frees its allocated memory.
 *
 * @param[in] task_ Pointer to the task to delete
 *
 * @return          ReturnOK if task was deleted successfully
 * @return          ReturnError if task is invalid or deallocation failed
 *
 * @warning Using the task handle after deletion results in undefined behavior
 */
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
/**
 * @brief Gets task handle by name
 * @details Searches for a task by its name and returns the task handle.
 *
 * @param[out] task_ Pointer to store the found task handle
 * @param[in]  name_ Task name to search for (null-terminated string)
 *
 * @return           ReturnOK if task was found
 * @return           ReturnError if task not found or invalid parameters
 */
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
/**
 * @brief Gets task handle by ID
 * @details Searches for a task by its unique identifier and returns the task
 * handle.
 *
 * @param[out] task_ Pointer to store the found task handle
 * @param[in]  id_   Task identifier to search for
 *
 * @return           ReturnOK if task was found
 * @return           ReturnError if task not found or invalid parameters
 */
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
/**
 * @brief Gets runtime statistics for all tasks
 * @details Allocates and returns an array of runtime statistics for all
 * registered tasks.
 *
 * @param[out] stats_ Pointer to store allocated statistics array
 * @param[out] tasks_ Pointer to store the number of tasks
 *
 * @return            ReturnOK if statistics were retrieved successfully
 * @return            ReturnError if allocation failed or invalid parameters
 *
 * @warning Caller is responsible for freeing the allocated array
 */
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
/**
 * @brief Gets runtime statistics for a specific task
 * @details Allocates and returns runtime statistics for the specified task.
 *
 * @param[in]  task_  Pointer to the task to query
 * @param[out] stats_ Pointer to store allocated statistics structure
 *
 * @return            ReturnOK if statistics were retrieved successfully
 * @return            ReturnError if task is invalid or allocation failed
 *
 * @warning Caller is responsible for freeing the allocated structure
 */
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
/**
 * @brief Gets the total number of registered tasks
 * @details Returns the count of all tasks currently in the scheduler.
 *
 * @param[out] tasks_ Pointer to store the task count
 *
 * @return            ReturnOK if count was retrieved successfully
 * @return            ReturnError if invalid parameter
 */
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
/**
 * @brief Gets information about a specific task
 * @details Allocates and returns detailed information about the specified task.
 *
 * @param[in]  task_ Pointer to the task to query
 * @param[out] info_ Pointer to store allocated information structure
 *
 * @return           ReturnOK if information was retrieved successfully
 * @return           ReturnError if task is invalid or allocation failed
 *
 * @warning Caller is responsible for freeing the allocated structure
 */
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
/**
 * @brief Gets information about all tasks
 * @details Allocates and returns an array of information structures for all
 * registered tasks.
 *
 * @param[out] info_  Pointer to store allocated information array
 * @param[out] tasks_ Pointer to store the number of tasks
 *
 * @return            ReturnOK if information was retrieved successfully
 * @return            ReturnError if allocation failed or invalid parameters
 *
 * @warning Caller is responsible for freeing the allocated array
 */
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
/**
 * @brief Gets the current state of a task
 * @details Returns the execution state (running, suspended, or waiting) of the
 * specified task.
 *
 * @param[in]  task_  Pointer to the task to query
 * @param[out] state_ Pointer to store the task state
 *
 * @return            ReturnOK if state was retrieved successfully
 * @return            ReturnError if task is invalid or invalid parameter
 */
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
/**
 * @brief Gets the name of a task
 * @details Returns a pointer to the task's name string.
 *
 * @param[in]  task_ Pointer to the task to query
 * @param[out] name_ Pointer to store the name string pointer
 *
 * @return           ReturnOK if name was retrieved successfully
 * @return           ReturnError if task is invalid or invalid parameter
 *
 * @note The returned pointer points to the internal task structure; do not
 * modify or free
 */
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
/**
 * @brief Gets the unique identifier of a task
 * @details Returns the task's unique ID number.
 *
 * @param[in]  task_ Pointer to the task to query
 * @param[out] id_   Pointer to store the task ID
 *
 * @return           ReturnOK if ID was retrieved successfully
 * @return           ReturnError if task is invalid or invalid parameter
 */
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
/**
 * @brief Clears a task's notification state
 * @details Removes any pending notification from the specified task.
 *
 * @param[in,out] task_ Pointer to the task to clear
 *
 * @return              ReturnOK if notification was cleared successfully
 * @return              ReturnError if task is invalid
 */
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
/**
 * @brief Checks if a task has a pending notification
 * @details Returns true if the task has a notification waiting to be consumed.
 *
 * @param[in]  task_ Pointer to the task to check
 * @param[out] res_  Pointer to store the result (true if notification pending,
 *                   false otherwise)
 *
 * @return           ReturnOK if check was successful
 * @return           ReturnError if task is invalid or invalid parameter
 */
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
/**
 * @brief Sends a notification to a task
 * @details Delivers a notification with optional data to the specified task.
 *
 * @param[in,out] task_  Pointer to the task to notify
 * @param[in]     bytes_ Number of bytes in the notification value
 * @param[in]     value_ Pointer to notification data (can be NULL if bytes_ is
 *                       0)
 *
 * @return               ReturnOK if notification was sent successfully
 * @return               ReturnError if task is invalid, buffer full, or invalid
 *                       parameters
 */
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
/**
 * @brief Receives a notification from a task
 * @details Allocates and returns the pending notification for the specified
 * task.
 *
 * @param[in,out] task_         Pointer to the task to receive from
 * @param[out]    notification_ Pointer to store allocated notification
 *                              structure
 *
 * @return                      ReturnOK if notification was received
 *                              successfully
 * @return                      ReturnError if no notification pending, task is
 *                              invalid, or allocation failed
 *
 * @warning Caller is responsible for freeing the allocated notification
 * structure
 */
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
/**
 * @brief Resumes a suspended task
 * @details Changes the task state to running, allowing it to be scheduled for
 * execution.
 *
 * @param[in,out] task_ Pointer to the task to resume
 *
 * @return              ReturnOK if task was resumed successfully
 * @return              ReturnError if task is invalid
 */
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
/**
 * @brief Suspends a task
 * @details Changes the task state to suspended, preventing it from being
 * scheduled.
 *
 * @param[in,out] task_ Pointer to the task to suspend
 *
 * @return              ReturnOK if task was suspended successfully
 * @return              ReturnError if task is invalid
 */
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
/**
 * @brief Puts a task into waiting state
 * @details Changes the task state to waiting for a notification or event.
 *
 * @param[in,out] task_ Pointer to the task to wait
 *
 * @return              ReturnOK if task state was changed successfully
 * @return              ReturnError if task is invalid
 */
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
/**
 * @brief Changes the period of a task's timer
 * @details Updates the task's periodic timer period without affecting its
 * running state.
 *
 * @param[in,out] task_   Pointer to the task to modify
 * @param[in]     period_ New timer period in system ticks
 *
 * @return                ReturnOK if period was changed successfully
 * @return                ReturnError if task is invalid
 */
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
/**
 * @brief Changes the watchdog timer period for a task
 * @details Updates the task's watchdog timer period for task monitoring.
 *
 * @param[in,out] task_   Pointer to the task to modify
 * @param[in]     period_ New watchdog timer period in system ticks
 *
 * @return                ReturnOK if period was changed successfully
 * @return                ReturnError if task is invalid or watchdog not enabled
 */
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

#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */

  FUNCTION_EXIT;

}
/**
 * @brief Gets the current period of a task's timer
 * @details Retrieves the configured timer period for the specified task.
 *
 * @param[in]  task_   Pointer to the task to query
 * @param[out] period_ Pointer to store the timer period in system ticks
 *
 * @return             ReturnOK if period was retrieved successfully
 * @return             ReturnError if task is invalid or invalid parameter
 */
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
/**
 * @brief Finds a task in the task list
 * @details Internal helper that searches for a specific task in the global task
 * list.
 *
 * @param[in] task_ Pointer to task to find
 *
 * @return          ReturnOK if task was found
 * @return          ReturnError if task was not found or invalid
 */
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
/**
 * @brief Resets a task's timer
 * @details Resets the task timer start time to the current system ticks.
 *
 * @param[in,out] task_ Pointer to the task to reset
 *
 * @return              ReturnOK if timer was reset successfully
 * @return              ReturnError if task is invalid
 */
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
/**
 * @brief Starts the task scheduler
 * @details Initiates the cooperative scheduler to begin executing tasks.
 *
 * @return ReturnOK if scheduler was started successfully
 * @return ReturnError if scheduler is already running or initialization failed
 *
 * @note This function should be called after all initial tasks are created
 */
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

  #else  /* if (0x0u == CONFIG_CONSOLE_TASK_MODE) */

              xTaskWait(consoleTask);

              xTaskChangePeriod(consoleTask, CONFIG_CONSOLE_TIMER_PERIOD_MS);

              xTaskResetTimer(consoleTask);

  #endif /* if (0x0u == CONFIG_CONSOLE_TASK_MODE) */

          }

        }

      }

#endif /* if defined(CONFIG_ENABLE_CONSOLE) */

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

#endif /* if defined(CONFIG_ENABLE_IDLE_HOOK) */

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
/**
 * @brief Resumes all suspended tasks
 * @details Changes all tasks in suspended state to running state.
 *
 * @return ReturnOK if tasks were resumed successfully
 * @return ReturnError if operation failed
 */
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
/**
 * @brief Suspends all running tasks
 * @details Changes all tasks in running state to suspended state.
 *
 * @return ReturnOK if tasks were suspended successfully
 * @return ReturnError if operation failed
 */
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
/**
 * @brief Gets the current scheduler state
 * @details Returns whether the scheduler is running or suspended.
 *
 * @param[out] state_ Pointer to store the scheduler state
 *
 * @return            ReturnOK if state was retrieved successfully
 * @return            ReturnError if invalid parameter
 */
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
/**
 * @brief Gets the watchdog timer period for a task
 * @details Retrieves the configured watchdog timer period for the specified
 * task.
 *
 * @param[in]  task_   Pointer to the task to query
 * @param[out] period_ Pointer to store the watchdog period in system ticks
 *
 * @return             ReturnOK if period was retrieved successfully
 * @return             ReturnError if task is invalid, watchdog not enabled, or
 *                     invalid parameter
 */
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

#endif /* if defined(CONFIG_TASK_WD_TIMER_ENABLE) */

  FUNCTION_EXIT;

}


#if defined(POSIX_ARCH_OTHER)
  void __TaskStateClear__(void) {

    tlist = null;

    return;

  }


#endif /* if defined(POSIX_ARCH_OTHER) */