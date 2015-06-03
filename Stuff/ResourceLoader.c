#include "ResourceLoader.h"

#include "SDL.h"
#include <Windows.h>
#include <stdio.h>

char gExecutingDir[MAX_PATH + 1] = {0};
char gPathSeparator[2];

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
        gPathSeparator[0] = gExecutingDir[i];
        gPathSeparator[1] = 0;
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
void ResourceLoader_LoadBinaryFile(const char * dirName, const char * fileName, void ** dataOut, long * lengthOut)
{
  FILE * file;
  LPVOID data;
  long dataLength;
  size_t numBytesAcquired;
  static char gTempFilePath[MAX_PATH + 1];

  // initialize in/out parameters
  *dataOut = 0;
  *lengthOut = 0;

  // assemble full path to resource file
  LoadExecutingDir();
  strncpy_s(gTempFilePath, sizeof(gTempFilePath), gExecutingDir, MAX_PATH);
  if (dirName != 0)
  {
    strncat_s(gTempFilePath, sizeof(gTempFilePath), dirName, MAX_PATH);
    strncat_s(gTempFilePath, sizeof(gTempFilePath), gPathSeparator, MAX_PATH);
  }
  strncat_s(gTempFilePath, sizeof(gTempFilePath), fileName, MAX_PATH);

  // open binary file
  file = 0;
  fopen_s(&file, gTempFilePath, "rb");
  if (file == 0)
  {
    SDL_SetError("Failed to fopen resource file");
    return;
  }
  
  // seek to end of resource file
  if (0 != fseek(file, 0L, SEEK_END))
  {
    fclose(file);
    SDL_SetError("Failed to fseek to end of resource file");
    return;
  }

  // get current position (indicates total file size)
  dataLength = ftell(file);
  if (dataLength < 0)
  {
    fclose(file);
    SDL_SetError("Failed to ftell resource file size");
    return;
  }

  // allocate memory for file data (plus trailing null byte, for ease of loading text files)
  data = malloc(dataLength + 1);
  if (data == 0)
  {
    fclose(file);
    SDL_SetError("Failed to allocate memory for resource file");
    return;
  }

  // seek to start of resource file
  if (0 != fseek(file, 0L, SEEK_SET))
  {
    fclose(file);
    free(data);
    SDL_SetError("Failed to fseek to start of resource file");
    return;
  }

  // read file data
  numBytesAcquired = fread(data, 1, dataLength, file);
  if ((size_t)numBytesAcquired != (size_t)dataLength)
  {
    fclose(file);
    free(data);
    SDL_SetError("Failed to fread resource file");
    return;
  }

  // add trailing null byte, for ease of loading text files
  // TODO: use a type that I feel safer is always 1-byte big
  ((char*)data)[dataLength] = 0;

  fclose(file);

  *dataOut = data;
  *lengthOut = dataLength;
}

// returns null and populates SDL_GetError() on failure
SDL_Surface * ResourceLoader_LoadBmp(const char * resourceFileName)
{
  void * data;
  long length;
  SDL_Surface * bmpSurface;

  // Load file data
  ResourceLoader_LoadBinaryFile("Resources", resourceFileName, &data, &length);
  if (data == 0)
  {
    return 0;
  }

  // convert to SDL bitmap
  bmpSurface = SDL_LoadBMP_RW(SDL_RWFromMem(data, length), 1);

  free(data);

  return bmpSurface;
}

// returns null and populates SDL_GetError() on failure
void * ResourceLoader_LoadLuaFile(const char * luaFileName, long * lengthOut)
{
  void * data;
  long length;

  // Load file data
  ResourceLoader_LoadBinaryFile("Lua Files", luaFileName, &data, &length);
  if (data == 0)
  {
    *lengthOut = 0;
    return 0;
  }

  *lengthOut = length;
  return data;
}

// returns null and populates SDL_GetError() on failure
char * ResourceLoader_LoadMeshFile(const char * meshFileName, size_t * lengthOut)
{
  void * data;
  long length;

  // Load file data
  ResourceLoader_LoadBinaryFile("Meshes", meshFileName, &data, &length);
  if (data == 0)
  {
    *lengthOut = 0;
    return 0;
  }

  *lengthOut = length;
  return data;
}
