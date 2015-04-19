#ifndef NATE_COMMANDER_RESOURCES_LOADER
#define NATE_COMMANDER_RESOURCES_LOADER

#include "SDL.h"

// on failure, returns null and populates SDL_GetError()
SDL_Surface * ResourceLoader_LoadBmp(const char * resourceFileName);
char * ResourceLoader_LoadLuaFile(const char * luaFileName, long * lengthOut);

#endif