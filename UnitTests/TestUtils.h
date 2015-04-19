#ifndef NATE_TEST_UTILS_H
#define NATE_TEST_UTILS_H

#include "FatalErrorHandler.h"

extern int numTestsPassed;
extern int numTestsFailed;

#define CHECK(condition, message) if (!(condition)) { numTestsFailed++; FatalError("Test Failed: " #condition ", " message); } else { numTestsPassed++; }

#endif