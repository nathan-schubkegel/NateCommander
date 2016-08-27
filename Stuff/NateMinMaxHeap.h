/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_MIN_MAX_HEAP
#define NATE_MIN_MAX_HEAP

#include <stdlib.h>

typedef int (NateMinMaxHeap_NodeComparer)(void * node1, void * node2);

typedef struct NateMinMaxHeap
{
  NateMinMaxHeap_NodeComparer * nodeComparer;
  size_t numNodesUsed;
  size_t numNodesAllocated;
  void ** nodes;
} NateMinMaxHeap;

#define NateMinMaxHeap_InvalidIndex 0xFFFFFFFF

// These methods malloc/free a NateMinMaxHeap and its internals
NateMinMaxHeap * NateMinMaxHeap_Create(NateMinMaxHeap_NodeComparer nodeComparer);
NateMinMaxHeap * NateMinMaxHeap_Create2(void);
void NateMinMaxHeap_Destroy(NateMinMaxHeap * obj);

// These methods do not malloc/free the NateMinMaxHeap, but they malloc free its internals
int NateMinMaxHeap_Init(NateMinMaxHeap * obj, NateMinMaxHeap_NodeComparer nodeComparer);
void NateMinMaxHeap_Uninit(NateMinMaxHeap * obj);

int NateMinMaxHeap_Add(NateMinMaxHeap * obj, void * node);
void NateMinMaxHeap_RemoveAt(NateMinMaxHeap * obj, size_t index);
void NateMinMaxHeap_RemoveMin(NateMinMaxHeap * obj);
void NateMinMaxHeap_RemoveMax(NateMinMaxHeap * obj);
void NateMinMaxHeap_Clear(NateMinMaxHeap * obj);
void NateMinMaxHeap_ClearAndShrink(NateMinMaxHeap * obj);

size_t NateMinMaxHeap_GetCapacity(NateMinMaxHeap * obj);
size_t NateMinMaxHeap_SetCapacity(NateMinMaxHeap * obj, size_t newCapacity);

size_t NateMinMaxHeap_GetCount(NateMinMaxHeap * obj);
void * NateMinMaxHeap_Get(NateMinMaxHeap * obj, size_t index);

// don't call these when the heap is empty
void * NateMinMaxHeap_GetMin(NateMinMaxHeap * obj, size_t * index); // index may be NULL if not needed
void * NateMinMaxHeap_GetMax(NateMinMaxHeap * obj, size_t * index); // index may be NULL if not needed

#endif