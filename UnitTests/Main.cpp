
#include "SDL.h"
#include <Windows.h>

// C stuff
extern "C"
{
#include "TestUtils.h"

extern void Test_MsCounter();
extern void Test_MeasuredString();
extern void Test_NateXml();
}

// C++ stuff
extern void Test_NateMinMaxHeap();
extern void Test_NateTStringList();
extern void Test_NateMesh();

#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4702) // unreachable code
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  SDL_MessageBoxData mbData;
  SDL_MessageBoxButtonData buttonData;

  Test_MsCounter();
  Test_MeasuredString();
  Test_NateXml();
  Test_NateMinMaxHeap();
  Test_NateTStringList();
  Test_NateMesh();

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