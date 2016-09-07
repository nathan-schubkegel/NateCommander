/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MainAppLua.h"

#include "FatalErrorHandler.h"
#include "LuaExports.h"
#include "ResourceLoader.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "MainAppLuaInputs.h"

int MyLuaPanic(lua_State * luaState);
void * MyLuaAlloc(void *ud, void *ptr, size_t osize, size_t nsize);
void MyLoadAndRunLuaFile(lua_State * luaState, const char * luaFileName);

char * MyWhitelistedGlobals[] = 
{
  "assert",
  "error",
  "_G",
  //"getmetatable",
  "ipairs",
  "next",
  "pairs",
  "pcall",
  "rawequal",
  "rawget",
  "rawlen",
  "rawset",
  "select",
  //"setmetatable",
  "tonumber",
  "tostring",
  "type",
  "_VERSION",
  "xpcall",
  "string",
  "table",
  "math",
  "bit32",
};

#define MY_WHITELISTED_GLOBALS_LEN sizeof(MyWhitelistedGlobals) / sizeof(char*)

void MyLoadStandardWhitelistedLibraries(MainAppHostStruct * hostStruct)
{
  lua_State * luaState;
  int stackTopBeforeWhitelisting;
  int stackTopAtStartOfLoop;
  int globalTableIndex;
  int keyIndex;
  int valueIndex;
  int keep;
  const char * key;
  int whitelistIndex;
  
  luaState = hostStruct->luaState;

#define MY_LOAD_STANDARD_LIB(name, openFunction) \
  luaL_requiref(luaState, (name), (openFunction), 1); /* 1 means "make it global" */ \
  lua_pop(luaState, 1); /*remove lib*/

  // Load standard methods for LUA libraries to call
  MY_LOAD_STANDARD_LIB("_G", luaopen_base);
  //MY_LOAD_STANDARD_LIB(LUA_LOADLIBNAME, luaopen_package);
  //MY_LOAD_STANDARD_LIB(LUA_COLIBNAME, luaopen_coroutine);
  MY_LOAD_STANDARD_LIB(LUA_TABLIBNAME, luaopen_table);
  //MY_LOAD_STANDARD_LIB(LUA_IOLIBNAME, luaopen_io);
  //MY_LOAD_STANDARD_LIB(LUA_OSLIBNAME, luaopen_os);
  MY_LOAD_STANDARD_LIB(LUA_STRLIBNAME, luaopen_string);
  MY_LOAD_STANDARD_LIB(LUA_BITLIBNAME, luaopen_bit32);
  MY_LOAD_STANDARD_LIB(LUA_MATHLIBNAME, luaopen_math);
  //MY_LOAD_STANDARD_LIB(LUA_DBLIBNAME, luaopen_debug);

  // enumerate lua global table _G[]
  stackTopBeforeWhitelisting = lua_gettop(luaState);
  lua_pushglobaltable(luaState);
  globalTableIndex = lua_gettop(luaState);
  lua_pushnil(luaState); // nil makes lua_next() get the first item in table
  while (lua_next(luaState, -2) != 0)
  {
    stackTopAtStartOfLoop = lua_gettop(luaState);

    // key is at index -2
    // value is at index -1
    // copy them, and only look at the copies, to guarantee we don't change them
    // because lua_next() can get goofed by lua_tostring() because it changes
    // the value on the stack to a string
    lua_pushnil(luaState);
    lua_pushnil(luaState);
    lua_copy(luaState, -4, -2);
    lua_copy(luaState, -3, -1);
    keyIndex = lua_gettop(luaState) - 1;
    valueIndex = keyIndex + 1;

    // only retain whitelisted LUA libraries
    keep = 0;
    if (lua_isstring(luaState, keyIndex))
    {
      key = lua_tostring(luaState, keyIndex);
      
      for (whitelistIndex = 0; whitelistIndex < MY_WHITELISTED_GLOBALS_LEN; whitelistIndex++)
      {
        if (strcmp(key, MyWhitelistedGlobals[whitelistIndex]) == 0)
        {
          keep = 1;
          break;
        }
      }
    }

    if (!keep)
    {
      // remove key from global table
      lua_pushnil(luaState);
      lua_copy(luaState, keyIndex, -1);
      lua_pushnil(luaState);
      lua_settable(luaState, globalTableIndex);
    }
    
    // remove 'value', keep 'key' and use it for next iteration
    NateCheck(lua_gettop(luaState) >= stackTopAtStartOfLoop, "this logic fails if code popped a bunch of stuff");
    lua_pop(luaState, lua_gettop(luaState) - stackTopAtStartOfLoop + 1);
  }

  // remove lua initialization stuff
  NateCheck(lua_gettop(luaState) >= stackTopBeforeWhitelisting, "this logic fails if code popped a bunch of stuff");
  lua_pop(luaState, lua_gettop(luaState) - stackTopBeforeWhitelisting);
}

void MainAppLua_InitLua(MainAppHostStruct * hostStruct)
{
  lua_State * luaState;
  
  // Initialize a LUA state (starting point for everything related to a single LUA instance)
  luaState = lua_newstate(MyLuaAlloc, 0);
  NateCheck(luaState != 0, "Failed to create LUA state");
  hostStruct->luaState = luaState;

  // When lua borks, display an error messagebox and quit
  lua_atpanic(luaState, MyLuaPanic);

  MyLoadStandardWhitelistedLibraries(hostStruct);

  // Initialize host/client table/reference stuff
  MainAppHostStruct_InitializeLuaState(hostStruct, luaState);

  // Initialize lua input event handler stuff
  MainAppLuaInputs_Initialize(hostStruct);

  // Load the MainApp lua file
  // (this halts the application on failure)
  MyLoadAndRunLuaFile(luaState, "MainApp.lua");

  // finally add the exported C methods
  // (this timing is clever - MainApp.lua didn't get to use the C methods when it was first loaded+executed)
  LuaExports_PublishCMethods(luaState);
}

