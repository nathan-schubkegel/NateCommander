#include "NateMinMaxHeap.h"

#include <stdlib.h>
#include <string.h>
//#include <assert.h>

void MyMoveLeafNodeToValidPosition(NateMinMaxHeap * obj, size_t index);

int MyPtrCompare(void* left, void* right)
{
  return (left > right ? 1 : left == right ? 0 : -1);
}

NateMinMaxHeap * NateMinMaxHeap_Create(NateMinMaxHeap_NodeComparer nodeComparer)
{
  NateMinMaxHeap * obj = malloc(sizeof(NateMinMaxHeap));
  if (obj == 0)
  {
    return 0;
  }
  NateMinMaxHeap_Init(obj, nodeComparer);
  return obj;
}

NateMinMaxHeap * NateMinMaxHeap_Create2()
{
  return NateMinMaxHeap_Create(MyPtrCompare);
}

void NateMinMaxHeap_Destroy(NateMinMaxHeap * obj)
{
  NateMinMaxHeap_Uninit(obj);
  free(obj);
}

int NateMinMaxHeap_Init(NateMinMaxHeap * obj, NateMinMaxHeap_NodeComparer nodeComparer)
{
  obj->nodeComparer = nodeComparer;
  obj->numNodesUsed = 0;
  obj->numNodesAllocated = 7; // see comments in MyGetParentIndex() for why 7 is a decent starting allocation size
  obj->nodes = malloc(sizeof(void*) * 7);
  if (obj->nodes == 0)
  {
    return 0;
  }
  memset(obj->nodes, 0, sizeof(void*) * 7);
  return 1;
}

void NateMinMaxHeap_Uninit(NateMinMaxHeap * obj)
{
  free(obj->nodes);
  obj->nodes = 0;
}

#define MySwap(a, b) { oldNode = (a); (a) = (b); (b) = oldNode; }
#define MyMin(a, b) ((a) <= (b) ? (a) : (b))
#define MyMax(a, b) ((a) >= (b) ? (a) : (b))

size_t MyIndexFromPointer(NateMinMaxHeap * obj, void ** node)
{
  size_t diff;
  
  //assert(node > obj->nodes);
  diff = node - obj->nodes;
  //assert(diff < obj->numNodesAllocated);
  
  // TODO: this will need to change for x64 compilation because >> 2 means (divide by sizeof(void*) == 4)
  //((size_t)((char*)(node) - (char*)(obj->nodes)) >> 2)
  return diff;
}

size_t MyGetNearestMaybeLowerPowerOf2(register size_t number)
{
  // restated: find position of most significant 1 bit in the integer's binary representation

  // example: given index=32, return 32
  // example: given index=31, return 16
  // example: given index=17, return 16
  // example: given index=16, return 16
  // example: given index=15, return 8
  // example: given index=3, return 2
  
  // values less than 2 aren't exactly obvious to me... but they work well enough
  // example: given index=2, return 2
  // example: given index=1, return 1
  // example: given index=0, return 0

  // stackoverflow made it obvious this was more efficient than a while loop
  // and more portable than assembly instructions

  // set all bits to the right of the most significant 1 bit to 1
  number |= (number >> 1);
  number |= (number >> 2);
  number |= (number >> 4);
  number |= (number >> 8);
  number |= (number >> 16);
  // TODO: for 64-bit architectures, would need to shift >> 32
  //number |= (number >> 32);

  // set all bits to the right of the most significant 1 bit to 0
  return number & ~(number >> 1);
}

// A min-max heap is a binary tree implemented as an array
// The indexes in the array appear in the binary tree like this:
//               0
//           /        \
//         1           2
//       /   \       /    \
//      3     4     5      6
//     / \   / \   /  \   /  \
//    7   8 9  10 11  12 13  14
//
// odd-indexes are "left" children (index & 0x01 == 1)
// even-indexes are "right" children (index & 0x01 == 0)
// a "left" child's parent is at (left child index / 2)
// a "right" child's parent is at (right child index / 2) - 1
// NOTE: for root node index==0 this returns 0, so watch out
// NOTE: using bitshift as cheap divide-by-two
#define MyGetParentIndex(index) ( \
  (size_t)(index) & 0x01 ? (size_t)(index) >> 1 : \
  (size_t)(index) == 0 ? 0 : \
  ((size_t)(index) >> 1) - 1 \
  )

#define MyGetGrandparentIndex(index) MyGetParentIndex(MyGetParentIndex(index))

