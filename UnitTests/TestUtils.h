/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_TEST_UTILS_H
#define NATE_TEST_UTILS_H

#include "FatalErrorHandler.h"

extern int numTestsPassed;
extern int numTestsFailed;

#define CHECK(condition, message) if (!(condition)) { numTestsFailed++; FatalError("Test Failed: " #condition ", " message); } else { numTestsPassed++; }

#endif