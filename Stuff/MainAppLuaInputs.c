/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MainAppLuaInputs.h"

#include "FatalErrorHandler.h"
#include "MainAppHostStruct.h"

void MainAppLuaInputs_Initialize(MainAppHostStruct * hostStruct)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);

  NateCheck0(lua_checkstack(luaState, 3));

  MainAppHostStruct_LuaPushHostTable(hostStruct);
  
  lua_pushstring(luaState, "MouseMotionNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "MouseMotionHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyUpNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyUpHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyDownNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyDownHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyResetNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "KeyResetHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MyCallHandlerEvent(MainAppHostStruct * hostStruct, 
                        const char * namesTableId, 
                        const char * handlersTableId, 
                        int handlerKey, 
                        int eventDataIndex)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int hostTableIndex;
  const char * handlerName;

  NateCheck0(lua_checkstack(luaState, 5));

  // get HostTable
  hostTableIndex = MainAppHostStruct_LuaPushHostTable(hostStruct);

  // get the KeyDownHandlerNames table
  // local table = HostTable["KeyDownHandlerNames"]
  lua_pushstring(luaState, namesTableId);
  lua_gettable(luaState, hostTableIndex);

  // get the name of the handler to be called.
  // local handlerName = KeyDownHandlerNames[e.keysym]
  // do it now (even though it's only used in failure scenarios)
  // because failure scenarios are likely out-of-memory scenarios
  lua_pushnumber(luaState, handlerKey);
  lua_gettable(luaState, -2);
  lua_replace(luaState, -2); // discard KeyDownHandlerNames table
  // the c handlerName string is valid as long as the lua value stays on the stack
  handlerName = lua_tostring(luaState, -1);

  // get the KeyDownHandlers table
  // local table = HostTable["KeyDownHandlers"]
  lua_pushstring(luaState, handlersTableId);
  lua_gettable(luaState, hostTableIndex);
  // get the lua function for the associated key
  // local function = KeyDownHandlers[e.keysym]
  lua_pushnumber(luaState, handlerKey);
  lua_gettable(luaState, -2);
  lua_replace(luaState, -2);// discard KeyDownHandlers table

  // only invoke if a handler was really present
  // most often 'nil' is returned
  if (lua_isfunction(luaState, -1))
  {
    // first argument is the ClientTable
    MainAppHostStruct_LuaPushClientTable2(hostStruct, hostTableIndex);

    // second argument is... generated outside this method
    lua_pushvalue(luaState, eventDataIndex);
    if (LUA_OK != lua_pcall(luaState, 2, 0, 0))
    {
      FatalError3(handlerName, ": Failure while running lua method: ", lua_tostring(luaState, -1));
    }
  }

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);

  // repeat for the "all events" handler (-1)
  if (handlerKey != -1)
  {
    MyCallHandlerEvent(hostStruct, namesTableId, handlersTableId, 
      -1, eventDataIndex);
  }
}

