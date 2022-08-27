/**
 * @file task.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief Kernel sources for task management
 * @version 0.3.5
 * @date 2022-01-31
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

#include "task.h"




/* Declare and initialize the task list to null. */
static TaskList_t *taskList = NULL;


/* Declare and initialize the scheduler state to
running. This is controlled with xTaskResumeAll()
and xTaskSuspendAll(). */
static SchedulerState_t schedulerState = SchedulerStateRunning;



/* The xTaskCreate() system call will create a new task. The task will be created with its
state set to suspended. The xTaskCreate() and xTaskDelete() system calls cannot be called within
a task. They MUST be called outside of the scope of the HeliOS scheduler. */
Task_t *xTaskCreate(const char *name_, void (*callback_)(Task_t *, TaskParm_t *), TaskParm_t *taskParameter_) {


  Task_t *ret = NULL;

  Task_t *taskCursor = NULL;

  SYSASSERT(false == SYSFLAG_RUNNING());

  SYSASSERT(ISNOTNULLPTR(name_));

  SYSASSERT(ISNOTNULLPTR(callback_));




  /* Make sure we aren't inside the scope of the scheduler and that the end-user didn't
  pass any null pointers.

  NOTE: It is okay for the task paramater to be null. */
  if ((false == SYSFLAG_RUNNING()) && (ISNOTNULLPTR(name_)) && (ISNOTNULLPTR(callback_))) {


    /* See if the task list needs to be created. */
    if (ISNULLPTR(taskList)) {




      taskList = (TaskList_t *)_KernelAllocateMemory_(sizeof(TaskList_t));
    }


    /* Assert if xMemAlloc() didn't do its job. */
    SYSASSERT(ISNOTNULLPTR(taskList));


    /* Check if xMemAlloc() did its job. */
    if (ISNOTNULLPTR(taskList)) {



      ret = (Task_t *)_KernelAllocateMemory_(sizeof(Task_t));


      /* Again, assert if xMemAlloc() didn't do its job. */
      SYSASSERT(ISNOTNULLPTR(ret));


      /* Check if xMemAlloc() did its job. If so, populate the task with all the
      pertinent details. */
      if (ISNOTNULLPTR(ret)) {

        taskList->nextId++;

        ret->id = taskList->nextId;

        _memcpy_(ret->name, name_, CONFIG_TASK_NAME_BYTES);

        ret->state = TaskStateSuspended;

        ret->callback = callback_;

        ret->taskParameter = taskParameter_;

        ret->next = NULL;

        taskCursor = taskList->head;

        /* Check if this is the first task in the task list. If it is just set
        the head to it. Otherwise we are going to have to traverse the list
        to find the end. */
        if (ISNOTNULLPTR(taskList->head)) {


          /* If the task cursor is not null, continue to traverse the list to find the end. */
          while (ISNOTNULLPTR(taskCursor->next)) {


            taskCursor = taskCursor->next;
          }

          taskCursor->next = ret;

        } else {

          taskList->head = ret;
        }

        taskList->length++;
      }
    }
  }

  return ret;
}



/* The xTaskDelete() system call will delete a task. The xTaskCreate() and xTaskDelete() system calls
cannot be called within a task. They MUST be called outside of the scope of the HeliOS scheduler. */
void xTaskDelete(Task_t *task_) {


  Task_t *taskCursor = NULL;

  Task_t *taskPrevious = NULL;


  /* Assert if we are within the scope of the scheduler. */
  SYSASSERT(false == SYSFLAG_RUNNING());


  /* Check to make sure we aren't in the scope of the scheduler. */
  if (false == SYSFLAG_RUNNING()) {


    /* Assert if we can't find the task in the task list. */
    SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


    /* Check if the task is in the task list, if not then head toward
    the exit. */
    if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {


      taskCursor = taskList->head;


      /* If the task is at the head of the task list then just remove
      it and free its heap memory. */
      if ((ISNOTNULLPTR(taskCursor)) && (taskCursor == task_)) {


        taskList->head = taskCursor->next;



        _KernelFreeMemory_(taskCursor);

        taskList->length--;

      } else {

        /* Well, it wasn't at the head of the task list so now we have to
        go hunt it down. */
        while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {

          taskPrevious = taskCursor;

          taskCursor = taskCursor->next;
        }


        /* Assert if we didn't find it which should be impossible given
        _TaskListFindTask_() found it. Maybe some cosmic rays bit flipped
        the DRAM!? */
        SYSASSERT(ISNOTNULLPTR(taskCursor));


        /* Check if the task cursor points to something. That something should
        be the task we want to free. */
        if (ISNOTNULLPTR(taskCursor)) {

          taskPrevious->next = taskCursor->next;




          _KernelFreeMemory_(taskCursor);


          taskList->length--;
        }
      }
    }
  }

  return;
}




