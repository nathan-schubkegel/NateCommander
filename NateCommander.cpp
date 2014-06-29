
//#include "SDL.h"
//#include <stdlib.h>
//#include "FatalErrorHandler.h"

int main1();

int __stdcall WinMain( int hInstance, int hPrevInstance, wchar_t * lpCmdLine, int nCmdShow )
{
  return main1();
}

int main1()
{
  /* Initialize defaults, Video and Audio */
  //if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)
  { 
    //FatalError(L"Could not initialize SDL: %s.\n", SDL_GetError());
    //exit(-1);
  }


	return 0;
}