void MainAppLuaInputs_CallKeyDownEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table with 'keysym' = e->keysym
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.keysym = e->keysym
  lua_pushstring(luaState, "keysym");
  lua_pushnumber(luaState, e->keysym.sym);
  lua_settable(luaState, eventDataIndex);

  // call the custom event handler (if it exists)
  MyCallHandlerEvent(hostStruct, "KeyDownNames", "KeyDownHandlers", e->keysym.sym, eventDataIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLuaInputs_CallKeyUpEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.keysym = e->keysym
  lua_pushstring(luaState, "keysym");
  lua_pushnumber(luaState, e->keysym.sym);
  lua_settable(luaState, eventDataIndex);

  // call the custom event handler (if it exists)
  MyCallHandlerEvent(hostStruct, "KeyUpNames", "KeyUpHandlers", e->keysym.sym, eventDataIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLuaInputs_CallMouseMotionEvent(MainAppHostStruct * hostStruct, SDL_MouseMotionEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.ChangeX = relative x movement
  lua_pushstring(luaState, "ChangeX");
  lua_pushnumber(luaState, e->xrel);
  lua_settable(luaState, eventDataIndex);
  // table.ChangeY = relative y movement
  lua_pushstring(luaState, "ChangeY");
  lua_pushnumber(luaState, e->yrel);
  lua_settable(luaState, eventDataIndex);
  // table.PositionX = absolute x position
  lua_pushstring(luaState, "PositionX");
  lua_pushnumber(luaState, e->x);
  lua_settable(luaState, eventDataIndex);
  // table.PositionY = absolute x position
  lua_pushstring(luaState, "PositionY");
  lua_pushnumber(luaState, e->y);
  lua_settable(luaState, eventDataIndex);

  // call the custom event handler (if it exists)
  MyCallHandlerEvent(hostStruct, "MouseMotionNames", "MouseMotionHandlers", 0, eventDataIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

int MyToAbsoluteIndex(lua_State * luaState, int index)
{
  if (index < 0)
  {
    return lua_gettop(luaState) + 1 + index;
  }
  return index;
}

void MyReplaceNilWithNumber(lua_State * luaState, int index, lua_Number number)
{
  int absoluteIndex;

  NateCheck0(lua_checkstack(luaState, 1));

  absoluteIndex = MyToAbsoluteIndex(luaState, index);
  if (lua_isnil(luaState, absoluteIndex))
  {
    lua_pushnumber(luaState, number);
    lua_replace(luaState, absoluteIndex);
  }
}

void MyRegisterEventHandler(lua_State * luaState,
                            int luaHandlerNameIndex,
                            int luaHandlerMethodIndex,
                            int eventKeyIndex,
                            const char * namesTableId, 
                            const char * handlersTableId)
{
  int hostTableIndex;
  int originalStackIndex = lua_gettop(luaState);

  NateCheck0(lua_checkstack(luaState, 4));

  // get the host table (pushes it on the lua stack)
  hostTableIndex = MainAppHostStruct_LuaPushHostTable2(luaState);

  // get the HandlerNames table
  lua_pushstring(luaState, namesTableId);
  lua_gettable(luaState, hostTableIndex);

  // HandlerNames[eventKeyIndex ?? -1] = handlerName
  lua_pushvalue(luaState, eventKeyIndex);
  MyReplaceNilWithNumber(luaState, -1, -1.0);
  lua_pushvalue(luaState, luaHandlerNameIndex);
  lua_settable(luaState, -3);
  lua_pop(luaState, 1); // pop the handlernames table

  // get the Handlers table
  lua_pushstring(luaState, handlersTableId);
  lua_gettable(luaState, hostTableIndex);

  // Handlers[eventKeyIndex ?? -1] = handlerMethod
  lua_pushvalue(luaState, eventKeyIndex);
  MyReplaceNilWithNumber(luaState, -1, -1.0);
  lua_pushvalue(luaState, luaHandlerMethodIndex);
  lua_settable(luaState, -3);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

// these are exported and invoked by Lua client code
// the indexes are checked in the C_LuaExport methods so in case of errors
// the user gets a message with significantly more context than he deserves
void MainAppLuaInputs_RegisterKeyDownHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex)
{
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, eventKeyIndex,
    "KeyDownNames", "KeyDownHandlers");
}

void MainAppLuaInputs_RegisterKeyUpHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex)
{
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, eventKeyIndex,
    "KeyUpNames", "KeyUpHandlers");
}

void MainAppLuaInputs_RegisterKeyResetHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex)
{
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, eventKeyIndex,
    "KeyResetNames", "KeyResetHandlers");
}

void MainAppLuaInputs_RegisterMouseMotionHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex)
{
  // fabricate eventKeyIndex = 0
  NateCheck0(lua_checkstack(luaState, 1));
  lua_pushnumber(luaState, 0);
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, lua_gettop(luaState),
    "MouseMotionNames", "MouseMotionHandlers");
}

void MainAppLuaInputs_CallMouseDownEvent(MainAppHostStruct * hostStruct, SDL_MouseButtonEvent * e)
{
  (void)hostStruct;
  (void)e;
}

void MainAppLuaInputs_CallMouseUpEvent(MainAppHostStruct * hostStruct, SDL_MouseButtonEvent * e)
{
  (void)hostStruct;
  (void)e;
}

void MainAppLuaInputs_CallMouseWheelEvent(MainAppHostStruct * hostStruct, SDL_MouseWheelEvent * e)
{
  (void)hostStruct;
  (void)e;
}

void MainAppLuaInputs_CallControllerAxisEvent(MainAppHostStruct * hostStruct, SDL_ControllerAxisEvent * e)
{
  (void)hostStruct;
  (void)e;
}

void MainAppLuaInputs_CallControllerButtonDownEvent(MainAppHostStruct * hostStruct, SDL_ControllerButtonEvent * e)
{
  (void)hostStruct;
  (void)e;
}

void MainAppLuaInputs_CallControllerButtonUpEvent(MainAppHostStruct * hostStruct, SDL_ControllerButtonEvent * e)
{
  (void)hostStruct;
  (void)e;
}

void MainAppLuaInputs_CallControllerAddedEvent(MainAppHostStruct * hostStruct, SDL_ControllerDeviceEvent * e)
{
  (void)hostStruct;
  (void)e;
}

void MainAppLuaInputs_CallControllerRemovedEvent(MainAppHostStruct * hostStruct, SDL_ControllerDeviceEvent * e)
{
  (void)hostStruct;
  (void)e;
}