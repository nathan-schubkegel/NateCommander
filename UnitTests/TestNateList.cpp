#include <queue>
#include <vector>
#include <functional>
#include <string>
#include <assert.h>

extern "C"
{
  #include "TestUtils.h"
  #include "ccan/NateList/NateList.h"
}

static void Test_ListPtrsAreSame(NateList * obj, std::vector<char*> &v)
{
  CHECK(NateList_GetCount(obj) == v.size(), );
  for (size_t index = 0; index < NateList_GetCount(obj); index++)
  {
    // check string
    char * string = (char*)NateList_GetPtr(obj, index);
    CHECK(string == v[index],);
  }
}

static char * MahStrings[] = 
{
  // all must be at least 5 characters long
  "bla ha ha",
  "you lose",
  "what a beautiful day",
  "ok then",
  "fjwoafje",
  "fee fie fo fum",
  "diefools",
  "fowlmouth",
  "okletsgo",
  "durpa durpa",
  ""
};

static int MahStringsCount = 11;

static void Test_ManyRandomPtrs_ForTargetSize(size_t targetSize, size_t numOps, size_t fullTestEveryNumOps)
{
  size_t i;
  size_t count;
  size_t iUntilNextFullTest;
  NateList * obj;
  std::vector<char*> v;
  char * object;
  size_t index;

  obj = NateList_Create();
  iUntilNextFullTest = fullTestEveryNumOps;

  // perform random 'set' operations
  // and verify it holds correct contents
  // TODO: verify no neighboring memory is bogus'd
  for (i = 0; i < numOps; i++)
  {
    iUntilNextFullTest--;
    if (rand() % 6 == 0) object = 0; // add a null ptr every once in a while
    else object = MahStrings[rand() % MahStringsCount];
    count = NateList_GetCount(obj);
    CHECK(v.size() == count, );

    if (count < targetSize)
    {
      // add a new entry
      CHECK(NateList_AddPtr(obj, object), );
      v.push_back(object);
      CHECK(v.size() == count + 1, );
      CHECK(v[count] == NateList_GetPtr(obj, count), );
      count++;
    }

    // set a random entry to some random value
    index = rand() % count;
    if (rand() % 6 == 0) object = 0; // set a null ptr every once in a while
    else object = MahStrings[rand() % MahStringsCount];
    NateList_SetPtr(obj, index, object);
    v[index] = object;
    
    if (iUntilNextFullTest == 0)
    {
      Test_ListPtrsAreSame(obj, v);
      iUntilNextFullTest = fullTestEveryNumOps;
    }
  }

  NateList_Destroy(obj);
}

static void Test_MuchRandom()
{
  Test_ManyRandomPtrs_ForTargetSize(7, 500, 1);
  Test_ManyRandomPtrs_ForTargetSize(8, 500, 1);
  Test_ManyRandomPtrs_ForTargetSize(9, 500, 1);
  Test_ManyRandomPtrs_ForTargetSize(15, 500, 2);
  Test_ManyRandomPtrs_ForTargetSize(33, 500, 2);
  Test_ManyRandomPtrs_ForTargetSize(150, 500, 5);


}

static void Test_OtherMethods()
{
  NateList * obj;
  char * object;
  char * object2;
  char * object3;
  char * object4;
  void * internalData;
  obj = NateList_Create();

  // populate the list
  object = MahStrings[rand() % MahStringsCount];
  object2 = object;
  object3 = "whatever man";
  object4 = object3;
  NateList_AddPtr(obj, object);
  NateList_AddZeroedData(obj);
  NateList_AddCopyOfData(obj, &object3);
  CHECK(object3 == object4, );
  CHECK(NateList_GetPtr(obj, 0) == object, );
  CHECK(NateList_GetPtr(obj, 1) == 0, );
  CHECK(NateList_GetPtr(obj, 2) == object3, );

  // verify that each method correctly responds for NateList_LastIndex
  CHECK(NateList_GetPtr(obj, NateList_LastIndex) == object3, );
  internalData = NateList_GetData(obj, NateList_LastIndex);
  CHECK(internalData != (void*)object3, );
  CHECK(*(char**)internalData == object3, );

  object = "durpity doo";
  NateList_SetPtr(obj, NateList_LastIndex, object);
  CHECK(NateList_GetPtr(obj, 2) == object,);

  object = "hoo haa";
  object2 = object;
  NateList_SetCopyOfData(obj, NateList_LastIndex, &object);
  CHECK(object == object2, );
  CHECK(NateList_GetPtr(obj, 2) == object,);

  NateList_SetZeroedData(obj, NateList_LastIndex);
  CHECK(NateList_GetPtr(obj, 2) == 0, );

  NateList_Destroy(obj);
}

/*
// finds the first item that causes the predicate to return true
// returns 1 if found and assigns 'index' = the index
// returns 0 if not found and assigns 'index' = 0xFFFFFFFF
// index may be null if not needed
typedef int (NateList_PredicatePtr)(void * userData, void * item); // passes stored value casted as pointer to predicate
typedef int (NateList_PredicateData)(void * userData, void * item); // passes pointer to stored value to predicate
int NateList_FindPtr(NateList * obj, NateList_PredicatePtr * predicate, void * userData, size_t * index); 
int NateList_FindData(NateList * obj, NateList_PredicateData * predicate, void * userData, size_t * index); 
*/

void Test_NateList()
{
  Test_MuchRandom();
  Test_OtherMethods();
}