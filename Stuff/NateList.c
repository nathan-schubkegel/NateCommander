/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "NateList.h"

#include <string.h>

NateList * NateList_Create(void)
{
  NateList * obj = malloc(sizeof(NateList));
  if (obj == 0) return 0;
  NateList_Init(obj);
  return obj;
}

void NateList_Destroy(NateList * obj)
{
  NateList_Uninit(obj);
  free(obj);
}

void NateList_Init(NateList * obj)
{
  memset(obj, 0, sizeof(NateList));
  obj->bytesPerItem = sizeof(void*);
}

void NateList_Uninit(NateList * obj)
{
  free(obj->items);
  memset(obj, 0, sizeof(NateList));
}

size_t NateList_GetBytesPerItem(NateList * obj)
{
  return obj->bytesPerItem;
}

void NateList_SetBytesPerItem(NateList * obj, size_t bytesPerItem)
{
  // wipe the list. Someday could do something more intelligent if needed.
  free(obj->items);
  memset(obj, 0, sizeof(NateList));

  // store the new bytesPerItem
  obj->bytesPerItem = bytesPerItem;
}

size_t NateList_GetCount(NateList * obj)
{
  return obj->numUsed;
}

#define MyIndexToPtr(obj, index) (&((char*)(obj)->items)[(index) * obj->bytesPerItem])

// you can provide index = NateList_LastIndex to reference last index
void * NateList_GetPtr(NateList * obj, size_t index) // gets stored value casted as a pointer
{
  if (index == NateList_LastIndex) index = obj->numUsed - 1;
  return *(void**)MyIndexToPtr(obj, index);
}

// you can provide index = NateList_LastIndex to reference last index
void * NateList_GetData(NateList * obj, size_t index) // gets pointer to stored value
{
  if (index == NateList_LastIndex) index = obj->numUsed - 1;
  return (void*)MyIndexToPtr(obj, index);
}

int MyAllocateIfNeeded(NateList * obj)
{
  void * newItems;

  if (obj->numAllocated == 0)
  {
    // return zero if size_t can't represent requested byte size
    if (obj->bytesPerItem == 0) return 0;
    if (0xFFFFFFFF / 8 < obj->bytesPerItem) return 0;

    newItems = malloc(obj->bytesPerItem * 8);
    if (newItems == 0) return 0;
    memset(newItems, 0, obj->bytesPerItem * 8);
    obj->items = newItems;
    obj->numAllocated = 8;
  }
  else if (obj->numAllocated == obj->numUsed)
  {
    // return zero if size_t isn't big enough for number of requested items
    if (obj->bytesPerItem == 0) return 0;
    if (0xFFFFFFFF / 2 / obj->numAllocated < obj->bytesPerItem) return 0;

    newItems = malloc(obj->bytesPerItem * obj->numAllocated * 2);
    if (newItems == 0) return 0;
    memcpy(newItems, obj->items, obj->numAllocated * obj->bytesPerItem);
    memset((char*)newItems + (obj->numAllocated * obj->bytesPerItem), 0, obj->numAllocated * obj->bytesPerItem);
    free(obj->items);
    obj->items = newItems;
    obj->numAllocated *= 2;
  }
  return 1;
}

// returns 1 on success, 0 on memory allocation failure
int NateList_AddPtr(NateList * obj, void * item) // stores pointer
{
  if (!MyAllocateIfNeeded(obj)) return 0;
  *(void**)MyIndexToPtr(obj, obj->numUsed) = item;
  obj->numUsed++;
  return 1;
}

// returns pointer to new data on success, 0 on memory allocation failure
void * NateList_AddCopyOfData(NateList * obj, void * source) // copies [bytesPerItem] bytes from source
{
  void * ptr;
  if (!MyAllocateIfNeeded(obj)) return 0;
  ptr = MyIndexToPtr(obj, obj->numUsed);
  memcpy(ptr, source, obj->bytesPerItem);
  obj->numUsed++;
  return ptr;
}

void * NateList_AddZeroedData(NateList * obj) // zeroes bytes in new item
{
  void * ptr;
  if (!MyAllocateIfNeeded(obj)) return 0;
  ptr = MyIndexToPtr(obj, obj->numUsed);
  memset(ptr, 0, obj->bytesPerItem);
  obj->numUsed++;
  return ptr;
}

// you can provide index = NateList_LastIndex to reference last index
void NateList_SetPtr(NateList * obj, size_t index, void * item) // stored pointer
{
  if (index == NateList_LastIndex) index = obj->numUsed - 1;
  *(void**)MyIndexToPtr(obj, index) = item;
}

// returns pointer to item data
void * NateList_SetCopyOfData(NateList * obj, size_t index, void * source) // copies [bytesPerItem] bytes from source
{
  void * ptr;
  if (index == NateList_LastIndex) index = obj->numUsed - 1;
  ptr = MyIndexToPtr(obj, index);
  memcpy(ptr, source, obj->bytesPerItem);
  return ptr;
}

void * NateList_SetZeroedData(NateList * obj, size_t index) // zeroes bytes in new item
{
  void * ptr;
  if (index == NateList_LastIndex) index = obj->numUsed - 1;
  ptr = MyIndexToPtr(obj, index);
  memset(ptr, 0, obj->bytesPerItem);
  return ptr;
}

// finds the first item that causes the predicate to return true
// returns 1 if found and assigns 'index' = the index
// returns 0 if not found and assigns 'index' = 0xFFFFFFFF
// index may be null if not needed
int NateList_FindPtr(NateList * obj, NateList_PredicatePtr * predicate, void * userData, size_t * index)
{
  size_t i;
  for (i = 0; i < obj->numUsed; i++)
  {
    if (predicate(userData, *(void**)MyIndexToPtr(obj, i)))
    {
      if (index != 0) *index = i;
      return 1;
    }
  }
  if (index != 0) *index = 0xFFFFFFFF;
  return 0;
}

int NateList_FindData(NateList * obj, NateList_PredicateData * predicate, void * userData, size_t * index)
{
  size_t i;
  for (i = 0; i < obj->numUsed; i++)
  {
    if (predicate(userData, (void*)MyIndexToPtr(obj, i)))
    {
      if (index != 0) *index = i;
      return 1;
    }
  }
  if (index != 0) *index = 0xFFFFFFFF;
  return 0;
}

void NateList_RemoveLast(NateList * obj)
{
  // TODO: could theoretically make this more efficient by not wiping it
  // but it makes me feel good to know I'm more likely to trip on bad code this way
  void* data;
  data = (void*)MyIndexToPtr(obj, obj->numUsed - 1);
  memset(data, 0, obj->bytesPerItem);
  obj->numUsed--;
}