/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "SDL.h"
#include <windows.h>
#include "FatalErrorHandler.h"
#include "Utils.h"
#include "MainApp.h"

#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4127) // conditional expression is constant
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  MainAppHostStruct * hostStruct;
  static SDL_Event sdlEvent;

  // Initialize video
  if (SDL_Init(SDL_INIT_VIDEO) == -1)
  {
    FatalError_Sdl("Could not initialize SDL Video");
  }

  // Initialize audio
  if (SDL_Init(SDL_INIT_AUDIO) == -1)
  {
    NonFatalError_Sdl("Could not initialize SDL Audio");
  }

  // Initialize joystick and/or game controller
  if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == -1)
  {
    NonFatalError_Sdl("Could not initialize SDL Joysticks/Gamecontrollers");
  }

  // Clean up on exit
  atexit(SDL_Quit);

  // Initialize app state
  hostStruct = MainApp_Initialize();

  while (1)
  {
    while (SDL_PollEvent(&sdlEvent))
    {
      if (sdlEvent.type == SDL_QUIT)
      {
        exit(0);
      }
      // TODO: there are bound to be events that I can't afford to miss

      // do not make any further LUA calls when a possibly-fatal error is being delivered
      if (!FatalError_IsDeliveringMessage())
      {
        // pass all events on to the app
        MainApp_HandleEvent(hostStruct, &sdlEvent);
      }
    }

    // do not make any further LUA calls when a possibly-fatal error is being delivered
    if (!FatalError_IsDeliveringMessage())
    {
      // give the app a chance to increment its game state
      MainApp_Process(hostStruct);

      // give the app a chance to draw
      MainApp_Draw(hostStruct);
    }

    Sleep(0); // give up execution to other threads that might want it
  }

  return 0; // this never executes
}
// TODO: need to figure out how to restore this warning
