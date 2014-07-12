#include "MsCounter.h"
#include "SDL.h"
#include "FatalErrorHandler.h"

void MsCounter_Reset(MsCounter * counter)
{
  Nate_Assert(counter != 0, "MsCounter_Reset");

  counter->LastGetTicksValue = SDL_GetTicks(); // counted in miliseconds
  counter->Count = 0;
}

Uint64 MsCounter_Update(MsCounter * counter)
{
  Uint32 newGetTicksValue;
  Uint32 msSinceLast;
  static int hasNotified = 0;

  Nate_Assert(counter != 0, "MsCounter_Update");
  newGetTicksValue = SDL_GetTicks();
  msSinceLast = newGetTicksValue - counter->LastGetTicksValue; // this will underflow sometimes - that's OK/intended
  
  // msSinceLast will normally be 0, 1, or 2 ms
  // msSinceLast might be 100 or 1000 or 10,000 if this process is starved for CPU time or is put to sleep
  // msSinceLast might be close to UINT32's max value if SDL_GetTicks() is flat-out broken
  // (this happens on my Windows XP VM (VMWare 6.5.1) when SDL_GetTicks() uses QueryPerformanceCounter())
#define TWENTY_DAYS_IN_MS (1000 * 60 * 60 * 24 * 20)
  if (msSinceLast > TWENTY_DAYS_IN_MS)
  {
    // Report zero passage of time when that happens
    msSinceLast = 0;

    // Notify the user (just once!) when that happens
    if (!hasNotified)
    {
      hasNotified = 1;
      NonFatalError("msSinceLast was > TWENTY_DAYS_IN_MS. Uninitialized MsCounter? Bad return value from SDL_GetTicks()?");
    }
  }

  counter->Count += msSinceLast;
  counter->LastGetTicksValue = newGetTicksValue;
  return counter->Count;
}

Uint64 MsCounter_GetTickCount(MsCounter * counter)
{
  Nate_Assert(counter != 0, "MsCounter_GetTickCount");

  return counter->Count;
}

void MsCounter_ResetToNewCount(MsCounter * counter, Uint64 newTickCount)
{
  Nate_Assert(counter != 0, "MsCounter_ResetToNewCount");

  counter->LastGetTicksValue = SDL_GetTicks();
  counter->Count = newTickCount;
}

void MsCounter_ResetToCurrentCount(MsCounter * counter)
{
  Nate_Assert(counter != 0, "MsCounter_ResetToCurrentCount");

  counter->LastGetTicksValue = SDL_GetTicks();
}