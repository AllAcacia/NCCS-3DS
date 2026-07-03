/*
 * Filename: ll2.h
 * Author:   AllAcacia
 * 
 * MODULE INFO:
 * Implements a doubly linked list,
 * to allow the maintenance of order
 * without moving anything at all!
 */

#include <stdint.h>
#include <stdlimits.h>

#define DEQUE_MAX_NODES UINT32_MAX

#ifndef u32
#define u32 uint32_t
#endif//u32, I do not want to import the entire 3ds.h library for this


typedef struct {
    void* n_item;
    u32 n_next;
    u32 n_prev;
} LL2_Node;


typedef struct {
    LL2_Node* nodes_arr; // pointer to all nodes
    u32 nodes_tot;  // total count of nodes possible to use in deque
    u32 nodes_cnt;  // count of nodes utilised
    
    u32 n_head;    // head node of deque
    u32 n_last;    // "tail" node of deque, of last inserted item of list
    u32 n_tail;    // TRUE tail node of deque
} LL2_Deque;


LL2_Deque* LL2_Deque_Init(size_t cnt);

int LL2_Deque_Free(LL2_Deque* deque);

int LL2_Deque_EnqueueL(LL2_Deque* deque, void* n_item);

int LL2_Deque_EnqueueR(LL2_Deque* deque, void* n_item);

void* LL2_Deque_DequeueL(LL2_Deque* deque);

void* LL2_Deque_DequeueR(LL2_Deque* deque);

int LL2_Deque_Insert(LL2_Deque* deque, void* value);

int LL2_Deque_Remove(LL2_Deque* deque, void* value);