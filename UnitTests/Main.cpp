
#include "SDL.h"
#include <Windows.h>
#include "TestUtils.h"

extern void Test_MsCounter();
extern void Test_MeasuredString();

#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4702) // unreachable code
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  SDL_MessageBoxData mbData;
  SDL_MessageBoxButtonData buttonData;

  Test_MsCounter();
  Test_MeasuredString();
  Test_NateXml();

  memset(&mbData, 0, sizeof(SDL_MessageBoxData));
  
  if (numTestsFailed > 0)
  {
    mbData.title = "Failure";
    mbData.message = "Some tests failed.";
  }
  else
  {
    mbData.title = "Success";
    mbData.message = "All tests passed.";
  }
  mbData.buttons = &buttonData;
  mbData.numbuttons = 1;
  memset(&buttonData, 0, sizeof(SDL_MessageBoxButtonData));
  buttonData.text = "&OK";
  SDL_ShowMessageBox(&mbData, NULL);
}