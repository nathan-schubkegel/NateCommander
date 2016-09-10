/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "LuaExports.h"

#include "lauxlib.h"
#include "FatalErrorHandler.h"
//#include <Windows.h>
#include "MsCounter.h"
#include "NateMash.h"
#include "NateMashDrawing.h"
#include "MainAppLua.h"
#include "MainAppLuaInputs.h"
#include "MainAppPhysics.h"
#include <GL\gl.h>
#include <GL\glu.h>
#include "ViewGraphics.h"
#include "BoxGraphics.h"
#include "Vectors2d.h"
#include <math.h>
#include "AngleMath.h"

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

int NateMashFinalizer(lua_State * luaState)
{
  NateMash * mash;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(IsNateUserData_NateMash(luaState, 1, &mash), "Expected argument 1 to be NateMash");

  NateMash_Uninit(mash);

  return 0;
}

NateUserData(MsCounter, 55, noFinalizer);
NateUserData(NateMash, 66, NateMashFinalizer);

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
  NateCheck(lua_isstring(luaState, 2), "Expected argument 2 \"message\" to be a string");

  NateAssert(lua_toboolean(luaState, 1), lua_tostring(luaState, 2));
  
  return 0;
}

int C_LoadLuaFile(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 1, "Expected 1 argument");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"fileName\" to be a string");

  MainAppLua_LoadAndRunLuaFile(luaState, lua_tostring(luaState, 1));
  
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
  NateCheck(lua_isnumber(luaState, 3) || lua_isnil(luaState, 3), "Expected argument 3 \"eventKey\" to be a number or nil");
  
  MainAppLuaInputs_RegisterKeyDownHandler(luaState, 1, 2, 3);

  return 0;
}

int C_RegisterKeyUpHandler(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 3, "Expected exactly three arguments");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"handlerName\" to be a string");
  NateCheck(lua_isfunction(luaState, 2) || lua_isnil(luaState, 2), "Expected argument 2 \"handlerFunction\" to be a function or nil");
  NateCheck(lua_isnumber(luaState, 3) || lua_isnil(luaState, 3), "Expected argument 3 \"eventKey\" to be a number or nil");
  
  MainAppLuaInputs_RegisterKeyUpHandler(luaState, 1, 2, 3);

  return 0;
}

int C_RegisterMouseMotionHandler(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 2, "Expected exactly two arguments");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"handlerName\" to be a string");
  NateCheck(lua_isfunction(luaState, 2) || lua_isnil(luaState, 2), "Expected argument 2 \"handlerFunction\" to be a function or nil");
  
  MainAppLuaInputs_RegisterMouseMotionHandler(luaState, 1, 2);

  return 0;
}

int C_RegisterMouseDownHandler(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 3, "Expected exactly three arguments");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"handlerName\" to be a string");
  NateCheck(lua_isfunction(luaState, 2) || lua_isnil(luaState, 2), "Expected argument 2 \"handlerFunction\" to be a function or nil");
  NateCheck(lua_isnumber(luaState, 3) || lua_isnil(luaState, 3), "Expected argument 3 \"mouseButton\" to be a number or nil");
  
  MainAppLuaInputs_RegisterMouseDownHandler(luaState, 1, 2, 3);

  return 0;
}

int C_RegisterMouseUpHandler(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 3, "Expected exactly three arguments");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"handlerName\" to be a string");
  NateCheck(lua_isfunction(luaState, 2) || lua_isnil(luaState, 2), "Expected argument 2 \"handlerFunction\" to be a function or nil");
  NateCheck(lua_isnumber(luaState, 3) || lua_isnil(luaState, 3), "Expected argument 3 \"mouseButton\" to be a number or nil");
  
  MainAppLuaInputs_RegisterMouseUpHandler(luaState, 1, 2, 3);

  return 0;
}

