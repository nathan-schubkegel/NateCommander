/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "LuaHelpers.h"

#include "FatalErrorHandler.h"

int LuaHelpers_AbsoluteIndex(lua_State * luaState, int index)
{
  if (index < 0)
  {
    return lua_gettop(luaState) + 1 + index;
  }
  return index;
}

int LuaHelpers_ReplaceNilWithNumber(lua_State * luaState, int index, lua_Number number)
{
  int absoluteIndex;

  NateCheck0(lua_checkstack(luaState, 1));

  absoluteIndex = LuaHelpers_AbsoluteIndex(luaState, index);
  if (lua_isnil(luaState, absoluteIndex))
  {
    lua_pushnumber(luaState, number);
    lua_replace(luaState, absoluteIndex);
  }

  return absoluteIndex;
}
