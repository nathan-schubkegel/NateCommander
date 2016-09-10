/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_MAIN_APP_HOST_TABLE_H
#define NATE_COMMANDER_MAIN_APP_HOST_TABLE_H

#include "lua.h"
#include "SDL.h"

// The "HostTable" is a Lua table in the lua registry, exclusively for C host use.

// initializes a luaState so the methods in this file work
void MainAppHostTable_Initialize(lua_State * luaState);

// pushes the lua HostTable onto the Lua Stack, returning the pushed index
int MainAppHostTable_LuaPushHostTable(lua_State * luaState);

#endif