int C_RegisterMouseWheelHandler(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 2, "Expected exactly two arguments");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 \"handlerName\" to be a string");
  NateCheck(lua_isfunction(luaState, 2) || lua_isnil(luaState, 2), "Expected argument 2 \"handlerFunction\" to be a function or nil");
  
  MainAppLuaInputs_RegisterMouseWheelHandler(luaState, 1, 2);

  return 0;
}

int C_NateMash_Uninit(lua_State * luaState)
{
  NateMash * mash;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(IsNateUserData_NateMash(luaState, 1, &mash), "Expected argument 1 to be NateMash");

  NateMash_Uninit(mash);

  return 0;
}

int C_NateMash_LoadFromColladaResourceFile(lua_State * luaState)
{
  NateMash * mash;
  const char * fileName;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(lua_isstring(luaState, 1), "Expected argument 1 to be string");
  
  fileName = lua_tostring(luaState, 1);
  mash = CreateNateUserData_NateMash(luaState);
  NateMash_Init(mash);
  NateMash_LoadFromColladaResourceFile(mash, fileName);

  return 1;
}

int C_NateMash_DrawUpright(lua_State * luaState)
{
  NateMash * mash;
  float position[3];
  float scale[3];
  float rotation[2];

  NateCheck(lua_gettop(luaState) == 9, "Expected exactly 9 argument");
  NateCheck(IsNateUserData_NateMash(luaState, 1, &mash), "Expected argument 1 to be NateMash");

  position[0] = (float)lua_tonumber(luaState, 2);
  position[1] = (float)lua_tonumber(luaState, 3);
  position[2] = (float)lua_tonumber(luaState, 4);

  rotation[0] = (float)lua_tonumber(luaState, 5);
  rotation[1] = (float)lua_tonumber(luaState, 6);

  scale[0] = (float)lua_tonumber(luaState, 7);
  scale[1] = (float)lua_tonumber(luaState, 8);
  scale[2] = (float)lua_tonumber(luaState, 9);

  NateMash_DrawUpright(mash, position, rotation, scale);

  return 0;
}

int C_NateMash_Draw(lua_State * luaState)
{
  NateMash * mash;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly 1 argument");
  NateCheck(IsNateUserData_NateMash(luaState, 1, &mash), "Expected argument 1 to be NateMash");

  NateMash_Draw(mash);

  return 0;
}

int C_PolarVector2dAdd(lua_State * luaState)
{
  PolarVector2d v1;
  PolarVector2d v2;
  PolarVector2d v3;

  NateCheck(lua_gettop(luaState) == 2, "Expected exactly two arguments");
  NateCheck(lua_istable(luaState, 1), "Expected argument 1 to be table");
  NateCheck(lua_istable(luaState, 2), "Expected argument 2 to be table");

  // inputs are
  // table { Angle = a1, Magnitude = m1 }
  // table { Angle = a2, Magnitude = m2 }

  lua_pushstring(luaState, "Angle");
  lua_gettable(luaState, 1);
  v1.Angle = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  lua_pushstring(luaState, "Magnitude");
  lua_gettable(luaState, 1);
  v1.Magnitude = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  lua_pushstring(luaState, "Angle");
  lua_gettable(luaState, 2);
  v2.Angle = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  lua_pushstring(luaState, "Magnitude");
  lua_gettable(luaState, 2);
  v2.Magnitude = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  v3 = PolarVector2dAdd(v1, v2);

  // return value is 
  // table { Angle = v3.Angle, Magnitude = v3.Magnitude }
  lua_createtable(luaState, 0, 2);
  lua_pushstring(luaState, "Angle");
  lua_pushnumber(luaState, v3.Angle);
  lua_settable(luaState, -3);
  lua_pushstring(luaState, "Magnitude");
  lua_pushnumber(luaState, v3.Magnitude);
  lua_settable(luaState, -3);

  return 1;
}