void MyLoadAndRunLuaFile(lua_State * luaState, const char * luaFileName)
{
  void* luaFileData;
  long luaFileDataLength;
  int loadResult;

  NateCheck0(lua_checkstack(luaState, 1));

  // get the file data
  luaFileData = ResourceLoader_LoadLuaFile(luaFileName, &luaFileDataLength);
  NateCheck(luaFileData != 0, "Failed to load LUA file contents");
  
  // let Lua parse/consume it
  // FUTURE: This could still be dorked up by unicode or whatever. Oh well. They'll learn.
  BuildAssertSize(sizeof(char), 1); // Char is supposed to be 1 byte for this to work
  loadResult = luaL_loadbufferx(luaState, (char*)luaFileData, luaFileDataLength, luaFileName, "t");
  free(luaFileData);
  if (LUA_OK != loadResult)
  {
    FatalError3(luaFileName, ": failed to load/parse/first-time-interpret LUA chunk: ", lua_tolstring(luaState, -1, 0));
  }

  // run it
  if (LUA_OK != lua_pcall(luaState, 0, 0, 0))
  {
    FatalError3(luaFileName, ": failed to execute LUA chunk: ", lua_tolstring(luaState, -1, 0));
  }
}

/*
The lua allocator function must provide a functionality similar to realloc, 
but not exactly the same. Its arguments are 
  ud, an opaque pointer passed to lua_newstate; 
  ptr, a pointer to the block being allocated/reallocated/freed; 
  osize, the original size of the block or some code about what is being allocated; 
  nsize, the new size of the block.

When ptr is not NULL, osize is the size of the block pointed by ptr, that is, 
the size given when it was allocated or reallocated.
When ptr is NULL, osize encodes the kind of object that Lua is allocating. 
osize is any of LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, or 
LUA_TTHREAD when (and only when) Lua is creating a new object of that type. 
When osize is some other value, Lua is allocating memory for something else.

Lua assumes the following behavior from the allocator function:
  When nsize is zero, the allocator should behave like free and return NULL.
  When nsize is not zero, the allocator should behave like realloc. 
  The allocator returns NULL if and only if it cannot fulfill the request. 
  Lua assumes that the allocator never fails when osize >= nsize.

Note that Standard C ensures that free(NULL) has no effect and 
that realloc(NULL, size) is equivalent to malloc(size). 
This code assumes that realloc does not fail when shrinking a block. 
(Although Standard C does not ensure this behavior, it seems to be a 
safe assumption.)
*/
void* MyLuaAlloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
  // eliminate compiler warning
  (void)ud;
  (void)osize;

  if (nsize == 0) 
  {
    free(ptr);
    return NULL;
  }
  else
  {
    return realloc(ptr, nsize);
  }
}

int MyLuaPanic(lua_State * luaState)
{
  // When lua borks, display an error messagebox and quit
  FatalError2("Lua panicked: ", lua_tostring(luaState, -1));
  return 0;
}

void MainAppLua_CallInitialize(MainAppHostStruct * hostStruct)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);

  NateCheck0(lua_checkstack(luaState, 2));

  // get the Lua 'Initialize' method
  lua_getglobal(luaState, "Initialize");
  if (lua_isfunction(luaState, -1))
  {
    // first argument is the ClientTable
    MainAppHostStruct_LuaPushClientTable(hostStruct);
    if (LUA_OK != lua_pcall(luaState, 1, 0, 0))
    {
      FatalError2("Failure while running lua Initialize() method: ", lua_tostring(luaState, -1));
    }
  }

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLua_CallProcess(MainAppHostStruct * hostStruct)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);

  NateCheck0(lua_checkstack(luaState, 2));

  // get the Lua 'Process' method
  lua_getglobal(luaState, "Process");
  if (lua_isfunction(luaState, -1))
  {
    // first argument is the ClientTable
    MainAppHostStruct_LuaPushClientTable(hostStruct);
    if (LUA_OK != lua_pcall(luaState, 1, 0, 0))
    {
      FatalError2("Failure while running lua Process() method: ", lua_tostring(luaState, -1));
    }
  }

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLua_CallDraw(MainAppHostStruct * hostStruct, 
                         int windowWidth,
                         int windowHeight)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);

  NateCheck0(lua_checkstack(luaState, 5));

  // Get the 'Draw' method
  lua_getglobal(luaState, "Draw");
  if (lua_isfunction(luaState, -1))
  {
    // first argument is the ClientTable
    MainAppHostStruct_LuaPushClientTable(hostStruct);

    // second argument is a table
    lua_createtable(luaState, 0, 2);

    // the table has some properties
    lua_pushstring(luaState, "WindowWidth");
    lua_pushnumber(luaState, windowWidth);
    lua_settable(luaState, -3);

    lua_pushstring(luaState, "WindowHeight");
    lua_pushnumber(luaState, windowHeight);
    lua_settable(luaState, -3);

    // call the lua function
    if (LUA_OK != lua_pcall(luaState, 2, 0, 0))
    {
      FatalError2("Failure while running lua Draw() method: ", lua_tostring(luaState, -1));
    }
  }

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLua_LoadAndRunLuaFile(lua_State * luaState, const char * luaFileName)
{
  MainAppHostStruct * hostStruct;
  hostStruct = MainAppHostStruct_GetHostStruct(luaState);
  MyLoadAndRunLuaFile(hostStruct->luaState, luaFileName);
}