/* The xTaskGetHandleByName() system call will return the task handle pointer to the
task specified by its ASCII name. The length of the task name is dependent on the
CONFIG_TASK_NAME_BYTES setting. The name is compared byte-for-byte so the name is
case sensitive. */
Task_t *xTaskGetHandleByName(const char *name_) {


  Task_t *ret = NULL;

  Task_t *taskCursor = NULL;


  /* Assert if the task list has not been initialized. We wouldn't have to do this
  if we called _TaskListFindTask_() but that isn't needed here. */
  SYSASSERT(ISNOTNULLPTR(taskList));

  /* Assert if the end-user passed a null pointer for the task name. */
  SYSASSERT(ISNOTNULLPTR(name_));

  /* Check if the task list is not null and the name parameter is also not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(name_))) {

    taskCursor = taskList->head;

    /* While the task cursor is not null, scan the task list for the task name. */
    while (ISNOTNULLPTR(taskCursor)) {

      /* Compare the task name of the task pointed to by the task cursor against the
      name parameter. */
      if (zero == _memcmp_(taskCursor->name, name_, CONFIG_TASK_NAME_BYTES)) {


        ret = taskCursor;

        break;
      }

      taskCursor = taskCursor->next;
    }
  }

  return ret;
}




/* The xTaskGetHandleById() system call will return a pointer to the task handle
specified by its identifier. */
Task_t *xTaskGetHandleById(Base_t id_) {


  Task_t *ret = NULL;

  Task_t *taskCursor = NULL;

  /* Assert if the task list has not been initialized. We wouldn't have to do this
  if we called _TaskListFindTask_() but that isn't needed here. */
  SYSASSERT(ISNOTNULLPTR(taskList));


  /* Assert if the task identifier is zero because it shouldn't be. */
  SYSASSERT(zero < id_);

  /* Check if the task list is not null and the identifier parameter is greater than
  zero. */
  if ((ISNOTNULLPTR(taskList)) && (zero < id_)) {


    taskCursor = taskList->head;

    /* While the task cursor is not null, check the task pointed to by the task cursor
    and compare its identifier against the identifier parameter being searched for. */
    while (ISNOTNULLPTR(taskCursor)) {

      if (id_ == taskCursor->id) {

        ret = taskCursor;

        break;
      }

      taskCursor = taskCursor->next;
    }
  }

  return ret;
}




/* The xTaskGetAllRunTimeStats() system call will return the runtime statistics for all
 of the tasks regardless of their state. The xTaskGetAllRunTimeStats() system call returns
 the xTaskRunTimeStats type. An xBase variable must be passed by reference to xTaskGetAllRunTimeStats()
 which will contain the number of tasks so the end-user can iterate through the tasks. */
