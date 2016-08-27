/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_LIST_H
#define NATE_LIST_H

#include <stdlib.h>

#define NateList_LastIndex 0xFFFFFFFF

typedef struct NateList
{
  void * items;
  size_t bytesPerItem;
  size_t numUsed;
  size_t numAllocated;
} NateList;

// These methods malloc/free a NateList and its internals
NateList * NateList_Create(void);
void NateList_Destroy(NateList * obj);

// These methods do not malloc/free the NateList, but they malloc/free its internals
void NateList_Init(NateList * obj);
void NateList_Uninit(NateList * obj);

// bytesPerItem defaults to sizeof(void*)
size_t NateList_GetBytesPerItem(NateList * obj);
void NateList_SetBytesPerItem(NateList * obj, size_t bytesPerItem);

size_t NateList_GetCount(NateList * obj);

// you can provide index = NateList_LastIndex to reference last index
void * NateList_GetPtr(NateList * obj, size_t index); // gets stored value casted as a pointer
void * NateList_GetData(NateList * obj, size_t index); // gets pointer to stored value

// returns 1 on success, 0 on memory allocation failure
int NateList_AddPtr(NateList * obj, void * item); // stores pointer
// returns pointer to new data on success, 0 on memory allocation failure
void * NateList_AddCopyOfData(NateList * obj, void * source); // copies [bytesPerItem] bytes from source
void * NateList_AddZeroedData(NateList * obj); // zeroes bytes in new item

// you can provide index = NateList_LastIndex to reference last index
void NateList_SetPtr(NateList * obj, size_t index, void * item); // stored pointer
// returns pointer to item data
void * NateList_SetCopyOfData(NateList * obj, size_t index, void * source); // copies [bytesPerItem] bytes from source
void * NateList_SetZeroedData(NateList * obj, size_t index); // zeroes bytes in new item

// finds the first item that causes the predicate to return true
// returns 1 if found and assigns 'index' = the index
// returns 0 if not found and assigns 'index' = 0xFFFFFFFF
// index may be null if not needed
typedef int (NateList_PredicatePtr)(void * userData, void * item); // passes stored value casted as pointer to predicate
typedef int (NateList_PredicateData)(void * userData, void * item); // passes pointer to stored value to predicate
int NateList_FindPtr(NateList * obj, NateList_PredicatePtr * predicate, void * userData, size_t * index); 
int NateList_FindData(NateList * obj, NateList_PredicateData * predicate, void * userData, size_t * index); 

void NateList_RemoveLast(NateList * obj);

#endif