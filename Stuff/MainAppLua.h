/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_MAIN_APP_LUA_H
#define NATE_COMMANDER_MAIN_APP_LUA_H

#include "lua.h"
#include "SDL.h"
#include "MainAppHostStruct.h"
#include "NateMash.h"

// These are invoked by C code to execute Lua client code
void MainAppLua_InitLua(MainAppHostStruct * hostStruct);
void MainAppLua_CallInitialize(MainAppHostStruct * hostStruct);
void MainAppLua_CallKeyDownEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e);
void MainAppLua_CallKeyUpEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e);
void MainAppLua_CallMouseMotionEvent(MainAppHostStruct * hostStruct, SDL_MouseMotionEvent * e);
void MainAppLua_CallProcess(MainAppHostStruct * hostStruct);
void MainAppLua_CallDraw(MainAppHostStruct * hostStruct, 
                         int windowWidth,
                         int windowHeight);

// these are exported and invoked by Lua client code
void MainAppLua_RegisterKeyDownHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex);
void MainAppLua_RegisterKeyUpHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex);
void MainAppLua_RegisterKeyResetHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex);
void MainAppLua_RegisterMouseMotionHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex);
void MainAppLua_LoadAndRunLuaFile(lua_State * luaState, const char * luaFileName);

#endif