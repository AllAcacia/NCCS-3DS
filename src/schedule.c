/*
 * Filename: schedule.c
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Handles running tasks at
 * scheduled priorities and
 * potentially asynchronous
 * intervals.
 */

#include "schedule.h"


static ScheduledTask* TASK_COLLECTION;
static int TASK_CAPACITY;
static int TASK_NUM;
static bool TASK_FLAG;
static u16 TASK_ID_CURR;


int Schedule_Init(int n)
{
    TASK_FLAG = false;
    TASK_NUM = 0;
    TASK_ID_CURR = TASK_NO_CAPACITY + 1; // ID=0 indicates no space free

    TASK_COLLECTION = calloc(n, sizeof(ScheduledTask));
    if (TASK_COLLECTION == NULL) {
        return EXIT_FAILURE;
    } else {
        TASK_CAPACITY = n;
        return EXIT_SUCCESS;
    }
}


int Schedule_ExecuteTasks(void)
{
    u64 tick_now = GS_UpdTickNow();

    for (int i = 0; i < TASK_NUM; i += 1) {
        if (!TASK_COLLECTION[i].lock) { // if locked, don't execute
            if (TASK_FLAG) {
                break;
            } else if (GS_CheckDelayTimer(TASK_COLLECTION[i].prevrun_ticks, TASK_COLLECTION[i].period_ticks)) {
                TASK_COLLECTION[i].active = true;
                TASK_COLLECTION[i].pfunc(TASK_COLLECTION[i].pparam);
                TASK_COLLECTION[i].active = false;
                TASK_COLLECTION[i].prevrun_ticks = tick_now;
            }
        }
    }

    if (TASK_FLAG) {
        Schedule_SortTasks();
        TASK_FLAG = false; // lower flag
    }

    return EXIT_SUCCESS;
}


int Schedule_SortCmp(const void* a, const void* b)
{
    ScheduledTask* task1 = (ScheduledTask*)a;
    ScheduledTask* task2 = (ScheduledTask*)b;

    // larger period means less frequent, so
    // it should be placed near end of array
    if (task1->period_ticks > task2->period_ticks) {
        return 1;
    } else if (task1->period_ticks < task2->period_ticks) {
        return -1;
    } else {
        return 0;
    }
}


void Schedule_SortTasks(void)
{
    qsort(TASK_COLLECTION, TASK_NUM, sizeof(ScheduledTask), &Schedule_SortCmp);
}


int Schedule_AppendTask(int (*pfunc)(void* param), void* pparam, u8 frequency)
{
    if (TASK_NUM < TASK_CAPACITY) {
        ScheduledTask* task = &TASK_COLLECTION[TASK_NUM];
        task->pfunc = pfunc;
        task->pparam = pparam;
        task->lock = false;
        task->active = false;
        task->period_ticks = GS_GetTickDelay_hz(frequency);
        task->prevrun_ticks = svcGetSystemTick();
        task->id = TASK_ID_CURR;
        
        TASK_ID_CURR += 1;
        if (TASK_ID_CURR == TASK_NO_CAPACITY) {
            TASK_ID_CURR = TASK_NO_CAPACITY + 1;
        }

        TASK_NUM += 1;
        TASK_FLAG = true; // signal collection has been altered

        printf("Appended task %u\n", task->id);

        return task->id;
    } else {
        return TASK_NO_CAPACITY; // no capacity left
    }
}


int Schedule_DeleteTask(u16 task_id)
{
    ScheduledTask* task = Schedule_FindTask(task_id);
    if (task != NULL) {
        *task = TASK_COLLECTION[TASK_NUM-1];
        TASK_NUM -= 1;
        TASK_FLAG = true;
        printf("Deleted task %u\n", task_id);
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}


ScheduledTask* Schedule_FindTask(u16 task_id)
{
    ScheduledTask* res = NULL;
    for (int i = 0; i < TASK_NUM; i += 1) {
        int id = TASK_COLLECTION[i].id;
        if (id == task_id) {
            res = &TASK_COLLECTION[i];
            break;
        }
    }
    return res;
}


int Schedule_LockTask(u16 task_id)
{
    ScheduledTask* task = Schedule_FindTask(task_id);
    if (task != NULL) {
        task->lock = true;
        printf("Locked task %u\n", task_id);
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}


int Schedule_UnlockTask(u16 task_id)
{
    ScheduledTask* task = Schedule_FindTask(task_id);
    if (task != NULL) {
        task->lock = false;
        printf("Unlocked task %u\n", task_id);
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}