// see comments and diagram in MyGetParentIndex
// a node's left child is at (index * 2 + 1)
// a node's right child is at (index * 2 + 2)
// using bit shift as cheap multiply by two
#define MyGetFirstChildIndex(index) (((size_t)(index) << 1) + 1)

// see comments and diagram in MyGetParentIndex
#define MyGetfirstGrandchildIndex(index) (((((size_t)(index) << 1) + 1) << 1) + 1)

// see comments and diagram in MyGetParentIndex
// every level n (root level n == 1) starts with the node at index 2^n - 1
// every odd level n indicates a min level
// every even level n indicates a max level
// 2^n is a number with a single 1-bit and the rest 0-bits
// get answer by checking every second bit at once (0x55555555)
#define MyIsMinLevel(index) (0x55555555 & MyGetNearestMaybeLowerPowerOf2((size_t)(index) + 1))

int NateMinMaxHeap_Add(NateMinMaxHeap * obj, void * node)
{
  size_t index;
  void ** newNodes;
  size_t newNumNodes;
  
  // add space if needed
  if (obj->numNodesAllocated - obj->numNodesUsed == 0)
  {
    // algorithms below compute "index of grandchildren" for leaf nodes
    // which means those need to be valid unused indexes
    if (obj->numNodesAllocated == 0x1FFFFFFF)
    {
      return 0;
    }

    // TODO: learn how to use realloc here. TODO: would it really be better?
    newNumNodes = obj->numNodesAllocated * 2 + 1;
    newNodes = malloc(sizeof(void*) * newNumNodes);
    if (newNodes == 0)
    {
      return 0;
    }
    memcpy(newNodes, obj->nodes, sizeof(void*) * obj->numNodesAllocated);
    free(obj->nodes);
    obj->nodes = newNodes;
    obj->numNodesAllocated = newNumNodes;
  }
  
  // add node to end (it is inherently a leaf node)
  // example: adding a node at index 5
  //         0
  //       /   \
  //      1      2
  //     / \    /
  //    3   4 [5]
  index = obj->numNodesUsed++;
  obj->nodes[index] = node;
  MyMoveLeafNodeToValidPosition(obj, index);
  return 1;
}

// I think some implementations call this "percolate up"
void MyMoveLeafNodeToValidPosition(NateMinMaxHeap * obj, size_t index)
{
  NateMinMaxHeap_NodeComparer * nodeComparer;
  void ** nodes;
  size_t parentIndex;
  size_t grandparentIndex;
  void * oldNode;

  // for first node (root node) nothing more is needed
  if (index == 0)
  {
    return;
  }

  nodes = obj->nodes;
  nodeComparer = obj->nodeComparer;

  // The min-max heap is a binary tree with these rules:
  // 0.) every level is either a Min-level or a Max-level
  // 1.) A node on a Min-level holds a value that is less than all nodes below it (in its sub-tree)
  // 2.) A node on a Max-level holds a value that is greater than all nodes below it (in its sub-tree)
  
  // This means:
  // The root node always holds the smallest value in the tree
  // One of the root node's 2 children always holds the largest value in the tree
  
  if (MyIsMinLevel(index))
  {
    // is my relationship with parent valid?
    // parent is max-level node, so valid means (child <= parent)
    parentIndex = MyGetParentIndex(index);
    if (nodeComparer(nodes[index], nodes[parentIndex]) <= 0)
    {
      // good! 
      // (note: this also implies child is valid (<=) all ancestor max-level nodes)      
      
      // does a grandparent exist?
      NateMinMaxHeap_Add_CheckMinLevelGrandParents:
      if (index > 2)
      {
        // is my relationship with grandparent valid?
        // grandparent is min-level node, so valid means (child >= grandparent)
        grandparentIndex = MyGetGrandparentIndex(index);
        if (nodeComparer(nodes[index], nodes[grandparentIndex]) >= 0)
        {
          // good! 
          // (note: this also implies child is valid (>=) all ancestor min-level nodes)
        }
        else
        {
          // swap with grandparent to make it better
          MySwap(nodes[index], nodes[grandparentIndex]);
          
          // (note: grandparents can always be moved to grandchild position without violating rules 1 or 2)
          // but grandchild swapped up needs to be checked against its new grandparents
          // (note: it already satisfies its new parents because it was <= the original parent node)
          index = grandparentIndex;
          goto NateMinMaxHeap_Add_CheckMinLevelGrandParents;
        }
      }
      // else, node is fine where it's at
    }
    else
    {
      // swap with parent to make it better
      MySwap(nodes[index], nodes[parentIndex]);
      
      // (note: parents can always be moved to child positions without violating rules 1 or 2, mostly because it's now a leaf node)
      // but child swapped up needs to be checked against its new grandparents
      index = parentIndex;
      goto NateMinMaxHeap_Add_CheckMaxLevelGrandParents;
    }
  }
  else
  {
    // is my relationship with parent valid?
    // parent is min-level node, so valid means (child >= parent)
    parentIndex = MyGetParentIndex(index);
    if (nodeComparer(nodes[index], nodes[parentIndex]) >= 0)
    {
      // good! 
      // (note: this also implies child is valid (>=) all ancestor min-level nodes)      
      
      // does a grandparent exist?
      NateMinMaxHeap_Add_CheckMaxLevelGrandParents:
      if (index > 2)
      {
        // is my relationship with grandparent valid?
        // grandparent is max-level node, so valid means (child <= grandparent)
        grandparentIndex = MyGetGrandparentIndex(index);
        if (nodeComparer(nodes[index], nodes[grandparentIndex]) <= 0)
        {
          // good! 
          // (note: this also implies child is valid (<=) all ancestor max-level nodes)
        }
        else
        {
          // swap with grandparent to make it better
          MySwap(nodes[index], nodes[grandparentIndex]);
          
          // (note: grandparents can always be moved to grandchild position without violating rules 1 or 2)
          // but grandchild swapped up needs to be checked against its new grandparents
          // (note: it already satisfies its new parents because it was >= the original parent node)
          index = grandparentIndex;
          goto NateMinMaxHeap_Add_CheckMaxLevelGrandParents;
        }
      }
      // else, node is fine where it's at
    }
    else
    {
      // swap with parent to make it better
      MySwap(nodes[index], nodes[parentIndex]);
      
      // (note: parents can always be moved to child positions without violating rules 1 or 2, mostly because it's now a leaf node)
      // but child swapped up needs to be checked against its new grandparents
      index = parentIndex;
      goto NateMinMaxHeap_Add_CheckMinLevelGrandParents;
    }
  }
}

