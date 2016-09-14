/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MainAppGameControllers.h"

#include "MainAppHostStruct.h"
#include "SDL.h"
#include "lua.h"
#include "FatalErrorHandler.h"
#include "MainAppLuaInputs.h"

void MainAppGameControllers_Initialize(MainAppHostStruct * hostStruct)
{
  lua_State * luaState = hostStruct->luaState;
  int originalStackIndex = lua_gettop(luaState);

  NateCheck0(lua_checkstack(luaState, 3));

  MainAppHostStruct_LuaPushHostTable(hostStruct);
  
  lua_pushstring(luaState, "GameControllers");
  lua_newtable(luaState);
  lua_settable(luaState, -3);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - originalStackIndex);
}

void MyAddController(MainAppHostStruct * hostStruct, SDL_GameController * controller)
{
  SDL_JoystickID joystickId;
  int stackTopBefore;
  lua_State * luaState = hostStruct->luaState;

  // get controller's joystick id, which is how we identify the joystick to lua client
  joystickId = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
  NateCheck(joystickId != -1, "we just opened the controller! joystickId should not be invalid!");

  stackTopBefore = lua_gettop(luaState);
  NateCheck0(lua_checkstack(luaState, 4));

  // push HostTable
  MainAppHostStruct_LuaPushHostTable(hostStruct);

  // push GameControllers table
  lua_pushstring(luaState, "GameControllers");
  lua_gettable(luaState, -2);

  // sanity check - that controller should not already be present in GameControllers table
  lua_pushlightuserdata(luaState, controller);
  lua_gettable(luaState, -2);
  NateCheck(lua_isnil(luaState, -1), "explode if GameControllers table already had that controller");
  lua_pop(luaState, 1);

  // sanity check - that controller's joystick ID should not be present either
  lua_pushnumber(luaState, joystickId);
  lua_gettable(luaState, -2);
  NateCheck(lua_isnil(luaState, -1), "explode if GameControllers table already had that controller's joystick id");
  lua_pop(luaState, 1);  
  
  // save the opened controller
  lua_pushlightuserdata(luaState, controller);
  lua_pushnumber(luaState, joystickId);
  lua_settable(luaState, -3);
  
  // save the opened controller's joystick id
  lua_pushnumber(luaState, joystickId);
  lua_pushlightuserdata(luaState, controller);
  lua_settable(luaState, -3);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - stackTopBefore);
}

void MyRemoveController(MainAppHostStruct * hostStruct, SDL_GameController * controller)
{
  SDL_JoystickID joystickId;
  SDL_GameController * controller2;
  int stackTopBefore;
  lua_State * luaState = hostStruct->luaState;

  stackTopBefore = lua_gettop(luaState);
  NateCheck0(lua_checkstack(luaState, 4));

  // push HostTable
  MainAppHostStruct_LuaPushHostTable(hostStruct);

  // push GameControllers table
  lua_pushstring(luaState, "GameControllers");
  lua_gettable(luaState, -2);

  // sanity check - that controller should be present in GameControllers table
  lua_pushlightuserdata(luaState, controller);
  lua_gettable(luaState, -2);
  NateCheck(lua_isnumber(luaState, -1), "explode if GameControllers table didn't have that controller");
  joystickId = (SDL_JoystickID)lua_tonumber(luaState, -1);
  lua_pop(luaState, 1);

  // sanity check - that controller's joystick ID should be present too
  // (note - can't successfully get a controller's joystick ID from SDL if it's been closed... which is true here)
  lua_pushnumber(luaState, joystickId);
  lua_gettable(luaState, -2);
  NateCheck(lua_isuserdata(luaState, -1), "explode if GameControllers table didn't have that controller's joystick id");
  controller2 = lua_touserdata(luaState, -1);
  NateCheck(controller2 == controller, "explode if CameControllers table mapping was wrong");
  lua_pop(luaState, 1);  
  
  // remove the controller
  lua_pushlightuserdata(luaState, controller);
  lua_pushnil(luaState);
  lua_settable(luaState, -3);
  
  // remove the controller's joystick id
  lua_pushnumber(luaState, joystickId);
  lua_pushnil(luaState);
  lua_settable(luaState, -3);

  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - stackTopBefore);
}

