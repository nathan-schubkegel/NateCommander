#include "MainApp.h"

#include "lua.h"
#include "CommonApp.h"
#include "Resources.h"
#include "MainAppHostStruct.h"
#include "MainAppLua.h"
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
    case SDL_KEYDOWN:
      MainAppLua_CallKeyDownEvent(hostStruct, &sdlEvent->key);
      break;

    case SDL_KEYUP:
      MainAppLua_CallKeyUpEvent(hostStruct, &sdlEvent->key);
      break;

    case SDL_MOUSEMOTION:
      MainAppLua_CallMouseMotionEvent(hostStruct, &sdlEvent->motion);
      break;

    // FUTURE: implement these
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEWHEEL:
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