int C_PolarVector2dDup(lua_State * luaState)
{
  double a1, m1;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(lua_istable(luaState, 1), "Expected argument 1 to be table");

  // inputs are
  // table { Angle = a1, Magnitude = m1 }

  lua_pushstring(luaState, "Angle");
  lua_gettable(luaState, 1);
  a1 = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  lua_pushstring(luaState, "Magnitude");
  lua_gettable(luaState, 1);
  m1 = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  // return value is 
  // table { Angle = a1, Magnitude = m1 }
  lua_createtable(luaState, 0, 2);
  lua_pushstring(luaState, "Angle");
  lua_pushnumber(luaState, a1);
  lua_settable(luaState, -3);
  lua_pushstring(luaState, "Magnitude");
  lua_pushnumber(luaState, m1);
  lua_settable(luaState, -3);

  return 1;
}

int C_PolarVector2dToCartesian(lua_State * luaState)
{
  PolarVector2d v1;
  CartesianVector2d v2;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(lua_istable(luaState, 1), "Expected argument 1 to be table");

  // inputs are
  // table { Angle = a1, Magnitude = m1 }

  lua_pushstring(luaState, "Angle");
  lua_gettable(luaState, 1);
  v1.Angle = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  lua_pushstring(luaState, "Magnitude");
  lua_gettable(luaState, 1);
  v1.Magnitude = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  v2 = PolarVector2dToCartesian(v1);

  // return value is 
  // table { X = x1, Y = y1 }
  lua_createtable(luaState, 0, 2);
  lua_pushstring(luaState, "X");
  lua_pushnumber(luaState, v2.X);
  lua_settable(luaState, -3);
  lua_pushstring(luaState, "Y");
  lua_pushnumber(luaState, v2.Y);
  lua_settable(luaState, -3);

  return 1;
}

int C_CartesianVector2dAdd(lua_State * luaState)
{
  CartesianVector2d v1, v2, v3;

  NateCheck(lua_gettop(luaState) == 2, "Expected exactly two arguments");
  NateCheck(lua_istable(luaState, 1), "Expected argument 1 to be table");
  NateCheck(lua_istable(luaState, 2), "Expected argument 2 to be table");

  // inputs are
  // table { X = x1, Y = y1 }
  // table { X = x2, Y = y2 }

  lua_pushstring(luaState, "X");
  lua_gettable(luaState, 1);
  v1.X = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  lua_pushstring(luaState, "Y");
  lua_gettable(luaState, 1);
  v1.Y = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  lua_pushstring(luaState, "X");
  lua_gettable(luaState, 2);
  v2.X = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  lua_pushstring(luaState, "Y");
  lua_gettable(luaState, 2);
  v2.Y = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  v3 = CartesianVector2dAdd(v1, v2);

  // return value is 
  // table { X = x1 + x2, Y = y1 + y2 }
  lua_createtable(luaState, 0, 2);
  lua_pushstring(luaState, "X");
  lua_pushnumber(luaState, v3.X);
  lua_settable(luaState, -3);
  lua_pushstring(luaState, "Y");
  lua_pushnumber(luaState, v3.Y);
  lua_settable(luaState, -3);

  return 1;
}

int C_CartesianVector2dDup(lua_State * luaState)
{
  double x1, y1;

  NateCheck(lua_gettop(luaState) == 1, "Expected exactly one argument");
  NateCheck(lua_istable(luaState, 1), "Expected argument 1 to be table");

  // inputs are
  // table { X = x1, Y = y1 }

  lua_pushstring(luaState, "X");
  lua_gettable(luaState, 1);
  x1 = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  lua_pushstring(luaState, "Y");
  lua_gettable(luaState, 1);
  y1 = lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  // return value is 
  // table { X = x1, Y = y1 }
  lua_createtable(luaState, 0, 2);
  lua_pushstring(luaState, "X");
  lua_pushnumber(luaState, x1);
  lua_settable(luaState, -3);
  lua_pushstring(luaState, "Y");
  lua_pushnumber(luaState, y1);
  lua_settable(luaState, -3);

  return 1;
}

