#include "LuaExports.h"

#include "lauxlib.h"
#include "FatalErrorHandler.h"
//#include <Windows.h>
#include "MsCounter.h"
#include "NateMesh.h"
#include "MainAppLua.h"
#include "MainAppPhysics.h"

// Design Hint: use this macro for every full userdata handed to lua
// so we can check the types of userdata passed from client lua scripts to C
// Design Hint: TypeId is 'int' so structs remain DWORD-aligned 
// (C/Win32 voodoo experience tells me this is a good idea)
#define NateUserData(typeName, typeId, finalizeFunction) \
  struct NateUserData_##typeName \
  { \
    int TypeId; \
    typeName Value; \
  }; \
  typedef struct NateUserData_##typeName NateUserData_##typeName; \
  const int NateUserDataTypeId_##typeName = typeId; \
  typeName * CreateNateUserData_##typeName(lua_State * luaState) \
  { \
    NateUserData_##typeName * thing; \
    NateCheck0(lua_checkstack(luaState, 4)); \
    thing = lua_newuserdata(luaState, sizeof(NateUserData_##typeName)); \
    thing->TypeId = NateUserDataTypeId_##typeName; \
    memset(&thing->Value, 0, sizeof(typeName)); \
    \
    if (luaL_newmetatable(luaState, "NateUserData_" #typeName "_Metatable")) \
    { \
      lua_pushstring(luaState, "__gc"); \
      lua_pushcfunction(luaState, finalizeFunction); \
      lua_settable(luaState, -3); \
    } \
    lua_setmetatable(luaState, -2); \
    \
    return &thing->Value; \
  } \
  int IsNateUserData_##typeName(lua_State * luaState, int luaStackIndex, typeName ** value) \
  { \
    NateUserData_##typeName * thing; \
    thing = lua_touserdata(luaState, luaStackIndex); \
    if (thing == 0 || thing->TypeId != typeId) \
    { \
      *value = 0; \
      return 0; \
    } \
    else \
    { \
      *value = &thing->Value; \
      return 1; \
    } \
  }

// use this if no finalization is needed for a NateUserData
int noFinalizer(lua_State * luaState)
{
  (void)luaState;
  return 0;
}

int NateMeshFinalizer(lua_State * luaState)
{
  NateMesh * mesh;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(IsNateUserData_NateMesh(luaState, 1, &mesh), "Expected argument 1 to be NateMesh");

  NateMesh_Uninit(mesh);

  return 0;
}

NateUserData(MsCounter, 55, noFinalizer);
NateUserData(NateMesh, 66, NateMeshFinalizer);

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

int C_Exit(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 1, "Expected 1 argument");
  NateCheck(lua_isnumber(luaState, 1), "Expected argument 1 \"exitCode\" to be a number");

  exit((int)lua_tonumber(luaState, 1));
  
  //return 0;
}

int C_FatalError(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 1, "Expected 1 argument");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"message\" to be a string");

  FatalError(lua_tostring(luaState, 1));
  
  return 0;
}

int C_NonFatalError(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 1, "Expected 1 argument");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"message\" to be a string");

  NonFatalError(lua_tostring(luaState, 1));
  
  return 0;
}

int C_NateAssert(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 2, "Expected 2 arguments");
  NateCheck(lua_isboolean(luaState, 1), "Expected argument 1 \"condition\" to be a boolean");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 2 \"message\" to be a string");

  NateAssert(lua_toboolean(luaState, 1), lua_tostring(luaState, 2));
  
  return 0;
}

int C_MsCounter_Create(lua_State * luaState)
{
  MsCounter * counter;

  NateCheck(lua_gettop(luaState) == 0, "Expected exactly zero arguments");

  counter = CreateNateUserData_MsCounter(luaState);
  MsCounter_Init(counter);

  return 1;
}

int C_MsCounter_Reset(lua_State * luaState)
{
  MsCounter * counter;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one arguments");
  NateCheck(IsNateUserData_MsCounter(luaState, 1, &counter), "Expected argument 1 to be MsCounter");

  MsCounter_Reset(counter);

  return 0;
}

int C_MsCounter_Update(lua_State * luaState)
{
  MsCounter * counter;
  Uint64 retValue;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one arguments");
  NateCheck(IsNateUserData_MsCounter(luaState, 1, &counter), "Expected argument 1 to be MsCounter");

  retValue = MsCounter_Update(counter);

  NateCheck0(lua_checkstack(luaState, 1));
  lua_pushnumber(luaState, (lua_Number)retValue);
  return 1;
}

int C_MsCounter_GetCount(lua_State * luaState)
{
  MsCounter * counter;
  Uint64 retValue;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one arguments");
  NateCheck(IsNateUserData_MsCounter(luaState, 1, &counter), "Expected argument 1 to be MsCounter");

  retValue = MsCounter_GetCount(counter);

  NateCheck0(lua_checkstack(luaState, 1));
  lua_pushnumber(luaState, (lua_Number)retValue);
  return 1;
}

