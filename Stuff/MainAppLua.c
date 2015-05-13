#include "MainAppLua.h"

#include "FatalErrorHandler.h"
#include "LuaExports.h"
#include "ResourceLoader.h"
#include "lua.h"
#include "lauxlib.h"

int MyLuaPanic(lua_State * luaState);
void * MyLuaAlloc(void *ud, void *ptr, size_t osize, size_t nsize);
void MyInitializeHostClientStuff(lua_State * luaState, MainAppHostStruct * hostStruct);
void MyInitializeLuaEventHandlerStuff(MainAppHostStruct * hostStruct);
void MyLoadAndRunLuaFile(lua_State * luaState, const char * luaFileName);

void MainAppLua_InitLua(MainAppHostStruct * hostStruct)
{
  lua_State * luaState;
  
  // Initialize a LUA state (starting point for everything related to a single LUA instance)
  luaState = lua_newstate(MyLuaAlloc, 0);
  NateCheck(luaState != 0, "Failed to create LUA state");
  hostStruct->luaState = luaState;

  // When lua borks, display an error messagebox and quit
  lua_atpanic(luaState, MyLuaPanic);

  // Initialize host/client table/reference stuff
  MyInitializeHostClientStuff(luaState, hostStruct);

  // Initialize lua event handler stuff
  MyInitializeLuaEventHandlerStuff(hostStruct);

  // Load the MainApp lua file
  // (this halts the application on failure)
  MyLoadAndRunLuaFile(luaState, "MainApp.lua");

  // finally add the exported C methods
  // (this timing is clever - MainApp.lua didn't get to use the C methods when it was first loaded+executed)
  LuaExports_PublishCMethods(luaState);
}

