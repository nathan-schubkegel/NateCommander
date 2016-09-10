/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_LUA_HELPERS_H
#define NATE_COMMANDER_LUA_HELPERS_H

#include "lua.h"

// converts a possibly-negative index to a positive index. Pseudo-indexes not supported.
int LuaHelpers_AbsoluteIndex(lua_State * luaState, int index);

// if the given index holds nil, set it to the given number
int LuaHelpers_ReplaceNilWithNumber(lua_State * luaState, int index, lua_Number number);

#endif