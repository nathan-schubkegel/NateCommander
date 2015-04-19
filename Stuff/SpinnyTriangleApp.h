#ifndef NATE_COMMANDER_SPINNY_TRIANGLE_APP_H
#define NATE_COMMANDER_SPINNY_TRIANGLE_APP_H

#include "MsCounter.h"
#include "SDL.h"
#include "CommonApp.h"

struct SpinnyTriangleApp_State;
typedef struct SpinnyTriangleApp_State SpinnyTriangleApp_State;

void SpinnyTriangleApp_Initialize(SpinnyTriangleApp_State ** state);
void SpinnyTriangleApp_HandleEvent(SpinnyTriangleApp_State * state, SDL_Event * sdlEvent);
void SpinnyTriangleApp_Process(SpinnyTriangleApp_State * state);
void SpinnyTriangleApp_Draw(SpinnyTriangleApp_State * state);

#endif