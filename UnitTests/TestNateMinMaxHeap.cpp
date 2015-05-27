#include <queue>
#include <vector>
#include <functional>

extern "C"
{
  #include "TestUtils.h"
  #include "ccan/NateMinMaxHeap/NateMinMaxHeap.h"

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
          CHECK(level % 2 == 0 ? childValue >= value : childValue <= value, );
        }
      }

      size_t grandchildrenFirstIndex = (((firstIndex + 1) << 2) - 1);
      size_t leftGrandchildIndex = grandchildrenFirstIndex + (offset * 4);
      for (size_t i = leftGrandchildIndex; i < leftGrandchildIndex + 4; i++)
      {
        if (i < count)
        {
          void * grandchildValue = NateMinMaxHeap_Get(obj, i);
          CHECK(level % 2 == 0 ? grandchildValue >= value : grandchildValue <= value, );
        }
      }
    }
  }
}

void Test_MuchRandom_ForTargetSize(size_t targetSize)
{
  int i, numOps;
  NateMinMaxHeap * obj;
  std::priority_queue<void*, std::vector<void*>, std::greater<void*> > q;

  obj = NateMinMaxHeap_Create2();
  numOps = targetSize >= 1000000 ? 10000000 : 1000000;

  // perform 1,000,000 random add/remove operations on a tree of the given height
  // and verify it holds correct contents and no neighboring memory is bogus'd
  for (i = 0; i < numOps; i++)
  {
    CHECK(q.size() == NateMinMaxHeap_GetCount(obj), );
    if (q.size() == 0)
    {
      // add
      int newValue = rand();
      NateMinMaxHeap_Add(obj, (void*)newValue);
      q.push((void*)newValue);
      CHECK(q.top() == NateMinMaxHeap_GetMin(obj, 0), "after first add");
    }
    else 
    {
#define ACTION_ADD 1
#define ACTION_REMOVE 0

      // action == random value between 0 and 7
      int action = rand() & 0x07;

      // favor additions when not quite up to max size yet
      if (q.size() < 0.90f * targetSize)
      {
        action = action >= 2 ? ACTION_ADD : ACTION_REMOVE;
      }
      // favor removals when leaving max size
      else if (q.size() > (1.10f * targetSize))
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
        q.push((void*)newValue);
        CHECK(q.top() == NateMinMaxHeap_GetMin(obj, 0), "after add");
        TestEntireTree(obj);
      }
      else
      {
        NateMinMaxHeap_RemoveMin(obj);
        q.pop();
        if (q.size() != 0)
        {
          CHECK(q.top() == NateMinMaxHeap_GetMin(obj, 0), "after remove");
        }
        TestEntireTree(obj);
      }
    }
  }

  NateMinMaxHeap_Destroy(obj);
}

void Test_MuchRandom()
{
  Test_MuchRandom_ForTargetSize(7);
  Test_MuchRandom_ForTargetSize(15);
  Test_MuchRandom_ForTargetSize(31);
  Test_MuchRandom_ForTargetSize(63);
  Test_MuchRandom_ForTargetSize(1000000);
}

void Test_OtherMethods()
{
}

void Test_NateMinMaxHeap()
{
  Test_MuchRandom();
  Test_OtherMethods();
}