/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include <queue>
#include <vector>
#include <functional>

extern "C"
{
  #include "TestUtils.h"
  #include "NateMinMaxHeap.h"

  extern size_t MyGetFirstChildIndex(size_t index);
}

void TestEntireTree(NateMinMaxHeap * obj)
{
  size_t count = NateMinMaxHeap_GetCount(obj);

  size_t level = 0;
  size_t firstIndex = 0;
  size_t numNodesAtLevel = 1;
  for (; level < 32; level++)
  {
    firstIndex = (1 << level) - 1;
    numNodesAtLevel = (firstIndex + 1);

    if (firstIndex >= count) return;

    for (size_t offset = 0; offset < numNodesAtLevel; offset++)
    {
      size_t myIndex = firstIndex + offset;
      if (myIndex >= count) return;
      
      void* value = NateMinMaxHeap_Get(obj, myIndex);
      
      size_t childrenFirstIndex = (((firstIndex + 1) << 1) - 1);
      size_t leftChildIndex = childrenFirstIndex + (offset * 2);
      for (size_t i = leftChildIndex; i < leftChildIndex + 2; i++)
      {
        if (i < count)
        {
          void * childValue = NateMinMaxHeap_Get(obj, i);
          CHECK((level & 0x01) == 0 ? childValue >= value : childValue <= value, );
        }
      }

      size_t grandchildrenFirstIndex = (((firstIndex + 1) << 2) - 1);
      size_t leftGrandchildIndex = grandchildrenFirstIndex + (offset * 4);
      for (size_t i = leftGrandchildIndex; i < leftGrandchildIndex + 4; i++)
      {
        if (i < count)
        {
          void * grandchildValue = NateMinMaxHeap_Get(obj, i);
          CHECK((level & 0x01) == 0 ? grandchildValue >= value : grandchildValue <= value, );
        }
      }
    }
  }
}

void Test_MuchRandom_ForTargetSize(size_t targetSize, size_t numOps, size_t testFullTreeEveryNumOps, int matchAgainstPriorityQueue)
{
  size_t i;
  size_t count;
  size_t iUntilNextFullTest;
  NateMinMaxHeap * obj;
  std::priority_queue<void*, std::vector<void*>, std::greater<void*> > q;

  obj = NateMinMaxHeap_Create2();
  iUntilNextFullTest = testFullTreeEveryNumOps;

  // perform random add/remove operations on a tree 
  // and verify it holds correct contents
  // TODO: verify no neighboring memory is bogus'd
  for (i = 0; i < numOps; i++)
  {
    iUntilNextFullTest--;
    count = NateMinMaxHeap_GetCount(obj);

    if (matchAgainstPriorityQueue)
    {
      CHECK(q.size() == count, );
    }

    if (count == 0)
    {
      // add
      int newValue = rand();
      NateMinMaxHeap_Add(obj, (void*)newValue);

      if (matchAgainstPriorityQueue)
      {
        q.push((void*)newValue);
        CHECK(q.top() == NateMinMaxHeap_GetMin(obj, 0), "after first add");
      }
      if (iUntilNextFullTest == 0) TestEntireTree(obj);
    }
    else 
    {
#define ACTION_ADD 1
#define ACTION_REMOVE 0

      // action == random value between 0 and 7
      int action = rand() & 0x07;

      // favor additions when not quite up to max size yet
      if (count < 0.80f * targetSize)
      {
        action = action >= 2 ? ACTION_ADD : ACTION_REMOVE;
      }
      // favor removals when leaving max size
      else if (count > (1.20f * targetSize))
      {
        action = action >= 2 ? ACTION_REMOVE : ACTION_ADD;
      }
      // pure random when around the target size
      else
      {
        action = action < 4 ? ACTION_ADD : ACTION_REMOVE;
      }

      if (action == ACTION_ADD)
      {
        // add
        int newValue = rand();
        NateMinMaxHeap_Add(obj, (void*)newValue);

        if (matchAgainstPriorityQueue)
        {
          q.push((void*)newValue);
          CHECK(q.top() == NateMinMaxHeap_GetMin(obj, 0), "after add");
        }
        if (iUntilNextFullTest == 0) TestEntireTree(obj);
      }
      else
      {
        NateMinMaxHeap_RemoveMin(obj);
        if (matchAgainstPriorityQueue)
        {
          q.pop();
          if (q.size() != 0)
          {
            CHECK(q.top() == NateMinMaxHeap_GetMin(obj, 0), "after remove");
          }
        }
        if (iUntilNextFullTest == 0) TestEntireTree(obj);
      }
    }

    if (iUntilNextFullTest == 0) iUntilNextFullTest = testFullTreeEveryNumOps;
  }

  NateMinMaxHeap_Destroy(obj);
}

void Test_MuchRandom()
{
  Test_MuchRandom_ForTargetSize(7, 5000, 1, 1);
  Test_MuchRandom_ForTargetSize(15, 5000, 1, 1);
  Test_MuchRandom_ForTargetSize(31, 5000, 1, 1);
  Test_MuchRandom_ForTargetSize(63, 5000, 1, 1);
  // these are cool to test, but they just take too much time to always leave on
  //Test_MuchRandom_ForTargetSize(50000, 500000, 1000, 0);
  //Test_MuchRandom_ForTargetSize(1000000, 5000000, 1000000, 0);
}

void Test_OtherMethods()
{
  // TODO: if someone cared, yay do this
}

void Test_NateMinMaxHeap()
{
  Test_MuchRandom();
  Test_OtherMethods();
}