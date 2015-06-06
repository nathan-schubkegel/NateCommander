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
  size_t i;
  char * string;
  void * object;

  CHECK(NateTStringList_GetCount(obj) == v.size(), );
  for (size_t index = 0; index < NateTStringList_GetCount(obj); index++)
  {
    // check string
    stringLength = 0xFFFFFFFF;
    string = NateTStringList_GetString(obj, index, &stringLength);
    if (v[index].stringPtrOriginal == 0)
    {
      CHECK(string == 0,);
      CHECK(stringLength == 0,);
    }
    else if (!v[index].stringCopied)
    {
      CHECK(string == v[index].stringPtrOriginal,);
      CHECK(stringLength == 0,);
    }
    else
    {
      CHECK(strncmp(string, v[index].string.c_str(), v[index].string.size()) == 0, );
      CHECK(stringLength == v[index].stringLength);
    }

    // check object
    objectLength = 0xFFFFFFFF;
    object = NateTStringList_GetObject(obj, index, &objectLength);
    if (v[index].object == 0)
    {
      CHECK(object == 0, );
      CHECK(objectLength == 0, );
    }
    else if (!v[index].objectCopied)
    {
      CHECK(object == v[index].object);
      CHECK(objectLength == 0);
    }
    else
    {
      CHECK(object != v[index].object);
      CHECK(objectLength == v[index].objectLength);
      for (i = 0; i < objectLength; i++)
      {
        CHECK(((char*)object)[i] == ((char*)(v[index].object))[i], );
      }
    }
    else
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

static int MahStringsCount = 10;

static void Test_MuchRandom_ForTargetSize(size_t targetSize, size_t numOps, size_t fullTestEveryNumOps)
{
  size_t i;
  size_t count;
  size_t iUntilNextFullTest;
  NateTStringList * obj;
  std::vector<MyThing> v;
  MyThing myThing;
  char * object;
  size_t index;
  size_t partialLength;

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
#define ACTION_SetStringStrcpy 0
#define ACTION_SetObjectStrcpy 1
#define ACTION_SetStringMemcpy 2
#define ACTION_SetObjectMemcpy 3
#define ACTION_SetStringMallocInit 4
#define ACTION_SetObjectMallocInit 5
#define ACTION_SetStringPtr 6
#define ACTION_SetObjectPtr 7
#define ACTION_Modulo 8

    // action = random value
    int action = rand() % ACTION_Modulo;
    switch (action)
    {
    case ACTION_SetStringStrcpy:
      index = rand() % count;
      object = MahStrings[rand() % MahStringsCount];
      NateTStringList_SetString_Strcpy(obj, index, object);
      v[index].string = object;
      v[index].stringLength = strlen(object);
      v[index].stringCopied = 1;
      v[index].stringPtrOriginal = object;
      break;

    case ACTION_SetObjectStrcpy:
      index = rand() % count;
      object = MahStrings[rand() % MahStringsCount];
      NateTStringList_SetObject_Strcpy(obj, index, object);
      v[index].object = object;
      v[index].objectCopied = 1;
      v[index].objectLength = strlen(object);
      break;
      
    case ACTION_SetStringMemcpy:
      index = rand() % count;
      object = MahStrings[rand() % MahStringsCount];
      if (rand() % 2 == 0)
      {
        // copy the whole string
        NateTStringList_SetString_Memcpy(obj, index, object, strlen(object));
        v[index].string = object;
        v[index].stringLength = strlen(object);
        v[index].stringCopied = 1;
        v[index].stringPtrOriginal = object;
      }
      else
      {
        // copy only part of the string
        partialLength = rand() % strlen(object);
        NateTStringList_SetString_Memcpy(obj, index, object, partialLength);
        v[index].string = object;
        v[index].string.erase(partialLength);
        assert(v[index].string.size() == partialLength);
        v[index].stringLength = partialLength;
        v[index].stringCopied = 1;
        v[index].stringPtrOriginal = object;
      }
      break;

    case ACTION_SetObjectMemcpy:
      index = rand() % count;
      object = MahStrings[rand() % MahStringsCount];
      if (rand() % 2 == 0)
      {
        // copy the whole string
        NateTStringList_SetObject_Memcpy(obj, index, object, strlen(object));
        v[index].object = object;
        v[index].objectCopied = 1;
        v[index].objectLength = strlen(object);
      }
      else
      {
        // copy only part of the string
        partialLength = rand() % strlen(object);
        NateTStringList_SetObject_Memcpy(obj, index, object, partialLength);
        v[index].object = object;
        v[index].objectCopied = 1;
        v[index].objectLength = partialLength;
      }
      break;
      
    case ACTION_SetStringMallocInit:
      index = rand() % count;
      partialLength = rand() % 10;
      NateTStringList_SetString_MallocInit(obj, index, partialLength);
      v[index].string = "\0\0\0\0\0\0\0\0\0\0";
      v[index].stringCopied = 1;
      v[index].stringLength = 0;
      v[index].stringPtrOriginal = "";
      break;

    case ACTION_SetObjectMallocInit:
      index = rand() % count;
      partialLength = rand() % 10;
      NateTStringList_SetObject_MallocInit(obj, index, partialLength);
      v[index].object = "\0\0\0\0\0\0\0\0\0\0";
      v[index].objectCopied = 1;
      v[index].objectLength = partialLength;
      break;

    case ACTION_SetStringPtr:
      index = rand() % count;
      if (rand() % 5 == 0) object = 0;
      else object = MahStrings[rand() % MahStringsCount];
      NateTStringList_SetString_Ptr(obj, index, object);
      v[index].string = object;
      v[index].stringCopied = 0;
      v[index].stringLength = 0;
      v[index].stringPtrOriginal = object;
      break;

    case ACTION_SetObjectPtr:
      index = rand() % count;
      if (rand() % 5 == 0) object = 0;
      else object = MahStrings[rand() % MahStringsCount];
      NateTStringList_SetObject_Ptr(obj, index, object);
      v[index].object = object;
      v[index].objectLength = 0;
      v[index].objectCopied = 0;
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

  NateTStringList_SetString_Strcpy(obj, NateTStringList_AddToEnd, "steve");
  CHECK(NateTStringList_GetCount(obj) == 1, );
  CHECK(0 == strcmp("steve", NateTStringList_GetString(obj, 0, 0)), );
  CHECK(0 == NateTStringList_GetObject(obj, 0, 0), );

  NateTStringList_SetString_Memcpy(obj, NateTStringList_AddToEnd, "haldo!", 5);
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