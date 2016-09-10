/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_MAIN_APP_LUA_INPUTS_H
#define NATE_COMMANDER_MAIN_APP_LUA_INPUTS_H

#include "lua.h"
#include "SDL.h"
#include "MainAppHostStruct.h"
#include "NateMash.h"

// to be invoked only while setting up a LuaState
void MainAppLuaInputs_Initialize(MainAppHostStruct * hostStruct);

// These are invoked by C code to execute Lua client code
void MainAppLuaInputs_CallKeyEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e);
void MainAppLuaInputs_CallMouseMotionEvent(MainAppHostStruct * hostStruct, SDL_MouseMotionEvent * e);
void MainAppLuaInputs_CallMouseButtonEvent(MainAppHostStruct * hostStruct, SDL_MouseButtonEvent * e);
void MainAppLuaInputs_CallMouseWheelEvent(MainAppHostStruct * hostStruct, SDL_MouseWheelEvent * e);
void MainAppLuaInputs_CallControllerAxisEvent(MainAppHostStruct * hostStruct, SDL_ControllerAxisEvent * e);
void MainAppLuaInputs_CallControllerButtonDownEvent(MainAppHostStruct * hostStruct, SDL_ControllerButtonEvent * e);
void MainAppLuaInputs_CallControllerButtonUpEvent(MainAppHostStruct * hostStruct, SDL_ControllerButtonEvent * e);
void MainAppLuaInputs_CallControllerAddedEvent(MainAppHostStruct * hostStruct, SDL_ControllerDeviceEvent * e);
void MainAppLuaInputs_CallControllerRemovedEvent(MainAppHostStruct * hostStruct, SDL_ControllerDeviceEvent * e);

// these are exported and invoked by Lua client code
void MainAppLuaInputs_RegisterKeyDownHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex);
void MainAppLuaInputs_RegisterKeyUpHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex);
void MainAppLuaInputs_RegisterMouseMotionHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex);
void MainAppLuaInputs_RegisterMouseDownHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int mouseButtonIndex);
void MainAppLuaInputs_RegisterMouseUpHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int mouseButtonIndex);
void MainAppLuaInputs_RegisterMouseWheelHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex);

#endif