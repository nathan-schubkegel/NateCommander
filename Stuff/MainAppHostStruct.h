/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_MAIN_APP_HOST_STRUCT_H
#define NATE_COMMANDER_MAIN_APP_HOST_STRUCT_H

#include "CommonApp.h"
#include "lua.h"
#include "ctokamak.h"

// The "HostStruct" is a C struct in the lua registry, exclusively for C host use.
// The "HostTable" is a Lua table in the lua registry, exclusively for C host use.
// The "ClientTable" is a Lua table in the lua registry, passed to all Lua functions invoked by C
//                   as a 'state' parameter. It is meant exclusively for Lua client use.

struct MainAppHostStruct
{
  CommonApp_WindowAndOpenGlContext MainWindow;
  lua_State * luaState;
  cneSimulator * simulator;
};
typedef struct MainAppHostStruct MainAppHostStruct;

// initializes a luaState so the methods in this file work
void MainAppHostStruct_InitializeLuaState(MainAppHostStruct * hostStruct, lua_State * luaState);

// pushes the lua HostTable onto the Lua Stack, acquiring it from the given inputs
// returns the pushed index
int MainAppHostStruct_LuaPushHostTable(MainAppHostStruct * hostStruct);

// pushes the lua ClientTable onto the Lua Stack, acquiring it from the given inputs
// returns the pushed index
int MainAppHostStruct_LuaPushClientTable(MainAppHostStruct * hostStruct);
int MainAppHostStruct_LuaPushClientTable2(MainAppHostStruct * hostStruct, int hostTableIndex);

// fetches the C HostStruct from the Lua Stack
MainAppHostStruct * MainAppHostStruct_GetHostStruct(lua_State * luaState);

#endif