TaskRunTimeStats_t *xTaskGetAllRunTimeStats(Base_t *tasks_) {


  Base_t i = zero;

  Base_t tasks = zero;

  Task_t *taskCursor = NULL;

  TaskRunTimeStats_t *ret = NULL;


  /* Assert if the task list has not been initialized. We wouldn't have to do this
  if we called _TaskListFindTask_() but that isn't needed here. */
  SYSASSERT(ISNOTNULLPTR(taskList));


  /* Check to make sure the pointer for the pass-by-reference is not null. */
  SYSASSERT(ISNOTNULLPTR(tasks_));


  /* Check if the task list is not null and the tasks parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(tasks_))) {

    taskCursor = taskList->head;

    /* While the task cursor is not null, continue to traverse the task list counting
    the number of tasks in the list. */
    while (ISNOTNULLPTR(taskCursor)) {

      tasks++;

      taskCursor = taskCursor->next;
    }


    /* Assert if the tasks counted does not agree with the length of the task list. */
    SYSASSERT(tasks == taskList->length);

    /* Check if the number of tasks is greater than zero and the length of the task list equals
    the number of tasks just counted (this is done as an integrity check). */
    if ((zero < tasks) && (tasks == taskList->length)) {


      ret = (TaskRunTimeStats_t *)xMemAlloc(tasks * sizeof(TaskRunTimeStats_t));

      /* Assert if xMemAlloc() didn't do its job. */
      SYSASSERT(ISNOTNULLPTR(ret));


      /* Check if xMemAlloc() successfully allocated the memory. */
      if (ISNOTNULLPTR(ret)) {

        taskCursor = taskList->head;

        /* While the task cursor is not null, continue to traverse the task list adding the
        runtime statistics of each task to the runtime stats array to be returned. */
        while (ISNOTNULLPTR(taskCursor)) {

          ret[i].id = taskCursor->id;

          ret[i].lastRunTime = taskCursor->lastRunTime;

          ret[i].totalRunTime = taskCursor->totalRunTime;

          taskCursor = taskCursor->next;

          i++;
        }

        *tasks_ = tasks;

      } else {

        *tasks_ = zero;
      }
    }
  }

  return ret;
}




/* The xTaskGetTaskRunTimeStats() system call returns the task runtime statistics for
one task. The xTaskGetTaskRunTimeStats() system call returns the xTaskRunTimeStats type.
The memory must be freed by calling xMemFree() after it is no longer needed. */
TaskRunTimeStats_t *xTaskGetTaskRunTimeStats(Task_t *task_) {

  TaskRunTimeStats_t *ret = NULL;



  /* Assert if the task cannot be found in the task list. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));

  /* Check if the task cannot be found in the task list. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {


    ret = (TaskRunTimeStats_t *)_HeapAllocateMemory_(sizeof(TaskRunTimeStats_t));


    /* Assert if xMemAlloc() didn't do its job. */
    SYSASSERT(ISNOTNULLPTR(ret));

    /* Check if xMemAlloc() successfully allocated the memory. */
    if (ISNOTNULLPTR(ret)) {

      ret->id = task_->id;

      ret->lastRunTime = task_->lastRunTime;

      ret->totalRunTime = task_->totalRunTime;
    }
  }

  return ret;
}




/* The xTaskGetNumberOfTasks() system call returns the current number of tasks
regardless of their state. */
Base_t xTaskGetNumberOfTasks(void) {


  Base_t ret = zero;

  Base_t tasks = zero;

  Task_t *taskCursor = NULL;

  /* Assert if the task list is not initialized. */
  SYSASSERT(ISNOTNULLPTR(taskList));


  /* Check if the task list is not initialized. */
  if (ISNOTNULLPTR(taskList)) {

    taskCursor = taskList->head;

    /* While the task cursor is not null, continue to count the number of
    tasks. */
    while (ISNOTNULLPTR(taskCursor)) {

      tasks++;

      taskCursor = taskCursor->next;
    }

    /* Assert if the number of tasks counted does not agree with the task list
    length. */
    SYSASSERT(tasks == taskList->length);

    /* Check if the length of the task list equals the number of tasks counted
    (this is an integrity check). */
    if (tasks == taskList->length) {

      ret = tasks;
    }
  }

  return ret;
}




/* The xTaskGetTaskInfo() system call returns the xTaskInfo structure containing
the details of the task including its identifier, name, state and runtime statistics. */
TaskInfo_t *xTaskGetTaskInfo(Task_t *task_) {

  TaskInfo_t *ret = NULL;



  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task cannot be found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    ret = (TaskInfo_t *)_HeapAllocateMemory_(sizeof(TaskInfo_t));


    /* Assert if xMemAlloc() failed to do its one job in life. */
    SYSASSERT(ISNOTNULLPTR(ret));

    /* Check if the task info memory has been allocated by xMemAlloc(). if it
    has then populate the task info and return. */
    if (ISNOTNULLPTR(ret)) {


      ret->id = task_->id;

      ret->state = task_->state;

      _memcpy_(ret->name, task_->name, CONFIG_TASK_NAME_BYTES);

      ret->lastRunTime = task_->lastRunTime;

      ret->totalRunTime = task_->totalRunTime;
    }
  }

  return ret;
}




