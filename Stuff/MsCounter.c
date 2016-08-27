/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MsCounter.h"
#include "FatalErrorHandler.h"

void MsCounter_Init(MsCounter * counter)
{
  NateAssert(counter != 0, "MsCounter_Init");
  counter->GetSystemTimeMs = SDL_GetTicks;
  // TODO: seems bogus not to just do this automatically...?
  //MsCounter_Reset(counter);
}

void MsCounter_Reset(MsCounter * counter)
{
  NateAssert(counter != 0, "MsCounter_Reset");

  counter->LastGetTicksValue = counter->GetSystemTimeMs();
  counter->Count = 0;
}

Uint64 MsCounter_Update(MsCounter * counter)
{
  Uint32 newGetTicksValue;
  Uint32 msSinceLast;
  static int hasNotified = 0;

  NateAssert(counter != 0, "MsCounter_Update");
  newGetTicksValue = counter->GetSystemTimeMs();
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

Uint64 MsCounter_GetCount(MsCounter * counter)
{
  NateAssert(counter != 0, "MsCounter_GetCount");

  return counter->Count;
}

void MsCounter_ResetToNewCount(MsCounter * counter, Uint64 newTickCount)
{
  NateAssert(counter != 0, "MsCounter_ResetToNewCount");

  counter->LastGetTicksValue = counter->GetSystemTimeMs();
  counter->Count = newTickCount;
}

void MsCounter_ResetToCurrentCount(MsCounter * counter)
{
  NateAssert(counter != 0, "MsCounter_ResetToCurrentCount");

  counter->LastGetTicksValue = counter->GetSystemTimeMs();
}
