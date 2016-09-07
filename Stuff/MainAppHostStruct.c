/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MainAppHostStruct.h"

#include "lua.h"
#include "FatalErrorHandler.h"

void MainAppHostStruct_InitializeLuaState(MainAppHostStruct * hostStruct, lua_State * luaState)
{
  int hostStructIndex;
  int hostTableIndex;
  int clientTableIndex;
  int originalStackIndex;

  originalStackIndex = lua_gettop(luaState);

  // Ensure that enough lua stack spaces are available
  NateCheck0(lua_checkstack(luaState, 5));

  // Initialize the "HostStruct" (a C struct exclusively for C host use)
  lua_pushlightuserdata(luaState, hostStruct);
  hostStructIndex = lua_gettop(luaState);

  // Initialize the "HostTable" (a LUA table exclusively for C host use)
  lua_newtable(luaState);
  hostTableIndex = lua_gettop(luaState);

  // Initialize the "ClientTable" (a LUA table passed to all invoked LUA functions,
  // exclusively for LUA client use)
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

  // HostTable["HostStruct"] = HostStruct
  lua_pushstring(luaState, "HostStruct");
  lua_pushvalue(luaState, hostStructIndex);
  lua_settable(luaState, hostTableIndex);

  // HostTable["ClientTable"] = ClientTable
  lua_pushstring(luaState, "ClientTable");
  lua_pushvalue(luaState, clientTableIndex);
  lua_settable(luaState, hostTableIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

int MainAppHostStruct_LuaPushHostTable(MainAppHostStruct * hostStruct)
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

int MainAppHostStruct_LuaPushHostTable2(lua_State * luaState)
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

int MainAppHostStruct_LuaPushClientTable(MainAppHostStruct * hostStruct)
{
  lua_State * luaState = hostStruct->luaState;
  int hostTableIndex;

  NateCheck0(lua_checkstack(luaState, 2));

  // get the HostTable (pushed onto lua stack)
  hostTableIndex = MainAppHostStruct_LuaPushHostTable(hostStruct);

  // get the ClientTable from HostTable["ClientTable"]
  lua_pushstring(luaState, "ClientTable");
  lua_gettable(luaState, hostTableIndex);
  // NOTE: the goal of this function was to push the HostTable onto the lua stack
  lua_replace(luaState, -2); // replace HostTable
  return lua_gettop(luaState);
}

int MainAppHostStruct_LuaPushClientTable2(MainAppHostStruct * hostStruct, int hostTableIndex)
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

MainAppHostStruct * MainAppHostStruct_GetHostStruct(lua_State * luaState)
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