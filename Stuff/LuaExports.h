/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_LUA_EXPORTS_H
#define NATE_COMMANDER_LUA_EXPORTS_H

#include "lua.h"
#include "NateMash.h"
#include "MsCounter.h"

void LuaExports_PublishCMethods(lua_State * luaState);

#define NateUserDataGetter(typeName) \
  int IsNateUserData_##typeName(lua_State * luaState, int luaStackIndex, typeName ** value);

NateUserDataGetter(MsCounter);
NateUserDataGetter(NateMash);

#endif