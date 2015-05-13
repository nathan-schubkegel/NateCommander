#ifndef NATE_COMMANDER_COMMON_APP_H
#define NATE_COMMANDER_COMMON_APP_H

#include "SDL.h"

typedef struct CommonApp_WindowAndOpenGlContext
{
  // NOTE: this struct is only allowed to contain values that are safe to copy by-value.
  SDL_Window * Window;
  SDL_GLContext GlContext;
} 
CommonApp_WindowAndOpenGlContext;

// On failure, this performs a FatalError
CommonApp_WindowAndOpenGlContext CommonApp_CreateMainWindow(const char * title, const char * iconFileName, int fullscreen);

#endif