int C_GetAbsAnglesBetween(lua_State * luaState)
{
  double a1, a2, best;

  NateCheck(lua_gettop(luaState) == 2, "Expected exactly two arguments");
  a1 = lua_tonumber(luaState, 1);
  a2 = lua_tonumber(luaState, 2);

  best = AngleMath_GetAbsAnglesBetween(a1, a2);

  // return the best found number
  lua_pushnumber(luaState, best);
  return 1;
}

int C_gluPerspective(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 4, "Expected exactly 4 arguments");
  NateCheck(lua_isnumber(luaState, 1), "Expected argument 1 to be number");
  NateCheck(lua_isnumber(luaState, 2), "Expected argument 2 to be number");
  NateCheck(lua_isnumber(luaState, 3), "Expected argument 3 to be number");
  NateCheck(lua_isnumber(luaState, 4), "Expected argument 4 to be number");

  gluPerspective(
    lua_tonumber(luaState, 1),
    lua_tonumber(luaState, 2),
    lua_tonumber(luaState, 3),
    lua_tonumber(luaState, 4));

  return 0;
}

int C_glMatrixMode(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 1, "Expected exactly 1 argument");
  NateCheck(lua_isnumber(luaState, 1), "Expected argument 1 to be number");

  glMatrixMode((GLenum)(int)lua_tonumber(luaState, 1));

  return 0;
}

int C_glLoadIdentity(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 0, "Expected exactly 0 arguments");

  glLoadIdentity();

  return 0;
}

int C_SetView_CameraLookingAtPoint_FromDistance_AtAngle(lua_State * luaState)
{
  float xyzFocalPoint[3];

  NateCheck(lua_gettop(luaState) == 6, "Expected exactly 6 arguments");
  NateCheck(lua_isnumber(luaState, 1), "Expected argument 1 to be number");
  NateCheck(lua_isnumber(luaState, 2), "Expected argument 2 to be number");
  NateCheck(lua_isnumber(luaState, 3), "Expected argument 3 to be number");
  NateCheck(lua_isnumber(luaState, 4), "Expected argument 4 to be number");
  NateCheck(lua_isnumber(luaState, 5), "Expected argument 5 to be number");
  NateCheck(lua_isnumber(luaState, 6), "Expected argument 6 to be number");

  xyzFocalPoint[0] = (float)lua_tonumber(luaState, 1);
  xyzFocalPoint[1] = (float)lua_tonumber(luaState, 2);
  xyzFocalPoint[2] = (float)lua_tonumber(luaState, 3);

  SetView_CameraLookingAtPoint_FromDistance_AtAngle(xyzFocalPoint,
    (float)lua_tonumber(luaState, 4),
    (float)lua_tonumber(luaState, 5),
    (float)lua_tonumber(luaState, 6));

  return 0;
}

int C_glClear(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 1, "Expected exactly 1 argument");
  NateCheck(lua_isnumber(luaState, 1), "Expected argument 1 to be number");

  glClear((GLbitfield)(int)lua_tonumber(luaState, 1));

  return 0;
}

int C_glRotate(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 4, "Expected exactly 4 arguments");

  glRotated(
    (double)lua_tonumber(luaState, 1),  // angle
    (double)lua_tonumber(luaState, 2),  // x of vector to rotate around
    (double)lua_tonumber(luaState, 3),  // y
    (double)lua_tonumber(luaState, 4)); // z

  return 0;
}

int C_glScale(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 3, "Expected exactly 3 arguments");

  glScaled(
    (double)lua_tonumber(luaState, 1),  // x amount to scale
    (double)lua_tonumber(luaState, 2),  // y
    (double)lua_tonumber(luaState, 3)); // z

  return 0;
}

int C_glTranslate(lua_State * luaState)
{
  NateCheck(lua_gettop(luaState) == 3, "Expected exactly 3 arguments");

  glTranslated(
    (double)lua_tonumber(luaState, 1),  // x amount to translate
    (double)lua_tonumber(luaState, 2),  // y
    (double)lua_tonumber(luaState, 3)); // z

  return 0;
}

