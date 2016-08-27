/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_RESOURCES_LOADER
#define NATE_COMMANDER_RESOURCES_LOADER

#include "SDL.h"

// on failure, returns null and populates SDL_GetError()
SDL_Surface * ResourceLoader_LoadBmp(const char * resourceFileName);
void * ResourceLoader_LoadLuaFile(const char * luaFileName, long * lengthOut);
char * ResourceLoader_LoadMashFile(const char * mashFileName, size_t * lengthOut);

#endif