int C_MsCounter_ResetToNewCount(lua_State * luaState)
{
  MsCounter * counter;

  NateCheck(lua_gettop(luaState) == 2, "Expected exactly two arguments");
  NateCheck(IsNateUserData_MsCounter(luaState, 1, &counter), "Expected argument 1 to be MsCounter");
  NateCheck(lua_isnumber(luaState, 2), "Expected argument 2 to be number");

  MsCounter_ResetToNewCount(counter, (Uint64)lua_tonumber(luaState, 2));

  return 0;
}

int C_MsCounter_ResetToCurrentCount(lua_State * luaState)
{
  MsCounter * counter;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(IsNateUserData_MsCounter(luaState, 1, &counter), "Expected argument 1 to be MsCounter");

  MsCounter_ResetToCurrentCount(counter);

  return 0;
}

int C_AdvanceGSIM(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(lua_isnumber(luaState, 1), "Expected argument 1 \"zoffset\" to be a number or string-number");

  MainAppPhysics_AdvanceGSIM((float)lua_tonumber(luaState, 1));

  return 0;
}

int C_RegisterKeyDownHandler(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 3, "Expected exactly three arguments");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"handlerName\" to be a string");
  NateCheck(lua_isfunction(luaState, 2) || lua_isnil(luaState, 2), "Expected argument 2 \"handlerFunction\" to be a function or nil");
  NateCheck(lua_isnumber(luaState, 3), "Expected argument 3 \"eventKey\" to be a number");
  
  MainAppLua_RegisterKeyDownHandler(luaState, 1, 2, 3);

  return 0;
}

int C_RegisterKeyUpHandler(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 3, "Expected exactly three arguments");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"handlerName\" to be a string");
  NateCheck(lua_isfunction(luaState, 2) || lua_isnil(luaState, 2), "Expected argument 2 \"handlerFunction\" to be a function or nil");
  NateCheck(lua_isnumber(luaState, 3), "Expected argument 3 \"eventKey\" to be a number");
  
  MainAppLua_RegisterKeyUpHandler(luaState, 1, 2, 3);

  return 0;
}

int C_RegisterKeyResetHandler(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 3, "Expected exactly three arguments");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"handlerName\" to be a string");
  NateCheck(lua_isfunction(luaState, 2) || lua_isnil(luaState, 2), "Expected argument 2 \"handlerFunction\" to be a function or nil");
  NateCheck(lua_isnumber(luaState, 3), "Expected argument 3 \"eventKey\" to be a number");
  
  MainAppLua_RegisterKeyResetHandler(luaState, 1, 2, 3);

  return 0;
}

int C_RegisterMouseMotionHandler(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 2, "Expected exactly two arguments");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"handlerName\" to be a string");
  NateCheck(lua_isfunction(luaState, 2) || lua_isnil(luaState, 2), "Expected argument 2 \"handlerFunction\" to be a function or nil");
  
  MainAppLua_RegisterMouseMotionHandler(luaState, 1, 2);

  return 0;
}

/*
int C_NateMesh_Create(lua_State * luaState)
{
  NateMesh * mesh;

  NateCheck(lua_gettop(luaState) == 0, "Expected exactly zero arguments");
  mesh = CreateNateUserData_NateMesh(luaState);
  NateMesh_Init(mesh);

  return 1;
}
*/

int C_NateMesh_Uninit(lua_State * luaState)
{
  NateMesh * mesh;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(IsNateUserData_NateMesh(luaState, 1, &mesh), "Expected argument 1 to be NateMesh");

  NateMesh_Uninit(mesh);

  return 0;
}

int C_NateMesh_LoadFromColladaResourceFile(lua_State * luaState)
{
  NateMesh * mesh;
  const char * fileName;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 to be string");
  
  fileName = lua_tostring(luaState, 1);
  mesh = CreateNateUserData_NateMesh(luaState);
  NateMesh_Init(mesh);
  NateMesh_LoadFromColladaResourceFile(mesh, fileName);

  return 1;
}

void LuaExports_PublishCMethods(lua_State * luaState)
{
  //lua_pushnumber(lua_state, LUA_RIDX_GLOBALS);
  //lua_gettable(luaState, LUA_REGISTRYINDEX);
 
#define PUBLISH_CMETHOD(m) \
  lua_pushcfunction(luaState, m); \
  lua_setglobal(luaState, #m);
//  lua_pushstring(luaState, ##m##); \
//  lua_pushcfunction(luaState, m); \
//  lua_settable(luaState, -3);

  NateCheck0(lua_checkstack(luaState, 1));

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
  PUBLISH_CMETHOD(C_RegisterKeyDownHandler);
  PUBLISH_CMETHOD(C_RegisterKeyUpHandler);
  PUBLISH_CMETHOD(C_RegisterKeyResetHandler);
  PUBLISH_CMETHOD(C_RegisterMouseMotionHandler);
  //PUBLISH_CMETHOD(C_NateMesh_Create);
  PUBLISH_CMETHOD(C_NateMesh_Uninit);
  PUBLISH_CMETHOD(C_NateMesh_LoadFromColladaResourceFile);

  //lua_pop(luaState, 1);
}