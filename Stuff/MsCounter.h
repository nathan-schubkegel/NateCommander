#ifndef NATE_COMMANDER_MS_COUNTER_H
#define NATE_COMMANDER_MS_COUNTER_H

#include "SDL.h"

typedef struct MsCounter MsCounter;

// Counts the passage of time as milliseconds
struct MsCounter
{
  // This is Uint64 because Uint32 would roll over after 49.7 days
  // (I want someone to be able to run a dedicated server that lasts more than 50 days =)
  Uint64 Count;
  Uint32 LastGetTicksValue;

  // The function to use to determine the current system time in milliseconds
  // (this is here to support unit tests)
  Uint32 (*GetSystemTimeMs) ();
};

void MsCounter_Init(MsCounter * counter);
void MsCounter_Reset(MsCounter * counter);
Uint64 MsCounter_Update(MsCounter * counter);
Uint64 MsCounter_GetCount(MsCounter * counter);
void MsCounter_ResetToNewCount(MsCounter * counter, Uint64 newCount);
void MsCounter_ResetToCurrentCount(MsCounter * counter);


#endif