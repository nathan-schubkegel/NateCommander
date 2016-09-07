/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MainApp.h"

#include "lua.h"
#include "CommonApp.h"
#include "Resources.h"
#include "MainAppHostStruct.h"
#include "MainAppLua.h"
#include "MainAppLuaInputs.h"
#include "MainAppDrawing.h"
#include "MainAppPhysics.h"
#include "FatalErrorHandler.h"

MainAppHostStruct * MainApp_Initialize()
{
  MainAppHostStruct * hostStruct;

  // Initialize C HostStruct
  hostStruct = malloc(sizeof(MainAppHostStruct));
  NateCheck0(hostStruct != 0);
  memset(hostStruct, 0, sizeof(MainAppHostStruct));

  // Initialize LUA stuff
  MainAppLua_InitLua(hostStruct);

  // Not calling this for today... tokamak is kinda crashing right now
  //MainAppPhysics_InitPhysics();

  // Initialize the main window w/ dorky smiley face icon
  hostStruct->MainWindow = CommonApp_CreateMainWindow("Nate Commander", Resource_MainWindowIcon_FileName, 0);

  // Let lua script intialize whatever it wants
  MainAppLua_CallInitialize(hostStruct);

  return hostStruct;
}

void MainApp_HandleEvent(MainAppHostStruct * hostStruct, SDL_Event * sdlEvent)
{
  switch (sdlEvent->type)
  {
    /* Key events */
    case SDL_KEYDOWN:
      MainAppLuaInputs_CallKeyDownEvent(hostStruct, &sdlEvent->key);
      break;

    case SDL_KEYUP:
      MainAppLuaInputs_CallKeyUpEvent(hostStruct, &sdlEvent->key);
      break;

    /* Mouse events */
    case SDL_MOUSEMOTION:
      MainAppLuaInputs_CallMouseMotionEvent(hostStruct, &sdlEvent->motion);
      break;

    case SDL_MOUSEBUTTONDOWN:
      MainAppLuaInputs_CallMouseDownEvent(hostStruct, &sdlEvent->button);
      break;

    case SDL_MOUSEBUTTONUP:
      MainAppLuaInputs_CallMouseUpEvent(hostStruct, &sdlEvent->button);
      break;

    case SDL_MOUSEWHEEL:
      MainAppLuaInputs_CallMouseWheelEvent(hostStruct, &sdlEvent->wheel);
      break;

    /* Game controller events */
    case SDL_CONTROLLERAXISMOTION:
      MainAppLuaInputs_CallControllerAxisEvent(hostStruct, &sdlEvent->caxis);
      break;
      
    case SDL_CONTROLLERBUTTONDOWN:
      MainAppLuaInputs_CallControllerButtonDownEvent(hostStruct, &sdlEvent->cbutton);
      break;

    case SDL_CONTROLLERBUTTONUP:
      MainAppLuaInputs_CallControllerButtonUpEvent(hostStruct, &sdlEvent->cbutton);
      break;

    case SDL_CONTROLLERDEVICEADDED:
      MainAppLuaInputs_CallControllerAddedEvent(hostStruct, &sdlEvent->cdevice);
      break;

    case SDL_CONTROLLERDEVICEREMOVED:
      MainAppLuaInputs_CallControllerRemovedEvent(hostStruct, &sdlEvent->cdevice);
      break;

    case SDL_CONTROLLERDEVICEREMAPPED:
      // TODO: I don't understand controller mappings yet
      break;
      
    case SDL_QUIT:
      // FUTURE: let Lua client script know
      exit(0);
      break;
  }
}

void MainApp_Process(MainAppHostStruct * hostStruct)
{
  // meh, just forward this to lua client script
  MainAppLua_CallProcess(hostStruct);
}

void MainApp_Draw(MainAppHostStruct * hostStruct)
{
  // meh, just forward this to the drawing file
  MainAppDrawing_Draw(hostStruct);
}