/* The xTaskGetAllTaskInfo() system call returns the xTaskInfo structure containing
the details of ALL tasks including its identifier, name, state and runtime statistics. */
TaskInfo_t *xTaskGetAllTaskInfo(Base_t *tasks_) {


  Base_t i = zero;

  Base_t tasks = zero;

  Task_t *taskCursor = NULL;

  TaskInfo_t *ret = NULL;


  /* Assert if the task list has not been initialized. */
  SYSASSERT(ISNOTNULLPTR(taskList));


  /* Assert if the end-user passed us a null pointer. */
  SYSASSERT(ISNOTNULLPTR(tasks_));


  /* Check if the task list is not null and the tasks parameter is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(tasks_))) {

    taskCursor = taskList->head;



    /* While the task cursor is not null, continue to traverse the task list counting
    the number of tasks in the list. */
    while (ISNOTNULLPTR(taskCursor)) {

      tasks++;

      taskCursor = taskCursor->next;
    }


    /* Assert if the number of tasks counted disagrees with the length of the task list. */
    SYSASSERT(tasks == taskList->length);

    /* Check if the number of tasks is greater than zero and the length of the task list equals
    the number of tasks just counted (this is done as an integrity check). */
    if ((zero < tasks) && (tasks == taskList->length)) {



      ret = (TaskInfo_t *)xMemAlloc(tasks * sizeof(TaskInfo_t));


      /* Assert if xMemAlloc() didn't do its job. */
      SYSASSERT(ISNOTNULLPTR(ret));

      /* Check if xMemAlloc() successfully allocated the memory. */
      if (ISNOTNULLPTR(ret)) {


        taskCursor = taskList->head;


        /* While the task cursor is not null, continue to traverse the task list adding the
        runtime statistics of each task to the runtime stats array to be returned. */
        while (ISNOTNULLPTR(taskCursor)) {


          ret[i].id = taskCursor->id;

          ret[i].state = taskCursor->state;

          _memcpy_(ret[i].name, taskCursor->name, CONFIG_TASK_NAME_BYTES);

          ret[i].lastRunTime = taskCursor->lastRunTime;

          ret[i].totalRunTime = taskCursor->totalRunTime;

          taskCursor = taskCursor->next;

          i++;
        }

        *tasks_ = tasks;

      } else {

        *tasks_ = zero;
      }
    }
  }

  return ret;
}



/* The xTaskGetTaskState() system call will return the state of the task. */
TaskState_t xTaskGetTaskState(Task_t *task_) {


  TaskState_t ret = TaskStateError;



  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check to make sure the task was found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    ret = task_->state;
  }


  return ret;
}



/* The xTaskGetName() system call returns the ASCII name of the task. The size of the
task is dependent on the setting CONFIG_TASK_NAME_BYTES. The task name is NOT a null
terminated char array. */
char *xTaskGetName(Task_t *task_) {


  char *ret = NULL;



  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task can be found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {


    ret = (char *)xMemAlloc(CONFIG_TASK_NAME_BYTES);


    /* Assert if xMemAlloc() didn't do its job. */
    SYSASSERT(ISNOTNULLPTR(ret));


    /* Check if the task info memory has been allocated by xMemAlloc(). */
    if (ISNOTNULLPTR(ret)) {



      _memcpy_(ret, task_->name, CONFIG_TASK_NAME_BYTES);
    }
  }


  return ret;
}



/* The xTaskGetId() system call returns the task identifier for the task. */
Base_t xTaskGetId(Task_t *task_) {


  Base_t ret = zero;


  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check that the task was found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    ret = task_->id;
  }


  return ret;
}




/* The xTaskNotifyStateClear() system call will clear a waiting task notification if one
exists without returning the notification. */
void xTaskNotifyStateClear(Task_t *task_) {


  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task was found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {


    /* If the notification bytes are greater than zero then there
    is a notification to be cleared. */
    if (zero < task_->notificationBytes) {

      task_->notificationBytes = zero;

      _memset_(task_->notificationValue, zero, CONFIG_NOTIFICATION_VALUE_BYTES);
    }
  }


  return;
}



/* The xTaskNotificationIsWaiting() system call will return true or false depending
on whether there is a task notification waiting for the task. */
Base_t xTaskNotificationIsWaiting(Task_t *task_) {
  Base_t ret = zero;



  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task was found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {


    /* If there are notification bytes, there is a notification
    waiting. */
    if (zero < task_->notificationBytes) {


      ret = true;
    }
  }


  return ret;
}




