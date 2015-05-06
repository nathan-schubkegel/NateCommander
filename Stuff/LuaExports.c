#include "LuaExports.h"

#include "lauxlib.h"
#include "FatalErrorHandler.h"
#include <Windows.h>
#include "MsCounter.h"
#include "MainApp.h"

/*
A C function receives its arguments from Lua in its stack in direct order 
(the first argument is pushed first). So when the function starts, 
lua_gettop(L) returns the number of arguments received by the function. 
The first argument (if any) is at index 1 and its last argument is at 
index lua_gettop(L). To return values to Lua, a C function just pushes 
them onto the stack, in direct order (the first result is pushed first), 
and returns the number of results. Any other value in the stack below the 
results will be properly discarded by Lua. Like a Lua function, a C 
function called by Lua can also return many results.
*/

int C_Exit(struct lua_State * luaState)
{
  lua_Number firstArg;

  // this is nonsense, it just makes a compiler warning go away
  if (luaState != 0)
  {
    if (lua_gettop(luaState) >= 1 && lua_isnumber(luaState, 1))
    {
      firstArg = lua_tonumber(luaState, 1);
      exit((int)firstArg);
    }
    else
    {
      exit(0);
    }
  }
  return 0;
}

// This is how I would do a shared metatable, if I cared to
/*
int C_GC_MsCounter(struct lua_State * luaState)
{
}

int C_CreateMsCounter(struct lua_State * luaState)
{
  MsCounter * newCounter = lua_newuserdata(luaState, sizeof(MsCounter));
  MsCounter_Init(newCounter);

  // get or create a shared metatable to be used to finalize MsCounter objects
  if (luaL_newmetatable(luaState, "C_MsCounter_Metatable"))
  {
    lua_pushstring("__gc");
    lua_pushcfunction(luaState, C_GC_MsCounter);
    lua_settable(luaState, -3);
  }

  // apply it to the new MsCounter object
  lua_setmetatable(luaState, -2);

  return 1;
}
*/

int C_FatalError(struct lua_State * luaState)
{
  const char * message = lua_tostring(luaState, -1);
  FatalError(message);
  return 0;
}

int C_NonFatalError(struct lua_State * luaState)
{
  const char * message = lua_tostring(luaState, -1);
  NonFatalError(message);
  return 0;
}

int C_NateAssert(struct lua_State * luaState)
{
  const char * message = lua_tostring(luaState, -1);
  int luaCondition = lua_toboolean(luaState, -2);
  if (!luaCondition) 
  { 
    FatalError(message); 
  }
  return 0;
}

int C_MsCounter_Create(struct lua_State * luaState)
{
  MsCounter * newCounter = lua_newuserdata(luaState, sizeof(MsCounter));
  MsCounter_Init(newCounter);
  return 1;
}

int C_MsCounter_Reset(struct lua_State * luaState)
{
  MsCounter * counter = lua_touserdata(luaState, -1);
  MsCounter_Reset(counter);
  return 0;
}

int C_MsCounter_Update(struct lua_State * luaState)
{
  MsCounter * counter = lua_touserdata(luaState, -1);
  lua_Number newValue = (lua_Number)MsCounter_Update(counter);
  lua_pushnumber(luaState, newValue);
  return 1;
}

int C_MsCounter_GetCount(struct lua_State * luaState)
{
  MsCounter * counter = lua_touserdata(luaState, -1);
  lua_Number newValue = (lua_Number)MsCounter_GetCount(counter);
  lua_pushnumber(luaState, newValue);
  return 1;
}

int C_MsCounter_ResetToNewCount(struct lua_State * luaState)
{
  MsCounter * counter = lua_touserdata(luaState, -2);
  lua_Number newValue = lua_tonumber(luaState, -1);
  MsCounter_ResetToNewCount(counter, (Uint64)newValue);
  return 0;
}

int C_MsCounter_ResetToCurrentCount(struct lua_State * luaState)
{
  MsCounter * counter = lua_touserdata(luaState, -1);
  MsCounter_ResetToCurrentCount(counter);
  return 0;
}

int C_AdvanceGSIM(struct lua_State * luaState)
{
  MainApp_AdvanceGSIM();
  return 0;
}

void LuaExports_PublishCMethods(struct lua_State * luaState)
{
  //lua_pushnumber(lua_state, LUA_RIDX_GLOBALS);
  //lua_gettable(luaState, LUA_REGISTRYINDEX);
 
#define PUBLISH_CMETHOD(m) \
  lua_pushcfunction(luaState, m); \
  lua_setglobal(luaState, #m);
//  lua_pushstring(luaState, ##m##); \
//  lua_pushcfunction(luaState, m); \
//  lua_settable(luaState, -3);

  PUBLISH_CMETHOD(C_Exit);
  PUBLISH_CMETHOD(C_FatalError);
  PUBLISH_CMETHOD(C_NonFatalError);
  PUBLISH_CMETHOD(C_NateAssert);
  PUBLISH_CMETHOD(C_MsCounter_Create);
  PUBLISH_CMETHOD(C_MsCounter_Reset);
  PUBLISH_CMETHOD(C_MsCounter_Update);
  PUBLISH_CMETHOD(C_MsCounter_GetCount);
  PUBLISH_CMETHOD(C_MsCounter_ResetToNewCount);
  PUBLISH_CMETHOD(C_MsCounter_ResetToCurrentCount);
  PUBLISH_CMETHOD(C_AdvanceGSIM);

  //lua_pop(luaState, 1);
}