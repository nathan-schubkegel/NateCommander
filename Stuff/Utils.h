/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_UTILS_H
#define NATE_COMMANDER_UTILS_H

void * MallocAndInitOrDie(int numBytes);
void FreeAndNil(void ** chunkPtr);

#endif