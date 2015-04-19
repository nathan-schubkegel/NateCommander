#ifndef NATE_COMMANDER_UTILS_H
#define NATE_COMMANDER_UTILS_H

void * MallocAndInitOrDie(int numBytes);
void FreeAndNil(void ** chunkPtr);

#endif