/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MainAppLuaInputs.h"

#include "FatalErrorHandler.h"
#include "MainAppHostStruct.h"
#include "MainAppHostTable.h"
#include "LuaHelpers.h"

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

  lua_pushstring(luaState, "MouseDownNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "MouseDownHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "MouseUpNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "MouseUpHandlers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "MouseWheelNames");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  lua_pushstring(luaState, "MouseWheelHandlers");
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

void MainAppLuaInputs_CallKeyEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;
  char * namesTable;
  char * handlersTable;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.keysym = e->keysym
  lua_pushstring(luaState, "keysym");
  lua_pushnumber(luaState, e->keysym.sym);
  lua_settable(luaState, eventDataIndex);

  // call the custom event handler (if it exists)
  if (e->type == SDL_KEYDOWN)
  {
    namesTable = "KeyDownNames";
    handlersTable = "KeyDownHandlers";
  }
  else
  {
    namesTable = "KeyUpNames";
    handlersTable = "KeyUpHandlers";
  }
  MyCallHandlerEvent(hostStruct, namesTable, handlersTable, e->keysym.sym, eventDataIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

int MyPushMouseMultiButtonTable(lua_State * luaState, Uint32 buttons)
{
  int tableIndex;

  NateCheck0(lua_checkstack(luaState, 3));

  // make a table
  lua_newtable(luaState);
  tableIndex = lua_gettop(luaState);

  // add a value to the table for each depressed button
  if (buttons & SDL_BUTTON_LMASK)
  {
    lua_pushnumber(luaState, SDL_BUTTON_LEFT);
    lua_pushnumber(luaState, SDL_BUTTON_LEFT);
    lua_settable(luaState, tableIndex);
  }

  if (buttons & SDL_BUTTON_RMASK)
  {
    lua_pushnumber(luaState, SDL_BUTTON_RIGHT);
    lua_pushnumber(luaState, SDL_BUTTON_RIGHT);
    lua_settable(luaState, tableIndex);
  }

  if (buttons & SDL_BUTTON_MMASK)
  {
    lua_pushnumber(luaState, SDL_BUTTON_MIDDLE);
    lua_pushnumber(luaState, SDL_BUTTON_MIDDLE);
    lua_settable(luaState, tableIndex);
  }

  if (buttons & SDL_BUTTON_X1MASK)
  {
    lua_pushnumber(luaState, SDL_BUTTON_X1);
    lua_pushnumber(luaState, SDL_BUTTON_X1);
    lua_settable(luaState, tableIndex);
  }

  if (buttons & SDL_BUTTON_X2MASK)
  {
    lua_pushnumber(luaState, SDL_BUTTON_X2);
    lua_pushnumber(luaState, SDL_BUTTON_X2);
    lua_settable(luaState, tableIndex);
  }

  return tableIndex;
}

void MainAppLuaInputs_CallMouseMotionEvent(MainAppHostStruct * hostStruct, SDL_MouseMotionEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;

  // the wiki recommended ignoring touchpad events. Not sure if I have to.
  if (e->which == SDL_TOUCH_MOUSEID) return;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.Buttons = table of buttons currently held down
  lua_pushstring(luaState, "Buttons");
  MyPushMouseMultiButtonTable(luaState, e->state);
  lua_settable(luaState, eventDataIndex);
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

void MainAppLuaInputs_CallMouseButtonEvent(MainAppHostStruct * hostStruct, SDL_MouseButtonEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;
  char * namesTable;
  char * handlersTable;

  // the wiki recommended ignoring touchpad events. Not sure if I have to.
  if (e->which == SDL_TOUCH_MOUSEID) return;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.PositionX = absolute x position
  lua_pushstring(luaState, "PositionX");
  lua_pushnumber(luaState, e->x);
  lua_settable(luaState, eventDataIndex);
  // table.PositionY = absolute x position
  lua_pushstring(luaState, "PositionY");
  lua_pushnumber(luaState, e->y);
  lua_settable(luaState, eventDataIndex);
  // table.Button = index of pressed/depressed mouse button
  lua_pushstring(luaState, "Button");
  lua_pushnumber(luaState, e->button);
  lua_settable(luaState, eventDataIndex);
  // table.Pressed = true if pressed, false if depressed
  lua_pushstring(luaState, "Pressed");
  lua_pushboolean(luaState, e->state == SDL_PRESSED);
  lua_settable(luaState, eventDataIndex);
  // table.Clicks = number of clicks so far
  lua_pushstring(luaState, "Clicks");
  lua_pushnumber(luaState, e->clicks);
  lua_settable(luaState, eventDataIndex);

  // call the custom event handler (if it exists)
  if (e->state == SDL_PRESSED)
  {
    namesTable = "MouseDownNames";
    handlersTable = "MouseDownHandlers";
  }
  else
  {
    namesTable = "MouseUpNames";
    handlersTable = "MouseUpHandlers";
  }
  MyCallHandlerEvent(hostStruct, namesTable, handlersTable, e->button, eventDataIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MainAppLuaInputs_CallMouseWheelEvent(MainAppHostStruct * hostStruct, SDL_MouseWheelEvent * e)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);
  int eventDataIndex;

  // the wiki recommended ignoring touchpad events. Not sure if I have to.
  if (e->which == SDL_TOUCH_MOUSEID) return;

  NateCheck0(lua_checkstack(luaState, 3));

  // make an event args table
  lua_newtable(luaState);
  eventDataIndex = lua_gettop(luaState);

  // table.ScrollX = relative x scroll movement
  lua_pushstring(luaState, "ScrollX");
  lua_pushnumber(luaState, e->x);
  lua_settable(luaState, eventDataIndex);
  // table.ScrollY = relative y scroll movement
  lua_pushstring(luaState, "ScrollY");
  lua_pushnumber(luaState, e->y);
  lua_settable(luaState, eventDataIndex);

  // call the custom event handler (if it exists)
  MyCallHandlerEvent(hostStruct, "MouseWheelNames", "MouseWheelHandlers", 0, eventDataIndex);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
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
  hostTableIndex = MainAppHostTable_LuaPushHostTable(luaState);

  // get the HandlerNames table
  lua_pushstring(luaState, namesTableId);
  lua_gettable(luaState, hostTableIndex);

  // HandlerNames[eventKeyIndex ?? -1] = handlerName
  lua_pushvalue(luaState, eventKeyIndex);
  LuaHelpers_ReplaceNilWithNumber(luaState, -1, -1.0);
  lua_pushvalue(luaState, luaHandlerNameIndex);
  lua_settable(luaState, -3);
  lua_pop(luaState, 1); // pop the handlernames table

  // get the Handlers table
  lua_pushstring(luaState, handlersTableId);
  lua_gettable(luaState, hostTableIndex);

  // Handlers[eventKeyIndex ?? -1] = handlerMethod
  lua_pushvalue(luaState, eventKeyIndex);
  LuaHelpers_ReplaceNilWithNumber(luaState, -1, -1.0);
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

void MainAppLuaInputs_RegisterMouseMotionHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex)
{
  // fabricate eventKeyIndex = 0
  NateCheck0(lua_checkstack(luaState, 1));
  lua_pushnumber(luaState, 0);

  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, lua_gettop(luaState),
    "MouseMotionNames", "MouseMotionHandlers");
}

void MainAppLuaInputs_RegisterMouseDownHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int mouseButtonIndex)
{
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, mouseButtonIndex,
    "MouseDownNames", "MouseDownHandlers");
}

void MainAppLuaInputs_RegisterMouseUpHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int mouseButtonIndex)
{
  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, mouseButtonIndex,
    "MouseUpNames", "MouseUpHandlers");
}

void MainAppLuaInputs_RegisterMouseWheelHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex)
{
  // fabricate eventKeyIndex = 0
  NateCheck0(lua_checkstack(luaState, 1));
  lua_pushnumber(luaState, 0);

  MyRegisterEventHandler(luaState, handlerNameIndex, handlerFunctionIndex, lua_gettop(luaState),
    "MouseWheelNames", "MouseWheelHandlers");
}
