
#include "SDL.h"
#include <windows.h>
#include "FatalErrorHandler.h"
#include "Utils.h"
#include "SpinnyTriangleApp.h"

#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4702) // unreachable code
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  static SpinnyTriangleApp_State state;
  static SDL_Event sdlEvent;

  // Initialize defaults, Video and Audio
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)
  {
    FatalError_Sdl("Could not initialize SDL");
  }

  // Clean up on exit
  atexit(SDL_Quit);

  // Initialize app state
  // Today's app is: SpinnyTriangleApp
  SpinnyTriangleApp_Initialize(&state);

again:
  while (SDL_PollEvent(&sdlEvent))
  {
    // pass all events on to the app
    SpinnyTriangleApp_HandleEvent(&state, &sdlEvent);
  }

  // give the app a chance to increment its game state
  SpinnyTriangleApp_Process(&state);

  // give the app a chance to draw
  SpinnyTriangleApp_Draw(&state);

  Sleep(0); // give up execution to other threads that might want it
  goto again;

  return 0; // this never executes
}
// TODO: need to figure out how to restore this warning
