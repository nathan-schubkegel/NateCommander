#ifndef NATE_COMMANDER_MAIN_APP_LUA_H
#define NATE_COMMANDER_MAIN_APP_LUA_H

#include "lua.h"
#include "SDL.h"
#include "MainAppHostStruct.h"

void MainAppLua_InitLua(MainAppHostStruct * hostStruct);
void MainAppLua_CallInitialize(MainAppHostStruct * hostStruct);
void MainAppLua_CallKeyDownEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e);
void MainAppLua_CallKeyUpEvent(MainAppHostStruct * hostStruct, SDL_KeyboardEvent * e);
void MainAppLua_CallMouseMotionEvent(MainAppHostStruct * hostStruct, SDL_MouseMotionEvent * e);
void MainAppLua_CallProcess(MainAppHostStruct * hostStruct);
void MainAppLua_CallDraw(MainAppHostStruct * hostStruct, 
                         lua_Number * spinnyCubeAngle, 
                         lua_Number * floorZOffset, 
                         lua_Number * viewAngleX, 
                         lua_Number * viewAngleY);

// these are exported and invoked by Lua client code
void MainAppLua_RegisterKeyDownHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex);
void MainAppLua_RegisterKeyUpHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex);
void MainAppLua_RegisterKeyResetHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex, int eventKeyIndex);
void MainAppLua_RegisterMouseMotionHandler(lua_State * luaState, int handlerNameIndex, int handlerFunctionIndex);

#endif