size_t MyPickGreatestIndex(void ** nodes, NateMinMaxHeap_NodeComparer nodeComparer, size_t startIndex, size_t endIndex)
{
  size_t greatestIndex;
  size_t i;
  
  greatestIndex = startIndex;
  for (i = startIndex + 1; i < endIndex; i++)
  {
    if (nodeComparer(nodes[i], nodes[greatestIndex]) > 0)
    {
      greatestIndex = i;
    }
  }
  
  return greatestIndex;
}

size_t MyPickLeastIndex(void ** nodes, NateMinMaxHeap_NodeComparer nodeComparer, size_t startIndex, size_t endIndex)
{
  size_t leastIndex;
  size_t i;
  
  leastIndex = startIndex;
  for (i = startIndex + 1; i < endIndex; i++)
  {
    if (nodeComparer(nodes[i], nodes[leastIndex]) < 0)
    {
      leastIndex = i;
    }
  }
  
  return leastIndex;
}

void NateMinMaxHeap_RemoveAt(NateMinMaxHeap * obj, size_t index)
{
  size_t firstGrandchildIndex;
  size_t firstChildIndex;
  size_t bestIndex;
  size_t bestIndex2;
  size_t bestIndex3;
  size_t count;
  void * oldNode;
  void ** nodes;
  NateMinMaxHeap_NodeComparer * nodeComparer;
  int isMinLevel;
  
  nodes = obj->nodes;
  nodeComparer = obj->nodeComparer;
  count = obj->numNodesUsed;

  isMinLevel = MyIsMinLevel(index);
  
  // 1.) swap 'node to remove' down the tree by grandchildren or parents until it becomes leaf
  // (note: it's always possible to validly swap the best child/grandchild up the tree,
  //        it's just hard to efficiently pick the best one)
  NateMinMaxHeap_Remove_Step1:
  firstChildIndex = MyGetFirstChildIndex(index);
  // if not a leaf node yet
  if (firstChildIndex < count)
  {
    firstGrandchildIndex = MyGetfirstGrandchildIndex(index);
    if (isMinLevel)
    {
      // only need to compare leaf nodes
      // (non-leaf parents are guaranteed to be undesirable, or at worst "as equally desirable" as leaf nodes)
#define MyMinIndex(i1, i2) (nodeComparer(nodes[i1], nodes[i2]) <= 0) ? (i1) : (i2)
      if (firstGrandchildIndex + 3 < count)
      {
        // only grandchildren are leaf nodes
        bestIndex2 = MyMinIndex(firstGrandchildIndex + 3, firstGrandchildIndex + 2);
        bestIndex3 = MyMinIndex(firstGrandchildIndex + 1, firstGrandchildIndex);
        bestIndex = MyMinIndex(bestIndex2, bestIndex3);
      }
      else if (firstGrandchildIndex + 2 < count)
      {
        // only grandchildren are leaf nodes
        bestIndex2 = MyMinIndex(firstGrandchildIndex + 2, firstGrandchildIndex + 1);
        bestIndex = MyMinIndex(bestIndex2, firstGrandchildIndex);
      }
      else if (firstGrandchildIndex + 1 < count)
      {
        // right child is a leaf node
        bestIndex2 = MyMinIndex(firstGrandchildIndex + 1, firstGrandchildIndex);
        bestIndex = MyMinIndex(firstChildIndex + 1, bestIndex2);
      }
      else if (firstGrandchildIndex < count)
      {
        // right child is a leaf node
        bestIndex2 = MyMinIndex(firstGrandchildIndex, firstChildIndex + 1);
        bestIndex = MyMinIndex(bestIndex2, firstChildIndex);
      }
      else if (firstChildIndex + 1 < count)
      {
        // both children are leaf nodes
        bestIndex = MyMinIndex(firstChildIndex + 1, firstChildIndex);
      }
      else
      {
        // left child is the only one that exists
        bestIndex = firstChildIndex;
      }

    }
    else
    {
      // only need to compare leaf nodes
      // (non-leaf parents are guaranteed to be undesirable, or at worst "as equally desirable" as leaf nodes)
#define MyMaxIndex(i1, i2) (nodeComparer(nodes[i1], nodes[i2]) >= 0) ? (i1) : (i2)
      if (firstGrandchildIndex + 3 < count)
      {
        // only grandchildren are leaf nodes
        bestIndex2 = MyMaxIndex(firstGrandchildIndex + 3, firstGrandchildIndex + 2);
        bestIndex3 = MyMaxIndex(firstGrandchildIndex + 1, firstGrandchildIndex);
        bestIndex = MyMaxIndex(bestIndex2, bestIndex3);
      }
      else if (firstGrandchildIndex + 2 < count)
      {
        // only grandchildren are leaf nodes
        bestIndex2 = MyMaxIndex(firstGrandchildIndex + 2, firstGrandchildIndex + 1);
        bestIndex = MyMaxIndex(bestIndex2, firstGrandchildIndex);
      }
      else if (firstGrandchildIndex + 1 < count)
      {
        // right child is a leaf node
        bestIndex2 = MyMaxIndex(firstGrandchildIndex + 1, firstGrandchildIndex);
        bestIndex = MyMaxIndex(firstChildIndex + 1, bestIndex2);
      }
      else if (firstGrandchildIndex < count)
      {
        // right child is a leaf node
        bestIndex2 = MyMaxIndex(firstGrandchildIndex, firstChildIndex + 1);
        bestIndex = MyMaxIndex(bestIndex2, firstChildIndex);
      }
      else if (firstChildIndex + 1 < count)
      {
        // both children are leaf nodes
        bestIndex = MyMaxIndex(firstChildIndex + 1, firstChildIndex);
      }
      else
      {
        // left child is the only one that exists
        bestIndex = firstChildIndex;
      }
    }

    MySwap(nodes[bestIndex], nodes[index]);
    index = bestIndex;
    goto NateMinMaxHeap_Remove_Step1;
  }

  // 3.) swap 'node to remove' with 'last node in tree' 
  // (at this point 'node to remove' is effectively removed, can do count--)
  MySwap(nodes[index], nodes[obj->numNodesUsed - 1]);
  obj->numNodesUsed--;
  obj->nodes[obj->numNodesUsed] = 0; // not needed, but it assists debugging
  
  // 4.) swap 'node just moved into its place' up the tree as needed to make it right, 
  // just as if we were adding it new
  if (index < obj->numNodesUsed)
  {
    MyMoveLeafNodeToValidPosition(obj, index);
  }
}

