#include <windows.h>

// Global Variables:
HINSTANCE hInst;								// current instance

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

  // take directory path from command line
  // enumerate every file
  //   compute checksum / hash / something
  //   combine them
  // compare against contents of a file (also from path on command line)
  // if different, write new contents to file

  return 0;
}

