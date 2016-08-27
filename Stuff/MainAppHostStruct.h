/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_MAIN_APP_HOST_STRUCT_H
#define NATE_COMMANDER_MAIN_APP_HOST_STRUCT_H

#include "CommonApp.h"
#include "lua.h"
#include "ctokamak.h"

struct MainAppHostStruct
{
  CommonApp_WindowAndOpenGlContext MainWindow;
  lua_State * luaState;
  cneSimulator * simulator;
};
typedef struct MainAppHostStruct MainAppHostStruct;

#endif