SDL_GameController * MyGetController(MainAppHostStruct * hostStruct, SDL_JoystickID joystickId)
{
  SDL_GameController * controller;
  int stackTopBefore;
  lua_State * luaState = hostStruct->luaState;

  stackTopBefore = lua_gettop(luaState);
  NateCheck0(lua_checkstack(luaState, 3));

  // push HostTable
  MainAppHostStruct_LuaPushHostTable(hostStruct);

  // push GameControllers table
  lua_pushstring(luaState, "GameControllers");
  lua_gettable(luaState, -2);

  // get controller
  lua_pushnumber(luaState, joystickId);
  lua_gettable(luaState, -2);
  NateCheck(lua_isuserdata(luaState, -1), "explode if GameControllers table didn't have that controller's joystick id");
  controller = (SDL_GameController *)lua_touserdata(luaState, -1);
  
  // restore stack
  lua_pop(luaState, lua_gettop(luaState) - stackTopBefore);

  return controller;
}

void MainAppGameControllers_OpenAllControllers(MainAppHostStruct * hostStruct)
{
  int numJoysticks;
  int i;
  SDL_GameController * controller;

  // open all the currently-plugged-in gamepads
  numJoysticks = SDL_NumJoysticks();
  for (i = 0; i < numJoysticks; i++)
  {
    if (SDL_IsGameController(i))
    {
      controller = SDL_GameControllerOpen(i);
      // some joysticks aren't supported as GameControllers. OK
      if (controller != NULL)
      {
        // record new controller
        MyAddController(hostStruct, controller);
      }
    }
  }

  // fire Lua client event that controllers changed
  MainAppLuaInputs_CallControllersChangedEvent(hostStruct);
}

void MainAppGameControllers_ControllerAdded(MainAppHostStruct * hostStruct, SDL_ControllerDeviceEvent * e)
{
  SDL_GameController * controller;

  // open the added joystick
  if (SDL_IsGameController(e->which))
  {
    controller = SDL_GameControllerOpen(e->which);
    // some joysticks aren't supported as GameControllers. OK
    if (controller != NULL)
    {
      // record new controller
      MyAddController(hostStruct, controller);

      // fire Lua client event that controllers changed
      MainAppLuaInputs_CallControllersChangedEvent(hostStruct);
    }
  }
}

void MainAppGameControllers_ControllerRemoved(MainAppHostStruct * hostStruct, SDL_ControllerDeviceEvent * e)
{
  SDL_GameController * controller;

  // get controller for the given joystick id
  controller = MyGetController(hostStruct, e->which);

  // remove that controller
  MyRemoveController(hostStruct, controller);

  // fire Lua client event that controllers changed
  MainAppLuaInputs_CallControllersChangedEvent(hostStruct);
}

SDL_GameController * MainAppGameControllers_GetControllerFromJoystickId(MainAppHostStruct * hostStruct, SDL_JoystickID joystickInstanceId)
{
  return MyGetController(hostStruct, joystickInstanceId);
}

void MainAppGameControllers_LuaPushAddedControllersTable(MainAppHostStruct * hostStruct, int oldControllersIndex)
{
  (void)hostStruct;
  (void)oldControllersIndex;
  lua_newtable(hostStruct->luaState);
  //FatalError("not implemented yet");
}

void MainAppGameControllers_LuaPushRemovedControllersTable(MainAppHostStruct * hostStruct, int oldControllersIndex)
{
  (void)hostStruct;
  (void)oldControllersIndex;
  lua_newtable(hostStruct->luaState);
  //FatalError("not implemented yet");
}

void MainAppGameControllers_LuaPushAllControllersTable(MainAppHostStruct * hostStruct)
{
  (void)hostStruct;
  lua_newtable(hostStruct->luaState);
  //FatalError("not implemented yet");
}