/* The xTaskNotifyGive() system call will send a task notification to the specified task. The
task notification bytes is the number of bytes contained in the notification value. The number of
notification bytes must be between one and the CONFIG_NOTIFICATION_VALUE_BYTES setting. The notification
value must contain a pointer to a char array containing the notification value. If the task already
has a waiting task notification, xTaskNotifyGive() will NOT overwrite the waiting task notification. */
Base_t xTaskNotifyGive(Task_t *task_, Base_t notificationBytes_, const char *notificationValue_) {



  Base_t ret = RETURN_FAILURE;

  /* Assert if the notification bytes are zero. */
  SYSASSERT(zero < notificationBytes_);

  /* Assert if the notification bytes exceeds the setting
  CONFIG_NOTIFICATION_VALUE_BYTES. */
  SYSASSERT(CONFIG_NOTIFICATION_VALUE_BYTES >= notificationBytes_);


  /* Assert if the end-user passed us a null pointer for the
  notification value. */
  SYSASSERT(ISNOTNULLPTR(notificationValue_));

  /* Check if the task list is not null and the task parameter is not null, the notification bytes are between
  one and CONFIG_NOTIFICATION_VALUE_BYTES and that the notification value char array pointer is not null. */
  if ((zero < notificationBytes_) && (CONFIG_NOTIFICATION_VALUE_BYTES >= notificationBytes_) && (ISNOTNULLPTR(notificationValue_))) {


    /* Assert if we can't find the task to receive the notification. */
    SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


    /* Check if the task can be found. */
    if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {


      /* Make sure there isn't a waiting notification. xTaskNotifyGive will NOT
      overwrite a waiting notification. */
      if (zero == task_->notificationBytes) {

        task_->notificationBytes = notificationBytes_;

        _memcpy_(task_->notificationValue, notificationValue_, CONFIG_NOTIFICATION_VALUE_BYTES);

        ret = RETURN_SUCCESS;
      }
    }
  }

  return ret;
}



/* The xTaskNotifyTake() system call will return the waiting task notification if there
is one. The xTaskNotifyTake() system call will return an xTaskNotification structure containing
the notification bytes and its value. */
TaskNotification_t *xTaskNotifyTake(Task_t *task_) {

  TaskNotification_t *ret = NULL;



  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task cannot be found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    /* If there are notification bytes, there is a notification
    waiting. */
    if (zero < task_->notificationBytes) {



      ret = (TaskNotification_t *)_HeapAllocateMemory_(sizeof(TaskNotification_t));


      /* Assert if xMemAlloc() didn't do its job. */
      SYSASSERT(ISNOTNULLPTR(ret));

      /* Check if xMemAlloc() successfully allocated the memory for the task notification
      structure. */
      if (ISNOTNULLPTR(ret)) {

        ret->notificationBytes = task_->notificationBytes;

        _memcpy_(ret->notificationValue, task_->notificationValue, CONFIG_NOTIFICATION_VALUE_BYTES);

        task_->notificationBytes = zero;

        _memset_(task_->notificationValue, zero, CONFIG_NOTIFICATION_VALUE_BYTES);
      }
    }
  }


  return ret;
}



/* The xTaskResume() system call will resume a suspended task. Tasks are suspended on creation
so either xTaskResume() or xTaskWait() must be called to place the task in a state that the scheduler
will execute. */
void xTaskResume(Task_t *task_) {



  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));

  /* Check if the task can be found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    task_->state = TaskStateRunning;
  }


  return;
}




/* The xTaskSuspend() system call will suspend a task. A task that has been suspended
will not be executed by the scheduler until xTaskResume() or xTaskWait() is called. */
void xTaskSuspend(Task_t *task_) {



  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task can be found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    task_->state = TaskStateSuspended;
  }


  return;
}




/* The xTaskWait() system call will place a task in the waiting state. A task must
be in the waiting state for event driven multitasking with either direct to task
notifications OR setting the period on the task timer with xTaskChangePeriod(). A task
in the waiting state will not be executed by the scheduler until an event has occurred. */
void xTaskWait(Task_t *task_) {


  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task can be found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    task_->state = TaskStateWaiting;
  }


  return;
}




