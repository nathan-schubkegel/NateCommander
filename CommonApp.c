#include "CommonApp.h"
#include "SDL.h"
#include "ResourcesLoader.h"
#include "FatalErrorHandler.h"

WindowAndOpenGlContext CreateMainWindow(const char * title, int iconId, int fullscreen)
{
  WindowAndOpenGlContext result;
  SDL_Surface * bmpSurface;
  Uint32 flags;

  flags = fullscreen
    ? SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL
    : SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;

  // Create a main window with the given title
  result.Window = SDL_CreateWindow(title,
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED,
                          640, 480, // width and height are ignored when SDL_WINDOW_FULLSCREEN_DESKTOP is provided
                          flags);

  if (result.Window == 0)
  {
    FatalError_Sdl("Failed to create window");
  }

  if (iconId != 0)
  {
    if (NULL == (bmpSurface = LoadEmbeddedResourceBmp(iconId)))
    {
      NonFatalError_Sdl("Could not load BMP for main window icon");
    }
    else
    {
      SDL_SetWindowIcon(result.Window, bmpSurface);
      SDL_FreeSurface(bmpSurface);
      bmpSurface = NULL;
    }
  }

  result.GlContext = SDL_GL_CreateContext(result.Window);
  if (result.GlContext == 0)
  {
    FatalError_Sdl("Failed to create OpenGL context");
  }

  return result;
}