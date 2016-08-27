/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "Utils.h"

#include "FatalErrorHandler.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void * MallocAndInitOrDie(int numBytes)
{
  void * chunk = malloc(numBytes);
  if (chunk == 0) FatalError_OutOfMemory("");
  memset(chunk, 0, numBytes);
  return chunk;
}

void FreeAndNil(void ** chunkPtr)
{
  assert(chunkPtr != 0);
  if (*chunkPtr != 0)
  {
    free(*chunkPtr);
    *chunkPtr = 0;
  }
}