void MyInitializeHostClientStuff(lua_State * luaState, MainAppHostStruct * hostStruct)
{
  int hostStructIndex;
  int hostTableIndex;
  int clientTableIndex;
  int originalStackIndex = lua_gettop(luaState);

  // Ensure that at least 5 stack spaces are available
  NateCheck0(lua_checkstack(luaState, 5));

  // Initialize the "HostStruct" (a C struct exclusively for C host use)
  lua_pushlightuserdata(luaState, hostStruct);
  hostStructIndex = lua_gettop(luaState);

  // Initialize the "HostTable" (a LUA table exclusively for C host use)
  lua_newtable(luaState);
  hostTableIndex = lua_gettop(luaState);

  // Initialize the "ClientTable" (a LUA table exclusively for LUA script use)
  lua_newtable(luaState);
  clientTableIndex = lua_gettop(luaState);

  // Registry[HostStruct] = HostTable
  lua_pushvalue(luaState, hostStructIndex);
  lua_pushvalue(luaState, hostTableIndex);
  lua_settable(luaState, LUA_REGISTRYINDEX);

  // Registry[luaState] = HostStruct
  lua_pushlightuserdata(luaState, luaState);
  lua_pushvalue(luaState, hostStructIndex);
  lua_settable(luaState, LUA_REGISTRYINDEX);

  // HostTable["ClientTable"] = ClientTable
  lua_pushstring(luaState, "ClientTable");
  lua_pushvalue(luaState, clientTableIndex);
  lua_settable(luaState, hostTableIndex);

  // HostTable["HostStruct"] = HostStruct
  lua_pushstring(luaState, "HostStruct");
  lua_pushvalue(luaState, hostStructIndex);
  lua_settable(luaState, hostTableIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

MainAppHostStruct * MyGetHostStruct(lua_State * luaState)
{
  MainAppHostStruct * hostStruct;

  NateCheck0(lua_checkstack(luaState, 1));

  // local HostStruct = Registry[luaState]
  lua_pushlightuserdata(luaState, luaState);
  lua_gettable(luaState, LUA_REGISTRYINDEX);
  hostStruct = lua_touserdata(luaState, -1);
  lua_pop(luaState, 1);
  return hostStruct;
}

int MyGetHostTable(MainAppHostStruct * hostStruct)
{
  lua_State * luaState = hostStruct->luaState;

  NateCheck0(lua_checkstack(luaState, 1));

  // local HostTable = Registry[HostStruct]
  lua_pushlightuserdata(luaState, hostStruct);
  lua_gettable(luaState, LUA_REGISTRYINDEX);
  // NOTE: the net impact of this function is to push the HostTable onto the lua stack
  // (which has now been accomplished)
  return lua_gettop(luaState);
}

int MyGetHostTable2(lua_State * luaState)
{
  MainAppHostStruct * hostStruct = MyGetHostStruct(luaState);

  NateCheck0(lua_checkstack(luaState, 1));

  // local HostTable = Registry[HostStruct]
  lua_pushlightuserdata(luaState, hostStruct);
  lua_gettable(luaState, LUA_REGISTRYINDEX);
  // NOTE: the goal of this function was to push the HostTable onto the lua stack
  // (which has now been accomplished)
  return lua_gettop(luaState);
}

int MyGetClientTable(MainAppHostStruct * hostStruct, int hostTableIndex)
{
  lua_State * luaState = hostStruct->luaState;

  NateCheck0(lua_checkstack(luaState, 1));

  // get the ClientTable from HostTable["ClientTable"]
  lua_pushstring(luaState, "ClientTable");
  lua_gettable(luaState, hostTableIndex);
  // NOTE: the goal of this function was to push the HostTable onto the lua stack
  // (which has now been accomplished)
  return lua_gettop(luaState);
}

int MyGetClientTable2(MainAppHostStruct * hostStruct)
{
  lua_State * luaState = hostStruct->luaState;
  int hostTableIndex;

  NateCheck0(lua_checkstack(luaState, 2));

  // get the HostTable (pushed onto lua stack)
  hostTableIndex = MyGetHostTable(hostStruct);

  // get the ClientTable from HostTable["ClientTable"]
  lua_pushstring(luaState, "ClientTable");
  lua_gettable(luaState, hostTableIndex);
  // NOTE: the goal of this function was to push the HostTable onto the lua stack
  lua_replace(luaState, -2); // replace HostTable
  return lua_gettop(luaState);
}

void MyInitializeLuaEventHandlerStuff(MainAppHostStruct * hostStruct)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);

  NateCheck0(lua_checkstack(luaState, 3));

  MyGetHostTable(hostStruct);
  
  lua_pushstring(luaState, "MouseMotionNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "MouseMotionHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyUpNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyUpHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyDownNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyDownHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyResetNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyResetHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
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
    MyGetClientTable2(hostStruct);
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
    MyGetClientTable2(hostStruct);
    if (LUA_OK != lua_pcall(luaState, 1, 0, 0))
    {
      FatalError2("Failure while running lua Process() method: ", lua_tostring(luaState, -1));
    }
  }

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLua_CallDraw(MainAppHostStruct * hostStruct, 
                         lua_Number * spinnyCubeAngle, 
                         lua_Number * floorZOffset, 
                         lua_Number * viewAngleX, 
                         lua_Number * viewAngleY)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);

  NateCheck0(lua_checkstack(luaState, 4));

  // Get the 'Draw' method
  lua_getglobal(luaState, "Draw");
  if (lua_isfunction(luaState, -1))
  {
    // first argument is the ClientTable
    MyGetClientTable2(hostStruct);

    // call the lua function
    if (LUA_OK != lua_pcall(luaState, 1, 4, 0))
    {
      FatalError2("Failure while running lua Draw() method: ", lua_tostring(luaState, -1));
    }

    // the return values are the spinny cube angle and the view angles
    *spinnyCubeAngle = lua_tonumber(luaState, -4);
    *floorZOffset = lua_tonumber(luaState, -3);
    *viewAngleX = lua_tonumber(luaState, -2);
    *viewAngleY = lua_tonumber(luaState, -1);
  }
  else
  {
    // return stinky default values
    *spinnyCubeAngle = 0;
    *floorZOffset = 0;
    *viewAngleX = 0;
    *viewAngleY = 0;
  }

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MyCallHandlerEvent(MainAppHostStruct * hostStruct, 
                        const char * namesTableId, 
                        const char * handlersTableId, 
                        int handlerKey, 
                        int eventDataIndex)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int hostTableIndex;
  const char * handlerName;

  NateCheck0(lua_checkstack(luaState, 5));

  // get HostTable
  hostTableIndex = MyGetHostTable(hostStruct);

  // get the KeyDownHandlerNames table
  // local table = HostTable["KeyDownHandlerNames"]
  lua_pushstring(luaState, namesTableId);
  lua_gettable(luaState, hostTableIndex);

  // get the name of the handler to be called.
  // local handlerName = KeyDownHandlerNames[e.keysym]
  // do it now (even though it's only used in failure scenarios)
  // because failure scenarios are likely out-of-memory scenarios
  lua_pushnumber(luaState, handlerKey);
  lua_gettable(luaState, -2);
  lua_replace(luaState, -2); // discard KeyDownHandlerNames table
  // the c handlerName string is valid as long as the lua value stays on the stack
  handlerName = lua_tostring(luaState, -1);

  // get the KeyDownHandlers table
  // local table = HostTable["KeyDownHandlers"]
  lua_pushstring(luaState, handlersTableId);
  lua_gettable(luaState, hostTableIndex);
  // get the lua function for the associated key
  // local function = KeyDownHandlers[e.keysym]
  lua_pushnumber(luaState, handlerKey);
  lua_gettable(luaState, -2);
  lua_replace(luaState, -2);// discard KeyDownHandlers table

  // only invoke if a handler was really present
  // most often 'nil' is returned
  if (lua_isfunction(luaState, -1))
  {
    // first argument is the ClientTable
    MyGetClientTable(hostStruct, hostTableIndex);

    // second argument is... generated outside this method
    lua_pushvalue(luaState, eventDataIndex);
    if (LUA_OK != lua_pcall(luaState, 2, 0, 0))
    {
      FatalError3(handlerName, ": Failure while running lua method: ", lua_tostring(luaState, -1));
    }
  }

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLua_CallKeyDownEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table with 'keysym' = e->keysym
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.keysym = e->keysym
  lua_pushstring(luaState, "keysym");
  lua_pushnumber(luaState, e->keysym.sym);
  lua_settable(luaState, eventDataIndex);

  // call the custom event handler (if it exists)
  MyCallHandlerEvent(hostStruct, "KeyDownNames", "KeyDownHandlers", e->keysym.sym, eventDataIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLua_CallKeyUpEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.keysym = e->keysym
  lua_pushstring(luaState, "keysym");
  lua_pushnumber(luaState, e->keysym.sym);
  lua_settable(luaState, eventDataIndex);

  // call the custom event handler (if it exists)
  MyCallHandlerEvent(hostStruct, "KeyUpNames", "KeyUpHandlers", e->keysym.sym, eventDataIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLua_CallMouseMotionEvent(MainAppHostStruct * hostStruct, SDL_MouseMotionEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.xrel = relative x movement
  lua_pushstring(luaState, "xrel");
  lua_pushnumber(luaState, e->xrel);
  lua_settable(luaState, eventDataIndex);
  // table.yrel = relative y movement
  lua_pushstring(luaState, "yrel");
  lua_pushnumber(luaState, e->yrel);
  lua_settable(luaState, eventDataIndex);
  // table.x = absolute x position
  lua_pushstring(luaState, "x");
  lua_pushnumber(luaState, e->x);
  lua_settable(luaState, eventDataIndex);
  // table.y = absolute x position
  lua_pushstring(luaState, "y");
  lua_pushnumber(luaState, e->y);
  lua_settable(luaState, eventDataIndex);

  // call the custom event handler (if it exists)
  MyCallHandlerEvent(hostStruct, "MouseMotionNames", "MouseMotionHandlers", 0, eventDataIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MyRegisterEventHandler(lua_State * luaState,
                            int luaHandlerNameIndex,
                            int luaHandlerMethodIndex,
                            int eventKeyIndex,
                            const char * namesTableId, 
                            const char * handlersTableId)
{
  int hostTableIndex;
  int originalStackIndex = lua_gettop(luaState);

  NateCheck0(lua_checkstack(luaState, 4));

  // get the host table (pushes it on the lua stack)
  hostTableIndex = MyGetHostTable2(luaState);

  // get the HandlerNames table
  lua_pushstring(luaState, namesTableId);
  lua_gettable(luaState, hostTableIndex);

  // HandlerNames[eventKeyIndex] = handlerName
  lua_pushvalue(luaState, eventKeyIndex);
  lua_pushvalue(luaState, luaHandlerNameIndex);
  lua_settable(luaState, -3);
  lua_pop(luaState, 1); // pop the handlernames table

  // get the Handlers table
  lua_pushstring(luaState, handlersTableId);
  lua_gettable(luaState, hostTableIndex);

  // Handlers[eventKeyIndex] = handlerMethod
  lua_pushvalue(luaState, eventKeyIndex);
  lua_pushvalue(luaState, luaHandlerMethodIndex);
  lua_settable(luaState, -3);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

// these are exported and invoked by Lua client code
// the indexes are checked in the C_LuaExport methods so in case of errors
// the user gets a message with significantly more context than he deserves
void MainAppLua_RegisterKeyDownHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex)
{
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, eventKeyIndex,
    "KeyDownNames", "KeyDownHandlers");
}

void MainAppLua_RegisterKeyUpHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex)
{
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, eventKeyIndex,
    "KeyUpNames", "KeyUpHandlers");
}

void MainAppLua_RegisterKeyResetHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex)
{
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, eventKeyIndex,
    "KeyResetNames", "KeyResetHandlers");
}

void MainAppLua_RegisterMouseMotionHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex)
{
  // fabricate eventKeyIndex = 0
  NateCheck0(lua_checkstack(luaState, 1));
  lua_pushnumber(luaState, 0);
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, lua_gettop(luaState),
    "MouseMotionNames", "MouseMotionHandlers");
}