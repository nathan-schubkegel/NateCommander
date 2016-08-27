/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_MAIN_APP_H
#define NATE_COMMANDER_MAIN_APP_H

#include "SDL.h"

// Launcher uses this, it doesn't need to know details of MainAppHostStruct
struct MainAppHostStruct;
typedef struct MainAppHostStruct MainAppHostStruct;

MainAppHostStruct * MainApp_Initialize();
void MainApp_HandleEvent(MainAppHostStruct* hostStruct, SDL_Event * sdlEvent);
void MainApp_Process(MainAppHostStruct* hostStruct);
void MainApp_Draw(MainAppHostStruct* hostStruct);
// FUTURE: MainApp_TearDown();

#endif