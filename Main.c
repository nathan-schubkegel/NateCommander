
#include "SDL.h"
#include <windows.h>
#include "FatalErrorHandler.h"
#include "MainEvents.h"

#pragma warning(disable : 4100) // unreferenced formal parameter
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  SDL_Surface * screen;

  // Initialize defaults, Video and Audio
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)
  {
    FatalError2("Could not initialize SDL: %s.\n", SDL_GetError());
  }

  // Clean up on exit
  atexit(SDL_Quit);
  
  // Have a preference for 8-bit, but accept any depth
  // (and software surface)
  screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE | SDL_ANYFORMAT);
  if (screen == NULL)
  {
    FatalError2("Couldn't set 640x480x8 video mode: %s", SDL_GetError());
  }
  //printf("Set 640x480 at %d bits-per-pixel mode\n",
  //       screen->format->BitsPerPixel);

  MainEvents_PollEventsForever();
  
  return 0; // this never executes
}
// TODO: need to figure out how to restore this warning
