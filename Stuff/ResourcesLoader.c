#include "ResourcesLoader.h"

#include "SDL.h"
#include <Windows.h>
#include <stdio.h>

char gExecutingDir[MAX_PATH + 1] = {0};
char gTempFilePath[MAX_PATH + 1] = {0};

void LoadExecutingDir()
{
  int i;

  // get executing directory
  if (gExecutingDir[0] == '\0')
  {
    memset(gExecutingDir, 0, sizeof(char) * (MAX_PATH + 1));
    GetModuleFileName(0, gExecutingDir, MAX_PATH);
    
    // remove everything after the last directory separator char
    for (i = MAX_PATH; i >= 0; i--)
    {
      if (gExecutingDir[i] == '\\' || gExecutingDir[i] == '/')
      {
        break;
      }
      else
      {
        gExecutingDir[i] = 0;
      }
    }
  }
}

// returns null and populates SDL_GetError() on failure
SDL_Surface * ResourcesLoader_LoadBmp(const char * resourceFileName)
{
  FILE * bmpFile;
  LPVOID bmpData;
  long bmpDataLength;
  size_t numBytesAcquired;
  SDL_Surface * bmpSurface;

  // assemble full path to resource file
  LoadExecutingDir();
  strncpy_s(gTempFilePath, sizeof(gTempFilePath), gExecutingDir, MAX_PATH);
  strncat_s(gTempFilePath, sizeof(gTempFilePath), resourceFileName, MAX_PATH);

  // open resource file
  bmpFile = 0;
  fopen_s(&bmpFile, gTempFilePath, "rb");
  if (bmpFile == 0)
  {
    SDL_SetError("Failed to fopen resource file");
    return 0;
  }
  
  // seek to end of resource file
  if (0 != fseek(bmpFile, 0L, SEEK_END))
  {
    fclose(bmpFile);
    SDL_SetError("Failed to fseek to end of resource file");
    return 0;
  }

  // get current position (indicates total file size)
  bmpDataLength = ftell(bmpFile);
  if (bmpDataLength == -1)
  {
    fclose(bmpFile);
    SDL_SetError("Failed to ftell resource file size");
    return 0;
  }

  // allocate memory for file data
  bmpData = malloc(bmpDataLength);
  if (bmpData == 0)
  {
    fclose(bmpFile);
    SDL_SetError("Failed to allocate memory for resource file");
    return 0;
  }

  // seek to start of resource file
  if (0 != fseek(bmpFile, 0L, SEEK_SET))
  {
    fclose(bmpFile);
    free(bmpData);
    SDL_SetError("Failed to fseek to start of resource file");
    return 0;
  }

  // read file data
  numBytesAcquired = fread(bmpData, 1, bmpDataLength, bmpFile);
  if ((size_t)numBytesAcquired != (size_t)bmpDataLength)
  {
    fclose(bmpFile);
    free(bmpData);
    SDL_SetError("Failed to fread resource file");
    return 0;
  }

  // convert to SDL bitmap
  bmpSurface = SDL_LoadBMP_RW(SDL_RWFromMem(bmpData, bmpDataLength), 1);

  fclose(bmpFile);
  free(bmpData);

  return bmpSurface;
}