void NateMinMaxHeap_RemoveMin(NateMinMaxHeap * obj)
{
  size_t index;
  NateMinMaxHeap_GetMin(obj, &index);
  NateMinMaxHeap_RemoveAt(obj, index);
}

void NateMinMaxHeap_RemoveMax(NateMinMaxHeap * obj)
{
  size_t index;
  NateMinMaxHeap_GetMax(obj, &index);
  NateMinMaxHeap_RemoveAt(obj, index);
}

void NateMinMaxHeap_Clear(NateMinMaxHeap * obj)
{
  obj->numNodesUsed = 0;
}

void NateMinMaxHeap_ClearAndShrink(NateMinMaxHeap * obj)
{
  obj->numNodesUsed = 0;
  NateMinMaxHeap_SetCapacity(obj, 0);
}

size_t NateMinMaxHeap_GetCapacity(NateMinMaxHeap * obj)
{
  return obj->numNodesAllocated;
}

size_t NateMinMaxHeap_SetCapacity(NateMinMaxHeap * obj, size_t newCapacity)
{
  size_t powerOf2;
  void ** newMemory;

  // determine the smallest capacity that will hold the given number of nodes
  if (newCapacity >= 0x1FFFFFFF)
  {
    newCapacity = 0x1FFFFFFF;
  }
  else if (newCapacity > 7)
  {
    powerOf2 = MyGetNearestMaybeLowerPowerOf2(newCapacity);

    // trees always hold 2^n - 1 nodes, so subtract 1 when figuring whether it's good
    if ((powerOf2 - 1) < newCapacity)
    {
      powerOf2 = powerOf2 << 1; // times 2
      //assert((powerOf2 - 1) >= newCapacity); // this should be mathematically guaranteed
    }

    newCapacity = powerOf2 - 1;
    
    // TODO: is this check really necessary? not sure. Better safe than sorry
    if (newCapacity >= 0x1FFFFFFF)
    {
      newCapacity = 0x1FFFFFFF;
    }
  }

  // originally this was to avoid mallocing 0 bytes
  // and to avoid thinking about (powerOf2 == 0)
  // Nice to have either way.
  if (newCapacity < 7)
  {
    newCapacity = 7;
  }

  if (newCapacity == obj->numNodesAllocated)
  {
    return newCapacity;
  }

  // try to allocate memory
  // TODO: look into realloc here
  newMemory = malloc(sizeof(void*) * newCapacity);
  if (newMemory == 0)
  {
    return obj->numNodesAllocated;
  }
  else
  {
    // TODO be more efficient here
    memset(newMemory, 0, sizeof(void*) * newCapacity);
    memcpy(newMemory, obj->nodes, sizeof(void*) * obj->numNodesAllocated);
    free(obj->nodes);
    obj->nodes = newMemory;
    obj->numNodesAllocated = newCapacity;
    return newCapacity;
  }
}

