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