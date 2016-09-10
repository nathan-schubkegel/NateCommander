/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MainAppHostTable.h"

#include "MainAppHostStruct.h"
#include "FatalErrorHandler.h"

// initializes a luaState so the methods in this file work
void MainAppHostTable_Initialize(lua_State * luaState)
{
  (void)luaState;
  // TODO: nothing to do here yet. But when there's something to do,
  //       here's some code that'll probably help do it!

  //MainAppHostStruct * hostStruct;
  //int hostTableIndex;

  //hostStruct = MainAppHostStruct_GetHostStruct(luaState);
  //hostTableIndex = MainAppHostStruct_LuaPushHostTable(hostStruct);
}

// pushes the lua HostTable onto the Lua Stack, returning the pushed index
int MainAppHostTable_LuaPushHostTable(lua_State * luaState)
{
  MainAppHostStruct * hostStruct;

  NateCheck0(lua_checkstack(luaState, 1));

  // local HostTable = Registry[HostStruct]
  hostStruct = MainAppHostStruct_GetHostStruct(luaState);
  lua_pushlightuserdata(luaState, hostStruct);
  lua_gettable(luaState, LUA_REGISTRYINDEX);
  // NOTE: the goal of this function was to push the HostTable onto the lua stack
  // (which has now been accomplished)
  return lua_gettop(luaState);
}
