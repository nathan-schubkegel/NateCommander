
#include "SDL.h"
#include <windows.h>
#include "FatalErrorHandler.h"
#include "Resources.h"
#include "ResourcesLoader.h"
#include "Utils.h"
#include "SpinnyTriangleApp.h"

void InitMainWindow2D()
{
  SDL_Surface * screen;

  // Have a preference for 8-bit, but accept any depth
  // (and software surface)
  screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE | SDL_ANYFORMAT);
  if (screen == NULL)
  {
    FatalError2("Couldn't set 640x480x8 video mode: %s", SDL_GetError());
  }
  //printf("Set 640x480 at %d bits-per-pixel mode\n",
  //       screen->format->BitsPerPixel);

  // TODO: need to do anything with screen?
}

void InitMainWindowGL()
{
  // Information about the current video settings.
  const SDL_VideoInfo* info = NULL;
  
  // Dimensions of our window.
  int width = 0;
  int height = 0;
  
  // Color depth in bits of our window.
  int bpp = 0;

  // Flags we will pass into SDL_SetVideoMode.
  int flags = 0;

  // Note - this assumes SDL_Init() has already been called

  // Let's get some video information.
  if (NULL == (info = SDL_GetVideoInfo()))
  {
    // This will probably never happen.
    FatalError2("Video query failed: %s\n", SDL_GetError());
  }

  // Set our width/height to resolution of primary display
  // TODO: does this code (incorrectly) span multiple monitors?
  // TODO: let the user pick a resolution / display
  width = info->current_w; 
  height = info->current_h;

  // We get the bpp we will request from
  // the display. On X11, VidMode can't change
  // resolution, so this is probably being overly
  // safe. Under Win32, ChangeDisplaySettings
  // can change the bpp.
  bpp = info->vfmt->BitsPerPixel;

  // Now, we want to set up our requested
  // window attributes for our OpenGL window.
  // We want *at least* 5 bits of red, green
  // and blue. We also want at least a 16-bit
  // depth buffer.
  // TODO: this is dumb. Why not 8 8 8?
  // TODO: what does the depth buffer impact?
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );

  // The last thing we do is request a double
  // buffered window. '1' turns on double
  // buffering, '0' turns it off.
  // Note that we do not use SDL_DOUBLEBUF in
  // the flags to SDL_SetVideoMode. That does
  // not affect the GL attribute state, only
  // the standard 2D blitting setup.
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  // We want to request that SDL provide us
  // with an OpenGL window, in a fullscreen
  // video mode.
  // EXERCISE:
  // Make starting windowed an option, and
  // handle the resize events properly with
  // glViewport.
  flags = SDL_OPENGL | SDL_FULLSCREEN;

  // Set the video mode
  if (SDL_SetVideoMode(width, height, bpp, flags) == 0)
  {
    // This could happen for a variety of reasons,
    // including DISPLAY not being set, the specified
    // resolution not being available, etc.
    FatalError2("Video mode set failed: %s", SDL_GetError());
  }
}

SDL_Event gSdlEvent;

#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4702) // unreachable code
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  SDL_Surface * bmpSurface;
  SpinnyTriangleApp_State * state;

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
  if (NULL == (bmpSurface = LoadEmbeddedResourceBmp(RES_ID_MAIN_WINDOW_ICON_BMP)))
  {
    FatalError2("Could not load BMP for main window icon: %s.\n", SDL_GetError());
	}
  SDL_WM_SetIcon(bmpSurface, NULL);
  SDL_FreeSurface(bmpSurface);
  bmpSurface = NULL;

  // Initialize the main window
  InitMainWindowGL();

  // Initialize app state
  // Today's app is: SpinnyTriangleApp
  state = MallocAndInitOrDie(sizeof(SpinnyTriangleApp_State));
  SpinnyTriangleApp_Initialize(state);

again:
  while (SDL_PollEvent(&gSdlEvent))
  {
    SpinnyTriangleApp_HandleEvent(state, &gSdlEvent);
  }
  SpinnyTriangleApp_Process(state);
  SpinnyTriangleApp_Draw(state);

  Sleep(0); // give up execution to other threads that might want it
  goto again;

  return 0; // this never executes
}
// TODO: need to figure out how to restore this warning
