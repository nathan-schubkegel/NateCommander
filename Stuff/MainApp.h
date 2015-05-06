#ifndef NATE_COMMANDER_MAIN_APP_H
#define NATE_COMMANDER_MAIN_APP_H

#include "MsCounter.h"
#include "SDL.h"
#include "CommonApp.h"

struct MainApp_State;
typedef struct MainApp_State MainApp_State;

void MainApp_Initialize(MainApp_State ** state);
void MainApp_HandleEvent(MainApp_State * state, SDL_Event * sdlEvent);
void MainApp_Process(MainApp_State * state);
void MainApp_Draw(MainApp_State * state);

// TODO: called by LUA, probly shouldn't be exposed here? maybe?
void MainApp_AdvanceGSIM();

#endif