/* The xTaskChangePeriod() system call will change the period (ticks) on the task timer
for the specified task. The timer period must be greater than zero. To have any effect, the task
must be in the waiting state set by calling xTaskWait() on the task. Once the timer period is set
and the task is in the waiting state, the task will be executed every N ticks based on the period.
Changing the period to zero will prevent the task from being executed even if it is in the waiting state. */
void xTaskChangePeriod(Task_t *task_, Ticks_t timerPeriod_) {


  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task can be found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    task_->timerPeriod = timerPeriod_;
  }


  return;
}




/* The xTaskGetPeriod() will return the period for the timer for the specified task. See
xTaskChangePeriod() for more information on how the task timer works. */
Ticks_t xTaskGetPeriod(Task_t *task_) {


  Ticks_t ret = zero;


  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task can be found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    ret = task_->timerPeriod;
  }


  return ret;
}



/* _TaskListFindTask_() is used to search the task list for a
task and returns RETURN_SUCCESS if the task is found. It also
always checks the health of the heap by calling _MemoryRegionCheckKernel_(). */
Base_t _TaskListFindTask_(const Task_t *task_) {


  Base_t ret = RETURN_FAILURE;

  Task_t *taskCursor = NULL;


  /* Assert if the task list is not initialized. */
  SYSASSERT(ISNOTNULLPTR(taskList));


  /* Assert if the task pointer is null. */
  SYSASSERT(ISNOTNULLPTR(task_));


  /* Check if the task list is initialized and the task pointer is not null. */
  if ((ISNOTNULLPTR(taskList)) && (ISNOTNULLPTR(task_))) {


    /* Assert if _MemoryRegionCheckKernel_() fails on the health check of the heap OR if
    the task pointer's entry cannot be found in the heap. */
    SYSASSERT(RETURN_SUCCESS == _MemoryRegionCheckKernel_(task_, MEMORY_REGION_CHECK_OPTION_W_ADDR));


    /* Check if _MemoryRegionCheckKernel_() passes on the health check and
    the task pointer's entry can be found in the heap. */
    if (RETURN_SUCCESS == _MemoryRegionCheckKernel_(task_, MEMORY_REGION_CHECK_OPTION_W_ADDR)) {


      taskCursor = taskList->head;

      /* Traverse the heap to find the task in the task list. */
      while ((ISNOTNULLPTR(taskCursor)) && (taskCursor != task_)) {

        taskCursor = taskCursor->next;
      }


      /* Assert if the task cannot be found. */
      SYSASSERT(ISNOTNULLPTR(taskCursor));


      /* Check if the task was found, if so then
      return success! */
      if (ISNOTNULLPTR(taskCursor)) {


        ret = RETURN_SUCCESS;
      }
    }
  }

  return ret;
}




/* The xTaskResetTimer() system call will reset the task timer. xTaskResetTimer() does not change
the timer period or the task state when called. See xTaskChangePeriod() for more details on task timers. */
void xTaskResetTimer(Task_t *task_) {



  /* Assert if the task cannot be found. */
  SYSASSERT(RETURN_SUCCESS == _TaskListFindTask_(task_));


  /* Check if the task was found. */
  if (RETURN_SUCCESS == _TaskListFindTask_(task_)) {

    task_->timerStartTime = _SysGetSysTicks_();
  }

  return;
}




