#include "ResourcesLoader.h"

#include <Windows.h>

// returns null and populates SDL_GetError() on failure
SDL_Surface * LoadEmbeddedResourceBmp(int resourceId)
{
  HINSTANCE hInstance;
  HRSRC hResource;
  HGLOBAL hGlobal;
  LPVOID bmpData;

  // TODO: this assumes I'll never use dlls. True! (Nevarrr!)
  hInstance = GetModuleHandle(NULL);
  hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_RCDATA);
  hGlobal = LoadResource(hInstance, hResource);
  bmpData = LockResource(hGlobal);
  if (bmpData == 0)
  {
    SDL_SetError("Failed to load win32 resource");
    return 0;
  }
  else
  {
    int bmpDataLength;
    SDL_Surface * bmpSurface;

    bmpDataLength = (DWORD)SizeofResource(hInstance, hResource);
    bmpSurface = SDL_LoadBMP_RW(SDL_RWFromMem(bmpData, bmpDataLength), 1);
    return bmpSurface;
  }
}
