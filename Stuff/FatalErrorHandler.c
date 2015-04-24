#include "FatalErrorHandler.h"

#include <windows.h>
#include "SDL.h"

#define ERROR_MESSAGE_BUFFER_SIZE 1024
char gErrorMessageBuffer[ERROR_MESSAGE_BUFFER_SIZE];
int gIsShowingErrorMessage;

void FatalError(const char * message)
{
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  MessageBox(0, gErrorMessageBuffer, "NateCommander Fatal Error", 0);
  SDL_Quit();
  exit(-1);
}

void FatalError_Sdl(const char* message)
{
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, "\n");
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, SDL_GetError());
  MessageBox(0, gErrorMessageBuffer, "NateCommander Fatal Error", 0);
  SDL_Quit();
  exit(-1);
}

void FatalError_OutOfMemory()
{
  FatalError("Out of memory (failed to allocate memory for something)");
}

void NonFatalError(const char * message)
{
  // FUTURE: append to log? display at top of screen? txt to your mother?
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  MessageBox(0, gErrorMessageBuffer, "NateCommander Non-Fatal Error", 0);

  gIsShowingErrorMessage--;
}

void NonFatalError_Sdl(const char * message)
{
  // FUTURE: append to log? display at top of screen? txt to your mother?
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, "\n");
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, SDL_GetError());
  MessageBox(0, gErrorMessageBuffer, "NateCommander Non-Fatal Error", 0);

  gIsShowingErrorMessage--;
}

int FatalError_IsDeliveringMessage()
{
  return gIsShowingErrorMessage;
}