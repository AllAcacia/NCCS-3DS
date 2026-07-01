/*
 * Filename: schedule.h
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles running tasks at
 * scheduled priorities and
 * potentially asynchronous
 * intervals.
 */


#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <stdint.h>
#include <stdlib.h>
#include <3ds.h>
#include <stdbool.h>
#include "schedule.h"
#include "gamestate.h"

#define TASK_NO_CAPACITY 0

typedef struct {
    u16 id;                    // identifier
    bool lock;                 // prevents function being called
    bool active;               // is function executing?
    int (*pfunc)(void* param); // pointer to function execution
    void* pparam;              // pointer to function parameters
    u32 period_ticks;          // period for counter
    u64 prevrun_ticks;         // tick of last run
} ScheduledTask;

int Schedule_Init(int n);

int Schedule_ExecuteTasks(void);

int Schedule_SortCmp(const void* a, const void* b);

void Schedule_SortTasks(void);

int Schedule_AppendTask(int (*pfunc)(void* param), void* pparam, u8 frequency);

int Schedule_DeleteTask(u16 task_id);

ScheduledTask* Schedule_FindTask(u16 task_id);

int Schedule_LockTask(u16 task_id);

int Schedule_UnlockTask(u16 task_id);

#endif // SCHEDULE_H