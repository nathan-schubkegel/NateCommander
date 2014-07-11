#include "SDL.h"
#include "ResourcesLoader.h"
#include "FatalErrorHandler.h"

SDL_Window * CreateMainWindow(const char * title, int iconId)
{
  SDL_Window * mainWindow;
  SDL_Surface * bmpSurface;

  // Create a main window with the given title
  mainWindow = SDL_CreateWindow(title,
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          640, 480,
                          SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);

  if (mainWindow != 0)
  {
    if (iconId != 0)
    {
      if (NULL == (bmpSurface = LoadEmbeddedResourceBmp(iconId)))
      {
        NonFatalError_Sdl("Could not load BMP for main window icon");
	    }
      else
      {
        SDL_SetWindowIcon(mainWindow, bmpSurface);
        SDL_FreeSurface(bmpSurface);
        bmpSurface = NULL;
      }
    }
  }

  return mainWindow;
}