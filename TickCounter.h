#ifndef NATE_COMMANDER_TICK_COUNTER_H
#define NATE_COMMANDER_TICK_COUNTER_H

#include "SDL.h"

typedef struct TickCounter
{
  Uint32 StartTime;
  Uint32 TickCount;
} TickCounter;

void TickCounter_Reset(TickCounter * counter);
Uint32 TickCounter_Update(TickCounter * counter);
void TickCounter_ResetToTickCount(TickCounter * counter, Uint32 newTickCount);

#endif