size_t NateMinMaxHeap_GetCount(NateMinMaxHeap * obj)
{
  return obj->numNodesUsed;
}

void * NateMinMaxHeap_Get(NateMinMaxHeap * obj, size_t index)
{
  //assert(index >= 0 && index < obj->numNodesUsed);
  return obj->nodes[index];
}

// returns null ptr and InvalidIndex if empty
void * NateMinMaxHeap_GetMin(NateMinMaxHeap * obj, size_t * index)// index may be NULL if not needed
{
  switch (obj->numNodesUsed)
  {
  case 0:
    //assert(obj->numNodesUsed != 0);
    if (index != 0) *index = NateMinMaxHeap_InvalidIndex;
    return 0;

  default:
    if (index != 0) *index = 0;
    return obj->nodes[0];
  }
}

// returns null ptr and InvalidIndex if empty
void * NateMinMaxHeap_GetMax(NateMinMaxHeap * obj, size_t * index)// index may be NULL if not needed
{
  switch (obj->numNodesUsed)
  {
  case 0:
    //assert(obj->numNodesUsed != 0);
    if (index != 0) *index = NateMinMaxHeap_InvalidIndex;
    return 0;
    
  // root node is always least (but if it's alone, it's greatest)
  case 1:
    if (index != 0) *index = 0;
    return obj->nodes[0];
    
  // root node's children are always greatest
  case 2:
    if (index != 0) *index = 1;
    return obj->nodes[1];
    
  default:
    if (obj->nodeComparer(obj->nodes[1], obj->nodes[2]) >= 0)
    {
      if (index != 0) *index = 1;
      return obj->nodes[1];
    }
    else
    {
      if (index != 0) *index = 2;
      return obj->nodes[2];
    }
  }
}