#include "FatalErrorHandler.h"

#include <windows.h>
#include "SDL.h"

int FatalError(const char * message)
{
  MessageBox(0, message, "NateCommander Fatal Error", 0);
  SDL_Quit();
  exit(-1);
}

int FatalError2(const char* message, const char * extraInfo)
{
  MessageBox(0, message, "NateCommander Fatal Error", 0);
  MessageBox(0, extraInfo, "NateCommander Supplemental Info to Error", 0);
  SDL_Quit();
  exit(-1);
}