/* The xTaskStartScheduler() system call passes control to the HeliOS scheduler. */
void xTaskStartScheduler(void) {



  Task_t *runTask = NULL;

  Task_t *taskCursor = NULL;

  /* Underflow unsigned least runtime to get maximum value */
  Ticks_t leastRunTime = -1;


  /* Assert if the scheduler is already running. */
  SYSASSERT(false == SYSFLAG_RUNNING());


  /* Assert if the task list has not been initialized. */
  SYSASSERT(ISNOTNULLPTR(taskList));


  /* Check that the scheduler is not already running and
  that the task list is initialized. */
  if ((false == SYSFLAG_RUNNING()) && (ISNOTNULLPTR(taskList))) {



    /* Now set the scheduler system flag to running because
    the scheduler IS running. */
    SYSFLAG_RUNNING() = true;



    /* Continue to loop while the scheduler running flag is true. */
    while (SchedulerStateRunning == schedulerState) {


      /* If the runtime overflow flag is true. Reset the runtimes on all of the tasks. */
      if (SYSFLAG_OVERFLOW()) {

        _RunTimeReset_();
      }


      taskCursor = taskList->head;

      /* While the task cursor is not null (i.e., there are further tasks in the task list). */
      while (ISNOTNULLPTR(taskCursor)) {


        /* If the task pointed to by the task cursor is waiting and it has a notification waiting, then execute it. */
        if ((TaskStateWaiting == taskCursor->state) && (zero < taskCursor->notificationBytes)) {

          _TaskRun_(taskCursor);

          /* If the task pointed to by the task cursor is waiting and its timer has expired, then execute it. */
        } else if ((TaskStateWaiting == taskCursor->state) && (zero < taskCursor->timerPeriod) && ((_SysGetSysTicks_() - taskCursor->timerStartTime) > taskCursor->timerPeriod)) {


          _TaskRun_(taskCursor);

          taskCursor->timerStartTime = _SysGetSysTicks_();

          /* If the task pointed to by the task cursor is running and it's total runtime is less than the
          least runtime from previous tasks, then set the run task pointer to the task cursor. This logic
          is used to achieve the runtime balancing. */
        } else if ((TaskStateRunning == taskCursor->state) && (leastRunTime > taskCursor->totalRunTime)) {


          leastRunTime = taskCursor->totalRunTime;

          runTask = taskCursor;

        } else {
          /* Nothing to do here.. Just for MISRA C:2012 compliance. */
        }

        taskCursor = taskCursor->next;
      }


      /* If the run task pointer is not null, then there is a running tasks to execute. */
      if (ISNOTNULLPTR(runTask)) {

        _TaskRun_(runTask);

        runTask = NULL;
      }

      /* Underflow unsigned least runtime to get maximum value */
      leastRunTime = -1;
    }


    SYSFLAG_RUNNING() = false;
  }

  return;
}




/* If the runtime overflow flag is set, then _RunTimeReset_() is called to reset all of the
total runtimes on tasks to their last runtime. */
void _RunTimeReset_(void) {


  Task_t *taskCursor = NULL;



  taskCursor = taskList->head;

  /* While the task cursor is not null (i.e., there are further tasks in the task list). */
  while (ISNOTNULLPTR(taskCursor)) {


    taskCursor->totalRunTime = taskCursor->lastRunTime;

    taskCursor = taskCursor->next;
  }

  SYSFLAG_OVERFLOW() = false;

  return;
}




/* Called by the xTaskStartScheduler() system call, _TaskRun_() executes a task and updates all of its
runtime statistics. */
void _TaskRun_(Task_t *task_) {


  Ticks_t taskStartTime = zero;

  Ticks_t prevTotalRunTime = zero;

  /* Record the total runtime before executing the task. */
  prevTotalRunTime = task_->totalRunTime;

  /* Record the start time of the task. */
  taskStartTime = _SysGetSysTicks_();



  /* Call the task from its callback pointer. */
  (*task_->callback)(task_, task_->taskParameter);


  /* Calculate the runtime and store it in last runtime. */
  task_->lastRunTime = _SysGetSysTicks_() - taskStartTime;

  /* Add last runtime to the total runtime. */
  task_->totalRunTime += task_->lastRunTime;

  /* Check if the new total runtime is less than the previous total runtime,
  if so an overflow has occurred so set the runtime over flow system flag. */
  if (task_->totalRunTime < prevTotalRunTime) {

    SYSFLAG_OVERFLOW() = true;
  }

  return;
}




/* The xTaskResumeAll() system call will set the scheduler system flag so the next
call to xTaskStartScheduler() will resume execute of all tasks. */
void xTaskResumeAll(void) {


  schedulerState = SchedulerStateRunning;

  return;
}




/* The xTaskSuspendAll() system call will set the scheduler system flag so the scheduler
will stop and return. */
void xTaskSuspendAll(void) {

  schedulerState = SchedulerStateSuspended;


  return;
}

/* The xTaskGetSchedulerState() system call will return the state of the scheduler. */
SchedulerState_t xTaskGetSchedulerState(void) {


  return schedulerState;
}


#if defined(POSIX_ARCH_OTHER)
void __TaskStateClear__(void) {

  taskList = NULL;

  return;
}
#endif