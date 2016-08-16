#include "TestUtils.h"
#include "MsCounter.h"
#include "SDL.h"

Uint32 fakeSystemTime;

Uint32 GetFakeSystemTime()
{
  return fakeSystemTime;
}

void Test_MsCounter()
{
  MsCounter m;

  // custom timer initialization for unit-testing purposes
  m.GetSystemTimeMs = GetFakeSystemTime;

  // Verify that each method basically looks like it works
  fakeSystemTime = 12;
  MsCounter_Reset(&m);
  CHECK(m.Count == 0, );
  CHECK(m.LastGetTicksValue == 12, );

  fakeSystemTime = 13;
  CHECK(MsCounter_Update(&m) == 1, );
  CHECK(m.Count == 1, );
  CHECK(m.LastGetTicksValue == 13, );

  fakeSystemTime = 14;
  CHECK(MsCounter_GetCount(&m) == 1, );
  CHECK(m.Count == 1, );
  CHECK(m.LastGetTicksValue == 13, );

  fakeSystemTime = 15;
  MsCounter_ResetToNewCount(&m, 6);
  CHECK(m.Count == 6, );
  CHECK(m.LastGetTicksValue == 15, );

  fakeSystemTime = 16;
  MsCounter_ResetToCurrentCount(&m);
  CHECK(m.Count == 6, );
  CHECK(m.LastGetTicksValue == 16, );

  // Verify that each method basically looks like it works in a rollover scenario
  fakeSystemTime = 12;

  m.LastGetTicksValue = (Uint32)-12;
  m.Count = 1;
  MsCounter_Reset(&m);
  CHECK(m.Count == 0, );
  CHECK(m.LastGetTicksValue == 12, );

  m.LastGetTicksValue = (Uint32)-12;
  m.Count = 1;
  CHECK(MsCounter_Update(&m) == 25, );
  CHECK(m.Count == 25, );
  CHECK(m.LastGetTicksValue == 12, );

  m.LastGetTicksValue = (Uint32)-12;
  m.Count = 24;
  CHECK(MsCounter_GetCount(&m) == 24, );
  CHECK(m.Count == 24, );
  CHECK(m.LastGetTicksValue == -12, );

  m.LastGetTicksValue = (Uint32)-12;
  m.Count = 24;
  MsCounter_ResetToNewCount(&m, 50);
  CHECK(m.Count == 50, );
  CHECK(m.LastGetTicksValue == 12, );

  m.LastGetTicksValue = (Uint32)-12;
  m.Count = 15;
  MsCounter_ResetToCurrentCount(&m);
  CHECK(m.Count == 15, );
  CHECK(m.LastGetTicksValue == 12, );
}