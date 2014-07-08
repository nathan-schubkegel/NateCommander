#include "TickCounter.h"

#include <assert.h>

void TickCounter_Reset(TickCounter * counter)
{
  assert(counter != 0);
  counter->StartTime = SDL_GetTicks(); // counted in miliseconds
  counter->TickCount = 0;
}

Uint32 TickCounter_Update(TickCounter * counter)
{
  assert(counter != 0);
  counter->TickCount = SDL_GetTicks() - counter->StartTime; // this will underflow at some point - that's OK/intended
  return counter->TickCount;
}

void TickCounter_ResetToTickCount(TickCounter * counter, Uint32 newTickCount)
{
  assert(counter != 0);
  counter->TickCount = newTickCount;
  counter->StartTime = SDL_GetTicks() - newTickCount; // this will underflow in some cases - that's OK/intended
}