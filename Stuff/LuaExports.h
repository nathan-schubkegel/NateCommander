#ifndef NATE_COMMANDER_LUA_EXPORTS_H
#define NATE_COMMANDER_LUA_EXPORTS_H

#include "lua.h"
#include "NateMesh.h"
#include "MsCounter.h"

void LuaExports_PublishCMethods(lua_State * luaState);

#define NateUserDataGetter(typeName) \
  int IsNateUserData_##typeName(lua_State * luaState, int luaStackIndex, typeName ** value);

NateUserDataGetter(MsCounter);
NateUserDataGetter(NateMesh);

#endif