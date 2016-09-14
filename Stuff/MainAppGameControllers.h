/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_MAIN_APP_GAME_CONTROLLERS_H
#define NATE_COMMANDER_MAIN_APP_GAME_CONTROLLERS_H

#include "SDL.h"
#include "MainAppHostStruct.h"

void MainAppGameControllers_Initialize(MainAppHostStruct * hostStruct);
void MainAppGameControllers_OpenAllControllers(MainAppHostStruct * hostStruct);
void MainAppGameControllers_ControllerAdded(MainAppHostStruct * hostStruct, SDL_ControllerDeviceEvent * e);
void MainAppGameControllers_ControllerRemoved(MainAppHostStruct * hostStruct, SDL_ControllerDeviceEvent * e);
SDL_GameController * MainAppGameControllers_GetControllerFromJoystickId(MainAppHostStruct * hostStruct, SDL_JoystickID joystickInstanceId);

void MainAppGameControllers_LuaPushAddedControllersTable(MainAppHostStruct * hostStruct, int oldControllersIndex);
void MainAppGameControllers_LuaPushRemovedControllersTable(MainAppHostStruct * hostStruct, int oldControllersIndex);
void MainAppGameControllers_LuaPushAllControllersTable(MainAppHostStruct * hostStruct);

#endif