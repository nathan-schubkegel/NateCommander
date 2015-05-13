#include "FatalErrorHandler.h"

#include <windows.h>
#include "SDL.h"

#define ERROR_MESSAGE_BUFFER_SIZE 1024
char gErrorMessageBuffer[ERROR_MESSAGE_BUFFER_SIZE];
int gIsShowingErrorMessage;

void DoFatalError2(const char * message, const char * message2)
{
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message2);
  MessageBox(0, gErrorMessageBuffer, "NateCommander Fatal Error", 0);
  SDL_Quit();
  exit(-1);
}

void DoFatalError3(const char * message, const char * message2, const char * message3)
{
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message2);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message3);
  MessageBox(0, gErrorMessageBuffer, "NateCommander Fatal Error", 0);
  SDL_Quit();
  exit(-1);
}

void DoFatalError4(const char * message, const char * message2, const char * message3, const char * message4)
{
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message2);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message3);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message4);
  MessageBox(0, gErrorMessageBuffer, "NateCommander Fatal Error", 0);
  SDL_Quit();
  exit(-1);
}

void DoFatalError_Sdl2(const char* message, const char* message2)
{
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message2);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, "\n");
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, SDL_GetError());
  MessageBox(0, gErrorMessageBuffer, "NateCommander Fatal Error", 0);
  SDL_Quit();
  exit(-1);
}

void DoFatalError_OutOfMemory2(const char* message, const char * message2)
{
  DoFatalError3(message, message2, " Out of memory (failed to allocate memory for something)");
}

void DoNonFatalError2(const char * message, const char * message2)
{
  // FUTURE: append to log? display at top of screen? txt to your mother?
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message2);
  MessageBox(0, gErrorMessageBuffer, "NateCommander Non-Fatal Error", 0);

  gIsShowingErrorMessage--;
}

void DoNonFatalError_Sdl2(const char * message, const char * message2)
{
  // FUTURE: append to log? display at top of screen? txt to your mother?
  if (gIsShowingErrorMessage) return;
  gIsShowingErrorMessage++;

  strcpy_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, message2);
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, "\n");
  strcat_s(gErrorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, SDL_GetError());
  MessageBox(0, gErrorMessageBuffer, "NateCommander Non-Fatal Error", 0);

  gIsShowingErrorMessage--;
}

int FatalError_IsDeliveringMessage()
{
  return gIsShowingErrorMessage;
}