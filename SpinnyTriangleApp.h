#ifndef NATE_COMMANDER_SPINNY_TRIANGLE_APP_H
#define NATE_COMMANDER_SPINNY_TRIANGLE_APP_H

#include "TickCounter.h"
#include "SDL.h"
#include "CommonApp.h"

typedef struct SpinnyTriangleApp_State
{
  WindowAndOpenGlContext MainWindow;
  double CurrentAngle;
  TickCounter Ticks;
  Uint8 ShouldRotate;

} SpinnyTriangleApp_State;

void SpinnyTriangleApp_Initialize(SpinnyTriangleApp_State * state);
void SpinnyTriangleApp_HandleEvent(SpinnyTriangleApp_State * state, SDL_Event * sdlEvent);
void SpinnyTriangleApp_Process(SpinnyTriangleApp_State * state);
void SpinnyTriangleApp_Draw(SpinnyTriangleApp_State * state);

#endif