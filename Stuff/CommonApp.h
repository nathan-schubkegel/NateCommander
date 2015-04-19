#ifndef NATE_COMMANDER_COMMON_APP_H
#define NATE_COMMANDER_COMMON_APP_H

#include "SDL.h"

typedef struct WindowAndOpenGlContext
{
  SDL_Window * Window;
  SDL_GLContext GlContext;
} 
WindowAndOpenGlContext;

// On failure, this performs a FatalError
WindowAndOpenGlContext CreateMainWindow(const char * title, const char * iconFileName, int fullscreen);

#endif