int C_DrawAxisLines(lua_State * luaState)
{
  (void)luaState;

  DrawAxisLineX();
  DrawAxisLineY();
  DrawAxisLineZ();

  return 0;
}

int C_DrawYAngledCube(lua_State * luaState)
{
  DrawYAngledCube((float)lua_tonumber(luaState, 1));

  return 0;
}

int C_DrawRainbowCube(lua_State * luaState)
{
  (void)luaState;

  DrawRainbowCube();

  return 0;
}

void LuaExports_PublishCMethods(lua_State * luaState)
{
  //lua_pushnumber(lua_state, LUA_RIDX_GLOBALS);
  //lua_gettable(luaState, LUA_REGISTRYINDEX);
 
#define PUBLISH_CMETHOD(m) \
  lua_pushcfunction(luaState, m); \
  lua_setglobal(luaState, #m);

  NateCheck0(lua_checkstack(luaState, 1));

  PUBLISH_CMETHOD(C_Exit);
  PUBLISH_CMETHOD(C_FatalError);
  PUBLISH_CMETHOD(C_NonFatalError);
  PUBLISH_CMETHOD(C_NateAssert);
  PUBLISH_CMETHOD(C_LoadLuaFile);
  PUBLISH_CMETHOD(C_MsCounter_Create);
  PUBLISH_CMETHOD(C_MsCounter_Reset);
  PUBLISH_CMETHOD(C_MsCounter_Update);
  PUBLISH_CMETHOD(C_MsCounter_GetCount);
  PUBLISH_CMETHOD(C_MsCounter_ResetToNewCount);
  PUBLISH_CMETHOD(C_MsCounter_ResetToCurrentCount);
  PUBLISH_CMETHOD(C_AdvanceGSIM);
  PUBLISH_CMETHOD(C_RegisterKeyDownHandler);
  PUBLISH_CMETHOD(C_RegisterKeyUpHandler);
  PUBLISH_CMETHOD(C_RegisterMouseMotionHandler);
  PUBLISH_CMETHOD(C_RegisterMouseDownHandler);
  PUBLISH_CMETHOD(C_RegisterMouseUpHandler);
  PUBLISH_CMETHOD(C_RegisterMouseWheelHandler);
  PUBLISH_CMETHOD(C_NateMash_Uninit);
  PUBLISH_CMETHOD(C_NateMash_LoadFromColladaResourceFile);
  PUBLISH_CMETHOD(C_NateMash_DrawUpright);
  PUBLISH_CMETHOD(C_NateMash_Draw);
  PUBLISH_CMETHOD(C_PolarVector2dAdd);
  PUBLISH_CMETHOD(C_PolarVector2dDup);
  PUBLISH_CMETHOD(C_PolarVector2dToCartesian);
  PUBLISH_CMETHOD(C_CartesianVector2dAdd);
  PUBLISH_CMETHOD(C_CartesianVector2dDup);
  PUBLISH_CMETHOD(C_GetAbsAnglesBetween);
  PUBLISH_CMETHOD(C_gluPerspective);
  PUBLISH_CMETHOD(C_glMatrixMode);
  PUBLISH_CMETHOD(C_glLoadIdentity);
  PUBLISH_CMETHOD(C_glClear);
  PUBLISH_CMETHOD(C_glRotate);
  PUBLISH_CMETHOD(C_glScale);
  PUBLISH_CMETHOD(C_glTranslate);
  PUBLISH_CMETHOD(C_SetView_CameraLookingAtPoint_FromDistance_AtAngle);
  PUBLISH_CMETHOD(C_DrawAxisLines);
  PUBLISH_CMETHOD(C_DrawYAngledCube);
  PUBLISH_CMETHOD(C_DrawRainbowCube);

  //lua_pop(luaState, 1);
}