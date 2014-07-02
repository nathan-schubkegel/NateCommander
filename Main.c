
#include "SDL.h"
#include <windows.h>
#include "FatalErrorHandler.h"
#include "MainEvents.h"
#include "Resources.h"

HINSTANCE gHInstance;

SDL_Surface * GetMainWindowIcon()
{
  HRSRC hResource;
  HGLOBAL hGlobal;
  LPVOID bmpData;

  hResource = FindResource(gHInstance, MAKEINTRESOURCE(RES_ID_MAIN_WINDOW_ICON_BMP), RT_RCDATA);
  hGlobal = LoadResource(gHInstance, hResource);
  bmpData = LockResource(hGlobal);
  if (bmpData == 0)
  {
    SDL_SetError("Failed to load win32 resource");
    return 0;
  }
  else
  {
    int bmpDataLength;
    SDL_Surface * bmpSurface;

    bmpDataLength = (DWORD)SizeofResource(gHInstance, hResource);
    bmpSurface = SDL_LoadBMP_RW(SDL_RWFromMem(bmpData, bmpDataLength), 1);
    return bmpSurface;
  }
}

#pragma warning(disable : 4100) // unreferenced formal parameter
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  SDL_Surface * screen;
  SDL_Surface * bmpSurface;

  // save our module instance for other code to use
  gHInstance = hInstance;

  // Initialize defaults, Video and Audio
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)
  {
    FatalError2("Could not initialize SDL: %s.\n", SDL_GetError());
  }

  // Clean up on exit
  atexit(SDL_Quit);
  
  // Main window will be titled "Nate Commander"
  SDL_WM_SetCaption("Nate Commander", NULL);

  // Main window icon will be a dorky smiley face
  if (NULL == (bmpSurface = GetMainWindowIcon()))
  {
    FatalError2("Could not load BMP for main window icon: %s.\n", SDL_GetError());
	}
  SDL_WM_SetIcon(bmpSurface, NULL);
  SDL_FreeSurface(bmpSurface);
  bmpSurface = NULL;

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
