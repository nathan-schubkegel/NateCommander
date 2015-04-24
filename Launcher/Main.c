
#include "SDL.h"
#include <windows.h>
#include "FatalErrorHandler.h"
#include "Utils.h"
#include "MainApp.h"

#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4127) // conditional expression is constant
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  MainApp_State * state;
  static SDL_Event sdlEvent;

  // Initialize defaults, Video and Audio
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)
  {
    FatalError_Sdl("Could not initialize SDL");
  }

  // Clean up on exit
  atexit(SDL_Quit);

  // Initialize app state
  MainApp_Initialize(&state);

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
        MainApp_HandleEvent(state, &sdlEvent);
      }
    }

    // do not make any further LUA calls when a possibly-fatal error is being delivered
    if (!FatalError_IsDeliveringMessage())
    {
      // give the app a chance to increment its game state
      MainApp_Process(state);

      // give the app a chance to draw
      MainApp_Draw(state);
    }

    Sleep(0); // give up execution to other threads that might want it
  }

  return 0; // this never executes
}
// TODO: need to figure out how to restore this warning
