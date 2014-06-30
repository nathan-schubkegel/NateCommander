
#include "SDL.h"
#include <windows.h>
#include "FatalErrorHandler.h"

#pragma warning(disable : 4100) // unreferenced formal parameter
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  // Initialize defaults, Video and Audio
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)
  { 
    FatalError2("Could not initialize SDL: %s.\n", SDL_GetError());
    exit(-1);
  }

	return 0;
}
// TODO: need to figure out how to restore this warning
