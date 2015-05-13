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