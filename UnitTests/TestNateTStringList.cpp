#include <queue>
#include <vector>
#include <functional>
#include <string>
#include <assert.h>

extern "C"
{
  #include "TestUtils.h"
  #include "ccan/NateTStringList/NateTStringList.h"
}

class MyThing
{
public:
  std::string string;
  size_t stringLength;
  char * stringPtrOriginal;
  int stringCopied;
  void * object;
  size_t objectLength;
  int objectCopied;
};

static void Test_ListsAreSame(NateTStringList * obj, std::vector<MyThing> &v)
{
  size_t stringLength;
  size_t objectLength;
  char * string;
  void * object;

  CHECK(NateTStringList_GetCount(obj) == v.size(), );
  for (size_t index = 0; index < NateTStringList_GetCount(obj); index++)
  {
    string = NateTStringList_GetString(obj, index, &stringLength);
    CHECK((string == 0 && v[index].string == "") || (string == v[index].string), );
    CHECK(stringLength == v[index].stringLength, );
    if (!v[index].stringCopied)
    {
      CHECK(string == v[index].stringPtrOriginal, );
    }

    object = NateTStringList_GetObject(obj, index, &objectLength);
    if (!v[index].objectCopied)
    {
      CHECK(object == v[index].object, );
    }
    else
    {
      if (v[index].objectLength > 0)
      {
        CHECK(memcmp(v[index].object, object, v[index].objectLength) == 0, );
      }
      else
      {
        // can't really verify anything from object ptr
      }
    }
    CHECK(objectLength == v[index].objectLength, );

    CHECK(NateTStringList_GetCount(obj) == v.size(), );
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
  "durpa durpa"
};

static void Test_MuchRandom_ForTargetSize(size_t targetSize, size_t numOps, size_t fullTestEveryNumOps)
{
  size_t i;
  size_t count;
  size_t iUntilNextFullTest;
  NateTStringList * obj;
  std::vector<MyThing> v;
  MyThing myThing;
  char * object;

  obj = NateTStringList_Create();
  iUntilNextFullTest = fullTestEveryNumOps;

  // perform random 'set' operations
  // and verify it holds correct contents
  // TODO: verify no neighboring memory is bogus'd
  for (i = 0; i < numOps; i++)
  {
    iUntilNextFullTest--;
    count = NateTStringList_GetCount(obj);
    CHECK(v.size() == count, );

    if (count < targetSize)
    {
      // add a new empty entry
      CHECK(NateTStringList_Add(obj), );
      count = NateTStringList_GetCount(obj);

      myThing.object = 0;
      myThing.objectLength = 0;
      myThing.objectCopied = 0;
      myThing.string = "";
      myThing.stringLength = 0;
      myThing.stringCopied = 0;
      myThing.stringPtrOriginal = 0;
      v.push_back(myThing);
      CHECK(v.size() == count, );

      if (iUntilNextFullTest == 0) 
      {
        Test_ListsAreSame(obj, v);
      }
    }

    // perform a random 'set' action on the tree
#define ACTION_SetStringMemcpy1 0
#define ACTION_SetStringMemcpy2 1
#define ACTION_SetObjectMemcpy 2
#define ACTION_SetStringPtr 3
#define ACTION_SetObjectPtr 4
#define ACTION_Max 5

    // action = random value
    int action = rand() % ACTION_Max;
    switch (action)
    {
    case ACTION_SetStringMemcpy1:
      // action = index
      action = rand() % count;
      object = MahStrings[rand() % 10];
      NateTStringList_SetString_Memcpy(obj, action, object);
      v[action].string = object;
      v[action].stringLength = strlen(object);
      v[action].stringCopied = 1;
      v[action].stringPtrOriginal = object;
      break;

    case ACTION_SetStringMemcpy2:
      // action = index
      action = rand() % count;
      object = MahStrings[rand() % 10];
      NateTStringList_SetString_Memcpy2(obj, action, object, 5);
      v[action].string = object;
      v[action].string.erase(5);
      assert(v[action].string.size() == 5);
      v[action].stringLength = 5;
      v[action].stringCopied = 1;
      v[action].stringPtrOriginal = object;
      break;
      
    case ACTION_SetObjectMemcpy:
      // action = index
      action = rand() % count;
      object = MahStrings[rand() % 10];
      NateTStringList_SetObject_Memcpy(obj, action, object, strlen(object) + 1);
      v[action].object = object;
      v[action].objectLength = strlen(object) + 1;
      v[action].objectCopied = 1;
      break;
      
    case ACTION_SetStringPtr:
      // action = index
      action = rand() % count;
      object = MahStrings[rand() % 10];
      NateTStringList_SetString_Ptr(obj, action, object);
      v[action].string = object;
      v[action].stringLength = 0;
      v[action].stringCopied = 0;
      v[action].stringPtrOriginal = object;
      break;
      
    case ACTION_SetObjectPtr:
      // action = index
      action = rand() % count;
      object = MahStrings[rand() % 10];
      NateTStringList_SetObject_Ptr(obj, action, object);
      v[action].object = object;
      v[action].objectLength = 0;
      v[action].objectCopied = 0;
      break;

    default:
      CHECK(0, "bad test needs revisiting");
    }
    
    if (iUntilNextFullTest == 0) 
    {
      Test_ListsAreSame(obj, v);
    }

    if (iUntilNextFullTest == 0) 
    {
      iUntilNextFullTest = fullTestEveryNumOps;
    }
  }

  NateTStringList_Destroy(obj);
}

static void Test_MuchRandom()
{
  Test_MuchRandom_ForTargetSize(7, 5000, 1);
  Test_MuchRandom_ForTargetSize(8, 5000, 1);
  Test_MuchRandom_ForTargetSize(9, 5000, 1);
  Test_MuchRandom_ForTargetSize(15, 5000, 1);
  Test_MuchRandom_ForTargetSize(33, 5000, 1);
  Test_MuchRandom_ForTargetSize(150, 5000, 1);
}

static void Test_OtherMethods()
{
  // verify that each method correctly responds for 
  // NateTStringList_LastIndex and NateTStringList_AddToEnd
  NateTStringList * obj;
  char * object;
  obj = NateTStringList_Create();

  NateTStringList_SetString_Memcpy(obj, NateTStringList_AddToEnd, "steve");
  CHECK(NateTStringList_GetCount(obj) == 1, );
  CHECK(0 == strcmp("steve", NateTStringList_GetString(obj, 0, 0)), );
  CHECK(0 == NateTStringList_GetObject(obj, 0, 0), );

  NateTStringList_SetString_Memcpy2(obj, NateTStringList_AddToEnd, "haldo!", 5);
  CHECK(NateTStringList_GetCount(obj) == 2, );
  CHECK(0 == strcmp("haldo", NateTStringList_GetString(obj, 1, 0)), );
  CHECK(0 == NateTStringList_GetObject(obj, 1, 0), );

  object = "awbject";
  NateTStringList_SetObject_Memcpy(obj, NateTStringList_AddToEnd, object, 4);
  CHECK(NateTStringList_GetCount(obj) == 3, );
  CHECK(0 == NateTStringList_GetString(obj, 2, 0), );
  CHECK(0 == strncmp("awbj", (char*)NateTStringList_GetObject(obj, 2, 0), 4), );

  object = "durp";
  NateTStringList_SetString_Ptr(obj, NateTStringList_AddToEnd, object);
  CHECK(NateTStringList_GetCount(obj) == 4, );
  CHECK(object == NateTStringList_GetString(obj, 3, 0), );
  CHECK(0 == NateTStringList_GetObject(obj, 3, 0), );

  object = "durka durka";
  NateTStringList_SetObject_Ptr(obj, NateTStringList_AddToEnd, object);
  CHECK(NateTStringList_GetCount(obj) == 5, );
  CHECK(0 == NateTStringList_GetString(obj, 4, 0), );
  CHECK(object == NateTStringList_GetObject(obj, 4, 0), );

  // TODO: finish testing this, I'm tired of writing these

  NateTStringList_Destroy(obj);
}

void Test_NateTStringList()
{
  Test_MuchRandom();
  Test_OtherMethods();
}